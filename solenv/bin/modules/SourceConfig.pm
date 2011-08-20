# -*- Mode: Perl; tab-width: 4; indent-tabs-mode: nil; -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

#*************************************************************************
#
# SourceConfig - Perl extension for parsing general info databases
#
# usage: see below
#
#*************************************************************************

package SourceConfig;

use strict;

use constant SOURCE_CONFIG_VERSION => 3;

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
    $self->{ACTIVATED_REPOSITORIES} = {};
    $self->{MODULE_PATHS} = {};
    $self->{MODULE_GBUILDIFIED} = {};
    $self->{MODULE_BUILD_LIST_PATHS} = {};
    $self->{MODULE_REPOSITORY} = {};
    $self->{REAL_MODULES} = {};
    $self->{NEW_MODULES} = [];
    $self->{REMOVE_MODULES} = {};
    $self->{REMOVE_REPOSITORIES} = {};
    $self->{NEW_REPOSITORIES} = [];
    $self->{WARNINGS} = [];
    $self->{REPORT_MESSAGES} = [];
    $self->{CONFIG_FILE_CONTENT} = [];
    if (defined $self->{USER_SOURCE_ROOT})
    {
        ${$self->{REPOSITORIES}}{File::Basename::basename($self->{USER_SOURCE_ROOT})} = $self->{USER_SOURCE_ROOT};
    }
    else
    {
        get_fallback_repository($self);
    };

    get_module_paths($self);
    bless($self, $class);
    return $self;
}

##### methods #####

sub get_version {
    return SOURCE_CONFIG_VERSION;
};

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

sub get_module_build_list {
    my $self = shift;
    my $module = shift;
    if (defined ${$self->{MODULE_BUILD_LIST_PATHS}}{$module}) {
        return ${$self->{MODULE_BUILD_LIST_PATHS}}{$module};
    }
    else
    {
        my $module_path = ${$self->{MODULE_PATHS}}{$module};
        if ( -e $module_path . "/prj/build.lst")
        {
            ${$self->{MODULE_BUILD_LIST_PATHS}}{$module} = $module_path . "/prj/build.lst";

            if (-e $module_path . "/prj/makefile.mk" )
            {
                print "module $module -> gbuild\n";
                ${$self->{MODULE_GBUILDIFIED}}{$module} = 1;
            }
            else
            {
                print "module $module -> dmake\n";
                ${$self->{MODULE_GBUILDIFIED}}{$module} = 0;
            }
            return $module_path . "/prj/build.lst";
        };
        Carp::cluck("No build list in module $module found!!\n") if ($self->{DEBUG});
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

sub is_gbuild
{
    my $self        = shift;
    my $module      = shift;
    if (defined ${$self->{MODULE_GBUILDIFIED}}{$module})
    {
        return ${$self->{MODULE_GBUILDIFIED}}{$module};
    };
    return undef;
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

