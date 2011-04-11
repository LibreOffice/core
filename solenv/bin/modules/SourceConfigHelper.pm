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
# SourceConfigHelper - Perl extension for parsing general info databases
#
# usage: see below
#
#*************************************************************************

package SourceConfigHelper;

use strict;

use RepositoryHelper;
use SourceConfig;
use Cwd qw (cwd);
use Carp;

my $debug = 0;
my @source_config_list; # array of sourceconfig objects

#-----------------------------------------------------------------------
#   Constants
#-----------------------------------------------------------------------

use constant SOURCE_CONFIG_NONE => 0;
use constant SOURCE_CONFIG_CURRENT_FIRST => 1;
use constant SOURCE_CONFIG_ENVIRONMENT_FIRST => 2;
use constant SOURCE_CONFIG_CURRENT_ONLY => 3;
use constant SOURCE_CONFIG_ENVIRONMENT_ONLY => 4;

use constant SOURCE_CONFIG_DEFAULT => SOURCE_CONFIG_CURRENT_FIRST;

#####  profiling #####

##### ctor #####

sub new {
    my $proto = shift;
    my $class = ref($proto) || $proto;
    my $init_action = shift;
    my $self = {};
    my $SourceConfigCurrent;
    my $SourceConfigEnvironment;

    $init_action = SOURCE_CONFIG_DEFAULT if (!defined ($init_action));
    if (!eval ($init_action) or ($init_action < SOURCE_CONFIG_NONE) or ($init_action > SOURCE_CONFIG_ENVIRONMENT_ONLY)) {
        croak("wrong initial parameter: $init_action\n");
    }

    if ($init_action != SOURCE_CONFIG_NONE) {
        my $repositoryHash_ref = {};
        if ($init_action != SOURCE_CONFIG_ENVIRONMENT_ONLY) {
            my $initial_directory = cwd();
            my $result = is_repository($initial_directory, $repositoryHash_ref);
            if ($result) {
                $SourceConfigCurrent = SourceConfig->new($repositoryHash_ref->{REPOSITORY_ROOT});
            }
        }
        if ($init_action != SOURCE_CONFIG_CURRENT_ONLY) {
            my $source_config = $ENV{SOURCE_ROOT_DIR} . '/' . SourceConfig::SOURCE_CONFIG_FILE_NAME;
            if (-f $source_config) {
                $SourceConfigEnvironment = SourceConfig->new($source_config);
            }
        }

        # fill array

        if (($init_action == SOURCE_CONFIG_CURRENT_FIRST) or ($init_action == SOURCE_CONFIG_CURRENT_ONLY)) {
            if (defined ($SourceConfigCurrent)) {
                push (@source_config_list, $SourceConfigCurrent);
            }
            if ($init_action == SOURCE_CONFIG_CURRENT_FIRST) {
                if (defined ($SourceConfigEnvironment)) {
                    push (@source_config_list, $SourceConfigEnvironment);
                }
            }
        }
        elsif (($init_action == SOURCE_CONFIG_ENVIRONMENT_FIRST) or ($init_action == SOURCE_CONFIG_ENVIRONMENT_ONLY)) {
            if (defined ($SourceConfigEnvironment)) {
                push (@source_config_list, $SourceConfigEnvironment);
            }
            if ($init_action == SOURCE_CONFIG_ENVIRONMENT_FIRST) {
                if (defined ($SourceConfigCurrent)) {
                    push (@source_config_list, $SourceConfigCurrent);
                }
            }
        }
    }

    $self->{SOURCE_CONFIG_LIST} = \@source_config_list;

    bless($self, $class);
    return $self;
}

##### methods #####

############################################################################################

sub add_SourceConfig {
    my $self = shift;
    my $source_config = shift;
    push (@{$self->{SOURCE_CONFIG_LIST}}, $source_config);
}

############################################################################################

sub get_SourceConfigList {
    my $self = shift;
    return @{$self->{SOURCE_CONFIG_LIST}};
}

############################################################################################

sub has_SourceConfig {
    my $self = shift;
    my $result = 0;
    my $count = @{$self->{SOURCE_CONFIG_LIST}};
    $result = 1 if ($count > 0);
    return $result;
}

############################################################################################

sub get_module_path {
    my $self = shift;
    my $module = shift;
    my $function = \&SourceConfig::get_module_path;
    my $result;
    $result = $self->get_StringResult ($function, $module);
    return $result;
}

############################################################################################

sub get_active_modules {
    my $self = shift;
    my $parameter; # empty
    my $function = \&SourceConfig::get_active_modules;
    my $array_ref;
    $array_ref = $self->get_ArrayResult ($function, $parameter);
    return @$array_ref;
}

############################################################################################

sub get_repositories {
    my $self = shift;
    my $parameter; # empty
    my $function = \&SourceConfig::get_repositories;
    my $array_ref;
    $array_ref = $self->get_ArrayResult ($function, $parameter);
    return @$array_ref;
}

############################################################################################

sub get_module_repository {
    my $self = shift;
    my $module = shift;
    my $function = \&SourceConfig::get_module_repository;
    my $result;
    $result = $self->get_StringResult ($function, $module);
    return $result;
}

############################################################################################

sub is_active {
    my $self = shift;
    my $module = shift;
    my $function = \&SourceConfig::is_active;
    my $result_ref;
    my $is_active = 0;
    $result_ref = $self->get_ResultOfList ($function, $module);
    my $count = @$result_ref;
    if ($count>0) {
        foreach my $active (@$result_ref) {
            if ($active) {
                $is_active = $active;
            }
        }
    }
    return $is_active;
}

##### private methods #####

############################################################################################
#
# is_repository () : check if the directory is a valid repository
#
# input: - directory
#        - hash reference, where the output will be stored
#
# output: 0 = FALSE, the directory is no valid repository
#         1 = TRUE, the repository root can be found in $repositoryHash_ref->{REPOSITORY_ROOT}
#
############################################################################################

sub is_repository {
    my $directory = shift;
    my $repositoryHash_ref = shift;
    $repositoryHash_ref->{INITIAL_DIRECTORY} = $directory;
    $repositoryHash_ref->{REPOSITORY_ROOT} = undef;
    $repositoryHash_ref->{REPOSITORY_NAME} = undef;
    my $result = RepositoryHelper::search_via_build_lst($repositoryHash_ref);
    chdir $repositoryHash_ref->{INITIAL_DIRECTORY};
    if (!$result) {
        $result = RepositoryHelper::search_for_hg($repositoryHash_ref);
    }
    return $result;
}

############################################################################################
#
# get_ResultOfList(): give back an array reference from all SourceConfig Objects results
#
# input: - function : reference to the called function of each SourceConfig Object
#        - parameter : parameter for the called function
#
# output: result : array of all results
#
############################################################################################

sub get_ResultOfList {
    my $self = shift;
    my $function = shift;
    my $parameter = shift;
    my @result;
    foreach my $source_config (@{$self->{SOURCE_CONFIG_LIST}}) {
        push (@result, &$function ($source_config, $parameter));
    }
    return \@result;
}

############################################################################################
#
# get_StringResult(): give back the first defined result from all SourceConfig Objects
#
# input: - function : reference to the called function of each SourceConfig Object
#        - parameter : parameter for the called function
#
# output: result : scalar variable (string), undef if no result
#
############################################################################################

sub get_StringResult {
    my $self = shift;
    my $function = shift;
    my $parameter = shift;
    my $result_ref;
    $result_ref = $self->get_ResultOfList ($function, $parameter);
    my $count = @$result_ref;
    if ($count>0) {
        my $value;
        my $i = 0;
        while (($i < $count) and !defined ($value)) { # search the first defined result
            $value = $$result_ref[$i];
            $i++;
        }
        return $value;
    }
    return undef;
}

############################################################################################
#
# get_StringResult(): give back a sorted and uniqe array reference of the results
#                     from all SourceConfig Objects
#
# input: - function : reference to the called function of each SourceConfig Object
#        - parameter : parameter for the called function
#
# output: result : sorted and uniqe array reference
#
############################################################################################

sub get_ArrayResult {
    my $self = shift;
    my $function = shift;
    my $parameter = shift;
    my $result_ref;
    my @modules;
    $result_ref = $self->get_ResultOfList ($function, $parameter);
    my $count = @$result_ref;
    if ($count>0) {
        my %moduleHash;
        foreach my $module (@$result_ref) {
            $moduleHash{$module}++;
        }
        @modules = sort keys %moduleHash;
    }
    return \@modules;
}

 ##### finish #####

1; # needed by use or require

__END__

=head1 NAME

SourceConfigHelper - Perl extension for handling with SourceConfigObjetcs

=head1 SYNOPSIS

    # example that will read source_config file and return the active repositories

    use SourceConfigHelper;

    # Create a new instance:
    $a = SourceConfigHelper->new();

    # Get repositories for the actual workspace:
    $a->get_repositories();

=head1 DESCRIPTION

SourceConfigHelper is a perl extension to handle more than one objects of SourceConfig
to set up a search order for modules.

Methods:

SourceConfigHelper::new()

Creates a new instance of SourceConfigHelper. Can be initialized by: default - empty or with a constant of search order. default: the source_config will be taken first from the current repository and second from the environment
Possible parameters are:
SourceConfigHelper::SOURCE_CONFIG_NONE - no SourceConfig Object will be created
SourceConfigHelper::SOURCE_CONFIG_CURRENT_FIRST - use the current repository first
SourceConfigHelper::SOURCE_CONFIG_ENVIRONMENT_FIRST - use the repository of the environment first
SourceConfigHelper::SOURCE_CONFIG_CURRENT_ONLY - use only the current repository
SourceConfigHelper::SOURCE_CONFIG_ENVIRONMENT_ONLY - use only the repository of the environment

SourceConfigHelper::get_repositories()

Returns sorted list of active repositories for the actual workspace

SourceConfigHelper::get_active_modules()

Returns a sorted list of active modules

SourceConfigHelper::get_all_modules()

Returns sorted list of all modules in active repositories.

SourceConfigHelper::get_module_path($module)

Returns absolute module path. If the module is not active or don't exists, "undef" will be returned.

SourceConfigHelper::get_module_repository($module)

Returns the module's repository. If the module is not active or don't exists, "undef" will be returned.

SourceConfigHelper::is_active()

Returns 1 (TRUE) if a module is active
Returns 0 (FALSE) if a module is not active

SourceConfigHelper::add_SourceConfig($SourceConfigObject)

Add the SourceConfigObject to the end of the list

SourceConfigHelper::get_SourceConfigList()

Return an array of SourceConfigObjects

SourceConfigHelper::has_SourceConfig()

Returns 1 (TRUE) if one or more SourceConfig Objects is in the list
Returns 0 (FALSE) if no SourceConfig Object is in the list (can happen if there is no valid repository)

=head2 EXPORT

SourceConfigHelper::new()
SourceConfigHelper::get_repositories()
SourceConfigHelper::get_active_modules()
SourceConfigHelper::get_all_modules()
SourceConfigHelper::get_module_path($module)
SourceConfigHelper::get_module_repository($module)
SourceConfigHelper::is_active($module)
SourceConfigHelper::add_SourceConfig($SourceConfigObject)
SourceConfigHelper::get_SourceConfigList()
SourceConfigHelper::has_SourceConfig()

=head1 AUTHOR

Kurt Zenker, kz@openoffice.org

=head1 SEE ALSO

perl(1).

=cut
