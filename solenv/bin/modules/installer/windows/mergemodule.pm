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

package installer::windows::mergemodule;

use Cwd;
use Digest::MD5;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::scriptitems;
use installer::systemactions;
use installer::worker;
use installer::windows::idtglobal;
use installer::windows::language;

#################################################################
# Merging the Windows MergeModules into the msi database.
#################################################################

sub merge_mergemodules_into_msi_database
{
    my ($mergemodules, $filesref, $msifilename, $languagestringref, $allvariables, $includepatharrayref, $allupdatesequences, $allupdatelastsequences, $allupdatediskids) = @_;

    my $domerge = 0;
    if (( $#{$mergemodules} > -1 ) && ( ! $installer::globals::patch ) && ( ! $installer::globals::languagepack ) && ( ! $installer::globals::helppack )) { $domerge = 1; }

    if ( $domerge )
    {
        installer::logger::include_header_into_logfile("Merging merge modules into msi database");
        installer::logger::print_message( "... merging msm files into msi database ... \n" );
        installer::logger::include_timestamp_into_logfile("\nPerformance Info: MergeModule into msi database, start");

        my $msidb = "msidb.exe";    # Has to be in the path
        my $cabinetfile = "MergeModule.CABinet"; # the name of each cabinet file in a merge file
        my $infoline = "";
        my $systemcall = "";
        my $returnvalue = "";

        # 1. Analyzing the MergeModule (has only to be done once)
        #   a. -> Extracting cabinet file: msidb.exe -d <msmfile> -x MergeModule.CABinet
        #   b. -> Number of files in cabinet file: msidb.exe -d <msmfile> -f <directory> -e File
        #   c. -> List of components: msidb.exe -d <msmfile> -f <directory> -e Component

        if ( ! $installer::globals::mergemodules_analyzed )
        {
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Analyzing MergeModules, start");
            $infoline = "Analyzing all Merge Modules\n\n";
            push( @installer::globals::logfileinfo, $infoline);

            %installer::globals::mergemodules = ();

            my $mergemoduledir = installer::systemactions::create_directories("mergefiles", $languagestringref);

            my $mergemodule;
            foreach $mergemodule ( @{$mergemodules} )
            {
                my $filename = $mergemodule->{'Name'};
                my $mergefile = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);

                if ( ! -f $$mergefile ) { installer::exiter::exit_program("ERROR: msm file not found: $filename !", "merge_mergemodules_into_msi_database"); }
                my $completesource = $$mergefile;

                my $mergegid = $mergemodule->{'gid'};
                my $workdir = $mergemoduledir . $installer::globals::separator . $mergegid;
                if ( ! -d $workdir ) { installer::systemactions::create_directory($workdir); }

                $infoline = "Analyzing Merge Module: $filename\n";
                push( @installer::globals::logfileinfo, $infoline);

                # copy msm file into working directory
                my $completedest = $workdir . $installer::globals::separator . $filename;
                installer::systemactions::copy_one_file($completesource, $completedest);
                if ( ! -f $completedest ) { installer::exiter::exit_program("ERROR: msm file not found: $completedest !", "merge_mergemodules_into_msi_database"); }

                # changing directory
                my $from = cwd();
                my $to = $workdir;
                chdir($to);

                # remove an existing cabinet file
                if ( -f $cabinetfile ) { unlink($cabinetfile); }

                # exclude cabinet file
                $systemcall = $msidb . " -d " . $filename . " -x " . $cabinetfile;
                $returnvalue = system($systemcall);

                $infoline = "Systemcall: $systemcall\n";
                push( @installer::globals::logfileinfo, $infoline);

                if ($returnvalue)
                {
                    $infoline = "ERROR: Could not execute $systemcall !\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    installer::exiter::exit_program("ERROR: Could not extract cabinet file from merge file: $completedest !", "merge_mergemodules_into_msi_database");
                }
                else
                {
                    $infoline = "Success: Executed $systemcall successfully!\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }

                # exclude tables from mergefile
                # Attention: All listed tables have to exist in the database. If they not exist, an error window pops up
                # and the return value of msidb.exe is not zero. The error window makes it impossible to check the existence
                # of a table with the help of the return value.
                # Solution: Export of all tables by using "*" . Some tables must exist (File Component Directory), other
                # tables do not need to exist (MsiAssembly).

                if ( $^O =~ /cygwin/i ) {
                    # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                    my $localworkdir = $workdir;
                    $localworkdir =~ s/\//\\\\/g;
                    $systemcall = $msidb . " -d " . $filename . " -f " . $localworkdir . " -e \\\*";
                }
                else
                {
                    $systemcall = $msidb . " -d " . $filename . " -f " . $workdir . " -e \*";
                }

                $returnvalue = system($systemcall);

                $infoline = "Systemcall: $systemcall\n";
                push( @installer::globals::logfileinfo, $infoline);

                if ($returnvalue)
                {
                    $infoline = "ERROR: Could not execute $systemcall !\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    installer::exiter::exit_program("ERROR: Could not exclude tables from merge file: $completedest !", "merge_mergemodules_into_msi_database");
                }
                else
                {
                    $infoline = "Success: Executed $systemcall successfully!\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }

                # Determining  files
                my $idtfilename = "File.idt"; # must exist
                if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: File \"$idtfilename\" not found in directory \"$workdir\" !", "merge_mergemodules_into_msi_database"); }
                my $filecontent = installer::files::read_file($idtfilename);
                my @file_idt_content = ();
                my $filecounter = 0;
                my %mergefilesequence = ();
                for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
                {
                    if ( $i <= 2 ) { next; }                        # ignoring first three lines
                    if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
                    $filecounter++;
                    push(@file_idt_content, ${$filecontent}[$i]);
                    if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.+?)\t(.+?)\t(.+?)\t(.*?)\t(.*?)\t(.*?)\t(\d+?)\s*$/ )
                    {
                        my $filename = $1;
                        my $filesequence = $8;
                        $mergefilesequence{$filename} = $filesequence;
                    }
                    else
                    {
                        my $linecount = $i + 1;
                        installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "merge_mergemodules_into_msi_database");
                    }
                }

                # Determining components
                $idtfilename = "Component.idt"; # must exist
                if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: File \"$idtfilename\" not found in directory \"$workdir\" !", "merge_mergemodules_into_msi_database"); }
                $filecontent = installer::files::read_file($idtfilename);
                my %componentnames = ();
                for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
                {
                    if ( $i <= 2 ) { next; }                        # ignoring first three lines
                    if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
                    if ( ${$filecontent}[$i] =~ /^\s*(\S+)\s+/ ) { $componentnames{$1} = 1; }
                }

                # Determining directories
                $idtfilename = "Directory.idt";  # must exist
                if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: File \"$idtfilename\" not found in directory \"$workdir\" !", "merge_mergemodules_into_msi_database"); }
                $filecontent = installer::files::read_file($idtfilename);
                my %mergedirectories = ();
                for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
                {
                    if ( $i <= 2 ) { next; }                        # ignoring first three lines
                    if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
                    if ( ${$filecontent}[$i] =~ /^\s*(\S+)\s+/ ) { $mergedirectories{$1} = 1; }
                }

                # Determining assemblies
                $idtfilename = "MsiAssembly.idt"; # does not need to exist
                my $hasmsiassemblies = 0;
                my %mergeassemblies = ();
                if ( -f $idtfilename )
                {
                    $filecontent = installer::files::read_file($idtfilename);
                    $hasmsiassemblies = 1;
                    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
                    {
                        if ( $i <= 2 ) { next; }                        # ignoring first three lines
                        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
                        if ( ${$filecontent}[$i] =~ /^\s*(\S+)\s+/ ) { $mergeassemblies{$1} = 1; }
                    }
                }

                # It is possible, that other tables have to be checked here. This happens, if tables in the
                # merge module have to know the "Feature" or the "Directory", under which the content of the
                # msm file is integrated into the msi database.

                # Determining name of cabinet file in installation set
                my $cabfilename = $mergemodule->{'Cabfilename'};
                if ( $cabfilename ) { installer::packagelist::resolve_packagevariables(\$cabfilename, $allvariables, 0); }

                # Analyzing styles
                # Flag REMOVE_FILE_TABLE is required for msvc9 Merge-Module, because otherwise msidb.exe
                # fails during integration of msm file into msi database.

                my $styles = "";
                my $removefiletable = 0;
                if ( $mergemodule->{'Styles'} ) { $styles = $mergemodule->{'Styles'}; }
                if ( $styles =~ /\bREMOVE_FILE_TABLE\b/ ) { $removefiletable = 1; }

                if ( $removefiletable )
                {
                    my $removeworkdir = $workdir . $installer::globals::separator . "remove_file_idt";
                    if ( ! -d $removeworkdir ) { installer::systemactions::create_directory($removeworkdir); }
                    my $completeremovedest = $removeworkdir . $installer::globals::separator . $filename;
                    installer::systemactions::copy_one_file($completedest, $completeremovedest);
                    if ( ! -f $completeremovedest ) { installer::exiter::exit_program("ERROR: msm file not found: $completeremovedest !", "merge_mergemodules_into_msi_database"); }

                    # Unpacking msm file
                    if ( $^O =~ /cygwin/i ) {
                        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                        my $localcompleteremovedest = $completeremovedest;
                        my $localremoveworkdir = $removeworkdir;
                        $localcompleteremovedest =~ s/\//\\\\/g;
                        $localremoveworkdir =~ s/\//\\\\/g;
                        $systemcall = $msidb . " -d " . $localcompleteremovedest . " -f " . $localremoveworkdir . " -e \\\*";
                    }
                    else
                    {
                        $systemcall = $msidb . " -d " . $completeremovedest . " -f " . $removeworkdir . " -e \*";
                    }

                    $returnvalue = system($systemcall);

                    my $idtfilename = $removeworkdir . $installer::globals::separator . "File.idt";
                    if ( -f $idtfilename ) { unlink $idtfilename; }
                    unlink $completeremovedest;

                    # Packing msm file without "File.idt"
                    if ( $^O =~ /cygwin/i ) {
                        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                        my $localcompleteremovedest = $completeremovedest;
                        my $localremoveworkdir = $removeworkdir;
                        $localcompleteremovedest =~ s/\//\\\\/g;
                        $localremoveworkdir =~ s/\//\\\\/g;
                        $systemcall = $msidb . " -c -d " . $localcompleteremovedest . " -f " . $localremoveworkdir . " -i \\\*";
                    }
                    else
                    {
                        $systemcall = $msidb . " -c -d " . $completeremovedest . " -f " . $removeworkdir . " -i \*";
                    }
                    $returnvalue = system($systemcall);

                    # Using this msm file for merging
                    if ( -f $completeremovedest ) { $completedest = $completeremovedest; }
                    else { installer::exiter::exit_program("ERROR: Could not find msm file without File.idt: $completeremovedest !", "merge_mergemodules_into_msi_database"); }
                }

                # Saving MergeModule info

                my %onemergemodulehash = ();
                $onemergemodulehash{'mergefilepath'} = $completedest;
                $onemergemodulehash{'workdir'} = $workdir;
                $onemergemodulehash{'cabinetfile'} = $workdir . $installer::globals::separator . $cabinetfile;
                $onemergemodulehash{'filenumber'} = $filecounter;
                $onemergemodulehash{'componentnames'} = \%componentnames;
                $onemergemodulehash{'componentcondition'} = $mergemodule->{'ComponentCondition'};
                $onemergemodulehash{'cabfilename'} = $cabfilename;
                $onemergemodulehash{'feature'} = $mergemodule->{'Feature'};
                $onemergemodulehash{'rootdir'} = $mergemodule->{'RootDir'};
                $onemergemodulehash{'name'} = $mergemodule->{'Name'};
                $onemergemodulehash{'mergefilesequence'} = \%mergefilesequence;
                $onemergemodulehash{'mergeassemblies'} = \%mergeassemblies;
                $onemergemodulehash{'mergedirectories'} = \%mergedirectories;
                $onemergemodulehash{'hasmsiassemblies'} = $hasmsiassemblies;
                $onemergemodulehash{'removefiletable'} = $removefiletable;
                $onemergemodulehash{'fileidtcontent'} = \@file_idt_content;

                $installer::globals::mergemodules{$mergegid} = \%onemergemodulehash;

                # Collecting all cab files, to copy them into installation set
                if ( $cabfilename ) { $installer::globals::copy_msm_files{$cabfilename} = $onemergemodulehash{'cabinetfile'}; }

                chdir($from);
            }

            $infoline = "All Merge Modules successfully analyzed\n";
            push( @installer::globals::logfileinfo, $infoline);

            $installer::globals::mergemodules_analyzed = 1;
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Analyzing MergeModules, stop");

            $infoline = "\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        # 2. Change msi database (has to be done for every msi database -> for every language)
        #   a. Merge msm file into msi database: msidb.exe -d <msifile> -m <mergefile>
        #   b. Extracting tables from msi database: msidb.exe -d <msifile> -f <directory> -e File Media, ...
        #   c. Changing content of msi database in tables: File, Media, Directory, FeatureComponent
        #   d. Including tables into msi database: msidb.exe -d <msifile> -f <directory> -i File Media, ...
        #   e. Copying cabinet file into installation set (later)

        my $counter = 0;
        my $mergemodulegid;
        foreach $mergemodulegid (keys %installer::globals::mergemodules)
        {
            my $mergemodulehash = $installer::globals::mergemodules{$mergemodulegid};
            $counter++;

            installer::logger::include_header_into_logfile("Merging Module: $mergemodulehash->{'name'}");
            installer::logger::print_message( "\t... $mergemodulehash->{'name'} ... \n" );

            $msifilename = installer::converter::make_path_conform($msifilename);
            my $workdir = $msifilename;
            installer::pathanalyzer::get_path_from_fullqualifiedname(\$workdir);

            # changing directory
            my $from = cwd();
            my $to = $workdir;
            chdir($to);

            # Saving original msi database
            installer::systemactions::copy_one_file($msifilename, "$msifilename\.$counter");

            # Merging msm file, this is the "real" merge command

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Before merging database");

            if ( $^O =~ /cygwin/i ) {
                # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                my $localmergemodulepath = $mergemodulehash->{'mergefilepath'};
                my $localmsifilename = $msifilename;
                $localmergemodulepath =~ s/\//\\\\/g;
                $localmsifilename =~ s/\//\\\\/g;
                $systemcall = $msidb . " -d " . $localmsifilename . " -m " . $localmergemodulepath;
            }
            else
            {
                $systemcall = $msidb . " -d " . $msifilename . " -m " . $mergemodulehash->{'mergefilepath'};
            }
            $returnvalue = system($systemcall);

            $infoline = "Systemcall: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

            if ($returnvalue)
            {
                $infoline = "ERROR: Could not execute $systemcall . Returnvalue: $returnvalue!\n";
                push( @installer::globals::logfileinfo, $infoline);
                installer::exiter::exit_program("ERROR: Could not merge msm file into database: $mergemodulehash->{'mergefilepath'} !", "merge_mergemodules_into_msi_database");
            }
            else
            {
                $infoline = "Success: Executed $systemcall successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: After merging database");

            # Saving original idt files
            if ( -f "File.idt" ) { installer::systemactions::rename_one_file("File.idt", "old.File.idt.$counter"); }
            if ( -f "Media.idt" ) { installer::systemactions::rename_one_file("Media.idt", "old.Media.idt.$counter"); }
            if ( -f "Directory.idt" ) { installer::systemactions::rename_one_file("Directory.idt", "old.Directory.idt.$counter"); }
            if ( -f "Director.idt" ) { installer::systemactions::rename_one_file("Director.idt", "old.Director.idt.$counter"); }
            if ( -f "FeatureComponents.idt" ) { installer::systemactions::rename_one_file("FeatureComponents.idt", "old.FeatureComponents.idt.$counter"); }
            if ( -f "FeatureC.idt" ) { installer::systemactions::rename_one_file("FeatureC.idt", "old.FeatureC.idt.$counter"); }
            if ( -f "MsiAssembly.idt" ) { installer::systemactions::rename_one_file("MsiAssembly.idt", "old.MsiAssembly.idt.$counter"); }
            if ( -f "MsiAssem.idt" ) { installer::systemactions::rename_one_file("MsiAssem.idt", "old.MsiAssem.idt.$counter"); }
            if ( -f "Componen.idt" ) { installer::systemactions::rename_one_file("Componen.idt", "old.Componen.idt.$counter"); }

            # Extracting tables

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Before extracting tables");

            my $workingtables = "File Media Directory FeatureComponents"; # required tables
            # Optional tables can be added now
            if ( $mergemodulehash->{'hasmsiassemblies'} ) { $workingtables = $workingtables . " MsiAssembly"; }
            if ( $mergemodulehash->{'componentcondition'} ) { $workingtables = $workingtables . " Component"; }

            # Table "Feature" has to be exported, but it is not necessary to import it.
            if ( $^O =~ /cygwin/i ) {
                # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                my $localmsifilename = $msifilename;
                my $localworkdir = $workdir;
                $localmsifilename =~ s/\//\\\\/g;
                $localworkdir =~ s/\//\\\\/g;
                $systemcall = $msidb . " -d " . $localmsifilename . " -f " . $localworkdir . " -e " . "Feature " . $workingtables;
            }
            else
            {
                $systemcall = $msidb . " -d " . $msifilename . " -f " . $workdir . " -e " . "Feature " . $workingtables;
            }
            $returnvalue = system($systemcall);

            $infoline = "Systemcall: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

            if ($returnvalue)
            {
                $infoline = "ERROR: Could not execute $systemcall !\n";
                push( @installer::globals::logfileinfo, $infoline);
                installer::exiter::exit_program("ERROR: Could not exclude tables from msi database: $msifilename !", "merge_mergemodules_into_msi_database");
            }
            else
            {
                $infoline = "Success: Executed $systemcall successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: After extracting tables");

            # Using 8+3 table names, that are used, when tables are integrated into database. The export of tables
            # creates idt-files, that have long names.

            if ( -f "Directory.idt" ) { installer::systemactions::rename_one_file("Directory.idt", "Director.idt"); }
            if ( -f "FeatureComponents.idt" ) { installer::systemactions::rename_one_file("FeatureComponents.idt", "FeatureC.idt"); }
            if ( -f "MsiAssembly.idt" ) { installer::systemactions::rename_one_file("MsiAssembly.idt", "MsiAssem.idt"); }
            if ( -f "Component.idt" ) { installer::systemactions::rename_one_file("Component.idt", "Componen.idt"); }

            # Changing content of tables: File, Media, Directory, FeatureComponent, MsiAssembly, Component
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing Media table");
            change_media_table($mergemodulehash, $workdir, $mergemodulegid, $allupdatelastsequences, $allupdatediskids);
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing File table");
            $filesref = change_file_table($mergemodulehash, $workdir, $allupdatesequences, $includepatharrayref, $filesref, $mergemodulegid);
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing FeatureComponent table");
            change_featurecomponent_table($mergemodulehash, $workdir);
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing Directory table");
            change_directory_table($mergemodulehash, $workdir);
            if ( $mergemodulehash->{'hasmsiassemblies'} )
            {
                installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing MsiAssembly table");
                change_msiassembly_table($mergemodulehash, $workdir);
            }

            if ( $mergemodulehash->{'componentcondition'} )
            {
                installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing Component table");
                change_component_table($mergemodulehash, $workdir);
            }

            # msidb.exe does not merge InstallExecuteSequence, AdminExecuteSequence and AdvtExecuteSequence. Instead it creates
            # new tables ModuleInstallExecuteSequence, ModuleAdminExecuteSequence and ModuleAdvtExecuteSequence that need to be
            # merged into the three ExecuteSequences with the following process (also into InstallUISequence.idt).

            # Saving original idt files
            if ( -f "InstallE.idt" ) { installer::systemactions::rename_one_file("InstallE.idt", "old.InstallE.idt.$counter"); }
            if ( -f "InstallU.idt" ) { installer::systemactions::rename_one_file("InstallU.idt", "old.InstallU.idt.$counter"); }
            if ( -f "AdminExe.idt" ) { installer::systemactions::rename_one_file("AdminExe.idt", "old.AdminExe.idt.$counter"); }
            if ( -f "AdvtExec.idt" ) { installer::systemactions::rename_one_file("AdvtExec.idt", "old.AdvtExec.idt.$counter"); }
            if ( -f "ModuleInstallExecuteSequence.idt" ) { installer::systemactions::rename_one_file("ModuleInstallExecuteSequence.idt", "old.ModuleInstallExecuteSequence.idt.$counter"); }
            if ( -f "ModuleAdminExecuteSequence.idt" ) { installer::systemactions::rename_one_file("ModuleAdminExecuteSequence.idt", "old.ModuleAdminExecuteSequence.idt.$counter"); }
            if ( -f "ModuleAdvtExecuteSequence.idt" ) { installer::systemactions::rename_one_file("ModuleAdvtExecuteSequence.idt", "old.ModuleAdvtExecuteSequence.idt.$counter"); }

            # Extracting tables
            my $moduleexecutetables = "ModuleInstallExecuteSequence ModuleAdminExecuteSequence ModuleAdvtExecuteSequence"; # new tables
            my $executetables = "InstallExecuteSequence InstallUISequence AdminExecuteSequence AdvtExecuteSequence"; # tables to be merged


            if ( $^O =~ /cygwin/i ) {
                # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                my $localmsifilename = $msifilename;
                my $localworkdir = $workdir;
                $localmsifilename =~ s/\//\\\\/g;
                $localworkdir =~ s/\//\\\\/g;
                $systemcall = $msidb . " -d " . $localmsifilename . " -f " . $localworkdir . " -e " . "Feature " . $moduleexecutetables;
            }
            else
            {
                $systemcall = $msidb . " -d " . $msifilename . " -f " . $workdir . " -e " . "Feature " . $moduleexecutetables;
            }
            $returnvalue = system($systemcall);

            if ( $^O =~ /cygwin/i ) {
                # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                my $localmsifilename = $msifilename;
                my $localworkdir = $workdir;
                $localmsifilename =~ s/\//\\\\/g;
                $localworkdir =~ s/\//\\\\/g;
                $systemcall = $msidb . " -d " . $localmsifilename . " -f " . $localworkdir . " -e " . "Feature " . $executetables;
            }
            else
            {
                $systemcall = $msidb . " -d " . $msifilename . " -f " . $workdir . " -e " . "Feature " . $executetables;
            }
            $returnvalue = system($systemcall);

            # Using 8+3 table names, that are used, when tables are integrated into database. The export of tables
            # creates idt-files, that have long names.

            if ( -f "InstallExecuteSequence.idt" ) { installer::systemactions::rename_one_file("InstallExecuteSequence.idt", "InstallE.idt"); }
            if ( -f "InstallUISequence.idt" ) { installer::systemactions::rename_one_file("InstallUISequence.idt", "InstallU.idt"); }
            if ( -f "AdminExecuteSequence.idt" ) { installer::systemactions::rename_one_file("AdminExecuteSequence.idt", "AdminExe.idt"); }
            if ( -f "AdvtExecuteSequence.idt" ) { installer::systemactions::rename_one_file("AdvtExecuteSequence.idt", "AdvtExec.idt"); }

            # Merging content of tables ModuleInstallExecuteSequence, ModuleAdminExecuteSequence and ModuleAdvtExecuteSequence
            # into tables InstallExecuteSequence, AdminExecuteSequence and AdvtExecuteSequence
            if ( -f "ModuleInstallExecuteSequence.idt" )
            {
                installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing InstallExecuteSequence table");
                change_executesequence_table($mergemodulehash, $workdir, "InstallE.idt", "ModuleInstallExecuteSequence.idt");
                installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing InstallUISequence table");
                change_executesequence_table($mergemodulehash, $workdir, "InstallU.idt", "ModuleInstallExecuteSequence.idt");
            }

            if ( -f "ModuleAdminExecuteSequence.idt" )
            {
                installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing AdminExecuteSequence table");
                change_executesequence_table($mergemodulehash, $workdir, "AdminExe.idt", "ModuleAdminExecuteSequence.idt");
            }

            if ( -f "ModuleAdvtExecuteSequence.idt" )
            {
                installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing AdvtExecuteSequence table");
                change_executesequence_table($mergemodulehash, $workdir, "AdvtExec.idt", "ModuleAdvtExecuteSequence.idt");
            }

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: All tables edited");

            # Including tables into msi database

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Before including tables");

            if ( $^O =~ /cygwin/i ) {
                # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
                my $localmsifilename = $msifilename;
                my $localworkdir = $workdir;
                $localmsifilename =~ s/\//\\\\/g;
                $localworkdir =~ s/\//\\\\/g;
        foreach $table (split / /, $workingtables . ' ' . $executetables) {
          $systemcall = $msidb . " -d " . $localmsifilename . " -f " . $localworkdir . " -i " . $table;
          my $retval = system($systemcall);
          $infoline = "Systemcall returned $retval: $systemcall\n";
          push( @installer::globals::logfileinfo, $infoline);
          $returnvalue |= $retval;
        }
            }
            else
            {
                $systemcall = $msidb . " -d " . $msifilename . " -f " . $workdir . " -i " . $workingtables. " " . $executetables;
        $returnvalue = system($systemcall);
        $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

            }

            if ($returnvalue)
            {
                $infoline = "ERROR: Could not execute $systemcall !\n";
                push( @installer::globals::logfileinfo, $infoline);
                installer::exiter::exit_program("ERROR: Could not include tables into msi database: $msifilename !", "merge_mergemodules_into_msi_database");
            }
            else
            {
                $infoline = "Success: Executed $systemcall successfully!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: After including tables");

            chdir($from);
        }

        if ( ! $installer::globals::mergefiles_added_into_collector ) { $installer::globals::mergefiles_added_into_collector = 1; } # Now all mergemodules are merged for one language.

        installer::logger::include_timestamp_into_logfile("\nPerformance Info: MergeModule into msi database, stop");
    }

    return $filesref;
}

#########################################################################
# Analyzing the content of the media table.
#########################################################################

sub analyze_media_file
{
    my ($filecontent, $workdir) = @_;

    my %filehash = ();
    my $linecount = 0;
    my $counter = 0;
    my $filename = "Media.idt";

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.+?)\t(.+?)\t(.+?)\t(.+?)\t(.*?)\s*$/ )
        {
            my %line = ();
            # Format: DiskId    LastSequence    DiskPrompt  Cabinet VolumeLabel Source
            $line{'DiskId'} = $1;
            $line{'LastSequence'} = $2;
            $line{'DiskPrompt'} = $3;
            $line{'Cabinet'} = $4;
            $line{'VolumeLabel'} = $5;
            $line{'Source'} = $6;

            $counter++;
            $filehash{$counter} = \%line;
        }
        else
        {
            $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$filename\" in \"$workdir\" (line $linecount) !", "analyze_media_file");
        }
    }

    return \%filehash;
}

#########################################################################
# Setting the DiskID for the new cabinet file
#########################################################################

sub get_diskid
{
    my ($mediafile, $allupdatediskids, $cabfilename) = @_;

    my $diskid = 0;
    my $line;

    if (( $installer::globals::updatedatabase ) && ( exists($allupdatediskids->{$cabfilename}) ))
    {
        $diskid = $allupdatediskids->{$cabfilename};
    }
    else
    {
        foreach $line ( keys %{$mediafile} )
        {
            if ( $mediafile->{$line}->{'DiskId'} > $diskid ) { $diskid = $mediafile->{$line}->{'DiskId'}; }
        }

        $diskid++;
    }

    return $diskid;
}

#########################################################################
# Setting the global LastSequence variable
#########################################################################

sub set_current_last_sequence
{
    my ($mediafile) = @_;

    my $lastsequence = 0;
    my $line;
    foreach $line ( keys %{$mediafile} )
    {
        if ( $mediafile->{$line}->{'LastSequence'} > $lastsequence ) { $lastsequence = $mediafile->{$line}->{'LastSequence'}; }
    }

    $installer::globals::lastsequence_before_merge = $lastsequence;
}

#########################################################################
# Setting the LastSequence for the new cabinet file
#########################################################################

sub get_lastsequence
{
    my ($mergemodulehash, $allupdatelastsequences) = @_;

    my $lastsequence = 0;

    if (( $installer::globals::updatedatabase ) && ( exists($allupdatelastsequences->{$mergemodulehash->{'cabfilename'}}) ))
    {
        $lastsequence = $allupdatelastsequences->{$mergemodulehash->{'cabfilename'}};
    }
    else
    {
        $lastsequence = $installer::globals::lastsequence_before_merge + $mergemodulehash->{'filenumber'};
    }

    return $lastsequence;
}

#########################################################################
# Setting the DiskPrompt for the new cabinet file
#########################################################################

sub get_diskprompt
{
    my ($mediafile) = @_;

    my $diskprompt = "";
    my $line;
    foreach $line ( keys %{$mediafile} )
    {
        if ( exists($mediafile->{$line}->{'DiskPrompt'}) )
        {
            $diskprompt = $mediafile->{$line}->{'DiskPrompt'};
            last;
        }
    }

    return $diskprompt;
}

#########################################################################
# Setting the VolumeLabel for the new cabinet file
#########################################################################

sub get_volumelabel
{
    my ($mediafile) = @_;

    my $volumelabel = "";
    my $line;
    foreach $line ( keys %{$mediafile} )
    {
        if ( exists($mediafile->{$line}->{'VolumeLabel'}) )
        {
            $volumelabel = $mediafile->{$line}->{'VolumeLabel'};
            last;
        }
    }

    return $volumelabel;
}

#########################################################################
# Setting the Source for the new cabinet file
#########################################################################

sub get_source
{
    my ($mediafile) = @_;

    my $source = "";
    my $line;
    foreach $line ( keys %{$mediafile} )
    {
        if ( exists($mediafile->{$line}->{'Source'}) )
        {
            $diskprompt = $mediafile->{$line}->{'Source'};
            last;
        }
    }

    return $source;
}

#########################################################################
# For each Merge Module one new line has to be included into the
# media table.
#########################################################################

sub create_new_media_line
{
    my ($mergemodulehash, $mediafile, $allupdatelastsequences, $allupdatediskids) = @_;

    my $diskid = get_diskid($mediafile, $allupdatediskids, $mergemodulehash->{'cabfilename'});
    my $lastsequence = get_lastsequence($mergemodulehash, $allupdatelastsequences);
    my $diskprompt = get_diskprompt($mediafile);
    my $cabinet = $mergemodulehash->{'cabfilename'};
    my $volumelabel = get_volumelabel($mediafile);
    my $source = get_source($mediafile);

    if ( $installer::globals::include_cab_in_msi ) { $cabinet = "\#" . $cabinet; }

    my $newline = "$diskid\t$lastsequence\t$diskprompt\t$cabinet\t$volumelabel\t$source\n";

    return $newline;
}

#########################################################################
# Setting the last diskid in media table.
#########################################################################

sub get_last_diskid
{
    my ($mediafile) = @_;

    my $lastdiskid = 0;
    my $line;
    foreach $line ( keys %{$mediafile} )
    {
        if ( $mediafile->{$line}->{'DiskId'} > $lastdiskid ) { $lastdiskid = $mediafile->{$line}->{'DiskId'}; }
    }

    return $lastdiskid;
}

#########################################################################
# Setting global variable for last cab file name.
#########################################################################

sub set_last_cabfile_name
{
    my ($mediafile, $lastdiskid) = @_;

    my $line;
    foreach $line ( keys %{$mediafile} )
    {
        if ( $mediafile->{$line}->{'DiskId'} == $lastdiskid ) { $installer::globals::lastcabfilename = $mediafile->{$line}->{'Cabinet'}; }
    }
    my $infoline = "Setting last cabinet file: $installer::globals::lastcabfilename\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#########################################################################
# In the media table the new cabinet file has to be added or the
# number of the last cabinet file has to be increased.
#########################################################################

sub change_media_table
{
    my ( $mergemodulehash, $workdir, $mergemodulegid, $allupdatelastsequences, $allupdatediskids ) = @_;

    my $infoline = "Changing content of table \"Media\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = "Media.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" in \"$workdir\" !", "change_media_table"); }

    my $filecontent = installer::files::read_file($filename);
    my $mediafile = analyze_media_file($filecontent, $workdir);
    set_current_last_sequence($mediafile);

    if ( $installer::globals::fix_number_of_cab_files )
    {
        # Determining the line with the highest sequencenumber. That file needs to be updated.
        my $lastdiskid = get_last_diskid($mediafile);
        if ( $installer::globals::lastcabfilename eq "" ) { set_last_cabfile_name($mediafile, $lastdiskid); }
        my $newmaxsequencenumber = $installer::globals::lastsequence_before_merge + $mergemodulehash->{'filenumber'};

        for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
        {
            if ( $i <= 2 ) { next; }                        # ignoring first three lines
            if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
            if ( ${$filecontent}[$i] =~ /^\s*(\Q$lastdiskid\E\t)\Q$installer::globals::lastsequence_before_merge\E(\t.*)$/ )
            {
                my $start = $1;
                my $final = $2;
                $infoline = "Merge: Old line in media table: ${$filecontent}[$i]\n";
                push( @installer::globals::logfileinfo, $infoline);
                my $newline = $start . $newmaxsequencenumber . $final . "\n";
                ${$filecontent}[$i] = $newline;
                $infoline = "Merge: Changed line in media table: ${$filecontent}[$i]\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
    }
    else
    {
        # the new line is identical for all localized databases, but has to be created for each MergeModule ($mergemodulegid)
        if ( ! exists($installer::globals::merge_media_line{$mergemodulegid}) )
        {
            $installer::globals::merge_media_line{$mergemodulegid} = create_new_media_line($mergemodulehash, $mediafile, $allupdatelastsequences, $allupdatediskids);
        }

        $infoline = "Adding line: $installer::globals::merge_media_line{$mergemodulegid}\n";
        push( @installer::globals::logfileinfo, $infoline);

        # adding new line
        push(@{$filecontent}, $installer::globals::merge_media_line{$mergemodulegid});
    }

    # saving file
    installer::files::save_file($filename, $filecontent);
}

#########################################################################
# Putting the directory table content into a hash.
#########################################################################

sub analyze_directorytable_file
{
    my ($filecontent, $idtfilename) = @_;

    my %dirhash = ();
    # Iterating over the file content
    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.*?)\t(.*?)\s*$/ )
        {
            my %line = ();
            # Format: Directory Directory_Parent    DefaultDir
            $line{'Directory'} = $1;
            $line{'Directory_Parent'} = $2;
            $line{'DefaultDir'} = $3;
            $line{'linenumber'} = $i; # saving also the line number for direct access

            my $uniquekey = $line{'Directory'};
            $dirhash{$uniquekey} = \%line;
        }
        else
        {
            my $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "analyze_directorytable_file");
        }
    }

    return \%dirhash;
}

#########################################################################
# Putting the msi assembly table content into a hash.
#########################################################################

sub analyze_msiassemblytable_file
{
    my ($filecontent, $idtfilename) = @_;

    my %assemblyhash = ();
    # Iterating over the file content
    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.+?)\t(.+?)\t(.*?)\t(.*?)\s*$/ )
        {
            my %line = ();
            # Format: Component_    Feature_    File_Manifest   File_Application    Attributes
            $line{'Component'} = $1;
            $line{'Feature'} = $2;
            $line{'File_Manifest'} = $3;
            $line{'File_Application'} = $4;
            $line{'Attributes'} = $5;
            $line{'linenumber'} = $i; # saving also the line number for direct access

            my $uniquekey = $line{'Component'};
            $assemblyhash{$uniquekey} = \%line;
        }
        else
        {
            my $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "analyze_msiassemblytable_file");
        }
    }

    return \%assemblyhash;
}

#########################################################################
# Putting the file table content into a hash.
#########################################################################

sub analyze_filetable_file
{
    my ( $filecontent, $idtfilename ) = @_;

    my %filehash = ();
    # Iterating over the file content
    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.+?)\t(.+?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.+?)\s*$/ )
        {
            my %line = ();
            # Format: File  Component_  FileName    FileSize    Version Language    Attributes  Sequence
            $line{'File'} = $1;
            $line{'Component'} = $2;
            $line{'FileName'} = $3;
            $line{'FileSize'} = $4;
            $line{'Version'} = $5;
            $line{'Language'} = $6;
            $line{'Attributes'} = $7;
            $line{'Sequence'} = $8;
            $line{'linenumber'} = $i; # saving also the line number for direct access

            my $uniquekey = $line{'File'};
            $filehash{$uniquekey} = \%line;
        }
        else
        {
            my $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "analyze_filetable_file");
        }
    }

    return \%filehash;
}

#########################################################################
# Creating a new line for the directory table.
#########################################################################

sub get_new_line_for_directory_table
{
    my ($dir) = @_;

    my $newline = "$dir->{'Directory'}\t$dir->{'Directory_Parent'}\t$dir->{'DefaultDir'}\n";

    return $newline;
}

#########################################################################
# Creating a new line for the file table.
#########################################################################

sub get_new_line_for_file_table
{
    my ($file) = @_;

    my $newline = "$file->{'File'}\t$file->{'Component'}\t$file->{'FileName'}\t$file->{'FileSize'}\t$file->{'Version'}\t$file->{'Language'}\t$file->{'Attributes'}\t$file->{'Sequence'}\n";

    return $newline;
}

#########################################################################
# Creating a new line for the msiassembly table.
#########################################################################

sub get_new_line_for_msiassembly_table
{
    my ($assembly) = @_;

    my $newline = "$assembly->{'Component'}\t$assembly->{'Feature'}\t$assembly->{'File_Manifest'}\t$assembly->{'File_Application'}\t$assembly->{'Attributes'}\n";

    return $newline;
}

#########################################################################
# Sorting the files collector, if there are files, following
# the merge module files.
#########################################################################

sub sort_files_collector_for_sequence
{
    my ($filesref) = @_;

    my @sortarray = ();
    my %helphash = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        if ( ! exists($onefile->{'sequencenumber'}) ) { installer::exiter::exit_program("ERROR: Could not find sequencenumber for file: $onefile->{'uniquename'} !", "sort_files_collector_for_sequence"); }
        my $sequence = $onefile->{'sequencenumber'};
        $helphash{$sequence} = $onefile;
    }

    foreach my $seq ( sort { $a <=> $b } keys %helphash ) { push(@sortarray, $helphash{$seq}); }

    return \@sortarray;
}

#########################################################################
# In the file table "Sequence" and "Attributes" have to be changed.
#########################################################################

sub change_file_table
{
    my ($mergemodulehash, $workdir, $allupdatesequenceshashref, $includepatharrayref, $filesref, $mergemodulegid) = @_;

    my $infoline = "Changing content of table \"File\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $idtfilename = "File.idt";
    if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "change_file_table"); }

    my $filecontent = installer::files::read_file($idtfilename);

    # If File.idt needed to be removed before the msm database was merged into the msi database,
    # now it is time to add the content into File.idt
    if ( $mergemodulehash->{'removefiletable'} )
    {
        for ( my $i = 0; $i <= $#{$mergemodulehash->{'fileidtcontent'}}; $i++ )
        {
            push(@{$filecontent}, ${$mergemodulehash->{'fileidtcontent'}}[$i]);
        }
    }

    # Unpacking the MergeModule.CABinet (only once)
    # Unpacking into temp directory. Warning: expand.exe has problems with very long unpack directories.

    my $unpackdir = installer::systemactions::create_directories("cab", "");
    push(@installer::globals::removedirs, $unpackdir);
    $unpackdir = $unpackdir . $installer::globals::separator . $mergemodulegid;

    my %newfileshash = ();
    if (( $installer::globals::fix_number_of_cab_files ) && ( ! $installer::globals::mergefiles_added_into_collector ))
    {
        if ( ! -d $unpackdir ) { installer::systemactions::create_directory($unpackdir); }

        # changing directory
        my $from = cwd();
        my $to = $mergemodulehash->{'workdir'};
         if ( $^O =~ /cygwin/i ) {
            $to = qx(cygpath -u "$to");
            chomp $to;
        }

        chdir($to) || die "Could not chdir to \"$to\"\n";

        # Unpack the cab file, so that in can be included into the last office cabinet file.
        # Not using cabarc.exe from cabsdk for unpacking cabinet files, but "expand.exe" that
        # should be available on every Windows system.

        $infoline = "Unpacking cabinet file: $mergemodulehash->{'cabinetfile'}\n";
        push( @installer::globals::logfileinfo, $infoline);

        # Avoid the Cygwin expand command
        my $expandfile = "expand.exe";  # Has to be in the path
         if ( $^O =~ /cygwin/i ) {
            $expandfile = qx(cygpath -u "$ENV{WINDIR}"/System32/expand.exe);
            chomp $expandfile;
        }

        my $cabfilename = "MergeModule.CABinet";

        my $systemcall = "";
        if ( $^O =~ /cygwin/i ) {
            my $localunpackdir = qx(cygpath -m "$unpackdir");
            chomp $localunpackdir;
            $systemcall = $expandfile . " " . $cabfilename . " -F:\\\* " . $localunpackdir;
        }
        else
        {
            $systemcall = $expandfile . " " . $cabfilename . " -F:\* " . $unpackdir . " 2\>\&1";
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

        chdir($from);
    }

    # For performance reasons creating a hash with file names and rows
    # The content of File.idt is changed after every merge -> content cannot be saved in global hash
    $merge_filetablehashref = analyze_filetable_file($filecontent, $idtfilename);

    my $attributes = "16384"; # Always

    my $filename;
    foreach $filename (keys %{$mergemodulehash->{'mergefilesequence'}} )
    {
        my $mergefilesequence = $mergemodulehash->{'mergefilesequence'}->{$filename};

        if ( ! exists($merge_filetablehashref->{$filename}) ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" in \"$idtfilename\" !", "change_file_table"); }
        my $filehash = $merge_filetablehashref->{$filename};
        my $linenumber = $filehash->{'linenumber'};

        # <- this line has to be changed concerning "Sequence" and "Attributes"
        $filehash->{'Attributes'} = $attributes;

        # If this is an update process, the sequence numbers have to be reused.
        if ( $installer::globals::updatedatabase )
        {
            if ( ! exists($allupdatesequenceshashref->{$filehash->{'File'}}) ) { installer::exiter::exit_program("ERROR: Sequence not defined for file \"$filehash->{'File'}\" !", "change_file_table"); }
            $filehash->{'Sequence'} = $allupdatesequenceshashref->{$filehash->{'File'}};
            # Saving all mergemodule sequence numbers. This is important for creating ddf files
            $installer::globals::allmergemodulefilesequences{$filehash->{'Sequence'}} = 1;
        }
        else
        {
            # Important saved data: $installer::globals::lastsequence_before_merge.
            # This mechanism keeps the correct order inside the new cabinet file.
            $filehash->{'Sequence'} = $filehash->{'Sequence'} + $installer::globals::lastsequence_before_merge;
        }

        my $oldline = ${$filecontent}[$linenumber];
        my $newline = get_new_line_for_file_table($filehash);
        ${$filecontent}[$linenumber] = $newline;

        $infoline = "Merge, replacing line:\n";
        push( @installer::globals::logfileinfo, $infoline);
        $infoline = "Old: $oldline\n";
        push( @installer::globals::logfileinfo, $infoline);
        $infoline = "New: $newline\n";
        push( @installer::globals::logfileinfo, $infoline);

        # Adding files to the files collector (but only once)
        if (( $installer::globals::fix_number_of_cab_files ) && ( ! $installer::globals::mergefiles_added_into_collector ))
        {
            # If the number of cabinet files is kept constant,
            # all files from the mergemodule cabinet files will
            # be integrated into the last office cabinet file
            # (installer::globals::lastcabfilename).
            # Therefore the files must now be added to the filescollector,
            # so that they will be integrated into the ddf files.

            # Problem with very long filenames -> copying to shorter filenames
            my $newfilename = "f" . $filehash->{'Sequence'};
            my $completesource = $unpackdir . $installer::globals::separator . $filehash->{'File'};
            my $completedest = $unpackdir . $installer::globals::separator . $newfilename;
            installer::systemactions::copy_one_file($completesource, $completedest);

            my $locallastcabfilename = $installer::globals::lastcabfilename;
            if ( $locallastcabfilename =~ /^\s*\#/ ) { $locallastcabfilename =~ s/^\s*\#//; }  # removing beginning hashes

            # Create new file hash for file collector
            my %newfile = ();
            $newfile{'sequencenumber'} = $filehash->{'Sequence'};
            $newfile{'assignedsequencenumber'} = $filehash->{'Sequence'};
            $newfile{'cabinet'} = $locallastcabfilename;
            $newfile{'sourcepath'} = $completedest;
            $newfile{'componentname'} = $filehash->{'Component'};
            $newfile{'uniquename'} = $filehash->{'File'};
            $newfile{'Name'} = $filehash->{'File'};

            # Saving in globals sequence hash
            $installer::globals::uniquefilenamesequence{$filehash->{'File'}} = $filehash->{'Sequence'};

            if ( ! -f $newfile{'sourcepath'} ) { installer::exiter::exit_program("ERROR: File \"$newfile{'sourcepath'}\" must exist!", "change_file_table"); }

            # Collecting all new files. Attention: This files must be included into files collector in correct order!
            $newfileshash{$filehash->{'Sequence'}} = \%newfile;
            # push(@{$filesref}, \%newfile); -> this is not the correct order
        }
    }

    # Now the files can be added to the files collector
    # In the case of an update process, there can be new files, that have to be added after the merge module files.
    # Warning: In multilingual installation sets, the files only have to be added once to the files collector!

    if ( ! $installer::globals::mergefiles_added_into_collector )
    {
        foreach my $localsequence ( sort { $a <=> $b } keys %newfileshash ) { push(@{$filesref}, $newfileshash{$localsequence}); }
        if ( $installer::globals::newfilesexist ) { $filesref = sort_files_collector_for_sequence($filesref); }
        # $installer::globals::mergefiles_added_into_collector = 1; -> Not yet. Only if all mergemodules are merged for one language.
    }

    # Saving the idt file (for every language)
    installer::files::save_file($idtfilename, $filecontent);

    return $filesref;
}

#########################################################################
# Reading the file "Director.idt". The Directory, that is defined in scp
# has to be defined in this table.
#########################################################################

sub collect_directories
{
    my $idtfilename = "Director.idt";
    my $filecontent = installer::files::read_file($idtfilename);

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        # Format: Directory Directory_Parent    DefaultDir
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.*?)\t(.*?)\s*$/ )
        {
            $installer::globals::merge_alldirectory_hash{$1} = 1;
        }
        else
        {
            my $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "collect_directories");
        }
    }
}

#########################################################################
# Reading the file "Feature.idt". The Feature, that is defined in scp
# has to be defined in this table.
#########################################################################

sub collect_feature
{
    my $idtfilename = "Feature.idt";
    if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "collect_feature"); }
    my $filecontent = installer::files::read_file($idtfilename);

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        # Format: Feature   Feature_Parent  Title   Description Display Level   Directory_  Attributes
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            $installer::globals::merge_allfeature_hash{$1} = 1;
        }
        else
        {
            my $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "collect_feature");
        }
    }
}

#########################################################################
# In the featurecomponent table, the new connections have to be added.
#########################################################################

sub change_featurecomponent_table
{
    my ($mergemodulehash, $workdir) = @_;

    my $infoline = "Changing content of table \"FeatureComponents\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $idtfilename = "FeatureC.idt";
    if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "change_featurecomponent_table"); }

    my $filecontent = installer::files::read_file($idtfilename);

    # Simply adding for each new component one line. The Feature has to be defined in scp project.
    my $feature = $mergemodulehash->{'feature'};

    if ( ! $installer::globals::mergefeaturecollected )
    {
        collect_feature(); # putting content into hash %installer::globals::merge_allfeature_hash
        $installer::globals::mergefeaturecollected = 1;
    }

    if ( ! exists($installer::globals::merge_allfeature_hash{$feature}) )
    {
        installer::exiter::exit_program("ERROR: Unknown feature defined in scp: \"$feature\" . Not defined in table \"Feature\" !", "change_featurecomponent_table");
    }

    my $component;
    foreach $component ( keys %{$mergemodulehash->{'componentnames'}} )
    {
        my $line = "$feature\t$component\n";
        push(@{$filecontent}, $line);
        $infoline = "Adding line: $line\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    # saving file
    installer::files::save_file($idtfilename, $filecontent);
}

###############################################################################
# In the components table, the conditions of merge modules should be updated
###############################################################################

sub change_component_table
{
    my ($mergemodulehash, $workdir) = @_;

    my $infoline = "Changing content of table \"Component\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $idtfilename = "Componen.idt";
    if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "change_component_table"); }

    my $filecontent = installer::files::read_file($idtfilename);

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $component;
        foreach $component ( keys %{$mergemodulehash->{'componentnames'}} )
        {
            if ( ${$filecontent}[$i] =~ /^\s*$component/)
            {
                if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.*?)\t(.+?)\t(.+?)\t(.*?)\t(.*?)\s*$/ )
                {
                    $infoline = "Adding condition ($mergemodulehash->{'componentcondition'}) from scp2 to component $1\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    if ($5)
                    {
                        $infoline = "Old condition: $5\nNew condition: ($5) AND ($mergemodulehash->{'componentcondition'})\n";
                        push( @installer::globals::logfileinfo, $infoline);
                        ${$filecontent}[$i] = "$1\t$2\t$3\t$4\t($5) AND ($mergemodulehash->{'componentcondition'})\t$6\n";
                    }
                    else
                    {
                        $infoline = "Old condition: <none>\nNew condition: $mergemodulehash->{'componentcondition'}\n";
                        push( @installer::globals::logfileinfo, $infoline);
                        ${$filecontent}[$i] = "$1\t$2\t$3\t$4\t$mergemodulehash->{'componentcondition'}\t$6\n";
                    }
                }
            }
        }
    }

    # saving file
    installer::files::save_file($idtfilename, $filecontent);
}

#########################################################################
# In the directory table, the directory parent has to be changed,
# if it is not TARGETDIR.
#########################################################################

sub change_directory_table
{
    my ($mergemodulehash, $workdir) = @_;

    # directory for MergeModule has to be defined in scp project
    my $scpdirectory = $mergemodulehash->{'rootdir'};

    if ( $scpdirectory ne "TARGETDIR" )  # TARGETDIR works fine, when using msidb.exe
    {
        my $infoline = "Changing content of table \"Directory\"\n";
        push( @installer::globals::logfileinfo, $infoline);

        my $idtfilename = "Director.idt";
        if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "change_directory_table"); }

        my $filecontent = installer::files::read_file($idtfilename);

        if ( ! $installer::globals::mergedirectoriescollected )
        {
            collect_directories(); # putting content into %installer::globals::merge_alldirectory_hash, only first column!
            $installer::globals::mergedirectoriescollected = 1;
        }

        if ( ! exists($installer::globals::merge_alldirectory_hash{$scpdirectory}) )
        {
            installer::exiter::exit_program("ERROR: Unknown directory defined in scp: \"$scpdirectory\" . Not defined in table \"Directory\" !", "change_directory_table");
        }

        # If the definition in scp is okay, now the complete content of "Director.idt" can be analyzed
        my $merge_directorytablehashref = analyze_directorytable_file($filecontent, $idtfilename);

        my $directory;
        foreach $directory (keys %{$mergemodulehash->{'mergedirectories'}} )
        {
            if ( ! exists($merge_directorytablehashref->{$directory}) ) { installer::exiter::exit_program("ERROR: Could not find directory \"$directory\" in \"$idtfilename\" !", "change_directory_table"); }
            my $dirhash = $merge_directorytablehashref->{$directory};
            my $linenumber = $dirhash->{'linenumber'};

            # <- this line has to be changed concerning "Directory_Parent",
            # if the current value is "TARGETDIR", which is the default value from msidb.exe

            if ( $dirhash->{'Directory_Parent'} eq "TARGETDIR" )
            {
                $dirhash->{'Directory_Parent'} = $scpdirectory;

                my $oldline = ${$filecontent}[$linenumber];
                my $newline = get_new_line_for_directory_table($dirhash);
                ${$filecontent}[$linenumber] = $newline;

                $infoline = "Merge, replacing line:\n";
                push( @installer::globals::logfileinfo, $infoline);
                $infoline = "Old: $oldline\n";
                push( @installer::globals::logfileinfo, $infoline);
                $infoline = "New: $newline\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }

        # saving file
        installer::files::save_file($idtfilename, $filecontent);
    }
}

#########################################################################
# In the msiassembly table, the feature has to be changed.
#########################################################################

sub change_msiassembly_table
{
    my ($mergemodulehash, $workdir) = @_;

    my $infoline = "Changing content of table \"MsiAssembly\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $idtfilename = "MsiAssem.idt";
    if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "change_msiassembly_table"); }

    my $filecontent = installer::files::read_file($idtfilename);

    # feature has to be defined in scp project
    my $feature = $mergemodulehash->{'feature'};

    if ( ! $installer::globals::mergefeaturecollected )
    {
        collect_feature();  # putting content into hash %installer::globals::merge_allfeature_hash
        $installer::globals::mergefeaturecollected = 1;
    }

    if ( ! exists($installer::globals::merge_allfeature_hash{$feature}) )
    {
        installer::exiter::exit_program("ERROR: Unknown feature defined in scp: \"$feature\" . Not defined in table \"Feature\" !", "change_msiassembly_table");
    }

    my $merge_msiassemblytablehashref = analyze_msiassemblytable_file($filecontent, $idtfilename);

    my $component;
    foreach $component (keys %{$mergemodulehash->{'mergeassemblies'}} )
    {
        if ( ! exists($merge_msiassemblytablehashref->{$component}) ) { installer::exiter::exit_program("ERROR: Could not find component \"$component\" in \"$idtfilename\" !", "change_msiassembly_table"); }
        my $assemblyhash = $merge_msiassemblytablehashref->{$component};
        my $linenumber = $assemblyhash->{'linenumber'};

        # <- this line has to be changed concerning "Feature"
        $assemblyhash->{'Feature'} = $feature;

        my $oldline = ${$filecontent}[$linenumber];
        my $newline = get_new_line_for_msiassembly_table($assemblyhash);
        ${$filecontent}[$linenumber] = $newline;

        $infoline = "Merge, replacing line:\n";
        push( @installer::globals::logfileinfo, $infoline);
        $infoline = "Old: $oldline\n";
        push( @installer::globals::logfileinfo, $infoline);
        $infoline = "New: $newline\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    # saving file
    installer::files::save_file($idtfilename, $filecontent);
}

#########################################################################
# Creating file content hash
#########################################################################

sub make_executeidtcontent_hash
{
    my ($filecontent, $idtfilename) = @_;

    my %newhash = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        # Format for all sequence tables: Action    Condition   Sequence
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.*?)\t(.*?)\s*$/ )
        {
            my %onehash = ();
            $onehash{'Action'} = $1;
            $onehash{'Condition'} = $2;
            $onehash{'Sequence'} = $3;
            $newhash{$onehash{'Action'}} = \%onehash;
        }
        else
        {
            my $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "make_executeidtcontent_hash");
        }
    }

    return \%newhash;
}

#########################################################################
# Creating file content hash
#########################################################################

sub make_moduleexecuteidtcontent_hash
{
    my ($filecontent, $idtfilename) = @_;

    my %newhash = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i <= 2 ) { next; }                        # ignoring first three lines
        if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
        # Format for all module sequence tables: Action Sequence    BaseAction  After Condition
        if ( ${$filecontent}[$i] =~ /^\s*(.+?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my %onehash = ();
            $onehash{'Action'} = $1;
            $onehash{'Sequence'} = $2;
            $onehash{'BaseAction'} = $3;
            $onehash{'After'} = $4;
            $onehash{'Condition'} = $5;
            $newhash{$onehash{'Action'}} = \%onehash;
        }
        else
        {
            my $linecount = $i + 1;
            installer::exiter::exit_program("ERROR: Unknown line format in table \"$idtfilename\" (line $linecount) !", "make_executeidtcontent_hash");
        }
    }

    return \%newhash;
}

#########################################################################
# ExecuteSequence tables need to be merged with
# ModuleExecuteSequence tables created by msidb.exe.
#########################################################################

sub change_executesequence_table
{
    my ($mergemodulehash, $workdir, $idtfilename, $moduleidtfilename) = @_;

    my $infoline = "Changing content of table \"$idtfilename\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "change_executesequence_table"); }
    if ( ! -f $moduleidtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$moduleidtfilename\" in \"$workdir\" !", "change_executesequence_table"); }

    # Reading file content
    my $idtfilecontent = installer::files::read_file($idtfilename);
    my $moduleidtfilecontent = installer::files::read_file($moduleidtfilename);

    # Converting to hash
    my $idtcontenthash = make_executeidtcontent_hash($idtfilecontent, $idtfilename);
    my $moduleidtcontenthash = make_moduleexecuteidtcontent_hash($moduleidtfilecontent, $moduleidtfilename);

    # Merging
    foreach my $action ( keys %{$moduleidtcontenthash} )
    {
        if ( exists($idtcontenthash->{$action}) ) { next; } # Action already exists, can be ignored

        if (( $idtfilename eq "InstallU.idt" ) && ( ! ( $action =~ /^\s*WindowsFolder\./ ))) { next; } # Only "WindowsFolder.*" CustomActions for UI Sequence table

        my $actionhashref = $moduleidtcontenthash->{$action};
        if ( $actionhashref->{'Sequence'} ne "" )
        {
            # Format for all sequence tables: Action Condition Sequence
            my $newline = $actionhashref->{'Action'} . "\t" . $actionhashref->{'Condition'} . "\t" . $actionhashref->{'Sequence'} . "\n";
            # Adding to table
            push(@{$idtfilecontent}, $newline);
            # Also adding to hash
            my %idttablehash = ();
            $idttablehash{'Action'} = $actionhashref->{'Action'};
            $idttablehash{'Condition'} = $actionhashref->{'Condition'};
            $idttablehash{'Sequence'} = $actionhashref->{'Sequence'};
            $idtcontenthash->{$action} = \%idttablehash;

        }
        else    # no sequence defined, using syntax "BaseAction" and "After"
        {
            my $baseactionname = $actionhashref->{'BaseAction'};
            # If this baseactionname is not defined in execute idt file, it is not possible to merge
            if ( ! exists($idtcontenthash->{$baseactionname}) ) { installer::exiter::exit_program("ERROR: Merge problem: Could not find action \"$baseactionname\" in file \"$idtfilename\" !", "change_executesequence_table"); }

            my $baseaction = $idtcontenthash->{$baseactionname};
            my $sequencenumber = $baseaction->{'Sequence'};
            if ( $actionhashref->{'After'} == 1 ) { $sequencenumber = $sequencenumber + 1; }
            else { $sequencenumber = $sequencenumber - 1; }

            # Format for all sequence tables: Action Condition Sequence
            my $newline = $actionhashref->{'Action'} . "\t" . $actionhashref->{'Condition'} . "\t" . $sequencenumber . "\n";
            # Adding to table
            push(@{$idtfilecontent}, $newline);
            # Also adding to hash
            my %idttablehash = ();
            $idttablehash{'Action'} = $actionhashref->{'Action'};
            $idttablehash{'Condition'} = $actionhashref->{'Condition'};
            $idttablehash{'Sequence'} = $sequencenumber;
            $idtcontenthash->{$action} = \%idttablehash;
        }
    }

    # saving file
    installer::files::save_file($idtfilename, $idtfilecontent);
}


1;
