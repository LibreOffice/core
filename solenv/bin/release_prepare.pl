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




sub ProcessLanguage ($$$$)
{
    my ($source_version, $language, $package_format, $product_name) = @_;

    $installer::logger::Info->printf("%s\n", $language);
    $installer::logger::Info->increase_indentation();

    # For every language we need
    # 1. have downloadable installation set available (download if missing)
    # 2. unpack it to get access to .cab and .msi
    # 3. unpack .cab so that msimsp.exe can be run

    # Create paths to unpacked contents of .exe and .cab and determine if they exist.
    # The existence of these paths is taken as flag whether the unpacking has already taken place.
    my $unpacked_exe_path = installer::patch::InstallationSet::GetUnpackedMsiPath(
        $source_version,
        $language,
        $package_format,
        $product_name);
    my $unpacked_cab_path = installer::patch::InstallationSet::GetUnpackedCabPath(
        $source_version,
        $language,
        $package_format,
        $product_name);
    my $exe_is_unpacked = -d $unpacked_exe_path;
    my $cab_is_unpacked = -d $unpacked_cab_path;

    if ( ! $exe_is_unpacked)
    {
        # Interpret existence of path as proof that the installation
        # set and the cab file have been successfully unpacked.
        # Nothing to do.
        my $filename = installer::patch::InstallationSet::ProvideDownloadSet(
            $source_version,
            $language,
            $package_format);
        if (defined $filename)
        {
            if ( ! -d $unpacked_exe_path)
            {
                installer::patch::InstallationSet::UnpackExe($filename, $unpacked_exe_path);
            }
        }
        else
        {
            installer::logger::PrintError("could not provide .exe installation set at '%s'\n", $filename);
        }
    }
    else
    {
        $installer::logger::Info->printf("downloadable installation set has already been unpacked to '%s'\n",
            $unpacked_exe_path);
    }

    if ( ! $cab_is_unpacked)
    {
        my $cab_filename = File::Spec->catfile($unpacked_exe_path, "openoffice1.cab");
        if ( ! -f $cab_filename)
        {
             # Cab file does not exist.
            installer::logger::PrintError(
                "could not find .cab file at '%s'.  Extraction of .exe seems to have failed.\n",
                $cab_filename);
        }

        # Unpack the cab file.
        my $msi = new installer::patch::Msi(
            $source_version,
            $language,
            $product_name);

        $installer::logger::Info->printf("unpacking cab file '%s' to '%s'\n",
            $cab_filename, $unpacked_cab_path);
        installer::patch::InstallationSet::UnpackCab(
            $cab_filename,
            $msi,
            $unpacked_cab_path);
    }
    else
    {
        $installer::logger::Info->printf("cab has already been unpacked to\n");
        $installer::logger::Info->printf("    %s\n", $unpacked_cab_path);
    }

    $installer::logger::Info->decrease_indentation();
}




installer::logger::SetupSimpleLogging("c:/tmp/log");

my $arguments = ProcessCommandline();
$arguments->{'package-format'} = 'msi';

print "preparing release build\n";
my ($variables, undef, undef)
    = installer::ziplist::read_openoffice_lst_file(
        $arguments->{'lst-file'},
        $arguments->{'product-name'},
        undef);
if ( ! defined $arguments->{'source-version'})
{
    $arguments->{'source-version'} = $variables->{'PREVIOUS_VERSION'};
}
$installer::logger::Info->printf("    reading data from '%s'\n", $arguments->{'lst-file'});
$installer::logger::Info->printf("    product name is '%s'\n", $arguments->{'product-name'});
$installer::logger::Info->printf("    output path is '%s'\n", $arguments->{'output-path'});
$installer::logger::Info->printf("    source version is '%s'\n", $arguments->{'source-version'});

foreach my $language (@{$arguments->{'languages'}})
{
    ProcessLanguage(
        $arguments->{'source-version'},
        $language,
        $arguments->{'package-format'},
        $arguments->{'product-name'});
}
