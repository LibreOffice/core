#!/bin/sh
#
# Output a PS file with a legend for the node state colors
#

cat <<EOF | dot -Tps
digraph State_Colors
{
    node0  [label="INVALID",style=filled,fillcolor="0.5,0.2,0.5"]
    node1  [label="UNRESOLVED",style=filled,fillcolor="0.0,1.0,1.0"]
    node2  [label="RESOLVED",style=filled,fillcolor="0.17328679513998632735430803036454,1.0,1.0"]
    node4  [label="ACTIVE",style=filled,fillcolor="0.34657359027997265470861606072909,1.0,1.0"]
    node8  [label="FROZEN",style=filled,fillcolor="0.51986038541995898206292409109363,1.0,1.0"]
    node16 [label="ENDED",style=filled,fillcolor="0.69314718055994530941723212145818,1.0,1.0"]
}
EOF