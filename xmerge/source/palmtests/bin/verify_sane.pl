#!/usr/bin/perl
#########################################################################

 #*************************************************************************
 #
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
 #*************************************************************************

####################################################################
# File Name: template.pl
# Version  : 1.0
# Project  : XMerge
# Author   : Brian Cameron
# Date	   : 5th Sept. 2001
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

