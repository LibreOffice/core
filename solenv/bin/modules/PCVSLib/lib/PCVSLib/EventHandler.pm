#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: EventHandler.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:34:23 $
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
