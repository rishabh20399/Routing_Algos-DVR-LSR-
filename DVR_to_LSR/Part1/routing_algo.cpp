#include "node.h"
#include <iostream>
#include <utility>
#include <queue>
#include <climits>

using namespace std;
int Node::id = 0; // from node.h class Node

void printRT(vector<RoutingNode *> nd)
{
	/*Print routing table entries*/
	for (int i = 0; i < nd.size(); i++)
	{
		nd[i]->printTable();
	}
}

void routingAlgo(vector<RoutingNode *> nd)
{

	bool saturation = false;

	for (int i = 1; i < nd.size(); ++i)
	{
		for (RoutingNode *node : nd)
		{
			node->sendMsg();
		}
	}

	/*Print routing table entries after routing algo converges */
	printf("Printing the routing tables after the convergence \n");
	printRT(nd);
}

void RoutingNode::recvMsg(RouteMsg *msg)
{
	// your code here

	// Traverse the routing table in the message.
	// Check if entries present in the message table is closer than already present
	// entries.
	// Update entries.

	// Message already seen
	if (this->path_ords.find(msg->from) != this->path_ords.end() && this->path_ords[msg->from] >= msg->ord)
		return;

	for (const pair<string, pair<int, int>> &nn : msg->neigbhours)
		this->adjacencyMatrix[msg->from_id][nn.second.first] = nn.second.second;

	this->connected_interefeces[msg->from] = msg->from_id;
	this->path_ords[msg->from] = msg->ord;

	this->computeShortestPaths();

	this->floodMessage(msg);
}

void Node::computeShortestPaths()
{
	this->resetTbl();
	vector<int> parents(this->size, -1);
	vector<int> distances(this->size, INT_MAX);
	priority_queue<pair<int, int>> pq;

	distances[this->my_id] = 0;
	pq.push({distances[this->my_id], this->my_id});
	while (!pq.empty())
	{
		int node_id = pq.top().second;
		pq.pop();

		if (distances[node_id] == INT_MAX)
			continue;

		for (int i = 0; i < this->size; ++i)
		{
			if (this->adjacencyMatrix[node_id][i] == 0 || adjacencyMatrix[node_id][i] == INT_MAX)
				continue;
			int new_distance = distances[node_id] + adjacencyMatrix[node_id][i];
			if (new_distance < distances[i])
			{
				distances[i] = new_distance;
				parents[i] = node_id;
				pq.push({-1 * distances[i], i});
			}
		}
	}

	for (const pair<string, int> node : this->connected_interefeces)
	{
		if (node.second != this->my_id)
		{
			int root_node = -1;
			int cur = node.second;
			while (parents[cur] != -1)
			{
				root_node = cur;
				cur = parents[cur];
			}

			for (const pair<NetInterface, Node *> &interface : this->interfaces)
			{
				if (interface.second->my_id != root_node)
					continue;
				RoutingEntry newEntry;
				newEntry.dstip = node.first;
				newEntry.cost = distances[node.second];
				newEntry.ip_interface = interface.first.getip();
				newEntry.nexthop = interface.first.getConnectedIp();
				mytbl.tbl.push_back(newEntry);
				break;
			}
		}
		else
			this->addTblEntry(node.first, 0);
	}
}
