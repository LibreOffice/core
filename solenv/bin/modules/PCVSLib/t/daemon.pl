#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: daemon.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:36:31 $
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
