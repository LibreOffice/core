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

package installer::control;

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

    foreach $onefile ( @{$filesref} )
    {
        installer::logger::print_message( "...... searching $onefile ..." );

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
    # The following files have to be found in the environment variable PATH
    # All platforms: zip
    # Windows only: "msiinfo.exe", "msidb.exe", "uuidgen.exe", "makecab.exe", "msitran.exe", "expand.exe" for msi database and packaging

    if ($ENV{'CROSS_COMPILING'} eq 'TRUE')
    {
        # we build our own msi* etc. tools when cross-compiling
        $ENV{'PATH'} .= $installer::globals::pathseparator . $ENV{'WORKDIR_FOR_BUILD'} . '/LinkTarget/Executable';
    }

    my $onefile;
    my $error = 0;
    my $pathvariable = $ENV{'PATH'};
    my $local_pathseparator = $installer::globals::pathseparator;

    if( $^O =~ /cygwin/i )
    {
        # When using cygwin's perl the PATH variable is POSIX style and
        # has to be converted to DOS style for further use.
        $pathvariable = join ';',
                        map { $dir = qx{cygpath -m "$_"}; chomp($dir); $dir }
                        split /\Q$local_pathseparator\E\s*/, $pathvariable;
        $local_pathseparator = ';';
    }
    my $patharrayref = installer::converter::convert_stringlist_into_array(\$pathvariable, $local_pathseparator);

    $installer::globals::patharray = $patharrayref;

    my @needed_files_in_path = ();

    if (($installer::globals::iswin) && ($installer::globals::iswindowsbuild))
    {
        @needed_files_in_path = ("zip.exe", "msiinfo.exe", "msidb.exe", "uuidgen", "makecab.exe", "msitran.exe", "expand.exe");
    }
    elsif ($installer::globals::isunix && $installer::globals::packageformat eq 'msi')
    {
        @needed_files_in_path = ("zip", "msiinfo.exe", "msidb.exe", "uuidgen", "makecab.exe", "msitran.exe", "cabextract");
    }
    elsif ($installer::globals::iswin)
    {
        @needed_files_in_path = ("zip.exe");
    }
    else
    {
        @needed_files_in_path = ("zip");
    }

    foreach $onefile ( @needed_files_in_path )
    {
        installer::logger::print_message( "...... searching $onefile ..." );

        my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$onefile, $patharrayref , 0);

        if ( $$fileref eq "" )
        {
            $error = 1;
            installer::logger::print_error( "$onefile not found\n" );
        }
        else
        {
            installer::logger::print_message( "\tFound: $$fileref\n" );
            # Saving the absolut path for msitran.exe. This is required for the determination of the checksum.
            if ( $onefile eq "msitran.exe" ) { $installer::globals::msitranpath = $$fileref; }
        }
    }

    if ( $error )
    {
        installer::exiter::exit_program("ERROR: Could not find all needed files in path!", "check_system_path");
    }

    # checking for epm, which has to be in the path or in the solver

    if (( $installer::globals::call_epm ) && (!($installer::globals::iswindowsbuild)))
    {
        my $onefile = "epm";
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
}

######################################################################
# Determining the version of file makecab.exe
######################################################################

sub get_makecab_version
{
    my $makecabversion = -1;

    my $systemcall = "makecab.exe |";
    if ( $installer::globals::isunix )
    {
        $systemcall = "$ENV{'WORKDIR_FOR_BUILD'}/LinkTarget/Executable/makecab.exe |";
    }
    my @makecaboutput = ();

    open (CAB, $systemcall);
    while (<CAB>) { push(@makecaboutput, $_); }
    close (CAB);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::globallogfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::globallogfileinfo, $infoline);

        my $versionline = "";

        for ( my $i = 0; $i <= $#makecaboutput; $i++ )
        {
            if ( $makecaboutput[$i] =~ /\bVersion\b/i )
            {
                $versionline = $makecaboutput[$i];
                last;
            }
        }

        $infoline = $versionline;
        push( @installer::globals::globallogfileinfo, $infoline);

        if ( $versionline =~ /\bVersion\b\s+(\d+[\d\.]+\d+)\s+/ )
        {
            $makecabversion = $1;
        }

        # Only using the first number

        if ( $makecabversion =~ /^\s*(\d+?)\D*/ )
        {
            $makecabversion = $1;
        }

        $infoline = "Using version: " . $makecabversion . "\n";
        push( @installer::globals::globallogfileinfo, $infoline);
    }

    return $makecabversion;
}

######################################################################
# Checking the version of file makecab.exe
######################################################################

sub check_makecab_version
{
    # checking version of makecab.exe
    # Now it is guaranteed, that makecab.exe is in the path

    my $do_check = 1;

    my $makecabversion = get_makecab_version();

    my $infoline = "Tested version: " . $installer::globals::controlledmakecabversion . "\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    if ( $makecabversion < 0 ) { $do_check = 0; } # version could not be determined

    if ( $do_check )
    {
        if ( $makecabversion < $installer::globals::controlledmakecabversion )
        {
            installer::exiter::exit_program("makecab.exe too old. Found version: \"$makecabversion\", required version: \"$installer::globals::controlledmakecabversion\"!", "check_makecab_version");
        }
    }
    else
    {
        $infoline = "Warning: No version check of makecab.exe\n";
        push( @installer::globals::globallogfileinfo, $infoline);
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
        $compareline =~ s/error\.cpython\-3.(\.opt\-.|)\.py[co]//g;  # removing all occurrences of "error-cpython"
        $compareline =~ s/libgpg-error-0.dll//g;
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
# Reading the Windows list file for Windows language codes
# Encoding field is no longer used. We use UTF-8 everywhere.
#############################################################

sub read_lcidlist
{
    my ($patharrayref) = @_;

    if ( ! -f $installer::globals::lcidlistname ) { installer::exiter::exit_program("ERROR: Did not find Windows LCID list $installer::globals::lcidlistname!", "read_lcidlist"); }

    my $infoline = "Found LCID file: $installer::globals::lcidlistname\n";
    push(@installer::globals::globallogfileinfo, $infoline);

    my $lcidlist = installer::files::read_file($installer::globals::lcidlistname);
    my %msilanguage = ();

    for ( my $i = 0; $i <= $#{$lcidlist}; $i++ )
    {
        my $line = ${$lcidlist}[$i];
        # de-mangle various potential DOS line-ending problems
        $line =~ s/\r//g;
        $line =~ s/\n//g;
        $line =~ s/\s*\#.*$//; # removing comments after "#"
        if ( $line =~ /^\s*$/ ) { next; }  # this is an empty line

        if ( $line =~ /^\s*([\w-]+)\s+(\d+)\s+(\d+)\s*$/ )
        {
            my $onelanguage = $1;
            my $windowslanguage = $3;
            $msilanguage{$onelanguage} = $windowslanguage;
        }
        else
        {
            installer::exiter::exit_program("ERROR: Wrong syntax in Windows LCID list $installer::globals::lcidlistname in line $i: '$line'", "read_lcidlist");
        }
    }
    $installer::globals::msilanguage = \%msilanguage;
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
                installer::exiter::exit_program("There is currently only for Linux (RPM) and Windows a reliable mechanism to register extensions during installation.\nPlease remove file \"$onefile->{'gid'}\" from your installation set!\nYou can use \"\#ifdef WNT\" and \"\#ifdef LINUX\" in scp.", "check_oxtfiles");
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
