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
# Project  : XMerge
# Author   : Brian Cameron
# Date     : 5th Sept. 2001
#
#
# Takes x and y from the command line and taps the screen there.
# Assumes pose is already running.
#
##########################################################################

use POSIX "sys_wait_h";   # Need this for waitpid with WNOHANG
use EmRPC;  # EmRPC::OpenConnection, CloseConnection
use EmFunctions;
use EmUtils;

if ($#ARGV != 0)
{
  print "\nUsage: $0 timeout\n\n";
  exit -1;
}

$timeout = $ARGV[0];

if (!defined($up_pid = fork()))
{
   print "ERROR, problem forking.\n"
}
elsif ($up_pid)
{
   print "\nChecking to see if pose is started properly.\n";

   # Parent process
   #
   sleep($timeout);

   waitpid($up_pid, WNOHANG);

   if (kill(0, $up_pid))
   {
      print "Pose did not start successfully...\n";
      kill(9, $up_pid);
      exit(-1);
   }
   else
   {
      # The child process exited okay, so we know it will not
      # hang...but the open_connection will just die if pose
      # isn't started...so try it in the parent.
      #
      open_connection();
      close_connection();

      print "Verified pose started successfully...\n";
      exit(0);
   }
}
else
{
   # Child process - Try to open/close the connection.  This
   # can hang if pose did not start properly...
   #
   open_connection();
   close_connection();
}

sub open_connection
{
  print "opening connection\n";
  EmRPC::OpenConnection(6415, "localhost");
}

sub close_connection
{
  print "closing connection\n";
  EmRPC::CloseConnection();
}

