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
#  $Revision: 1.3 $
#
#  Last changes: $Author$ $Date$
#
#  Generate node tree from verbose logfile
#
#  HOWTO USE
#  =========
#
#  First of all, you need to recompile modules slideshow and canvas
#  with VERBOSE=t set in the environment, and debug=t defined at the
#  build tool command line. Then run your slideshow and redirect stdout
#  to a file.
#
#  Then, call 'nodetree.sh trace-file > tree.ps' to generate a
#  PostScript file for the AnimationNode tree generated during the show.
#  Since these easily get pretty huge, use only one page, and try to
#  limit the slide content (which have effects assigned, that is) to
#  the bare minimum (to reproduce your bug, or whatever you need this
#  for).
#
#  The generated output will have all nodes color-coded with their state
#  at the point when the tree was dumped (see colors.sh for the color
#  codes)
#
#  When looking for the mechanics that generate the relevant output,
#  grep for the DEBUG_NODES_SHOWTREE macros in the slideshow source:
#  Each such place dumps the current node tree to the trace output. Thus,
#  if you need to check the tree state at other places or times, just add
#  a DEBUG_NODES_SHOWTREE (or DEBUG_NODES_SHOWTREE_WITHIN, that is).
#

###################################################
#
# Generate node tree
#
###################################################

egrep "Node connection|Node state" $1 |                                         \
    sed -e '/Node state/ s/.*Node state.*: \(.*\)/\1/'                          \
        -e '/Node connection/ s/.*Node connection.*: \(n.*\)/\1/' |             \
                                                                                \
awk 'BEGIN { print "digraph Event_network {" }; { print } END { print "}" }' |  \
                                                                                \
                                                                                \
# fill a complete A1 page with graph output
dot -Gratio=fill -Gsize=23,33 -Tps
#dot -Gratio=fill -Gorientation=land -Tps
