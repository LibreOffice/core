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
    # Windows only: msvcp70.dll, msvcr70.dll for regcomp.exe
    # Windows only: "msiinfo.exe", "msidb.exe", "uuidgen.exe", "makecab.exe", "msitran.exe", "expand.exe" for msi database and packaging

    my $onefile;
    my $error = 0;
    my $pathvariable = $ENV{'PATH'};
    my $local_pathseparator = $installer::globals::pathseparator;

    if( $^O =~ /cygwin/i )
    {   # When using cygwin's perl the PATH variable is POSIX style and ...
        $pathvariable = qx{cygpath -mp "$pathvariable"} ;
        # has to be converted to DOS style for further use.
        $local_pathseparator = ';';
    }
    my $patharrayref = installer::converter::convert_stringlist_into_array(\$pathvariable, $local_pathseparator);

    $installer::globals::patharray = $patharrayref;

    my @needed_files_in_path = ();

    if (($installer::globals::iswin) && ($installer::globals::iswindowsbuild))
    {
        @needed_files_in_path = ("zip.exe", "msiinfo.exe", "msidb.exe", "uuidgen.exe", "makecab.exe", "msitran.exe", "expand.exe");

        if ( $installer::globals::compiler eq "wntmsci8" )
        {
            push(@needed_files_in_path, "msvcp70.dll");
            push(@needed_files_in_path, "msvcr70.dll");
        }

        if ( $installer::globals::compiler eq "wntmsci10" )
        {
            push(@needed_files_in_path, "msvcp71.dll");
            push(@needed_files_in_path, "msvcr71.dll");
        }

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

    # checking, if upx can be found in path

    if ( $installer::globals::iswindowsbuild ) { $installer::globals::upxfile = "upx.exe"; }
    else { $installer::globals::upxfile = "upx"; }

    my $upxfilename = $installer::globals::upxfile;
    my $upxfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$upxfilename, $patharrayref , 0);

    if (!( $$upxfileref eq "" ))
    {
        $installer::globals::upx_in_path = 1;
        $installer::globals::upxfile = $$upxfileref;
        installer::logger::print_message( "\tFound: $$upxfileref\n" );
    }

}

######################################################################
# Determining the version of file makecab.exe
######################################################################

sub get_makecab_version
{
    my $makecabversion = -1;

    my $systemcall = "makecab.exe |";
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
            # warning for OOo, error for inhouse products
            if ( $installer::globals::isopensourceproduct )
            {
                installer::logger::print_warning("Old version of makecab.exe. Found version: \"$makecabversion\", tested version: \"$installer::globals::controlledmakecabversion\"!\n");
            }
            else
            {
                installer::exiter::exit_program("makecab.exe too old. Found version: \"$makecabversion\", required version: \"$installer::globals::controlledmakecabversion\"!", "check_makecab_version");
            }
        }
    }
    else
    {
        $infoline = "Warning: No version check of makecab.exe\n";
        push( @installer::globals::globallogfileinfo, $infoline);
    }
}

######################################################################
# Reading the environment variables for the pathes in ziplist.
# solarpath, solarenvpath, solarcommonpath, os, osdef, pmiscpath
######################################################################

sub check_system_environment
{
    my %variables = ();
    my $key;
    my $error = 0;

    foreach $key ( @installer::globals::environmentvariables )
    {
        my $value = "";
        if ( $ENV{$key} ) { $value = $ENV{$key}; }
        $variables{$key} = $value;

        if ( $value eq "" )
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

    if (( ! $installer::globals::pro ) && ( $installer::globals::ignore_error_in_logfile )) { $ignore_error = 1; }

    for ( my $i = 0; $i <= $#{$logfile}; $i++ )
    {
        my $line = ${$logfile}[$i];

        # Errors are all errors, but not the Windows installer table "Error.idt"

        my $compareline = $line;
        $compareline =~ s/Error\.idt//g;    # removing all occurences of "Error.idt"
        $compareline =~ s/Error\.mlf//g;    # removing all occurences of "Error.mlf"
        $compareline =~ s/Error\.ulf//g;    # removing all occurences of "Error.ulf"
        $compareline =~ s/Error\.idl//g;    # removing all occurences of "Error.idl"
        $compareline =~ s/Error\.html//g;   # removing all occurences of "Error.html"

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
        $line = "ERROR: The following errors occured in packaging process:\n\n";
        push(@output, $line);

        for ( my $i = 0; $i <= $#errors; $i++ )
        {
            $line = "$errors[$i]";
            push(@output, $line);
        }

        $line = "*********************************************************************\n";
        push(@output, $line);
#       exit(-1);
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
# Determining the ship installation directory
#############################################################

sub determine_ship_directory
{
    my ($languagesref) = @_;

    if (!( $ENV{'SHIPDRIVE'} )) { installer::exiter::exit_program("ERROR: SHIPDRIVE must be set for updater!", "determine_ship_directory"); }

    my $shipdrive = $ENV{'SHIPDRIVE'};

    my $languagestring = $$languagesref;

    if (length($languagestring) > $installer::globals::max_lang_length )
    {
        my $number_of_languages = installer::systemactions::get_number_of_langs($languagestring);
        chomp(my $shorter = `echo $languagestring | md5sum | sed -e "s/ .*//g"`);
        # $languagestring = $shorter;
        my $id = substr($shorter, 0, 8); # taking only the first 8 digits
        $languagestring = "lang_" . $number_of_languages . "_id_" . $id;
    }

    my $productstring = $installer::globals::product;
    my $productsubdir = "";

    if ( $productstring =~ /^\s*(.+?)\_\_(.+?)\s*$/ )
    {
        $productstring = $1;
        $productsubdir = $2;
    }

    if ( $installer::globals::languagepack ) { $productstring = $productstring . "_languagepack"; }
    if ( $installer::globals::patch ) { $productstring = $productstring . "_patch"; }

    my $destdir = $shipdrive . $installer::globals::separator . $installer::globals::compiler .
                $installer::globals::productextension . $installer::globals::separator .
                $productstring . $installer::globals::separator;

    if ( $productsubdir ) { $destdir = $destdir . $productsubdir . $installer::globals::separator; }

    $destdir = $destdir . $installer::globals::installertypedir . $installer::globals::separator .
                $installer::globals::build . "_" . $installer::globals::lastminor . "_" .
                "native_inprogress-number_" . $languagestring . "\." . $installer::globals::buildid;

    my $infoline = "\nSetting ship directory: $destdir\n";
    push(@installer::globals::globallogfileinfo, $infoline);

    return $destdir;
}

#############################################################
# Controlling if this is an official RE pack process
#############################################################

sub check_updatepack
{
    my $shipdrive = "";
    my $filename = "";
    my $infoline = "";

    if ( $ENV{'UPDATER'} )  # the environment variable UPDATER has to be set
    {
        $infoline = "\nEnvironment variable UPDATER set\n";
        push(@installer::globals::globallogfileinfo, $infoline);

        if ( ! $ENV{'CWS_WORK_STAMP'} ) # the environment variable CWS_WORK_STAMP must not be set (set only in CWS)
        {
            $infoline = "Environment variable CWS_WORK_STAMP not set\n";
            push(@installer::globals::globallogfileinfo, $infoline);

            if ( $ENV{'SHIPDRIVE'} )    # the environment variable SHIPDRIVE must be set
            {
                $shipdrive = $ENV{'SHIPDRIVE'};
                $infoline = "Ship drive defined: $shipdrive\n";
                push(@installer::globals::globallogfileinfo, $infoline);

                if ( -d $shipdrive )    # SHIPDRIVE must be a directory
                {
                    $infoline = "Ship drive exists\n";
                    push(@installer::globals::globallogfileinfo, $infoline);

                    # try to write into $shipdrive

                    $directory = $installer::globals::product . "_" . $installer::globals::compiler . "_" . $installer::globals::buildid . "_" . $installer::globals::languageproducts[0] . "_test_$$";
                    $directory =~ s/\,/\_/g;    # for the list of languages
                    $directory =~ s/\-/\_/g;    # for en-US, pt-BR, ...
                    $directory = $shipdrive . $installer::globals::separator . $directory;

                    $infoline = "Try to create directory: $directory\n";
                    push(@installer::globals::globallogfileinfo, $infoline);

                    # saving this directory for later removal
                    $installer::globals::shiptestdirectory = $directory;

                    if ( installer::systemactions::try_to_create_directory($directory))
                    {
                        $infoline = "Write access on Ship drive\n";
                        push(@installer::globals::globallogfileinfo, $infoline);
                        $infoline = "Ship test directory $installer::globals::shiptestdirectory was successfully created\n";
                        push(@installer::globals::globallogfileinfo, $infoline);
                        my $systemcall = "rmdir $directory";
                        my $returnvalue = system($systemcall);

                        # 5th condition: No local build environment.
                        # In this case the content of SOLARENV starts with the content of SOL_TMP

                        my $solarenv = "";
                        my $sol_tmp;
                        if ( $ENV{'SOLARENV'} ) { $solarenv = $ENV{'SOLARENV'}; }

                        $infoline = "Environment variable SOLARENV: $solarenv\n";
                        push(@installer::globals::globallogfileinfo, $infoline);

                        if ( $ENV{'SOL_TMP'} )
                        {
                            $sol_tmp = $ENV{'SOL_TMP'};
                            $infoline = "Environment variable SOL_TMP: $sol_tmp\n";
                        } else {
                            $infoline = "Environment variable SOL_TMP not set\n";
                        }
                        push(@installer::globals::globallogfileinfo, $infoline);

                        if ( defined $sol_tmp && ( $solarenv =~ /^\s*\Q$sol_tmp\E/ ))
                        {
                            $infoline = "Content of SOLARENV starts with the content of SOL_TMP\: Local environment -\> No Updatepack\n";
                            push(@installer::globals::globallogfileinfo, $infoline);
                        }
                        else
                        {
                            $infoline = "Content of SOLARENV does not start with the content of SOL_TMP: No local environment\n";
                            push(@installer::globals::globallogfileinfo, $infoline);

                            $installer::globals::updatepack = 1;    # That's it
                        }

                        # Additional logging information for the temporary ship directory

                        if ( -d $installer::globals::shiptestdirectory )
                        {
                            $infoline = "Ship test directory $installer::globals::shiptestdirectory still exists. Trying removal later again.\n";
                            push(@installer::globals::globallogfileinfo, $infoline);
                        }
                        else
                        {
                            $infoline = "Ship test directory $installer::globals::shiptestdirectory was successfully removed.\n";
                            push(@installer::globals::globallogfileinfo, $infoline);
                        }
                    }
                    else
                    {
                        $infoline = "No write access on Ship drive\n";
                        push(@installer::globals::globallogfileinfo, $infoline);
                        $infoline = "Failed to create directory $directory\n";
                        push(@installer::globals::globallogfileinfo, $infoline);
                        if ( defined $ENV{'BSCLIENT'} && ( uc $ENV{'BSCLIENT'} eq 'TRUE' ) ) {
                            installer::exiter::exit_program("ERROR: No write access to SHIPDRIVE allthough BSCLIENT is set.", "check_updatepack");
                        }
                    }
                }
                else
                {
                    $infoline = "Ship drive not found: No updatepack\n";
                    push(@installer::globals::globallogfileinfo, $infoline);
                }
            }
            else
            {
                $infoline = "Environment variable SHIPDRIVE not set: No updatepack\n";
                push(@installer::globals::globallogfileinfo, $infoline);
            }
        }
        else
        {
            $infoline = "Environment variable CWS_WORK_STAMP defined: No updatepack\n";
            push(@installer::globals::globallogfileinfo, $infoline);
        }
    }

    if ( $installer::globals::updatepack ) { $infoline = "Setting updatepack true\n\n"; }
    else { $infoline = "\nNo updatepack\n"; }
    push(@installer::globals::globallogfileinfo, $infoline);

}

#############################################################
# Reading the Windows list file for language encodings
#############################################################

sub read_encodinglist
{
    my ($patharrayref) = @_;

    my $fileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$installer::globals::encodinglistname, $patharrayref , 0);

    if ( $$fileref eq "" ) { installer::exiter::exit_program("ERROR: Did not find Windows encoding list $installer::globals::encodinglistname!", "read_encodinglist"); }

    my $infoline = "Found encoding file: $$fileref\n";
    push(@installer::globals::globallogfileinfo, $infoline);

    my $encodinglist = installer::files::read_file($$fileref);

    my %msiencoding = ();
    my %msilanguage = ();

    # Controlling the encoding list

    for ( my $i = 0; $i <= $#{$encodinglist}; $i++ )
    {
        my $line = ${$encodinglist}[$i];

        if ( $line =~ /^\s*\#/ ) { next; }  # this is a comment line

        if ( $line =~ /^(.*?)(\#.*)$/ ) { $line = $1; } # removing comments after "#"

        if ( $line =~ /^\s*([\w-]+)\s*(\d+)\s*(\d+)\s*$/ )
        {
            my $onelanguage = $1;
            my $codepage = $2;
            my $windowslanguage = $3;

            $msiencoding{$onelanguage} = $codepage;
            $msilanguage{$onelanguage} = $windowslanguage;
        }
        else
        {
            installer::exiter::exit_program("ERROR: Wrong syntax in Windows encoding list $installer::globals::encodinglistname : en-US 1252 1033 !", "read_encodinglist");
        }
    }

    $installer::globals::msiencoding = \%msiencoding;
    $installer::globals::msilanguage = \%msilanguage;

    # my $key;
    # foreach $key (keys %{$installer::globals::msiencoding}) { print "A Key: $key : Value: $installer::globals::msiencoding->{$key}\n"; }
    # foreach $key (keys %{$installer::globals::msilanguage}) { print "B Key: $key : Value: $installer::globals::msilanguage->{$key}\n"; }

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

#############################################################
# Check if Java is available to create xpd installer
#############################################################

sub check_java_for_xpd
{
    my ( $allvariables ) = @_;

    if ( ! $installer::globals::solarjavaset ) { $allvariables->{'XPDINSTALLER'} = 0; }
}

####################################################################
# Setting global variable "$installer::globals::addchildprojects"
####################################################################

sub set_addchildprojects
{
    my ($allvariables) = @_;

    if (( $allvariables->{'JAVAPRODUCT'} ) ||
        ( $allvariables->{'ADAPRODUCT'} ) ||
        ( $allvariables->{'UREPRODUCT'} ) ||
        ( $allvariables->{'ADDREQUIREDPACKAGES'} )) { $installer::globals::addchildprojects = 1; }

    if ( $installer::globals::patch )
    {
        $installer::globals::addchildprojects = 0;  # no child projects for patches
    }

    my $infoline = "Value of \$installer::globals::addchildprojects: $installer::globals::addchildprojects\n";
    push( @installer::globals::globallogfileinfo, $infoline);
}

####################################################################
# Setting global variable "$installer::globals::addjavainstaller"
####################################################################

sub set_addjavainstaller
{
    my ($allvariables) = @_;

    if ( $allvariables->{'JAVAINSTALLER'} ) { $installer::globals::addjavainstaller = 1; }

    if ( $installer::globals::patch ) { $installer::globals::addjavainstaller = 0; }
    if ( $installer::globals::languagepack ) { $installer::globals::addjavainstaller = 0; }
    if ( $allvariableshashref->{'XPDINSTALLER'} ) { $installer::globals::addjavainstaller = 0; }

    my $infoline = "Value of \$installer::globals::addjavainstaller: $installer::globals::addjavainstaller\n";
    push( @installer::globals::globallogfileinfo, $infoline);
}

#######################################################################
# Setting global variable "$installer::globals::addsystemintegration"
#######################################################################

sub set_addsystemintegration
{
    my ($allvariables) = @_;

    if ( $allvariables->{'ADDSYSTEMINTEGRATION'} ) { $installer::globals::addsystemintegration = 1; }

    if ( $installer::globals::patch ) { $installer::globals::addsystemintegration = 0; }
    if ( $installer::globals::languagepack ) { $installer::globals::addsystemintegration = 0; }
    if (( $installer::globals::packageformat eq "native" ) || ( $installer::globals::packageformat eq "portable" )) { $installer::globals::addsystemintegration = 0; }

    my $infoline = "Value of \$installer::globals::addsystemintegration: $installer::globals::addsystemintegration\n";
    push( @installer::globals::globallogfileinfo, $infoline);
}

1;
