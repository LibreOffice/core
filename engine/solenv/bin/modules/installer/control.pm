#
# This file is part of the Collabora Office project.
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

package installer::control;

use strict;
use warnings;

use Cwd;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::systemactions;

#########################################################
# Function that can be used for additional controls.
# Search happens in $installer::globals::patharray.
#########################################################

sub check_needed_files_in_path
{
    my ( $filesref ) = @_;
    my $error = 0;

    foreach my $onefile ( @{$filesref} )
    {
        installer::logger::print_message( "... searching $onefile ..." );

        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$onefile, $installer::globals::patharray , 0);

        if ( $$fileref eq "" )
        {
            $error = 1;
            installer::logger::print_error( "$onefile not found\n" );
        }
        else
        {
            installer::logger::print_message( "\tFound: $$fileref\n" );
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Could not find all needed files in path!", "check_needed_files_in_path");
    }
}

#########################################################
# Checking the local system
# Checking existence of needed files in include path
#########################################################

sub check_system_path
{
    # The following files have to be found in the environment variable PATH:
    # zip

    my $onefile;
    my $error = 0;
    my $pathvariable = $ENV{'PATH'};
    my $local_pathseparator = $installer::globals::pathseparator;

    if( $^O =~ /cygwin/i )
    {
        # When using cygwin's perl the PATH variable is POSIX style and
        # has to be converted to DOS style for further use.
        $pathvariable = join ';',
                        map { my $dir = qx{cygpath -m "$_"}; chomp($dir); $dir }
                        split /\Q$local_pathseparator\E\s*/, $pathvariable;
        $local_pathseparator = ';';
    } elsif ( $^O =~ /MSWin/i ) {
        $local_pathseparator = ';';
    }
    my $patharrayref = installer::converter::convert_stringlist_into_array(\$pathvariable, $local_pathseparator);

    $installer::globals::patharray = $patharrayref;

    my @needed_files_in_path = ();

    if ($installer::globals::iswin)
    {
        @needed_files_in_path = ("zip.exe");
    }
    else
    {
        @needed_files_in_path = ("zip");
    }

    foreach $onefile ( @needed_files_in_path )
    {
        installer::logger::print_message( "... searching $onefile ..." );

        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$onefile, $patharrayref , 0);

        if ( $$fileref eq "" )
        {
            $error = 1;
            installer::logger::print_error( "$onefile not found\n" );
        }
        else
        {
            installer::logger::print_message( "\tFound: $$fileref\n" );
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Could not find all needed files in path!", "check_system_path");
    }

    # checking for epm, which has to be in the path or in the solver

    $onefile = "epm";
    my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onefile, $patharrayref , 0);
    if (!( $$fileref eq "" ))
    {
        $installer::globals::epm_in_path = 1;

        if ( $$fileref =~ /^\s*\.\/epm\s*$/ )
        {
            my $currentdir = cwd();
            $$fileref =~ s/\./$currentdir/;
        }

        $installer::globals::epm_path = $$fileref;
    }
}

######################################################################
# Reading the environment variables for the paths in ziplist.
# solarenvpath, os, pmiscpath
######################################################################

sub check_system_environment
{
    my %variables = ();
    my $error = 0;

    my @environmentvariables = qw(
        LIBO_VERSION_MAJOR
        LIBO_VERSION_MINOR
        CPUNAME
        OS
        COM
        PLATFORMID
        LOCAL_OUT
        LOCAL_COMMON_OUT
        WORKDIR
        SRCDIR
    );

    for my $key ( @environmentvariables )
    {
        $variables{$key} = defined($ENV{$key}) ? $ENV{$key} : "";

        if ( $variables{$key} eq "" )
        {
            installer::logger::print_error( "$key not set in environment\n" );
            $error = 1;
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Environment variable not set!", "check_system_environment");
    }

    return \%variables;
}

#############################################################
# Controlling the log file at the end of the
# packaging process
#############################################################

sub check_logfile
{
    my ($logfile) = @_;

    my @errors = ();
    my @output = ();
    my $contains_error = 0;

    my $ignore_error = 0;
    my $make_error_to_warning = 0;

    for ( my $i = 0; $i <= $#{$logfile}; $i++ )
    {
        my $line = ${$logfile}[$i];

        # Errors are all errors, but not the Windows installer table "Error.idt"

        my $compareline = $line;
        $compareline =~ s/Error\.idt//g;    # removing all occurrences of "Error.idt"
        $compareline =~ s/Error\.ulf//g;    # removing all occurrences of "Error.ulf"
        $compareline =~ s/Error\.idl//g;    # removing all occurrences of "Error.idl"
        $compareline =~ s/Error\.html//g;   # removing all occurrences of "Error.html"
        $compareline =~ s/error\.py//g;     # removing all occurrences of "error.py"
        $compareline =~ s/error\.cpython\-3\d{1,2}(\.opt\-.|)\.py[co]//g;  # removing all occurrences of "error-cpython"
        $compareline =~ s/libgpg-error//g;
        $compareline =~ s/Error-xref\.html//g;

        if ( $compareline =~ /\bError\b/i )
        {
            $contains_error = 1;
            push(@errors, $line);

            if ( $ignore_error )
            {
                $contains_error = 0;
                $make_error_to_warning = 1;
            }
        }
    }

    if ($contains_error)
    {
        my $line = "\n*********************************************************************\n";
        push(@output, $line);
        $line = "ERROR: The following errors occurred in packaging process:\n\n";
        push(@output, $line);

        for ( my $i = 0; $i <= $#errors; $i++ )
        {
            $line = "$errors[$i]";
            push(@output, $line);
        }

        $line = "*********************************************************************\n";
        push(@output, $line);
    }
    else
    {
        my $line = "";

        if ( $make_error_to_warning )
        {
            $line = "\n*********************************************************************\n";
            push(@output, $line);
            $line = "The following errors in the log file were ignored:\n\n";
            push(@output, $line);

            for ( my $i = 0; $i <= $#errors; $i++ )
            {
                $line = "$errors[$i]";
                push(@output, $line);
            }

            $line = "*********************************************************************\n";
            push(@output, $line);
        }

        $line = "\n***********************************************************\n";
        push(@output, $line);
        $line = "Successful packaging process!\n";
        push(@output, $line);
        $line = "***********************************************************\n";
        push(@output, $line);
    }

    # printing the output file and adding it to the logfile

    installer::logger::include_header_into_logfile("Summary:");

    my $force = 1; # print this message even in 'quiet' mode
    for ( my $i = 0; $i <= $#output; $i++ )
    {
        my $line = "$output[$i]";
        installer::logger::print_message( "$line", $force );
        push( @installer::globals::logfileinfo, $line);
        push( @installer::globals::errorlogfileinfo, $line);
    }

    return $contains_error;
}

#############################################################
# Only for Windows and Linux (RPM)there is currently
# a reliable mechanism to register extensions during
# installation process. Therefore it is for all other
# platforms forbidden to install oxt files into that
# directory, in which they are searched for registration.
#############################################################

sub check_oxtfiles
{
    my ( $filesarray ) = @_;

    for ( my $i = 0; $i <= $#{$filesarray}; $i++ )
    {
        my $onefile = ${$filesarray}[$i];

        if (( $onefile->{'Name'} ) && ( $onefile->{'Dir'} ))
        {
            if (( $onefile->{'Name'} =~ /\.oxt\s*$/ ) && ( $onefile->{'Dir'} eq $installer::globals::extensioninstalldir ))
            {
                installer::exiter::exit_program("There is currently only for Linux (RPM) and Windows a reliable mechanism to register extensions during installation.\nPlease remove file \"$onefile->{'gid'}\" from your installation set!\nYou can use \"\#ifdef _WIN32\" and \"\#ifdef LINUX\" in scp.", "check_oxtfiles");
            }
        }
    }
}

#######################################################################
# Setting global variable "$installer::globals::addsystemintegration"
#######################################################################

sub set_addsystemintegration
{
    my ($allvariables) = @_;

    if ( $allvariables->{'ADDSYSTEMINTEGRATION'} ) { $installer::globals::addsystemintegration = 1; }

    if ( $installer::globals::languagepack ) { $installer::globals::addsystemintegration = 0; }
    if ( $installer::globals::helppack ) { $installer::globals::addsystemintegration = 0; }

    my $infoline = "Value of \$installer::globals::addsystemintegration: $installer::globals::addsystemintegration\n";
    push( @installer::globals::globallogfileinfo, $infoline);
}

1;
