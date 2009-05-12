#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: binary.pm,v $
#
# $Revision: 1.5 $
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

package installer::windows::sign;

use Cwd;
use installer::converter;
use installer::existence;
use installer::files;
use installer::globals;
use installer::scriptitems;
use installer::worker;
use installer::windows::admin;

########################################################
# Copying an existing Windows installation set.
########################################################

sub copy_install_set
{
    my ( $installsetpath ) = @_;

    installer::logger::include_header_into_logfile("Start: Copying installation set $installsetpath");

    my $infoline = "";

    my $dirname = $installsetpath;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$dirname);

    my $path = $installsetpath;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$path);

    $path =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( $dirname =~ /\./ ) { $dirname =~ s/\./_signed_inprogress./; }
    else { $dirname = $dirname . "_signed_inprogress"; }

    my $newpath = $path . $installer::globals::separator . $dirname;
    my $removepath = $newpath;
    $removepath =~ s/_inprogress/_witherror/;

    if ( -d $newpath ) { installer::systemactions::remove_complete_directory($newpath, 1); }
    if ( -d $removepath ) { installer::systemactions::remove_complete_directory($removepath, 1); }

    $infoline = "Copy installation set from $installsetpath to $newpath\n";
    push( @installer::globals::logfileinfo, $infoline);

    $installsetpath = installer::systemactions::copy_complete_directory($installsetpath, $newpath);

    installer::logger::include_header_into_logfile("End: Copying installation set $installsetpath");

    return $newpath;
}

########################################################
# Renaming an existing Windows installation set.
########################################################

sub rename_install_set
{
    my ( $installsetpath ) = @_;

    my $infoline = "";

    my $dirname = $installsetpath;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$dirname);

    my $path = $installsetpath;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$path);

    $path =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( $dirname =~ /\./ ) { $dirname =~ s/\./_inprogress./; }
    else { $dirname = $dirname . "_inprogress"; }

    my $newpath = $path . $installer::globals::separator . $dirname;
    my $removepath = $newpath;
    $removepath =~ s/_inprogress/_witherror/;

    if ( -d $newpath ) { installer::systemactions::remove_complete_directory($newpath, 1); }
    if ( -d $removepath ) { installer::systemactions::remove_complete_directory($removepath, 1); }

    $installsetpath = installer::systemactions::rename_directory($installsetpath, $newpath);

    return $newpath;
}

#########################################################
# Checking the local system
# Checking existence of needed files in include path
#########################################################

sub check_system_path
{
    # The following files have to be found in the environment variable PATH
    # Only, if \"-sign\" is used.
    # Windows : "msicert.exe", "msidb.exe", "signtool.exe"

    my @needed_files_in_path = ("msicert.exe", "msidb.exe", "signtool.exe");

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

    foreach my $onefile ( @needed_files_in_path )
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
        }
    }

    $installer::globals::signfiles_checked = 1;

    if ( $error ) { installer::exiter::exit_program("ERROR: Could not find all needed files in path!", "check_system_path"); }
}

######################################################
# Making systemcall
######################################################

sub make_systemcall
{
    my ($systemcall, $displaysystemcall) = @_;

    installer::logger::print_message( "... $displaysystemcall ...\n" );

    my $success = 1;
    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $displaysystemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$displaysystemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
        $success = 0;
    }
    else
    {
        $infoline = "Success: Executed \"$displaysystemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $success;
}

########################################################
# Reading first line of pw file.
########################################################

sub get_pw
{
    my ( $file ) = @_;

    my $filecontent = installer::files::read_file($file);

    my $pw = ${$filecontent}[0];
    $pw =~ s/^\s*//;
    $pw =~ s/\s*$//;

    return $pw;
}

########################################################
# Counting the keys of a hash.
########################################################

sub get_hash_count
{
    my ($hashref) = @_;

    my $counter = 0;

    foreach my $key ( keys %{$hashref} ) { $counter++; }

    return $counter;
}

############################################################
# Collect all DiskIds to the corresponding cabinet files.
############################################################

sub analyze_media_file
{
    my ($filecontent) = @_;

    my %diskidhash = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i < 3 ) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $diskid = $1;
            my $cabfile = $4;

            $diskidhash{$cabfile} = $diskid;
        }
    }

    return \%diskidhash;
}

########################################################
# Collect all DiskIds from database table "Media".
########################################################

sub collect_diskid
{
    my ($msidatabase, $languagestring) = @_;

    # creating working directory
    my $workdir = installer::systemactions::create_directories("media", \$languagestring);
    installer::windows::admin::extract_tables_from_pcpfile($msidatabase, $workdir, "Media");

    # Reading tables
    my $filename = $workdir . $installer::globals::separator . "Media.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find required file: $filename !", "collect_diskid"); }
    my $filecontent = installer::files::read_file($filename);
    my $diskidhash = analyze_media_file($filecontent);

    return $diskidhash;
}

########################################################
# Collecting all files in an installation set.
########################################################

sub analyze_installset_content
{
    my ( $installsetpath ) = @_;

    my @sourcefiles = ();
    my $pathstring = "";
    installer::systemactions::read_complete_directory($installsetpath, $pathstring, \@sourcefiles);

    if ( ! ( $#sourcefiles > -1 )) { installer::exiter::exit_program("ERROR: No file in installation set. Path: $installsetpath !", "analyze_installset_content"); }

    my %allcabfileshash = ();
    my %allmsidatabaseshash = ();
    my %allfileshash = ();
    my $contains_cab_file = 0;
    my $msidatabase = "";

    for ( my $j = 0; $j <= $#sourcefiles; $j++ )
    {
        if ( $sourcefiles[$j] =~ /\.cab\s*$/ ) { $allcabfileshash{$sourcefiles[$j]} = 1; }
        else
        {
            if ( $sourcefiles[$j] =~ /instmsi\w+.exe\s*$/ ) { next; } # no signing of instmsia.exe and instmsiw.exe
            if ( $sourcefiles[$j] =~ /jre[-\w]+.exe\s*$/ ) { next; } # no signing of java executable
            if ( $sourcefiles[$j] =~ /\.txt\s*$/ ) { next; }
            if ( $sourcefiles[$j] =~ /\.html\s*$/ ) { next; }
            if ( $sourcefiles[$j] =~ /\.ini\s*$/ ) { next; }
            if ( $sourcefiles[$j] =~ /\.msi\s*$/ )
            {
                if ( $msidatabase eq "" ) { $msidatabase = $sourcefiles[$j]; }
                else { installer::exiter::exit_program("ERROR: There is more than one msi database in installation set. Path: $installsetpath !", "analyze_installset_content"); }
            }
            $allfileshash{$sourcefiles[$j]} = 1;
        }
    }

    # Is there at least one cab file in the installation set?
    my $cabcounter = get_hash_count(\%allcabfileshash);
    if ( $cabcounter > 0 ) { $contains_cab_file = 1; }

    # How about a cab file without a msi database?
    if (( $cabcounter > 0 ) && ( $msidatabase eq "" )) { installer::exiter::exit_program("ERROR: There is no msi database in the installation set, but an external cabinet file. Path: $installsetpath !", "collect_installset_content"); }

    return (\%allcabfileshash, \%allfileshash, $msidatabase, $contains_cab_file);
}

########################################################
# Adding content of external cabinet files into the
# msi database
########################################################

sub msicert_database
{
    my ($msidatabase, $allcabfiles, $languagestring) = @_;

    # exclude media table from msi database and get all diskids.
    my $cabfilehash = collect_diskid($msidatabase, $languagestring);

    my $fullsuccess = 1;

    foreach my $cabfile ( keys %{$allcabfiles} )
    {
        if ( ! exists($cabfilehash->{$cabfile}) ) { installer::exiter::exit_program("ERROR: Could not determine DiskId from media table for cabinet file \"$cabfile\" !", "msicert_database"); }
        my $diskid = $cabfilehash->{$cabfile};
        my $systemcall = "msicert.exe -d $msidatabase -m $diskid -c $cabfile -h";
         $success = make_systemcall($systemcall, $systemcall);
        if ( ! $success ) { $fullsuccess = 0; }
    }

    return $fullsuccess;
}

########################################################
# Signing a list of files
########################################################

sub sign_files
{
    my ( $followmeinfohash, $allfiles, $pw ) = @_;

    my $infoline = "";
    my $fullsuccess = 1;

    my $productname = "";
    if ( $followmeinfohash->{'allvariableshash'}->{'PRODUCTNAME'} ) { $productname = "/d " . "\"$followmeinfohash->{'allvariableshash'}->{'PRODUCTNAME'}\""; }
    my $url = "";
    if ( $followmeinfohash->{'allvariableshash'}->{'OPENSOURCE'} == 0 ) { $url = "/du " . "\"http://www.sun.com\""; }
    else { $url = "/du " . "\"http://www.openoffice.org\""; }
    my $timestampurl = "http://timestamp.verisign.com/scripts/timestamp.dll";

    foreach my $onefile ( keys %{$allfiles} )
    {
        my $systemcall = "signtool.exe sign /f \"$installer::globals::pfxfile\" /p $pw $productname $url /t \"$timestampurl\" \"$onefile\"";
        my $displaysystemcall = "signtool.exe sign /f \"$installer::globals::pfxfile\" /p ***** $productname $url /t \"$timestampurl\" \"$onefile\"";
         my $success = make_systemcall($systemcall, $displaysystemcall);
         if ( ! $success ) { $fullsuccess = 0; }
    }

    return $fullsuccess;
}

########################################################
# Signing an existing Windows installation set.
########################################################

sub sign_install_set
{
    my ($followmeinfohash, $make_copy) = @_;

    my $installsetpath = $followmeinfohash->{'finalinstalldir'};

    installer::logger::include_header_into_logfile("Start: Signing installation set $installsetpath");

    my $complete_success = 1;
    my $success = 1;

    my $infoline = "Signing installation set in $installsetpath\n";
    push( @installer::globals::logfileinfo, $infoline);

    # check required files.
    if ( ! $installer::globals::signfiles_checked ) { check_system_path(); }

    # get cerficate information
    my $pw = get_pw($installer::globals::pwfile);

    # making a copy of the installation set, if required
    if ( $make_copy ) { $installsetpath = copy_install_set($installsetpath); }
    else { $installsetpath = rename_install_set($installsetpath); }

    # collecting all files in the installation set
    my ($allcabfiles, $allfiles, $msidatabase, $contains_cab_file) = analyze_installset_content($installsetpath);

    # changing into installation set
    my $from = cwd();
    chdir($installsetpath);

    # Warning: There might be a problem with very big cabinet files
    # signing all external cab files first
    if ( $contains_cab_file )
    {
        $success = sign_files($followmeinfohash, $allcabfiles, $pw);
        if ( ! $success ) { $complete_success = 0; }
        $success = msicert_database($msidatabase, $allcabfiles, $followmeinfohash->{'languagestring'});
        if ( ! $success ) { $complete_success = 0; }
    }

    # finally all other files can be signed
    $success = sign_files($followmeinfohash, $allfiles, $pw);
    if ( ! $success ) { $complete_success = 0; }

    # and changing back
    chdir($from);

    installer::logger::include_header_into_logfile("End: Signing installation set $installsetpath");

    return ($installsetpath);
}

1;
