#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: SourceConfig.pm,v $
#
# $Revision: 1.0 $
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

use constant SOURCE_CONFIG_FILE_NAME => 'source_config';

use Carp;
use Cwd;
use File::Basename;

my $debug = 0;

#####  profiling #####

##### ctor #####

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $source_root = shift;
    if (defined $source_root) {
        $source_root =~ s/\\|\/$//;
    } else {
        $source_root = $ENV{SOLARSRC};
    };
    my $self = {};
    $self->{DEBUG} = 0;
    $self->{SOURCE_ROOT} = $source_root;
    $self->{REPOSITORIES} = {};
    $self->{MODULE_PATHS} = {};
    $self->{MODULE_BUILD_LIST_PATHS} = {};
    $self->{ACTIVATED_MODULES} = {};
    $self->{MODULE_REPOSITORY} = {};
    $self->{REAL_MODULES} = {};
    $self->{SOURCE_CONFIG_FILE} = get_config_file($source_root);
    $self->{SOURCE_CONFIG_DEFAULT} = Cwd::realpath($source_root) .'/'.SOURCE_CONFIG_FILE_NAME;
    read_config_file($self);
    bless($self, $class);
    return $self;
}

##### methods #####

sub get_repositories
{
    my $self        = shift;
    return sort keys %{$self->{REPOSITORIES}};
}

sub get_config_file_default_path {
    my $self        = shift;
    return $self->{SOURCE_CONFIG_DEFAULT};
}

sub get_config_file_path {
    my $self = shift;
    return $self->{SOURCE_CONFIG_FILE};
}

sub get_module_repository {
    my $self = shift;
    my $module = shift;
    $self -> get_module_paths() if (!scalar keys %{$self->{MODULE_PATHS}});
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
    $self -> get_module_paths() if (!scalar keys %{$self->{MODULE_PATHS}});
    if (defined ${$self->{MODULE_PATHS}}{$module}) {
        return ${$self->{MODULE_PATHS}}{$module};
    } else {
        Carp::cluck("No path for module $module in active repositories!!\n") if ($debug);
        return undef;
    };
}

sub get_module_build_list {
    my $self = shift;
    my $module = shift;
    $self -> get_buildlist_paths() if (!scalar keys %{$self->{MODULE_BUILD_LIST_PATHS}});
    if (defined ${$self->{MODULE_BUILD_LIST_PATHS}}{$module}) {
        return ${$self->{MODULE_BUILD_LIST_PATHS}}{$module};
    } else {
        Carp::cluck("No build list in module $module found!!\n") if ($self->{DEBUG});
        return undef;
    };
}

sub get_all_modules
{
    my $self = shift;
    my $module = shift;
    $self -> get_module_paths() if (!scalar keys %{$self->{MODULE_PATHS}});
    return sort keys %{$self->{MODULE_PATHS}};
};

sub get_active_modules
{
    my $self        = shift;
    $self -> get_module_paths() if (!scalar keys %{$self->{MODULE_PATHS}});
    if (scalar keys %{$self->{ACTIVATED_MODULES}}) {
        return sort keys %{$self->{ACTIVATED_MODULES}};
    };
    return sort keys %{$self->{REAL_MODULES}};
}

sub is_active
{
    my $self        = shift;
    my $module      = shift;
    $self -> get_module_paths() if (!scalar keys %{$self->{MODULE_PATHS}} && !scalar keys %{$self->{ACTIVATED_MODULES}});
    if (scalar keys %{$self->{ACTIVATED_MODULES}}) {
        return exists ($self->{ACTIVATED_MODULES}{$module});
    } else {
        return exists ($self->{REAL_MODULES}{$module});
    }
}

##### private methods #####

sub get_buildlist_paths {
    my $self        = shift;
    $self -> get_module_paths() if (!scalar keys %{$self->{MODULE_PATHS}});
    my @possible_build_lists = ('build.lst', 'build.xlist'); # build lists names
    foreach my $module (keys %{$self->{MODULE_PATHS}}) {
        foreach (@possible_build_lists) {
            my $possible_path = ${$self->{MODULE_PATHS}}{$module} . "/prj/$_";
            ${$self->{MODULE_BUILD_LIST_PATHS}}{$module} = $possible_path if (-e $possible_path);
        };
    };
};

sub get_module_paths {
    my $self        = shift;
    foreach my $repository (keys %{$self->{REPOSITORIES}}) {
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
            croak("Cannot read $_ repository content");
        };
    };
    croak("No modules found!") if (!scalar keys %{$self->{MODULE_PATHS}});
};

sub get_config_file {
    my $source_root = shift;
    foreach ($source_root, $source_root . '/..') {
        if (-f $_ . '/' . SOURCE_CONFIG_FILE_NAME) {
            return Cwd::realpath($_) .'/'.SOURCE_CONFIG_FILE_NAME;
        };
    };
    return '';
};

sub read_config_file {
    my $self = shift;
    if (!$self->{SOURCE_CONFIG_FILE}) {
        ${$self->{REPOSITORIES}}{File::Basename::basename($self->{SOURCE_ROOT})} = $self->{SOURCE_ROOT};
        return;
    };
    my $repository_section = 0;
    my $module_section = 0;
    my $line = 0;
    if (open(SOURCE_CONFIG_FILE, $self->{SOURCE_CONFIG_FILE})) {
        foreach (<SOURCE_CONFIG_FILE>) {
            $line++;
            chomp;
            next if (!/^\S+/);
            next if (/^\s*#+/);
            s/\r\n//;
            if (/^\[repositories\]\s*(\s+#)*/) {
                $module_section = 0;
                $repository_section = 1;
                next;
            };
            if (/^\[modules\]\s*(\s+#)*/) {
                $module_section = 1;
                $repository_section = 0;
                next;
            };
            next if (!$repository_section && !$module_section);
            if (/\s*(\S+)=active\s*(\s+#)*/) {
                if ($repository_section) {
                    ${$self->{REPOSITORIES}}{$1} = File::Basename::dirname($self->{SOURCE_ROOT}) . "/$1";
                    next;
                }
                if ($module_section) {
                    ${$self->{ACTIVATED_MODULES}}{$1}++;
                    next;
                };
            };
            croak("Line $line in " . $self->{SOURCE_CONFIG_FILE} . 'violates format. Please make your checks!!');
        };
        close SOURCE_CONFIG_FILE;
    } else {
        croak('Cannot open ' . $self->{SOURCE_CONFIG_FILE} . 'for reading');
    };
};

##### finish #####

1; # needed by use or require

__END__

=head1 NAME

SourceConfig - Perl extension for parsing general info databases

=head1 SYNOPSIS

    # example that will read source_config file and return the active repositories

    use SourceConfig;

    # Create a new instance of the parser:
    $a = SourceConfig->new();

    # Get repositories for the actual workspace:
    $a->get_repositories();


=head1 DESCRIPTION

SourceConfig is a perl extension to load and parse General Info Databses.
It uses a simple object oriented interface to retrieve the information stored
in the database.

Methods:

SourceConfig::new()

Creates a new instance of SourceConfig. Can't fail.


SourceConfig::get_repositories()

Returns sorted list of active repositories for the actual workspace


SourceConfig::get_active_modules()

Returns a sorted list of active modules

SourceConfig::get_all_modules()

Returns sorted list of all modules in active repositories.

SourceConfig::get_module_path($module)

Returns absolute module path

SourceConfig::get_module_build_list($module)

Returns absolute module build list path

SourceConfig::get_module_repository($module)

Returns the module's repository

SourceConfig::get_config_file_path()

Returns absolute module to the source configuration file

SourceConfig::get_config_file_default_path()

Returns default path for source configuration file

SourceConfig::is_active()

Returns 1 (TRUE) if a module is active
Returns 0 (FALSE) if a module is not active

=head2 EXPORT

SourceConfig::new()
SourceConfig::get_repositories()
SourceConfig::get_active_modules()
SourceConfig::get_all_modules()
SourceConfig::get_module_path($module)
SourceConfig::get_module_build_list($module)
SourceConfig::get_module_repository($module)
SourceConfig::get_config_file_path()
SourceConfig::get_config_file_default_path()
SourceConfig::is_active($module)

=head1 AUTHOR

Vladimir Glazunov, vg@openoffice.com

=head1 SEE ALSO

perl(1).

=cut
