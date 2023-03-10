#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

using namespace std;

/*
  Each row in the table will have these fields
  dstip:	Destination IP address
  nexthop: 	Next hop on the path to reach dstip
  ip_interface: nexthop is reachable via this interface (a node can have multiple interfaces)
  cost: 	cost of reaching dstip (number of hops)
*/
class RoutingEntry
{
public:
	string dstip, nexthop;
	string ip_interface;
	int cost;
};

/*
 * Class for specifying the sort order of Routing Table Entries
 * while printing the routing tables
 *
 */
class Comparator
{
public:
	bool operator()(const RoutingEntry &R1, const RoutingEntry &R2)
	{
		if (R1.cost == R2.cost)
		{
			return R1.dstip.compare(R2.dstip) < 0;
		}
		else if (R1.cost > R2.cost)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
};

/*
  This is the routing table
*/
struct routingtbl
{
	vector<RoutingEntry> tbl;
};

/*
  Message format to be sent by a sender
  from: 		Sender's ip
  mytbl: 		Senders routing table
  recvip:		Receiver's ip
*/
class RouteMsg
{
public:
	string from;			  // I am sending this message, so it must be me i.e. if A is sending mesg to B then it is A's ip address
	struct routingtbl *mytbl; // This is routing table of A
	string recvip;			  // B ip address that will receive this message

	int from_id;
	map<string, pair<int, int>> neigbhours;
	int ord;
};

/*
  Emulation of network interface. Since we do not have a wire class,
  we are showing the connection by the pair of IP's

  ip: 		Own ip
  connectedTo: 	An address to which above mentioned ip is connected via ethernet.
*/
class NetInterface
{
private:
	string ip;
	string connectedTo; // this node is connected to this ip
	int cost;

public:
	string getip() const
	{
		return this->ip;
	}
	string getConnectedIp() const
	{
		return this->connectedTo;
	}
	void setip(string ip)
	{
		this->ip = ip;
	}
	void setConnectedip(string ip)
	{
		this->connectedTo = ip;
	}
	void setCost(int cost)
	{
		this->cost = cost;
	}
	int getCost() const
	{
		return this->cost;
	}
};

/*
  Struct of each node
  name: 	It is just a label for a node
  interfaces: 	List of network interfaces a node have
  Node* is part of each interface, it easily allows to send message to another node
  mytbl: 		Node's routing table
*/
class Node
{
private:
	static int id;
	int size;
	string name;
	int ord;

protected:
	struct routingtbl mytbl;
	int my_id;
	vector<pair<NetInterface, Node *>> interfaces;
	vector<vector<int>> adjacencyMatrix;
	map<string, int> connected_interefeces;
	map<string, pair<int, int>> neigbhours;

	virtual void recvMsg(RouteMsg *msg)
	{
		cout << "Base" << endl;
	}
	bool isMyInterface(string eth)
	{
		for (int i = 0; i < interfaces.size(); ++i)
		{
			if (interfaces[i].first.getip() == eth)
				return true;
		}
		return false;
	}

public:
	Node(int n)
	{
		this->size = n;
		this->my_id = Node::id;
		++Node::id;
		this->name = name;
		this->ord = 0;
		this->adjacencyMatrix = vector<vector<int>>(this->size, vector<int>(this->size, INT_MAX));
		for (int i = 0; i < this->size; ++i)
			this->adjacencyMatrix[i][i] = 0;
	}
	void setName(string name)
	{
		this->name = name;
	}

	void addInterface(string ip, string connip, int cost, Node *nextHop)
	{
		NetInterface eth;
		eth.setip(ip);
		eth.setConnectedip(connip);
		eth.setCost(cost);
		interfaces.push_back({eth, nextHop});

		this->neigbhours[nextHop->name] = make_pair(nextHop->my_id, cost);
		this->adjacencyMatrix[this->my_id][nextHop->my_id] = cost;

		this->computeShortestPaths();
	}

	void resetTbl()
	{
		this->mytbl.tbl.clear();
	}

	void addTblEntry(string myip, int cost)
	{
		RoutingEntry entry;
		entry.dstip = myip;
		entry.nexthop = myip;
		entry.ip_interface = myip;
		entry.cost = cost;
		mytbl.tbl.push_back(entry);
	}

	void updateTblEntry(string dstip, int cost)
	{
		// to update the dstip hop count in the routing table (if dstip already exists)
		// new hop count will be equal to the cost
		for (int i = 0; i < mytbl.tbl.size(); i++)
		{
			RoutingEntry entry = mytbl.tbl[i];

			if (entry.dstip == dstip)
				mytbl.tbl[i].cost = cost;
		}

		// remove interfaces
		for (int i = 0; i < interfaces.size(); ++i)
		{
			// if the interface ip is matching with dstip then remove
			// the interface from the list
			if (interfaces[i].first.getConnectedIp() == dstip)
			{
				interfaces.erase(interfaces.begin() + i);
			}
		}
	}

	string getName()
	{
		return this->name;
	}

	struct routingtbl getTable()
	{
		return mytbl;
	}

	void printTable()
	{
		Comparator myobject;
		sort(mytbl.tbl.begin(), mytbl.tbl.end(), myobject);
		cout << this->getName() << ":" << endl;
		for (int i = 0; i < mytbl.tbl.size(); ++i)
		{
			cout << mytbl.tbl[i].dstip << " | " << mytbl.tbl[i].nexthop << " | " << mytbl.tbl[i].ip_interface << " | " << mytbl.tbl[i].cost << endl;
		}
	}

	void sendMsg()
	{
		struct routingtbl ntbl;
		for (int i = 0; i < mytbl.tbl.size(); ++i)
		{
			ntbl.tbl.push_back(mytbl.tbl[i]);
		}

		for (int i = 0; i < interfaces.size(); ++i)
		{
			RouteMsg msg;
			msg.from = interfaces[i].first.getip();
			// printf("i=%d, msg-from-interface=%s\n",i, msg.from.c_str());
			msg.mytbl = &ntbl;
			msg.recvip = interfaces[i].first.getConnectedIp();
			msg.from_id = this->my_id;
			msg.ord = this->ord;
			++this->ord;
			msg.neigbhours = this->neigbhours;
			interfaces[i].second->recvMsg(&msg);
		}
	}
	void floodMessage(RouteMsg *msg)
	{
		for (const pair<NetInterface, Node *> &interface : this->interfaces)
			interface.second->recvMsg(msg);
	}

	void computeShortestPaths();
};

class RoutingNode : public Node
{
private:
	map<string, int> path_ords;

public:
	void recvMsg(RouteMsg *msg);
	RoutingNode(int n) : Node(n)
	{
		this->path_ords.clear();
	}
};
