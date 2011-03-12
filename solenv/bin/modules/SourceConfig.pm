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

use constant SOURCE_CONFIG_FILE_NAME => 'source_config';
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
    $self->{SOURCE_CONFIG_FILE} = undef;
    if (defined $source_root) {
        $source_root = Cwd::realpath($source_root);
        $source_root =~ s/\\|\/$//;
        if (-f $source_root) {
            # We have path to source_config
            if (File::Basename::basename($source_root) eq 'source_config') {
                # We have path to source_config
                $self->{SOURCE_CONFIG_FILE} = $source_root;
                $source_root = File::Basename::dirname($source_root);
            } else {
                croak("$source_root is not a source_config file");
            };
        } else {
            $self->{USER_SOURCE_ROOT} = $source_root;
            $source_root .= '/..';
        }
    } else {
        $source_root = $ENV{SRC_ROOT};
    };
    $source_root = Cwd::realpath($source_root);
    $self->{SOURCE_ROOT} = $source_root;
    $self->{DEBUG} = 0;
    $self->{VERBOSE} = 0;
    $self->{REPOSITORIES} = {};
    $self->{ACTIVATED_REPOSITORIES} = {};
    $self->{MODULE_PATHS} = {};
    $self->{MODULE_BUILD_LIST_PATHS} = {};
    $self->{ACTIVATED_MODULES} = {};
    $self->{MODULE_REPOSITORY} = {};
    $self->{REAL_MODULES} = {};
    $self->{NEW_MODULES} = [];
    $self->{REMOVE_MODULES} = {};
    $self->{REMOVE_REPOSITORIES} = {};
    $self->{NEW_REPOSITORIES} = [];
    $self->{WARNINGS} = [];
    $self->{REPORT_MESSAGES} = [];
    $self->{CONFIG_FILE_CONTENT} = [];
    if (defined $self->{USER_SOURCE_ROOT}) {
        ${$self->{REPOSITORIES}}{File::Basename::basename($self->{USER_SOURCE_ROOT})} = $self->{USER_SOURCE_ROOT};
    };
    $self->{SOURCE_CONFIG_FILE} = get_config_file($self->{SOURCE_ROOT}) if (!defined $self->{SOURCE_CONFIG_FILE});
    $self->{SOURCE_CONFIG_DEFAULT} = $self->{SOURCE_ROOT} .'/'.SOURCE_CONFIG_FILE_NAME;
    if (defined $self->{USER_SOURCE_ROOT}) {
        ${$self->{REPOSITORIES}}{File::Basename::basename($self->{USER_SOURCE_ROOT})} = $self->{USER_SOURCE_ROOT};
    };
    read_config_file($self);
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

sub add_repository
{
    my $self        = shift;
    my $new_rep_path = shift;
    $new_rep_path = Cwd::realpath($new_rep_path);
    my $new_rep_name = File::Basename::basename($new_rep_path);
    if (defined ${$self->{REPOSITORIES}}{$new_rep_name}) {
        croak("Repository $new_rep_name is already defined!!");
    };
    ${$self->{REPOSITORIES}}{$new_rep_name} = $new_rep_path;
    $self -> get_repository_module_paths($new_rep_name);
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
        Carp::cluck("No path for module $module in active repositories!!\n") if ($debug);
        return undef;
    };
}

sub get_module_build_list {
    my $self = shift;
    my $module = shift;
    if (defined ${$self->{MODULE_BUILD_LIST_PATHS}}{$module}) {
        return ${$self->{MODULE_BUILD_LIST_PATHS}}{$module};
    } else {
        my @possible_build_lists = ('build.lst', 'build.xlist'); # build lists names
        foreach (@possible_build_lists) {
            my $possible_path = ${$self->{MODULE_PATHS}}{$module} . "/prj/$_";
            if (-e $possible_path) {
                ${$self->{MODULE_BUILD_LIST_PATHS}}{$module} = $possible_path;
                return $possible_path;
            };
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
    if (scalar keys %{$self->{ACTIVATED_MODULES}}) {
        return sort keys %{$self->{ACTIVATED_MODULES}};
    }
       return sort keys %{$self->{REAL_MODULES}};
}

sub is_active
{
    my $self        = shift;
    my $module      = shift;
    if (scalar keys %{$self->{ACTIVATED_MODULES}}) {
        return exists ($self->{ACTIVATED_MODULES}{$module});
    }
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
    my @false_actives = ();
    foreach (keys %{$self->{ACTIVATED_MODULES}}) {
        push(@false_actives, $_) if (!defined  ${$self->{MODULE_PATHS}}{$_});
    };
    croak("Error!! Activated module(s): @false_actives\nnot found in the active repositories!! Please check your " . $self->{SOURCE_CONFIG_FILE} . "\n") if (scalar @false_actives);
    croak("No modules found!") if (!scalar keys %{$self->{MODULE_PATHS}});
};

sub get_config_file {
    my $source_root = shift;
    my $possible_path = $source_root . '/' . SOURCE_CONFIG_FILE_NAME;
    return $possible_path if (-f $possible_path);
    return '';
};

#
# Fallback - fallback repository is based on RepositoryHelper educated guess
#
sub get_fallback_repository {
    my $self = shift;
    my $repository_root = RepositoryHelper->new()->get_repository_root();
    ${$self->{REPOSITORIES}}{File::Basename::basename($repository_root)} = $repository_root;
};

sub read_config_file {
    my $self = shift;
    if (!$self->{SOURCE_CONFIG_FILE}) {
        if (!defined $self->{USER_SOURCE_ROOT}) {
            get_fallback_repository($self);
        };
        return;
    };
    my $repository_section = 0;
    my $module_section = 0;
    my $line = 0;
    my @file_content = ();

    if (open(SOURCE_CONFIG_FILE, $self->{SOURCE_CONFIG_FILE})) {
        foreach (<SOURCE_CONFIG_FILE>) {
            push (@{$self->{CONFIG_FILE_CONTENT}}, $_);
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
                    my $repository_source_path = $self->{SOURCE_ROOT} . "/$1";
                    if (defined $ENV{UPDMINOREXT}) {
                        $repository_source_path .= $ENV{UPDMINOREXT};
                        if (defined ${$self->{REPOSITORIES}}{$1.$ENV{UPDMINOREXT}}) {
                            delete ${$self->{REPOSITORIES}}{$1.$ENV{UPDMINOREXT}};
                        };
                    };
                    ${$self->{REPOSITORIES}}{$1} = $repository_source_path;
                    ${$self->{ACTIVATED_REPOSITORIES}}{$1}++;
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
        if (!scalar keys %{$self->{REPOSITORIES}}) {
            get_fallback_repository($self);
        };
    } else {
        croak('Cannot open ' . $self->{SOURCE_CONFIG_FILE} . 'for reading');
    };
};

sub remove_all_activated_repositories {
    my $self = shift;
    $self->remove_activated_repositories([keys %{$self->{ACTIVATED_REPOSITORIES}}]);
};

sub remove_activated_repositories {
    my $self = shift;
    my $new_repositories_ref = shift;
    push(@{$self->{WARNINGS}}, "\nWARNING: Empty repository list passed for removing from source_config\n") if (!scalar @$new_repositories_ref);
    $self->{VERBOSE} = shift;
    $self->{REMOVE_REPOSITORIES} = {};
    foreach (@$new_repositories_ref) {
        if (!defined ${$self->{ACTIVATED_REPOSITORIES}}{$_}) {
            push (@{$self->{WARNINGS}}, "\nWARNING: repository $_ is not activated in ". $self->get_config_file_default_path()."\n");
        } else {
            ${$self->{REMOVE_REPOSITORIES}}{$_}++;
            delete ${$self->{ACTIVATED_REPOSITORIES}}{$_};
        };
    };
    generate_config_file($self);
};

sub remove_all_activated_modules {
    my $self = shift;
    $self->remove_activated_modules([keys %{$self->{ACTIVATED_MODULES}}]);
};

sub remove_activated_modules {
    my $self = shift;
    my $new_modules_ref = shift;
    push(@{$self->{WARNINGS}}, "\nWARNING: Empty module list passed for removing from source_config\n") if (!scalar @$new_modules_ref);
    $self->{VERBOSE} = shift;
    $self->{REMOVE_MODULES} = {};
    foreach (@$new_modules_ref) {
        if (!defined ${$self->{ACTIVATED_MODULES}}{$_}) {
            push (@{$self->{WARNINGS}}, "\nWARNING: module $_ is not activated in ". $self->get_config_file_default_path()."\n");
        } else {
            ${$self->{REMOVE_MODULES}}{$_}++;
            delete ${$self->{ACTIVATED_MODULES}}{$_};
        };
    };
    generate_config_file($self);
};

sub add_active_repositories {
    my $self = shift;
    $self->{NEW_REPOSITORIES} = shift;
    croak('Empty repository list passed for addition to source_config') if (!scalar @{$self->{NEW_REPOSITORIES}});
    $self->{VERBOSE} = shift;
    foreach (@{$self->{NEW_REPOSITORIES}}) {
        $self->add_repository($_);
    };
    generate_config_file($self);
};

sub add_active_modules {
    my $self = shift;
    $self->{NEW_MODULES} = shift;
    croak('Empty module list passed for addition to source_config') if (!scalar @{$self->{NEW_MODULES}});
    $self->{VERBOSE} = shift;
    generate_config_file($self);
};

sub add_content {
    my $self = shift;
    my $content = shift;
    my $entries_to_add = shift;
    return if (!scalar @$entries_to_add);
    my $message;
    my $message_part1;
    my $warning_message;
    my $activated_entries;

    if ($entries_to_add == $self->{NEW_MODULES}) {
        $self->{NEW_MODULES} = [];
        $message_part1 = "Module(s):\n";
        $activated_entries = $self->{ACTIVATED_MODULES};
    } elsif ($entries_to_add == $self->{NEW_REPOSITORIES}) {
        $self->{NEW_REPOSITORIES} = [];
        $message_part1 = "Repositories:\n";
        $activated_entries = $self->{ACTIVATED_REPOSITORIES};
    };
    foreach my $entry (@$entries_to_add) {
        if (defined $$activated_entries{$entry}) {
            $warning_message .= "$entry "
        } else {
            push(@$content, "$entry=active\n");
            ${$activated_entries}{$entry}++;
            $message .= "$entry "
        };
    };

    push(@{$self->{REPORT_MESSAGES}}, "\n$message_part1 $message\nhave been added to the ". $self->get_config_file_default_path()."\n") if ($message);
    push (@{$self->{WARNINGS}}, "\nWARNING: $message_part1 $warning_message\nare already added to the ". $self->get_config_file_default_path()."\n") if ($warning_message);
};

sub generate_config_file {
    my $self = shift;
    my @config_content_new = ();
    my ($module_section, $repository_section);
    my %removed_modules = ();
    my %removed_repositories = ();
    foreach (@{$self->{CONFIG_FILE_CONTENT}}) {
        if (/^\[repositories\]\s*(\s+#)*/) {
            if ($module_section) {
                $self->add_content(\@config_content_new, $self->{NEW_MODULES});
            };
            $module_section = 0;
            $repository_section = 1;
        };
        if (/^\[modules\]\s*(\s+#)*/) {
            if ($repository_section) {
                $self->add_content(\@config_content_new, $self->{NEW_REPOSITORIES});
            };
            $module_section = 1;
            $repository_section = 0;
        };
        if ($module_section && /\s*(\S+)=active\s*(\s+#)*/) {
            if (defined ${$self->{REMOVE_MODULES}}{$1}) {
                $removed_modules{$1}++;
                next;
            };
        }
        if ($repository_section && /\s*(\S+)=active\s*(\s+#)*/) {
            if (defined ${$self->{REMOVE_REPOSITORIES}}{$1}) {
                $removed_repositories{$1}++;
                next;
            };
        }
        push(@config_content_new, $_);
    };
    if (scalar @{$self->{NEW_MODULES}}) {
        push(@config_content_new, "[modules]\n") if (!$module_section);
        $self->add_content(\@config_content_new, $self->{NEW_MODULES});
    };
    if (scalar @{$self->{NEW_REPOSITORIES}}) {
        push(@config_content_new, "[repositories]\n") if (!$repository_section);
        $self->add_content(\@config_content_new, $self->{NEW_REPOSITORIES});
    };
    if (scalar keys %removed_modules) {
        my @deleted_modules = keys %removed_modules;
        push(@{$self->{REPORT_MESSAGES}}, "\nModules: @deleted_modules\nhave been removed from the ". $self->get_config_file_default_path()."\n");

    };
    if (scalar keys %removed_repositories) {
        my @deleted_repositories = keys %removed_repositories;
        push(@{$self->{REPORT_MESSAGES}}, "\nRepositories: @deleted_repositories\nhave been removed from the ". $self->get_config_file_default_path()."\n");

    };

    # Writing file, printing warnings and reports

    #check if we need to write a new file
    my $write_needed = 0;
    if ((scalar @{$self->{CONFIG_FILE_CONTENT}}) != (scalar @config_content_new)) {
        $write_needed++;
    } else {
        foreach my $i (0 .. $#{$self->{CONFIG_FILE_CONTENT}}) {
            if (${$self->{CONFIG_FILE_CONTENT}}[$i] ne $config_content_new[$i]) {
                $write_needed++;
                last;
            };
        };
    };
    if ($write_needed) {
        my $temp_config_file = File::Temp::tmpnam($ENV{TMP});
        die("Cannot open $temp_config_file") if (!open(NEW_CONFIG, ">$temp_config_file"));
        print NEW_CONFIG $_ foreach (@config_content_new);
        close NEW_CONFIG;
        rename($temp_config_file, $self->get_config_file_default_path()) or  system("mv", $temp_config_file, $self->get_config_file_default_path());
        if (-e $temp_config_file) {
            system("rm -rf $temp_config_file") if (!unlink $temp_config_file);
        };
        $self->{CONFIG_FILE_CONTENT} = \@config_content_new;
    };
    if ($self->{VERBOSE}) {
        print $_ foreach (@{$self->{WARNINGS}});
        $self->{VERBOSE} = 0;
    };
    $self->{WARNINGS} = [];
    print $_ foreach (@{$self->{REPORT_MESSAGES}});
    $self->{REPORT_MESSAGES} = [];
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

    # Add a repository new_repository for the actual workspace (via full path):
    $a->add_repository(/DEV300/new_repository);

=head1 DESCRIPTION

SourceConfig is a perl extension to load and parse General Info Databses.
It uses a simple object oriented interface to retrieve the information stored
in the database.

Methods:

SourceConfig::new()

Creates a new instance of SourceConfig. Can be initialized by: path to the default repository, path to the source_config, default - empty, the source_config will be taken from the environment


SourceConfig::get_version()

Returns version number of the module. Can't fail.


SourceConfig::get_repositories()

Returns sorted list of active repositories for the actual workspace


SourceConfig::add_repository(REPOSITORY_PATH)

Adds a repository to the list of active repositories


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

SourceConfig::add_active_modules($module_array_ref)

Adds modules from the @$module_array_ref as active to the source_config file

SourceConfig::add_active_repositories($repository_array_ref)

Adds repositories from the @$repository_array_ref as active to the source_config file

SourceConfig::remove_activated_modules($module_array_ref)

Removes modules from the @$module_array_ref from the source_config file

SourceConfig::remove_all_activated_modules()

Removes all activated modules from the source_config file

SourceConfig::remove_activated_repositories($repository_array_ref)

Removes repositories from the @$repository_array_ref from the source_config file

SourceConfig::remove_all_activated_repositories()

Removes all activated repositories from the source_config file


=head2 EXPORT

SourceConfig::new()
SourceConfig::get_version()
SourceConfig::get_repositories()
SourceConfig::add_repository()
SourceConfig::get_active_modules()
SourceConfig::get_all_modules()
SourceConfig::get_module_path($module)
SourceConfig::get_module_build_list($module)
SourceConfig::get_module_repository($module)
SourceConfig::get_config_file_path()
SourceConfig::get_config_file_default_path()
SourceConfig::is_active($module)
SourceConfig::add_active_modules($module_array_ref)
SourceConfig::add_active_repositories($repository_array_ref)
SourceConfig::remove_activated_modules($module_array_ref)
SourceConfig::remove_all_activated_modules()
SourceConfig::remove_activated_repositories($repository_array_ref)
SourceConfig::remove_all_activated_repositories()

=head1 AUTHOR

Vladimir Glazunov, vg@openoffice.org

=head1 SEE ALSO

perl(1).

=cut
