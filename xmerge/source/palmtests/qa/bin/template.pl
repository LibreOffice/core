#!/usr/bin/perl
#########################################################################

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



####################################################################
# File Name: template.pl
# Version  : 1.0
# Project  : XMERGE
# Author   : Brian Cameron
# Date     : 5th Sept. 2001
#
# This is just a useful script to use as a template to run
# commands.  Assumes that pose is already running.
#
##########################################################################

use lib "$ENV{qa-dir}/lib";
use converterlib;

# Put commands to run between the open_connection() and
# close_connection() calls...
#
open_connection();

# command(s) go here.

close_connection();


