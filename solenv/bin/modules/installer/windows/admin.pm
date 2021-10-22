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

package installer::windows::admin;

use File::Copy;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::systemactions;
use installer::worker;
use installer::windows::idtglobal;

#################################################################################
# Unpacking cabinet files with expand
#################################################################################

sub unpack_cabinet_file
{
    my ($cabfilename, $unpackdir) = @_;

    my $infoline = "Unpacking cabinet file: $cabfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $expandfile = "expand.exe";  # Has to be in the path

    # expand.exe has to be located in the system directory.
    # Cygwin has another tool expand.exe, that converts tabs to spaces. This cannot be used of course.
    # But this wrong expand.exe is typically in the PATH before this expand.exe, to unpack
    # cabinet files.

    if ( $^O =~ /cygwin/i )
    {
        $expandfile = qx(cygpath -u "$ENV{WINDIR}"/System32/expand.exe);
        chomp $expandfile;
    }

    my $expandlogfile = $unpackdir . $installer::globals::separator . "expand.log";

    # exclude cabinet file

    my $systemcall = "";
    if ( $^O =~ /cygwin/i ) {
        my $localunpackdir = qx{cygpath -w "$unpackdir"};
        chomp ($localunpackdir);
        $localunpackdir =~ s/\\/\\\\/g;
        $cabfilename =~ s/\\/\\\\/g;
        $cabfilename =~ s/\s*$//g;
        $systemcall = $expandfile . " " . $cabfilename . " -F:\* " . $localunpackdir . " \> " . $expandlogfile;
    }
    else
    {
        $systemcall = $expandfile . " " . $cabfilename . " -F:\* " . $unpackdir . " \> " . $expandlogfile;
    }

    my $returnvalue = system($systemcall);
    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program("ERROR: Could not extract cabinet file: $mergemodulehash->{'cabinetfile'} !", "change_file_table");
    }
    else
    {
        $infoline = "Success: Executed $systemcall successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#################################################################################
# Extracting tables from msi database
#################################################################################

sub extract_tables_from_pcpfile
{
    my ($fullmsidatabasepath, $workdir, $tablelist) = @_;

    my $msidb = "msidb.exe";    # Has to be in the path
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";

    my $localfullmsidatabasepath = $fullmsidatabasepath;

    # Export of all tables by using "*"

    if ( $^O =~ /cygwin/i ) {
        # Copying the msi database locally guarantees the format of the directory.
        # Otherwise it is defined in the file of UPDATE_DATABASE_LISTNAME

        my $msifilename = $localfullmsidatabasepath;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$msifilename);
        my $destdatabasename = $workdir . $installer::globals::separator . $msifilename;
        installer::systemactions::copy_one_file($localfullmsidatabasepath, $destdatabasename);
        $localfullmsidatabasepath = $destdatabasename;

        chomp( $localfullmsidatabasepath = qx{cygpath -w "$localfullmsidatabasepath"} );
        chomp( $workdir = qx{cygpath -w "$workdir"} );

        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
        $localfullmsidatabasepath =~ s/\\/\\\\/g;
        $workdir =~ s/\\/\\\\/g;

        # and if there are still slashes, they also need to be double backslash
        $localfullmsidatabasepath =~ s/\//\\\\/g;
        $workdir =~ s/\//\\\\/g;
    }

    $systemcall = $msidb . " -d " . $localfullmsidatabasepath . " -f " . $workdir . " -e $tablelist";
    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program("ERROR: Could not exclude tables from pcp file: $localfullmsidatabasepath !", "extract_tables_from_pcpfile");
    }
    else
    {
        $infoline = "Success: Executed $systemcall successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

################################################################################
# Analyzing the content of Directory.idt
#################################################################################

sub analyze_directory_file
{
    my ($filecontent) = @_;

    my %table = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if (( $i == 0 ) || ( $i == 1 ) || ( $i == 2 )) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $dir = $1;
            my $parent = $2;
            my $name = $3;

            if ( $name =~ /^\s*(.*?)\s*\:\s*(.*?)\s*$/ ) { $name = $2; }
            if ( $name =~ /^\s*(.*?)\s*\|\s*(.*?)\s*$/ ) { $name = $2; }

            my %helphash = ();
            $helphash{'Directory_Parent'} = $parent;
            $helphash{'DefaultDir'} = $name;
            $table{$dir} = \%helphash;
        }
    }

    return \%table;
}

#################################################################################
# Analyzing the content of Component.idt
#################################################################################

sub analyze_component_file
{
    my ($filecontent) = @_;

    my %table = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if (( $i == 0 ) || ( $i == 1 ) || ( $i == 2 )) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $component = $1;
            my $dir = $3;

            $table{$component} = $dir;
        }
    }

    return \%table;
}

#################################################################################
# Analyzing the content of File.idt
#################################################################################

sub analyze_file_file
{
    my ($filecontent) = @_;

    my %table = ();
    my %fileorder = ();
    my $maxsequence = 0;

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if (( $i == 0 ) || ( $i == 1 ) || ( $i == 2 )) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $file = $1;
            my $comp = $2;
            my $filename = $3;
            my $sequence = $8;

            if ( $filename =~ /^\s*(.*?)\s*\|\s*(.*?)\s*$/ ) { $filename = $2; }

            my %helphash = ();
            $helphash{'Component'} = $comp;
            $helphash{'FileName'} = $filename;
            $helphash{'Sequence'} = $sequence;

            $table{$file} = \%helphash;

            $fileorder{$sequence} = $file;

            if ( $sequence > $maxsequence ) { $maxsequence = $sequence; }
        }
    }

    return (\%table, \%fileorder, $maxsequence);
}

####################################################################################
# Recursively creating the directory tree
####################################################################################

sub create_directory_tree
{
    my ($parent, $pathcollector, $fulldir, $dirhash) = @_;

    foreach my $dir ( keys %{$dirhash} )
    {
        if (( $dirhash->{$dir}->{'Directory_Parent'} eq $parent ) && ( $dirhash->{$dir}->{'DefaultDir'} ne "." ))
        {
            my $dirname = $dirhash->{$dir}->{'DefaultDir'};
            # Create the directory
            my $newdir = $fulldir . $installer::globals::separator . $dirname;
            if ( ! -f $newdir ) { mkdir $newdir; }
            # Saving in collector
            $pathcollector->{$dir} = $newdir;
            # Iteration
            create_directory_tree($dir, $pathcollector, $newdir, $dirhash);
        }
    }
}

####################################################################################
# Creating the directory tree
####################################################################################

sub create_directory_structure
{
    my ($dirhash, $targetdir) = @_;

    my %fullpathhash = ();

    my @startparents = ("TARGETDIR", "INSTALLLOCATION");

    foreach $dir (@startparents) { create_directory_tree($dir, \%fullpathhash, $targetdir, $dirhash); }

    # Also adding the paths of the startparents
    foreach $dir (@startparents)
    {
        if ( ! exists($fullpathhash{$dir}) ) { $fullpathhash{$dir} = $targetdir; }
    }
    # FIXME why is this hack needed? ERROR: Did not find full directory path for dir: "SystemFolder_x86_VC.194841A2_D0F2_3B96_9F71_05BA91BEA0FA"
    $fullpathhash{"SystemFolder_x86_VC.194841A2_D0F2_3B96_9F71_05BA91BEA0FA"} = $targetdir . $installer::globals::separator . "System";
    $fullpathhash{"SystemFolder_x86_VC.E281B893_10D7_34CE_BB0E_B69D88E154A5"} = $targetdir . $installer::globals::separator . "System";
    $fullpathhash{"System64Folder_amd64_VC.AF4EABEE_4589_3789_BA0A_C83A71662E1D"} = $targetdir . $installer::globals::separator . "System64";
    # FIXME DefaultDir in MSI was '.', why is this a problem now?
    $fullpathhash{"Colla_progr_shlxt_a8b47146a"} = $targetdir . $installer::globals::separator . "program" . $installer::globals::separator . "shlxthdl";

    return \%fullpathhash;
}

####################################################################################
# Copying files into installation set
####################################################################################

sub copy_files_into_directory_structure
{
    my ($fileorder, $filehash, $componenthash, $fullpathhash, $maxsequence, $unpackdir, $installdir, $dirhash) = @_;

    for ( my $i = 1; $i <= $maxsequence; $i++ )
    {
        if ( exists($fileorder->{$i}) )
        {
            my $file = $fileorder->{$i};
            if ( ! exists($filehash->{$file}->{'Component'}) ) { installer::exiter::exit_program("ERROR: Did not find component for file: \"$file\".", "copy_files_into_directory_structure"); }
            my $component = $filehash->{$file}->{'Component'};
            if ( ! exists($componenthash->{$component}) ) { installer::exiter::exit_program("ERROR: Did not find directory for component: \"$component\".", "copy_files_into_directory_structure"); }
            my $dirname = $componenthash->{$component};
            if ( ! exists($fullpathhash->{$dirname}) ) { installer::exiter::exit_program("ERROR: Did not find full directory path for dir: \"$dirname\".", "copy_files_into_directory_structure"); }
            my $destdir = $fullpathhash->{$dirname};
            if ( ! exists($filehash->{$file}->{'FileName'}) ) { installer::exiter::exit_program("ERROR: Did not find \"FileName\" for file: \"$file\".", "copy_files_into_directory_structure"); }
            my $destfile = $filehash->{$file}->{'FileName'};

            $destfile = $destdir . $installer::globals::separator . $destfile;
            my $sourcefile = $unpackdir . $installer::globals::separator . $file;

            if ( ! -f $sourcefile )
            {
                # It is possible, that this was an unpacked file
                # Looking in the dirhash, to find the subdirectory in the installation set (the id is $dirname)
                # subdir is not recursively analyzed, only one directory.

                my $oldsourcefile = $sourcefile;
                my $subdir = "";
                if ( exists($dirhash->{$dirname}->{'DefaultDir'}) ) { $subdir = $dirhash->{$dirname}->{'DefaultDir'} . $installer::globals::separator; }
                my $realfilename = $filehash->{$file}->{'FileName'};
                my $localinstalldir = $installdir;

                $localinstalldir =~ s/\\\s*$//;
                $localinstalldir =~ s/\/\s*$//;

                $sourcefile = $localinstalldir . $installer::globals::separator . $subdir . $realfilename;

                if ( ! -f $sourcefile )
                {
                    installer::exiter::exit_program("ERROR: File not found: \"$oldsourcefile\" (or \"$sourcefile\").", "copy_files_into_directory_structure");
                }
            }

            my $copyreturn = copy($sourcefile, $destfile);

            if ( ! $copyreturn) # only logging problems
            {
                my $infoline = "ERROR: Could not copy $sourcefile to $destfile (insufficient disc space for $destfile ?)\n";
                $returnvalue = 0;
                push(@installer::globals::logfileinfo, $infoline);
                installer::exiter::exit_program($infoline, "copy_files_into_directory_structure");
            }
        }
    }
}


###############################################################
# Setting the time string for the
# Summary Information stream in the
# msi database of the admin installations.
###############################################################

sub get_sis_time_string
{
    # Syntax: <yyyy/mm/dd hh:mm:ss>
    my $second = (localtime())[0];
    my $minute = (localtime())[1];
    my $hour = (localtime())[2];
    my $day = (localtime())[3];
    my $month = (localtime())[4];
    my $year = 1900 + (localtime())[5];

    $month++; # zero based month

    if ( $second < 10 ) { $second = "0" . $second; }
    if ( $minute < 10 ) { $minute = "0" . $minute; }
    if ( $hour < 10 ) { $hour = "0" . $hour; }
    if ( $day < 10 ) { $day = "0" . $day; }
    if ( $month < 10 ) { $month = "0" . $month; }

    my $timestring = $year . "/" . $month . "/" . $day . " " . $hour . ":" . $minute . ":" . $second;

    return $timestring;
}

###############################################################
# Writing content of administrative installations into
# Summary Information Stream of msi database.
# This is required for example for following
# patch processes using Windows Installer service.
###############################################################

sub write_sis_info
{
    my ($msidatabase) = @_ ;

    if ( ! -f $msidatabase ) { installer::exiter::exit_program("ERROR: Cannot find file $msidatabase", "write_sis_info"); }

    my $msiinfo = "msiinfo.exe";    # Has to be in the path
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";

    # Required setting for administrative installations:
    # -w 4   (source files are unpacked),  wordcount
    # -s <date of admin installation>, LastPrinted, Syntax: <yyyy/mm/dd hh:mm:ss>
    # -l <person_making_admin_installation>, LastSavedBy

    my $wordcount = 4;  # Unpacked files
    my $lastprinted = get_sis_time_string();
    my $lastsavedby = "Installer";

    my $localmsidatabase = $msidatabase;

    if( $^O =~ /cygwin/i )
    {
        $localmsidatabase = qx{cygpath -w "$localmsidatabase"};
        $localmsidatabase =~ s/\\/\\\\/g;
        $localmsidatabase =~ s/\s*$//g;
    }

    $systemcall = $msiinfo . " " . "\"" . $localmsidatabase . "\"" . " -w " . $wordcount . " -s " . "\"" . $lastprinted . "\"" . " -l $lastsavedby";
    push(@installer::globals::logfileinfo, $systemcall);
    $returnvalue = system($systemcall);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        push(@installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program($infoline, "write_sis_info");
    }
}

####################################################################################
# Simulating an administrative installation
####################################################################################

sub make_admin_install
{
    my ($databasepath, $targetdir) = @_;

    # Create helper directory

    installer::logger::print_message( "... installing $databasepath in directory $targetdir ...\n" );

    my $helperdir = $targetdir . $installer::globals::separator . "installhelper";
    installer::systemactions::create_directory($helperdir);

    # Get File.idt, Component.idt and Directory.idt from database

    my $tablelist = "File Directory Component Registry";
    extract_tables_from_pcpfile($databasepath, $helperdir, $tablelist);

    # Unpack all cab files into $helperdir, cab files must be located next to msi database
    my $installdir = $databasepath;

    if ( $^O =~ /cygwin/i ) { $installdir =~ s/\\/\//g; } # backslash to slash

    installer::pathanalyzer::get_path_from_fullqualifiedname(\$installdir);

    if ( $^O =~ /cygwin/i ) { $installdir =~ s/\//\\/g; } # slash to backslash

    my $databasefilename = $databasepath;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$databasefilename);

    my $cabfiles = installer::systemactions::find_file_with_file_extension("cab", $installdir);

    if ( $#{$cabfiles} < 0 ) { installer::exiter::exit_program("ERROR: Did not find any cab file in directory $installdir", "make_admin_install"); }

    # Set unpackdir
    my $unpackdir = $helperdir . $installer::globals::separator . "unpack";
    installer::systemactions::create_directory($unpackdir);

    for ( my $i = 0; $i <= $#{$cabfiles}; $i++ )
    {
        my $cabfile = "";
        if ( $^O =~ /cygwin/i )
        {
            $cabfile = $installdir . ${$cabfiles}[$i];
        }
        else
        {
            $cabfile = $installdir . $installer::globals::separator . ${$cabfiles}[$i];
        }
        unpack_cabinet_file($cabfile, $unpackdir);
    }

    # Reading tables
    my $filename = $helperdir . $installer::globals::separator . "Directory.idt";
    my $filecontent = installer::files::read_file($filename);
    my $dirhash = analyze_directory_file($filecontent);

    $filename = $helperdir . $installer::globals::separator . "Component.idt";
    my $componentfilecontent = installer::files::read_file($filename);
    my $componenthash = analyze_component_file($componentfilecontent);

    $filename = $helperdir . $installer::globals::separator . "File.idt";
    $filecontent = installer::files::read_file($filename);
    my ( $filehash, $fileorder, $maxsequence ) = analyze_file_file($filecontent);

    # Creating the directory structure
    my $fullpathhash = create_directory_structure($dirhash, $targetdir);

    # Copying files
    copy_files_into_directory_structure($fileorder, $filehash, $componenthash, $fullpathhash, $maxsequence, $unpackdir, $installdir, $dirhash);

    my $msidatabase = $targetdir . $installer::globals::separator . $databasefilename;
    installer::systemactions::copy_one_file($databasepath, $msidatabase);

    # Saving info in Summary Information Stream of msi database (required for following patches)
    write_sis_info($msidatabase);

    return $msidatabase;
}

1;
