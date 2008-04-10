#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: Client.pm,v $
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
# Client.pm - package Client encapsulates a CVS client
#
#
# TODO implement a 'working_dir' for the client, where all operations
#      happen
#

package PCVSLib::Client;

use Carp;

use strict;
use warnings;

use PCVSLib::EventHandler;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{connection} = shift;

    # private members
    $self->{first_command_} = 1;  # do send Root request before first command

    bless ($self, $class);

    return $self;
}

#### instance accessors #####

for my $datum qw(connection) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub execute_command
{
    my $self    = shift;
    my $command = shift;

    $command->io_handle($self->{connection}->io_handle());
    $command->root($self->{connection}->root());
    if ( $self->{first_command_} ) {
        $command->first_command(1);
        $self->{first_command_} = 0;
    }
    $command->execute();
}

#### private methods ####

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
