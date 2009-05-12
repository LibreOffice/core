#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: remover.pm,v $
#
# $Revision: 1.4 $
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
make_download V1.0 (c) Ingo Schmidt-Rosbiegal 2009
The following parameter are needed:
-d: Full path to the file containing the follow-me info or to a directory
    containing the follow-me info files. In the latter case, all follow-me
    info files are evaluated.
The following parameter are optional:
-sign: Uses signing mechanism to sign installation sets
If \"-sign\" is set, the following two parameter are required:
-pfx: Full path to the pfx file
-pw: Full path to the file, containing the pfx password.

Examples:

Specifying an installation set (with "-d"):

perl make_sign_and_download.pl -d <followmeinfofilename>
perl make_sign_and_download.pl -d <followmeinfofilename>
                               -sign
                               -pfx <pfxfilename>
                               -pw <passwordfilename>

or without specifying an installation set:

perl make_sign_and_download.pl -sign
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
    # installer::logger::print_message("Checking parameter");

    while ( $#ARGV >= 0 )
    {
        my $param = shift(@ARGV);

        if ($param eq "-d") { $installer::globals::followmeinfofilename = shift(@ARGV); }
        elsif ($param eq "-pw") { $installer::globals::pwfile = shift(@ARGV); }
        elsif ($param eq "-pfx") { $installer::globals::pfxfile = shift(@ARGV); }
        elsif ($param eq "-sign") { $installer::globals::dosign = 1; }
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
    # if ( $filename =~ /log_\d+_/ ) { $iscws = 0; }

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

    if ( -f $installer::globals::followmeinfofilename )
    {
        push(@infofilelist, $installer::globals::followmeinfofilename);
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
                if (( $installdir ne "" ) && ( -d $installdir )) { push(@infofilelist, $fullfilename); }
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
                if (( $installdir ne "" ) && ( -d $installdir )) { push(@infofilelist, $fullfilename); }
            }
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

1;
