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
# SourceConfig - Perl extension for parsing general info databases
#
# usage: see below
#
#*************************************************************************

package SourceConfig;

use strict;

use Carp;
use Cwd;
use RepositoryHelper;
use File::Basename;
use File::Temp qw(tmpnam);

my $debug = 0;

#####  profiling #####

##### ctor #####

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $source_root = shift;
    my @additional_repositories = @_;

    my $self = {};
    $self->{USER_SOURCE_ROOT} = undef;
    if (defined $source_root) {
        $source_root = Cwd::realpath($source_root);
        $source_root =~ s/\\|\/$//;
        $self->{USER_SOURCE_ROOT} = $source_root;
        $source_root .= '/..';
    }
    else
    {
        $source_root = $ENV{SRC_ROOT};
    };
    $source_root = Cwd::realpath($source_root);
    $self->{SOURCE_ROOT} = $source_root;
    $self->{DEBUG} = 0;
    $self->{VERBOSE} = 0;
    $self->{REPOSITORIES} = {};
    $self->{MODULE_PATHS} = {};
    $self->{MODULE_REPOSITORY} = {};
    $self->{REAL_MODULES} = {};
    if (defined $self->{USER_SOURCE_ROOT})
    {
        ${$self->{REPOSITORIES}}{File::Basename::basename($self->{USER_SOURCE_ROOT})} = $self->{USER_SOURCE_ROOT};
    }
    else
    {
        get_fallback_repository($self);
    };
    foreach my $additional_repository (@additional_repositories)
    {
        ${$self->{REPOSITORIES}}{File::Basename::basename($additional_repository)} = $additional_repository;
    }

    get_module_paths($self);
    bless($self, $class);
    return $self;
}

##### methods #####

sub get_repositories
{
    my $self        = shift;
    return sort keys %{$self->{REPOSITORIES}};
}

sub get_module_repository {
    my $self = shift;
    my $module = shift;
    if (defined ${$self->{MODULE_REPOSITORY}}{$module}) {
        return ${$self->{MODULE_REPOSITORY}}{$module};
    } else {
        Carp::cluck("No such module $module in active repositories!!\n");
        return undef;
    };
}

sub get_module_path {
    my $self = shift;
    my $module = shift;
    if (defined ${$self->{MODULE_PATHS}}{$module}) {
        return ${$self->{MODULE_PATHS}}{$module};
    } else {
        Carp::cluck("No path for module $module in active repositories!!\n");
        return undef;
    };
}

sub get_all_modules
{
    my $self = shift;
    my $module = shift;
    return sort keys %{$self->{MODULE_PATHS}};
};

sub get_active_modules
{
    my $self        = shift;
    return sort keys %{$self->{REAL_MODULES}};
}

sub is_active
{
    my $self        = shift;
    my $module      = shift;
    return exists ($self->{REAL_MODULES}{$module});
}

##### private methods #####

sub get_repository_module_paths {
    my $self        = shift;
    my $repository        = shift;
    my $repository_path = ${$self->{REPOSITORIES}}{$repository};
    if (opendir DIRHANDLE, $repository_path) {
        foreach my $module (readdir(DIRHANDLE)) {
            next if (($module =~ /^\.+/) || (!-d "$repository_path/$module"));
            my $module_entry = $module;
            if (($module !~ s/\.lnk$//) && ($module !~ s/\.link$//)) {
                $self->{REAL_MODULES}{$module}++;
            }
            my $possible_path = "$repository_path/$module_entry";
            if (-d $possible_path) {
                if (defined ${$self->{MODULE_PATHS}}{$module}) {
                    close DIRHANDLE;
                    croak("Ambiguous paths for module $module: $possible_path and " . ${$self->{MODULE_PATHS}}{$module});
                };
                ${$self->{MODULE_PATHS}}{$module} = $possible_path;
                ${$self->{MODULE_REPOSITORY}}{$module} = $repository;
            }
        };
        close DIRHANDLE;
    } else {
        croak("Cannot read $repository_path repository content");
    };
};

sub get_module_paths {
    my $self        = shift;
    foreach my $repository (keys %{$self->{REPOSITORIES}}) {
        get_repository_module_paths($self, $repository);
    };
    croak("No modules found!") if (!scalar keys %{$self->{MODULE_PATHS}});
};

#
# Fallback - fallback repository is based on RepositoryHelper educated guess
#
sub get_fallback_repository {
    my $self = shift;
    my $repository_root = RepositoryHelper->new()->get_repository_root();
    ${$self->{REPOSITORIES}}{File::Basename::basename($repository_root)} = $repository_root;
};

##### finish #####

1; # needed by use or require

