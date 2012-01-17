:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#**************************************************************
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
#**************************************************************



#
# sort.pl - Simply sort the input from stdin and pipe it to stdout.
#           The sort needs to be *independent* of the settings of
#           LC_ALL resp. LC_COLLATE
#

use strict;
# be explicit: we want the perl standard sorting regardless the locale
no locale;


my @buffer;

while(<>) {
    push(@buffer, $_);
}

foreach (sort @buffer) {
    print $_;
}
