# -*- Mode: Perl; tab-width: 4; indent-tabs-mode: nil; -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

#*************************************************************************
#
# RepositoryHelper - Perl for working with repositories
#
# usage: see below
#
#*************************************************************************

package RepositoryHelper;

use strict;


use Carp;
use Cwd qw (cwd);
use File::Basename;
#use File::Temp qw(tmpnam);

my $debug = 0;

#####  profiling #####

##### ctor #####

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $initial_directory = shift;
    if ($initial_directory) {
        $initial_directory = Cwd::realpath($initial_directory);
    } else {
        if ( defined $ENV{PWD} ) {
            $initial_directory = $ENV{PWD};
        } elsif (defined $ENV{_cwd}) {
            $initial_directory = $ENV{_cwd};
        } else {
            $initial_directory = cwd();
        };
    };
    my $self = {};
    $self->{INITIAL_DIRECTORY} = $initial_directory;
    $self->{REPOSITORY_ROOT} = undef;
    if (! search_via_build_lst($self))
    {
        croak('Cannot determine source directory/repository for ' . $self->{INITIAL_DIRECTORY});
    }
    bless($self, $class);
    return $self;
}

##### methods #####
sub get_repository_root
{
    my $self        = shift;
    return $self->{REPOSITORY_ROOT};
}

sub get_initial_directory
{
    my $self        = shift;
    return $self->{INITIAL_DIRECTORY};
}

sub search_via_build_lst {
    my $self = shift;
    my $previous_dir = '';
    my $rep_root_candidate = $self->{INITIAL_DIRECTORY};
    do {
        my $test_file;
        if ($rep_root_candidate eq '/') {
            $test_file = '/prj/build.lst';
        } else {
            $test_file = $rep_root_candidate . '/prj/build.lst';
        };
        if (-e $test_file) {
            $self->{REPOSITORY_ROOT} = File::Basename::dirname($rep_root_candidate);
            return 1;
        };
        $previous_dir = $rep_root_candidate;
        $rep_root_candidate = File::Basename::dirname($rep_root_candidate);
        return 0 if ((!$rep_root_candidate) || ($rep_root_candidate eq $previous_dir));
    }
    while (chdir "$rep_root_candidate");
};

##### finish #####

1; # needed by use or require

__END__

=head1 NAME

RepositoryHelper - Perl module for working with repositories

=head1 SYNOPSIS

    # example that will analyze sources and return the source root directory

    use RepositoryHelper;

    # Create a new instance:
    $a = RepositoryHelper->new();

    # Get repositories for the actual workspace:
    $a->get_repository_root();


=head1 DESCRIPTION

RepositoryHelper is a perlPerl module for working with repositories
in the database.

Methods:

RepositoryHelper::new()

Creates a new instance of RepositoryHelper. Can be initialized by: some path which likely to belong to a repository, default - empty, the current dir will be taken.

RepositoryHelper::get_repository_root()

Returns the repository root, retrieved by educated guess...

RepositoryHelper::get_initial_directory()

Returns full path to the initialistion directory.

=head2 EXPORT

RepositoryHelper::new()
RepositoryHelper::get_repository_root()
RepositoryHelper::get_initial_directory()

=head1 AUTHOR

Vladimir Glazunov, vg@openoffice.org

=head1 SEE ALSO

perl(1).

=cut
