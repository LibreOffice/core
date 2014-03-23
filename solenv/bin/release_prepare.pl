#!/usr/bin/perl -w

#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************

use lib ("$ENV{SOLARENV}/bin/modules");
use installer::patch::InstallationSet;
use installer::patch::Msi;
use installer::patch::ReleasesList;
use installer::ziplist;
use installer::logger;

use Getopt::Long;
use Pod::Usage;
use Digest;

#use Carp::Always;

use strict;

=head1 NAME

    release_prepare.pl - Several functions to prepare release builds

=head1 SYNOPSIS

    release_prepare.pl [options] <language1> <language2> ...

    Options:
        --lst-file <filename>
             Path to the .lst file, eg ../util/openoffice.lst
        --product-name <product-name>
             The product name, eg Apache_OpenOffice
        --output-path <path>
             Path to the instsetoo_native platform output tree
        --source-version <major>.<minor>.<micro>
             Override version number of the source.  If not given it is computed from the target version.

=head1 DESCRIPTION

    Prepare a release build:

        - Provide installation sets of the previous version.
          If they are not in ext_sources/ then they are downloaded.

        - Unpack the installation sets.

=cut


sub ProcessCommandline ()
{
    my $arguments = {
        'lst-file' => undef,
        'product-name' => undef,
        'output-path' => undef,
        'source-version' => undef};

    if ( ! GetOptions(
               "lst-file=s", \$arguments->{'lst-file'},
               "product-name=s", \$arguments->{'product-name'},
               "output-path=s", \$arguments->{'output-path'},
               "source-version:s" => \$arguments->{'source-version'}
        ))
    {
        pod2usage(1);
    }

    if ( ! defined $arguments->{'lst-file'})
    {
        print STDERR "lst-file missing, please provide --lst-file\n";
        pod2usage(2);
    }
    if ( ! defined $arguments->{'product-name'})
    {
        print STDERR "product name missing, please provide --product-name\n";
        pod2usage(2);
    }
    if ( ! defined $arguments->{'output-path'})
    {
        print STDERR "output path missing, please provide --output-path\n";
        pod2usage(2);
    }

    $arguments->{'languages'} = \@ARGV;

    return $arguments;
}




sub ProcessLanguage ($$$$$)
{
    my ($version, $is_current_version, $language, $package_format, $product_name) = @_;

    $installer::logger::Info->printf("%s\n", $language);
    $installer::logger::Info->increase_indentation();

    if ( ! defined installer::patch::ReleasesList::Instance()
        ->{$version}
        ->{$package_format}
        ->{$language})
    {
        $installer::logger::Info->printf(
            "there is no recorded information about language '%s' in version '%s'\n",
            $language,
            $version);
        $installer::logger::Info->printf("    skipping\n");
    }
    else
    {
        # For every language we need
        # 1. have downloadable installation set available (download if missing)
        # 2. unpack it to get access to .cab and .msi
        # 3. unpack .cab so that msimsp.exe can be run

        installer::patch::InstallationSet::ProvideUnpackedCab(
            $version,
            $is_current_version,
            $language,
            $package_format,
            $product_name);
    }

    $installer::logger::Info->decrease_indentation();
}




sub main ()
{
    installer::logger::SetupSimpleLogging();

    my $arguments = ProcessCommandline();
    $arguments->{'package-format'} = 'msi';

    $installer::logger::Info->print("preparing release build\n");
    my ($variables, undef, undef)
        = installer::ziplist::read_openoffice_lst_file(
        $arguments->{'lst-file'},
        $arguments->{'product-name'},
        undef);
    if ( ! defined $arguments->{'source-version'})
    {
        $arguments->{'source-version'} = $variables->{'PREVIOUS_VERSION'};
        if ( ! defined $arguments->{'source-version'})
        {
            $arguments->{'source-version'} = installer::patch::ReleasesList::GetPreviousVersion(
                $variables->{'PRODUCTVERSION'});
            if ( ! defined $arguments->{'source-version'})
            {
                $installer::logger::Info->printf("ERROR: can not autodetect previous version\n");
                $installer::logger::Info->printf("       please specify via 'PREVIOUS_VERSION' in %s\n",
                    $arguments->{'lst-file'});
                $installer::logger::Info->printf("       or the --source-version commandline option\n");
                exit(1);
            }
        }
    }
    my $current_version = $variables->{'PRODUCTVERSION'};
    $installer::logger::Info->printf("data from '%s'\n", $arguments->{'lst-file'});
    $installer::logger::Info->printf("name is '%s'\n", $arguments->{'product-name'});
    $installer::logger::Info->printf("path is '%s'\n", $arguments->{'output-path'});
    $installer::logger::Info->printf("source version is '%s'\n", $arguments->{'source-version'});
    $installer::logger::Info->printf("target version is '%s'\n", $current_version);

    foreach my $language (@{$arguments->{'languages'}})
    {
        ProcessLanguage(
            $arguments->{'source-version'},
            $arguments->{'source-version'} eq $current_version,
            $language,
            $arguments->{'package-format'},
            $arguments->{'product-name'});
    }
}


main();
