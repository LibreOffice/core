#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: EventHandler.pm,v $
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
# EventHandler.pm - notifies listeners about incoming events
#

package PCVSLib::EventHandler;

use Carp;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    # private members
    $self->{listeners_}  = ();
    bless ($self, $class);
    return $self;
}

#### public methods ####

sub add_listener
{
    my $self     = shift;
    my $listener = shift;

    if ( $listener->can('notify') ) {
        push(@{$self->{listeners_}}, $listener);
    }
    else {
        croak("PCVSLIB::EventHandler::add_listener(): added object is not a listener");
    }
}

sub remove_listener
{
    my $self     = shift;
    my $listener = shift;

    my $num_listeners = scalar(@{$self->{listeners_}});

    my $index = -1;
    for (my $i = 0; $i < $num_listeners; $i++) {
         if ( $self->{listeners_}->[$i] == $listener ) {
             $index = $i;
             last;
         }
    }

    if ( $index == -1 ) {
        croak("PCVSLIB::EventHandler::remove_listener(): to be removed object not in listener list");
    }
    else {
        splice(@{$self->{listeners_}}, $index, 1);
    }
}

sub send_event
{
    my $self  = shift;
    my $event = shift;

    foreach ( @{$self->{listeners_}} ) {
        $_->notify($event);
    }
}

#### private methods ####

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
