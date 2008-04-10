#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: Event.pm,v $
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
# Event.pm - encapuslates events triggered by Responses
#

package PCVSLib::Event;

use Carp;

use PCVSLib::Time;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};
    $self->{event_data} = shift;
    # private members

    bless ($self, $class);

    return $self;
}

#### instance accessors #####

for my $datum qw(event_data) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

#### private methods ####

package PCVSLib::MessageEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_message
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::MTEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_message
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::ErrorMessageEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_message
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::TerminatedEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub is_success
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::ValidRequestsEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_valid_requests
{
    my $self          = shift;

    my @valid_requests = split(" ", $self->{event_data});

    return \@valid_requests;
}

package PCVSLib::ClearStickyEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_directory
{
    my $self          = shift;

    return $self->{event_data}->local_directory();
}

sub get_repository
{
    my $self          = shift;

    return $self->{event_data}->repository();
}

package PCVSLib::SetStickyEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_directory
{
    my $self          = shift;

    return $self->{event_data}->[0]->local_directory();
}

sub get_repository
{
    my $self          = shift;

    return $self->{event_data}->[0]->repository();
}

sub get_tag
{
    my $self          = shift;

    return $self->{event_data}->[1];
}

package PCVSLib::SetStaticDirectoryEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_directory
{
    my $self          = shift;

    return $self->{event_data}->local_directory();
}

sub get_repository
{
    my $self          = shift;

    return $self->{event_data}->repository();
}

package PCVSLib::ClearStaticDirectoryEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_directory
{
    my $self          = shift;

    return $self->{event_data}->local_directory();
}

sub get_repository
{
    my $self          = shift;

    return $self->{event_data}->repository();
}

package PCVSLib::ModuleExpansionEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_module_expansion
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::UpdatedEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_file
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::MergedEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_file
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::CreatedEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_file
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::UpdateExistingEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_file
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::ModTimeEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_modification_time
{
    my $self          = shift;

    return PCVSLib::Time::rfc822_1123_to_seconds($self->{event_data});
}

package PCVSLib::ModeEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_mode
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::CheckedInEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_path_name
{
    my $self          = shift;

    return $self->{event_data}->[0];
}

sub get_entry
{
    my $self          = shift;

    return $self->{event_data}->[1];
}

package PCVSLib::RemoveEntryEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_path_name
{
    my $self          = shift;

    return $self->{event_data};
}

package PCVSLib::CopyFileEvent;
use vars('@ISA');
@ISA=('PCVSLib::Event');

sub get_path_name
{
    my $self          = shift;

    return $self->{event_data}->[0];
}

sub get_new_name
{
    my $self          = shift;

    return $self->{event_data}->[1];
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
