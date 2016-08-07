###############################################################
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
###############################################################

#!/bin/bash

SEARCHIN=$2

echo '<?xml version="1.0"?>'
echo "<stage1>"

cat $1 | tail -n +2

find $SEARCHIN -name "*.cxx" -exec grep -nH -A 1 "case NS_" {} \; |
grep -v "//.*case NS_" |
sed 's#'$SEARCHIN'##' |
sed 's#\(^[^:]*\):\([0-9]*\):#<qname file="\1" line="\2"#' |
sed 's#[/* ]*case \(NS_.*\):.*# qname="\1"/>#' |
sed 's#.*WRITERFILTERSTATUS: done: \([0-9]*\), planned: \([0-9.]*\), spent: \([0-9.]*\).*#<status done="\1" planned="\2" spent="\3"/>#' |
sed 's#.*WRITERFILTERSTATUS:.*#<status done="100"/>#' |
sed 's#^.*-[0-9][0-9]*-.*$#<nostatus/>#' |
grep -v "^--"

echo "</stage1>"
