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

package installer::downloadsigner;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;

############################################
# Parameter Operations
############################################

sub usage
{
    print <<Ende;
--------------------------------------------------------------------------------
make_download V1.0
The following parameter are needed:
-d: Full path to the file containing the follow-me info or to a directory
    containing the follow-me info files. In the latter case, all follow-me
    info files are evaluated. If a directory is used, the successfully used
    follow-me info files are renamed using a string "success". Files with
    this string are ignored in repeated processes using "-d" with a
    directory.

The following parameter are optional:
-nodownload: Only signing, no creation of download sets (Windows only)
-useminor: Important for installation sets, created without minor set
-writetotemp: Necessary, if you do not want to write into solver
              This can be caused by missing privileges (Windows only)
-internalcabinet: Not only the cabinet files are signed, but also all
                  files included in the cabinet files (Windows only).

-sign: Uses signing mechanism to sign installation sets
If \"-sign\" is set, the following two parameter are required:
-pfx: Full path to the pfx file
-pw: Full path to the file, containing the pfx password.

Examples:

Specifying an installation set (with "-d"):

perl make_download.pl -d <followmeinfofilename>

perl make_download.pl -d <followmeinfofilename>
                         -sign
                         -pfx <pfxfilename>
                         -pw <passwordfilename>

or without specifying an installation set:

perl make_download.pl -d <followmedirectory>
                      -sign
                      -pfx <pfxfilename>
                      -pw <passwordfilename>
--------------------------------------------------------------------------------
Ende
    exit(-1);
}

#####################################
# Reading parameter
#####################################

sub getparameter
{
    while ( $#ARGV >= 0 )
    {
        my $param = shift(@ARGV);

        if ($param eq "-d") { $installer::globals::followmeinfofilename = shift(@ARGV); }
        elsif ($param eq "-pw") { $installer::globals::pwfile = shift(@ARGV); }
        elsif ($param eq "-pfx") { $installer::globals::pfxfile = shift(@ARGV); }
        elsif ($param eq "-sign") { $installer::globals::dosign = 1; }
        elsif ($param eq "-nodownload") { $installer::globals::nodownload = 1; }
        elsif ($param eq "-writetotemp") { $installer::globals::writetotemp = 1; }
        elsif ($param eq "-useminor") { $installer::globals::useminor = 1; }
        elsif ($param eq "-internalcabinet") { $installer::globals::internal_cabinet_signing = 1; }
        else
        {
            installer::logger::print_error( "unknown parameter: $param" );
            usage();
            exit(-1);
        }
    }
}

#####################################
# Controlling required parameter
#####################################

sub checkparameter
{
    if ( $installer::globals::followmeinfofilename eq "" )
    {
        installer::logger::print_error( "Error: Required parameter is missing: -d\n" );
        usage();
        exit(-1);
    }

    if ( $installer::globals::dosign )
    {
        # -pfx and -pw have to be set
        if ( $installer::globals::pfxfile eq "" )
        {
            installer::logger::print_error( "Error: If \"-sign\" is set, a pfx file has to be specified: -pfx\n" );
            usage();
            exit(-1);
        }

        # -pfx and -pw have to be set
        if ( $installer::globals::pwfile eq "" )
        {
            installer::logger::print_error( "Error: If \"-sign\" is set, a password file has to be specified: -pw\n" );
            usage();
            exit(-1);
        }

        # and both files have to exist
        if ( ! -f $installer::globals::pfxfile )
        {
            installer::logger::print_error( "Error: pfx file \"$installer::globals::pfxfile\" does not exist.\n" );
            usage();
            exit(-1);
        }

        if ( ! -f $installer::globals::pwfile )
        {
            installer::logger::print_error( "Error: Password file \"$installer::globals::pwfile\" does not exist (-pw).\n" );
            usage();
            exit(-1);
        }
    }
}

#############################################
# Setting the temporary path for the download
# and signing process
#############################################

sub set_temp_path
{
    my $temppath = "";
    my $pid = $$;           # process id
    my $time = time();      # time
    my $helperdir = "unpackdir_" . $pid . $time;

    if (( $ENV{'TMP'} ) || ( $ENV{'TEMP'} ))
    {
        if ( $ENV{'TMP'} ) { $temppath = $ENV{'TMP'}; }
        elsif ( $ENV{'TEMP'} )  { $temppath = $ENV{'TEMP'}; }
        $temppath =~ s/\Q$installer::globals::separator\E\s*$//;    # removing ending slashes and backslashes
        $temppath = $temppath . $installer::globals::separator . $helperdir;

        if( $^O =~ /cygwin/i )
        {
            $temppath = qx{cygpath -w "$temppath"};
            $temppath =~ s/\\/\//g;
            $temppath =~ s/\s*$//g;
        }

        installer::systemactions::create_directory_structure($temppath);
    }
    else
    {
        installer::logger::print_error( "Error: TMP and TEMP not defined. This is required for this process.\n" );
        usage();
        exit(-1);
    }

    installer::logger::print_message( "\n... using output path: $temppath ...\n" );

    push(@installer::globals::removedirs, $temppath);

    return $temppath;
}

#############################################
# Setting output pathes to temp directory
# This are the:
# unpackpath and the loggingdir
#############################################

sub set_output_pathes_to_temp
{
    my ($followmeinfohash, $temppath) = @_;

    $followmeinfohash->{'loggingdir'} = $temppath . $installer::globals::separator;
    $installer::globals::unpackpath = $temppath;
}

#############################################
# Setting the minor into the pathes. This is
# required, if the original installation set
# was created without minor
# Value is always saved in
# $installer::globals::lastminor
# which is saved in the follow_me file
#############################################

sub set_minor_into_pathes
{
    my ($followmeinfohash, $temppath) = @_;

    installer::logger::print_message( "\n... forcing into minor: $installer::globals::lastminor ...\n" );

    my @pathnames = ("bin", "doc", "inc", "lib", "pck", "res", "xml");
    my $sourcename = "src";
    my $srcpath = $installer::globals::separator . $sourcename . $installer::globals::separator;

    if ( $installer::globals::minor ne "" )
    {
        installer::logger::print_message( "\n... already defined minor: $installer::globals::minor -> ignoring parameter \"-useminor\" ...\n" );
        return;
    }

    # Affected pathes:
    # $contenthash{'installlogdir'}
    # $contenthash{'includepatharray'}
    # $installer::globals::unpackpath
    # $installer::globals::idttemplatepath
    # $installer::globals::idtlanguagepath

    installer::logger::include_header_into_logfile("Changing saved pathes to add the minor");
    my $infoline = "Old pathes:\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$followmeinfohash->{'installlogdir'}: $followmeinfohash->{'installlogdir'}\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$installer::globals::unpackpath: $installer::globals::unpackpath\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$installer::globals::idttemplatepath: $installer::globals::idttemplatepath\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$installer::globals::idtlanguagepath: $installer::globals::idtlanguagepath\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "Include pathes:\n";
    push( @installer::globals::logfileinfo, $infoline);
    foreach my $path ( @{$followmeinfohash->{'includepatharray'}} ) { push( @installer::globals::logfileinfo, $path); }

    foreach $onepath ( @pathnames )
    {
        my $oldvalue = $installer::globals::separator . $onepath . $installer::globals::separator;
        my $newvalue = $installer::globals::separator . $onepath . "\." . $installer::globals::lastminor . $installer::globals::separator;

        if (( $followmeinfohash->{'installlogdir'} =~ /\Q$oldvalue\E/ ) && ( ! ( $followmeinfohash->{'installlogdir'} =~ /\Q$srcpath\E/ ))) { $followmeinfohash->{'installlogdir'} =~ s/\Q$oldvalue\E/$newvalue/; }
        if (( $installer::globals::unpackpath =~ /\Q$oldvalue\E/ ) && ( ! ( $installer::globals::unpackpath =~ /\Q$srcpath\E/ ))) { $installer::globals::unpackpath =~ s/\Q$oldvalue\E/$newvalue/; }
        if (( $installer::globals::idttemplatepath =~ /\Q$oldvalue\E/ ) && ( ! ( $installer::globals::idttemplatepath =~ /\Q$srcpath\E/ ))) { $installer::globals::idttemplatepath =~ s/\Q$oldvalue\E/$newvalue/; }
        if (( $installer::globals::idtlanguagepath =~ /\Q$oldvalue\E/ ) && ( ! ( $installer::globals::idtlanguagepath =~ /\Q$srcpath\E/ ))) { $installer::globals::idtlanguagepath =~ s/\Q$oldvalue\E/$newvalue/; }
        foreach my $path ( @{$followmeinfohash->{'includepatharray'}} ) { if (( $path =~ /\Q$oldvalue\E/ ) && ( ! ( $path =~ /\Q$srcpath\E/ ))) { $path =~ s/\Q$oldvalue\E/$newvalue/; } }

        # Checking for the end of the path
        $oldvalue = $installer::globals::separator . $onepath;
        $newvalue = $installer::globals::separator . $onepath . "\." . $installer::globals::lastminor;

        if (( $followmeinfohash->{'installlogdir'} =~ /\Q$oldvalue\E\s*$/ ) && ( ! ( $followmeinfohash->{'installlogdir'} =~ /\Q$srcpath\E/ ))) { $followmeinfohash->{'installlogdir'} =~ s/\Q$oldvalue\E\s*$/$newvalue/; }
        if (( $installer::globals::unpackpath =~ /\Q$oldvalue\E\s*$/ ) && ( ! ( $installer::globals::unpackpath =~ /\Q$srcpath\E/ ))) { $installer::globals::unpackpath =~ s/\Q$oldvalue\E\s*$/$newvalue/; }
        if (( $installer::globals::idttemplatepath =~ /\Q$oldvalue\E\s*$/ ) && ( ! ( $installer::globals::idttemplatepath =~ /\Q$srcpath\E/ ))) { $installer::globals::idttemplatepath =~ s/\Q$oldvalue\E\s*$/$newvalue/; }
        if (( $installer::globals::idtlanguagepath =~ /\Q$oldvalue\E\s*$/ ) && ( ! ( $installer::globals::idtlanguagepath =~ /\Q$srcpath\E/ ))) { $installer::globals::idtlanguagepath =~ s/\Q$oldvalue\E\s*$/$newvalue/; }
        foreach my $path ( @{$followmeinfohash->{'includepatharray'}} )
        {
            if (( $path =~ /\Q$oldvalue\E\s*$/ ) && ( ! ( $path =~ /\Q$srcpath\E/ )))
            {
                $path =~ s/\Q$oldvalue\E\s*$/$newvalue/;
                $path = $path . "\n";
            }
        }
    }

    # And now can follow the replacement for the source path "src". Subdirs like "bin" in the source tree
    # must not get the minor. This is instead "src.m9/instsetoo_native/common.pro/bin/..."
    # Directory "src" can never be the end of the path

    my $newsrcpath = $installer::globals::separator . $sourcename . "\." . $installer::globals::lastminor . $installer::globals::separator;

    if ( $followmeinfohash->{'installlogdir'} =~ /\Q$srcpath\E/ ) { $followmeinfohash->{'installlogdir'} =~ s/\Q$srcpath\E/$newsrcpath/; }
    if ( $installer::globals::unpackpath =~ /\Q$srcpath\E/ ) { $installer::globals::unpackpath =~ s/\Q$srcpath\E/$newsrcpath/; }
    if ( $installer::globals::idttemplatepath =~ /\Q$srcpath\E/ ) { $installer::globals::idttemplatepath =~ s/\Q$srcpath\E/$newsrcpath/; }
    if ( $installer::globals::idtlanguagepath =~ /\Q$srcpath\E/ ) { $installer::globals::idtlanguagepath =~ s/\Q$srcpath\E/$newsrcpath/; }
    foreach my $path ( @{$followmeinfohash->{'includepatharray'}} ) { if ( $path =~ /\Q$srcpath\E/ ) { $path =~ s/\Q$srcpath\E/$newsrcpath/; } }

    $infoline = "\nNew pathes:\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$followmeinfohash->{'installlogdir'}: $followmeinfohash->{'installlogdir'}\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$installer::globals::unpackpath: $installer::globals::unpackpath\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$installer::globals::idttemplatepath: $installer::globals::idttemplatepath\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "\$installer::globals::idtlanguagepath: $installer::globals::idtlanguagepath\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "Include pathes:\n";
    push( @installer::globals::logfileinfo, $infoline);
    foreach my $path ( @{$followmeinfohash->{'includepatharray'}} ) { push( @installer::globals::logfileinfo, $path); }
}

#############################################
# Setting the name of the log file
#############################################

sub setlogfilename
{
    if ( $installer::globals::dosign ) { $installer::globals::logfilename = "sign_and_download_" . $installer::globals::logfilename; }
    else { $installer::globals::logfilename = "download_" . $installer::globals::logfilename; }
    # reset the log file
    @installer::globals::logfileinfo = ();
}

#########################################################
# Checking, if this is a task in a cws or
# on the master. Simple check of naming schema:
# CWS: follow_me_DEV300_m40_de.log
# Master: follow_me_4_DEV300_m40_en-US.log
#########################################################

sub check_cws_build
{
    my ( $filename ) = @_;

    my $iscws = 1;

    if ( $filename =~ /follow_me_\d+_/ ) { $iscws = 0; }

    return $iscws;
}

#########################################################
# Reading a specific key from a follow-me file
#########################################################

sub get_property_from_file
{
    my ($followmefile, $key) = @_;

    my $value = "";

    my $filecontent = installer::files::read_file($followmefile);

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( ${$filecontent}[$i] =~ /^\s*\Q$key\E\s*\:\s*(.*?)\s*$/ )
        {
            $value = $1;
            last;
        }
    }

    return $value;
}

#########################################################
# Publishing the content of the product list
#########################################################

sub publishproductlist
{
    my ($infofilelist) = @_;

    installer::logger::print_message( "\n... found products: ...\n" );

    for ( my $i = 0; $i <= $#{$infofilelist}; $i++ )
    {
        my $onefile = ${$infofilelist}[$i];
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$onefile);
        installer::logger::print_message( "...... $onefile ...\n" );
    }

    installer::logger::print_message( "\n" );
}

#########################################################
# Filtering all files, that have correct minor
# and work stamp.
# Master: follow_me_4_DEV300_m40_en-US.log
#########################################################

sub filter_all_files_with_correct_settings
{
    my ($allfollowmefiles) = @_;

    my @allfiles = ();
    my @allfiles2 = ();
    my $maxhash = ();

    my $minor = "";
    my $workstamp = "";

    if ( $ENV{'WORK_STAMP'} ) { $workstamp = $ENV{'WORK_STAMP'}; }
    if ( $ENV{'UPDMINOR'} ) { $minor = $ENV{'UPDMINOR'}; }

    if ( $minor eq "" ) { installer::exiter::exit_program("ERROR: Environment variable \"UPDMINOR\" not set!", "filter_all_files_with_correct_settings"); }
    if ( $workstamp eq "" ) { installer::exiter::exit_program("ERROR: Environment variable \"WORK_STAMP\" not set!", "filter_all_files_with_correct_settings"); }

    foreach my $onefile ( @{$allfollowmefiles} )
    {
        if (( $onefile =~ /_\Q$minor\E_/i ) && ( $onefile =~ /_\Q$workstamp\E_/i ))
        {
            push(@allfiles, $onefile);

            # also collecting maximum hash

            if ( $onefile =~ /follow_me_(\d+)_\Q$workstamp\E_\Q$minor\E_([-\w]+)\.log\s*$/i )
            {
                my $sequence = $1;
                my $lang = $2;

                if (( ! exists($maxhash{$lang})) || ( $maxhash{$lang} < $sequence )) { $maxhash{$lang} = $sequence; }
            }
        }
    }

    # second run, because of sequence numbers

    foreach my $onefile ( @allfiles )
    {
        if ( $onefile =~ /follow_me_(\d+)_\Q$workstamp\E_\Q$minor\E_([-\w]+)\.log\s*$/i )
        {
            my $sequence = $1;
            my $lang = $2;

            if ( $sequence == $maxhash{$lang} ) { push(@allfiles2, $onefile); }
        }
    }

    return ( \@allfiles2 );
}

#########################################################
# Creating a list of products, that need to be signed
# or for which download sets need to be created.
#########################################################

sub createproductlist
{
    # If "-d" specifies an installation set, there is only one product

    my @infofilelist = ();
    my @infofilelist2 = ();

    if ( -f $installer::globals::followmeinfofilename )
    {
        push(@infofilelist, $installer::globals::followmeinfofilename);
        # Saving info, that this is a file
        $installer::globals::followme_from_directory = 0;
    }
    elsif ( -d $installer::globals::followmeinfofilename )
    {
        installer::logger::print_message( "\n... reading directory: $installer::globals::followmeinfofilename ...\n" );
        $installer::globals::followmeinfofilename =~ s/$installer::globals::separator\s*$//;
        my $allfollowmefiles = installer::systemactions::find_file_with_file_extension("log", $installer::globals::followmeinfofilename);

        if ( ! ( $#{$allfollowmefiles} > -1 ))
        {
            installer::logger::print_error( "Error: Nothing to do! No follow-me file in directory \"$installer::globals::followmeinfofilename\"!.\n" );
            usage();
            exit(-1);
        }

        # Saving info, that this is a directory
        $installer::globals::followme_from_directory = 1;

        # Collect all possible installation sets
        # CWS: All installation sets
        # Master: All installation sets with same major, minor and buildid. Additionally using the highest number.

        my $iscws = check_cws_build(${$allfollowmefiles}[0]);

        if ( $iscws )
        {
            # Simply read all follow-me files and check existence of installation sets
            foreach my $onefile ( @{$allfollowmefiles} )
            {
                my $fullfilename = $installer::globals::followmeinfofilename . $installer::globals::separator . $onefile;
                my $installdir = get_property_from_file($fullfilename, "finalinstalldir");
                if (( $installdir ne "" ) && ( -d $installdir )) { push(@infofilelist2, $fullfilename); }
            }
        }
        else
        {
            $allfollowmefiles = filter_all_files_with_correct_settings($allfollowmefiles);

            foreach my $onefile ( @{$allfollowmefiles} )
            {
                my $fullfilename = $installer::globals::followmeinfofilename . $installer::globals::separator . $onefile;
                # Check, if installation set still exists
                my $installdir = get_property_from_file($fullfilename, "finalinstalldir");
                if (( $installdir ne "" ) && ( -d $installdir )) { push(@infofilelist2, $fullfilename); }
            }
        }

        # Removing all files, starting with "follow_me_success_" in their names. This have already been used successfully.

        foreach my $onefile ( @infofilelist2 )
        {
            if ( $onefile =~ /follow_me_success_/ ) { next; }
            push(@infofilelist, $onefile);
        }

        # Checking, if there is content in the list
        if ( ! ( $#infofilelist > -1 ))
        {
            installer::logger::print_error( "Error: Nothing to do! No installation set found for follow-me files in directory \"$installer::globals::followmeinfofilename\"!.\n" );
            usage();
            exit(-1);
        }
    }
    else
    {
        installer::logger::print_error( "Error: Nothing to do! \"$installer::globals::followmeinfofilename\" is no file and no directory (-d).\n" );
        usage();
        exit(-1);
    }

    return \@infofilelist;
}

#############################################
# Logging the content of the download hash
#############################################

sub logfollowmeinfohash
{
    my ( $followmehash ) = @_;

    print "\n*****************************************\n";
    print "Content of follow-me info file:\n";
    print "finalinstalldir: $followmehash->{'finalinstalldir'}\n";
    print "downloadname: $followmehash->{'downloadname'}\n";
    print "languagestring: $followmehash->{'languagestring'}\n";
    foreach my $lang ( @{$followmehash->{'languagesarray'}} ) { print "languagesarray: $lang\n"; }
    foreach my $path ( @{$followmehash->{'includepatharray'}} ) { print "includepatharray: $path"; }
    foreach my $key ( sort keys %{$followmehash->{'allvariableshash'}} ) { print "allvariableshash: $key : $followmehash->{'allvariableshash'}->{$key}\n"; }
}

########################################################################
# Renaming the follow me info file, if it was successfully used.
# This can only be done, if the parameter "-d" was used with a
# directory, not a name. In this case the repeated use of parameter
# "-d" with this directory has to ignore this already successfully
# used file.
########################################################################

sub rename_followme_infofile
{
    my ( $filename ) = @_;

    my $newfilename = $filename;
    $newfilename =~ s/follow_me_/follow_me_success_/;   # including "_success" after "follow_me"

    if ( $filename ne $newfilename )
    {
        my $returnvalue = rename($filename, $newfilename);
        if ( $returnvalue ) { installer::logger::print_message( "\n... renamed file \"$filename\" to \"$newfilename\" ...\n" ); }
    }
}

1;
