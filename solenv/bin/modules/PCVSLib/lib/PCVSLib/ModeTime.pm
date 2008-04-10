#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: ModeTime.pm,v $
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
# ModeTime.pm - helper class for getting and setting file mode and
#               modification time
#

package PCVSLib::ModeTime;

use Carp;
use Fcntl ':mode';

use strict;
use warnings;

use PCVSLib::Time;

#### ctor ####

sub new
{
    my $invocant = shift;
    my $class = ref($invocant) || $invocant;
    my $path_string = shift;
    my $self = {};

    $self->{path} = undef;

    bless ($self, $class);

    if ( defined($path_string) ) {
        $self->{path} = $path_string;
    }
    return $self;
}

#### instance accessors #####

for my $datum qw(path) {
    no strict "refs";
    *$datum = sub {
        my $self = shift;
        $self->{$datum} = shift if @_;
        return $self->{$datum};
    }
}

#### public methods ####

sub get_mode
{
    my $self = shift;

    my $path = $self->{path};
    my @sb = stat($path);
    if ( !@sb ) {
        croak("PCVSLib::Mode::get_mode(): can't stat file '$path': $!");
    }

    return $self->numeric_to_mode($sb[2]);
}

sub set_mode
{
    my $self = shift;
    my $mode = shift;

    my $numeric_mode = $self->mode_to_numeric($mode);
    my $path = $self->{path};
    if ( !chmod($numeric_mode, $path) ) {
        croak("PCVSLib::File::set_mode(): can't change permission on file '$path': $!");
    }
}

sub touch
{
    my $self = shift;
    my $time = shift;

    my $path = $self->{path};

    if ( !utime($time, $time, $path) ) {
        croak("PCVSLib::ModeTime::touch(): can't set modification time of '$path': $!");
    }
}

#### private methods ####

sub numeric_to_mode
{
    my $self         = shift;
    my $numeric_mode = shift;

    my $mode = 'u=';
    $mode .= 'r' if $numeric_mode & S_IRUSR;
    $mode .= 'w' if $numeric_mode & S_IWUSR;
    $mode .= 'x' if $numeric_mode & S_IXUSR;
    $mode .= ',g=';
    $mode .= 'r' if $numeric_mode & S_IRGRP;
    $mode .= 'w' if $numeric_mode & S_IWGRP;
    $mode .= 'x' if $numeric_mode & S_IXGRP;
    $mode .= ',o=';
    $mode .= 'r' if $numeric_mode & S_IROTH;
    $mode .= 'w' if $numeric_mode & S_IWOTH;
    $mode .= 'x' if $numeric_mode & S_IXOTH;

    return $mode;
}

sub mode_to_numeric
{
    my $self         = shift;
    my $mode         = shift;

    my ($user, $group, $other) = split(/,/, $mode);
    my $numeric_mode = 0;

    $numeric_mode |= S_IRUSR if $user  =~ /r/;
    $numeric_mode |= S_IWUSR if $user  =~ /w/;
    $numeric_mode |= S_IXUSR if $user  =~ /x/;
    $numeric_mode |= S_IRGRP if $group =~ /r/;
    $numeric_mode |= S_IWGRP if $group =~ /w/;
    $numeric_mode |= S_IXGRP if $group =~ /x/;
    $numeric_mode |= S_IROTH if $other =~ /r/;
    $numeric_mode |= S_IWOTH if $other =~ /w/;
    $numeric_mode |= S_IXOTH if $other =~ /x/;

    return $numeric_mode;
}

1;
# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
