#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: Connection.pm,v $
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
# Connection.pm - package for creating/manipulating a connection to
#                 a CVS server
#

package PCVSLib::Connection;

use Class::Struct;
use Carp;
use IO::Socket;

use PCVSLib::Root;
use PCVSLib::LogHandle;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{root} = shift;
    $self->{scrambled_password} = shift;
    $self->{io_handle} = undef;
    # private members
    $self->{is_open_} = undef;
    bless ($self, $class);
    return $self;
}

for my $datum qw(root scrambled_password io_handle) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub open
{
    my $self = shift;

    my $method = $self->{root}->method();
    if ( !$method ) {
        croak("PCVSLIB::Connection::open(): method not set");
    }
    # pserver style connection
    if ( $method eq 'pserver' ) {
        return $self->pserver_open();
    }
    else {
        croak("PCVSLIB::Connection::open(): unsupported method '$method'");
    }
}

sub close
{
    my $self = shift;

    return if !$self->{is_open_};

    my $method = $self->{root}->method();
    if ( !$method ) {
        croak("PCVSLIB::Connection::close(): method not set");
    }
    # pserver style connection
    if ( $method eq 'pserver' ) {
         $self->pserver_close();
         return;
    }
    else {
        croak("PCVSLIB::Connection::close(): unsupported method '$method'");
    }
    return;
}

#### private methods ####

sub pserver_open
{
    my $self = shift;

    my $user     = $self->{root}->user();
    my $host     = $self->{root}->host();
    my $port     = $self->{root}->port();
    my $root_dir = $self->{root}->root_dir();
    my $password = $self->{scrambled_password}
                     or croak("PCVSLib::Connection::pserver_open(): scrambled_password not set");

    # open connection
    my $cvs_socket = IO::Socket::INET->new($host . ':' . $port)
          or croak("PCVSLib::Connection::pserver_open(): couldn't connect to host '$host', port '$port': $!");

    # authenticate
    $cvs_socket->print("BEGIN AUTH REQUEST\n");
    $cvs_socket->print("$root_dir\n");
    $cvs_socket->print("$user\n");
    $cvs_socket->print("$password\n");
    $cvs_socket->print("END AUTH REQUEST\n");

    my $response = $cvs_socket->getline();

    if ( $response =~ /^I HATE YOU/ ) {
        croak("PCVSLib::Connection::pserver_open(): '$host': authentication failed");
    }

    if ( $response =~ /^E / || $response =~ /^error \d+ / ) {
        my $error = $';
        chomp($error);
        croak("PCVSLib::Connection::pserver_open(): '$host': $error");
    }

    if ( $response =~ /^I LOVE YOU/ ) {
        $self->{io_handle} = $cvs_socket;
        $self->{is_open_} = 1;
        return $cvs_socket;
    }

    # shouldn't happen
    croak("PCVSLib::Connection::pserver_open(): '$host': unknown authorization response: $response");
}

sub pserver_close
{
    my $self = shift;
    $self->{io_handle}->shutdown(2);
    $self->{io_handle} = undef;
    $self->{is_open_} = 0;
    return;
}

sub DESTROY
{
    my $self = shift;
    $self->close();
}

####

1; # needed by "use" or "require"
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
