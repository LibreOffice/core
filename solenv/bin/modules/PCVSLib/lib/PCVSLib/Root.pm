#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: Root.pm,v $
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
# Root.pm - package for maninpulating Root objects
#

package PCVSLib::Root;

use Carp;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $root_string = shift;
    my $self = {};
    $self->{method}   = shift;
    $self->{user}     = shift;
    $self->{host}     = shift;
    $self->{port}     = shift;
    $self->{root_dir} = shift;
    # private members
    bless ($self, $class);

    if ( defined($root_string) ) {
        $self->parse_root_string($root_string);
    }
    return $self;
}

#### instance accessors #####

for my $datum qw(method user host port root_dir) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub to_string
{
    my $self = shift;

    return ':' . join(':', $self->{method},
                           $self->{user} . '@' . $self->{host},
                           $self->{root_dir});
}

sub to_string_with_port
{
    my $self = shift;

    return ':' . join(':', $self->{method},
                           $self->{user} . '@' . $self->{host},
                           $self->port . $self->{root_dir});
}

#### private methods ####

sub parse_root_string
{
    my $self = shift;
    my $root_string = shift;

    my @parts = split(':', $root_string);
    if ( $parts[1] eq 'pserver' ) {
        $self->{method} = 'pserver';
        my ($user, $host) = split('@', $parts[2]);
        $self->{user} = $user;
        $self->{host} = $host;
        if ( $parts[3] =~ /^(\d+)(\/\S+)/ ) {
            $self->{port} = $1;
            $self->{root_dir} = $2;
        }
        else {
            $self->{port} = 2401;
            $self->{root_dir} = $parts[3];
        }
        # sanity check
        if ( !defined($self->{user}) || !defined($self->{host})
             || !defined($self->{port}) || !defined($self->{root_dir}) )
        {
            croak("PCVSLIB::Root::parse_root_string(): can't parse root: '$root_string'");
        }
    }
    else {
        my $unsupported = $parts[1];
        croak("PCVSLIB::Root::parse_root_string(): sorry, CVS method '$unsupported' is not implemented yet");
    }
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
