#!/bin/sh
# *************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
# *************************************************************
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
