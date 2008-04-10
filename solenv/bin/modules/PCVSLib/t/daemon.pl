#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: daemon.pl,v $
#
# $Revision: 1.3 $
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

#
# dameon.pl - rudimentary daemon which spawns a CVS pserver instance
#

use IO::Socket::INET;
use IO::Select;

my $cvs_port = $ARGV[1];

my $server = IO::Socket::INET->new(LocalPort => $cvs_port,
                                  Type      => SOCK_STREAM,
                                  Reuse     => 1,
                                  Listen    => 10 )
             or die "couldn't listen on port '$cvs_port'";

my $select = IO::Select->new();
my $client;
REQUEST:
    while ( $client = $server->accept() ) {
        if ( $child_pid = fork() ) {
            $client->close();
            next REQUEST;
        }

        die "cannot fork: $!" if !defined($child_pid);

        $server->close();
        $select->add($client);

        $select->can_read();
        $client->autoflush(1);

        my $infh  = IO::Handle->new_from_fd($client, 'r');
        my $outfh = IO::Handle->new_from_fd($client, 'w');


        open(STDIN,  "<&=".fileno($infh))  or die "can't dup client: $!";
        open(STDOUT,  ">&=".fileno($outfh)) or die "can't dup client: $!";

        $infh->close();
        $outfh->close();

        system("cvs -f --allow-root $ARGV[0] pserver");

        $client->close();
        $select->remove($client);
    exit(0);
    }
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
