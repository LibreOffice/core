#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: PathName.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2007-08-27 13:35:11 $
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
# PathName.pm - package for encapsulating 'pathnames'. Many Responses return
#               a pathname, see the cvs client documentation for details
#

package PCVSLib::PathName;

use strict;
use warnings;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $self = {};

    $self->{local_directory} = undef;
    $self->{repository}      = undef;
    $self->{name}            = undef;

    bless ($self, $class);
    return $self;
}

#### instance accessors #####

for my $datum qw(local_directory repository name) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

# conveniance method
sub local_path
{
    my $self = shift;
    my $dir  = $self->{local_directory};
    my $name = $self->{name};

    if ( !defined($dir) || !defined($name) ) {
        croak("PCVSLib::PathName::local_path(): path_name not set");
    }
    else {
        return "$dir/$name";
    }
}

#### public methods ####

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
