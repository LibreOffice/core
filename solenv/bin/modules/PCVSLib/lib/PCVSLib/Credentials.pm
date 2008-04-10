#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: Credentials.pm,v $
#
# $Revision: 1.4 $
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
# Credentials.pm - package for aquiring the CVS credentials from .cvspass
#

package PCVSLib::Credentials;

use Carp;
use IO::File;

use PCVSLib::Root;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{passfile}  = shift;
    # private members
    $self->{parsed_} = 0;
    $self->{passwords_} = {};
    bless ($self, $class);
    return $self;
}

#### instance accessors #####

for my $datum qw(passfile) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub get_password
{
    my $self = shift;
    my $root = shift;

    if ( !$self->{parsed_} ) {
        $self->parse_passfile();
    }

    my $root_string = $root->to_string();
    my $password = $self->{passwords_}->{$root_string};

    if ( !defined($password) ) {
        # try again with port number
        my $root_string_with_port = $root->to_string_with_port();
        $password = $self->{passwords_}->{$root_string_with_port};
    }

    if ( !defined($password) ) {
        my $passfile = $self->{passfile};
        croak("PCVSLIB::Credentials::get_password(): missing entry in '$passfile': '$root_string'");
    }

    return $password;
}


#### private methods ####

sub parse_passfile
{
    my $self = shift;
    my $passfile = $self->{passfile};

    if ( !defined($passfile) ) {
        my $home = $ENV{HOME};
        if ( !defined($home) ) {
            croak("PCVSLIB::Credentials::parse_passfile(): environment variable HOME not set");
        }
        $self->{passfile} = $passfile = "$home/.cvspass";
    }

    my $fh = IO::File->new("<$passfile");
    if ( !defined($fh) ) {
        croak("PCVSLIB::Credentials::parse_passfile(): can't open CVS password file: '$passfile': $!");
    }
    while(<$fh>) {
        tr/\r\n//d;
        if ( /^\/1 (:pserver:\S+) (\S.*)$/ ) {
            # new style .cvspass entries
            $self->{passwords_}->{$1} = $2;
        }
        elsif ( /(:pserver:\S+) (\S+.*)$/ ) {
            # old style .cvspass entries
            $self->{passwords_}->{$1} = $2;
        }
    }
    $fh->close();
    $self->{parsed_}++;
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
