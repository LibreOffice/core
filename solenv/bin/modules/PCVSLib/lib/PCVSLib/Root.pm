#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: Root.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:35:41 $
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
