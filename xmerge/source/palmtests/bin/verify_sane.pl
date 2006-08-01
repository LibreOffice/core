#!/usr/bin/perl
#########################################################################

 #*************************************************************************
 #
 #   OpenOffice.org - a multi-platform office productivity suite
 #
 #   $RCSfile: verify_sane.pl,v $
 #
 #   $Revision: 1.3 $
 #
 #   last change: $Author: ihi $ $Date: 2006-08-01 12:45:52 $
 #
 #   The Contents of this file are made available subject to
 #   the terms of GNU Lesser General Public License Version 2.1.
 #
 #
 #     GNU Lesser General Public License Version 2.1
 #     =============================================
 #     Copyright 2005 by Sun Microsystems, Inc.
 #     901 San Antonio Road, Palo Alto, CA 94303, USA
 #
 #     This library is free software; you can redistribute it and/or
 #     modify it under the terms of the GNU Lesser General Public
 #     License version 2.1, as published by the Free Software Foundation.
 #
 #     This library is distributed in the hope that it will be useful,
 #     but WITHOUT ANY WARRANTY; without even the implied warranty of
 #     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 #     Lesser General Public License for more details.
 #
 #     You should have received a copy of the GNU Lesser General Public
 #     License along with this library; if not, write to the Free Software
 #     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 #     MA  02111-1307  USA
 #
 #*************************************************************************

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

