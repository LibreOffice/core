#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: mergemodule.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: ihi $ $Date: 2007-11-23 13:34:23 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
    my ($mergemodules, $msifilename, $languagestringref, $language, $languagefile, $allvariables, $includepatharrayref) = @_;

    my $domerge = 0;
    if (( $#{$mergemodules} > -1 ) && ( ! $installer::globals::patch ) && ( ! $installer::globals::languagepack )) { $domerge = 1; }

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
            # push(@installer::globals::removedirs, $mergemoduledir);

            my $mergemodule;
            foreach $mergemodule ( @{$mergemodules} )
            {
                my $filename = $mergemodule->{'Name'};
                my $mergefile = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);

                if ( ! -f $$mergefile ) { installer::exiter::exit_program("ERROR: msm file not found: $$mergefile !", "merge_mergemodules_into_msi_database"); }
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

                # $systemcall = $msidb . " -d " . $filename . " -f " . $workdir . " -e File Component MsiAssembly Directory";
                $systemcall = $msidb . " -d " . $filename . " -f " . $workdir . " -e \*";
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
                my $filecounter = 0;
                my %mergefilesequence = ();
                for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
                {
                    if ( $i <= 2 ) { next; }                        # ignoring first three lines
                    if ( ${$filecontent}[$i] =~ /^\s*$/ ) { next; } # ignoring empty lines
                    $filecounter++;
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
                installer::packagelist::resolve_packagevariables(\$cabfilename, $allvariables, 0);

                # Saving MergeModule info

                my %onemergemodulehash = ();
                $onemergemodulehash{'mergefilepath'} = $completedest;
                $onemergemodulehash{'cabinetfile'} = $workdir . $installer::globals::separator . $cabinetfile;
                $onemergemodulehash{'filenumber'} = $filecounter;
                $onemergemodulehash{'componentnames'} = \%componentnames;
                $onemergemodulehash{'cabfilename'} = $cabfilename;
                $onemergemodulehash{'feature'} = $mergemodule->{'Feature'};
                $onemergemodulehash{'rootdir'} = $mergemodule->{'RootDir'};
                $onemergemodulehash{'name'} = $mergemodule->{'Name'};
                $onemergemodulehash{'mergefilesequence'} = \%mergefilesequence;
                $onemergemodulehash{'mergeassemblies'} = \%mergeassemblies;
                $onemergemodulehash{'mergedirectories'} = \%mergedirectories;
                $onemergemodulehash{'hasmsiassemblies'} = $hasmsiassemblies;

                $installer::globals::mergemodules{$mergegid} = \%onemergemodulehash;

                # Collecting all msm files, to copy them into installation set
                $installer::globals::copy_msm_files{$cabfilename} = $onemergemodulehash{'cabinetfile'};

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

            $systemcall = $msidb . " -d " . $msifilename . " -m " . $mergemodulehash->{'mergefilepath'};
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
            if ( -f "File.idt" ) { installer::systemactions::rename_one_file("File.idt", "File.idt.$counter"); }
            if ( -f "Media.idt" ) { installer::systemactions::rename_one_file("Media.idt", "Media.idt.$counter"); }
            if ( -f "Directory.idt" ) { installer::systemactions::rename_one_file("Directory.idt", "Directory.idt.$counter"); }
            if ( -f "Director.idt" ) { installer::systemactions::rename_one_file("Director.idt", "Director.idt.$counter"); }
            if ( -f "FeatureComponents.idt" ) { installer::systemactions::rename_one_file("FeatureComponents.idt", "FeatureComponents.idt.$counter"); }
            if ( -f "FeatureC.idt" ) { installer::systemactions::rename_one_file("FeatureC.idt", "FeatureC.idt.$counter"); }
            if ( -f "MsiAssembly.idt" ) { installer::systemactions::rename_one_file("MsiAssembly.idt", "MsiAssembly.idt.$counter"); }
            if ( -f "MsiAssem.idt" ) { installer::systemactions::rename_one_file("MsiAssem.idt", "MsiAssem.idt.$counter"); }

            # Extracting tables

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Before extracting tables");

            my $workingtables = "File Media Directory FeatureComponents"; # required tables
            # Optional tables can be added now
            if ( $mergemodulehash->{'hasmsiassemblies'} ) { $workingtables = $workingtables . " MsiAssembly"; }

            # Table "Feature" has to be exported, but it is not necessary to import it.
            $systemcall = $msidb . " -d " . $msifilename . " -f " . $workdir . " -e " . "Feature " . $workingtables;
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

            # Changing content of tables: File, Media, Directory, FeatureComponent, MsiAssembly
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing Media table");
            change_media_table($mergemodulehash, $workdir, $mergemodulegid);
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing File table");
            change_file_table($mergemodulehash, $workdir);
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing FeatureComponent table");
            change_featurecomponent_table($mergemodulehash, $workdir);
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing Directory table");
            change_directory_table($mergemodulehash, $workdir);
            if ( $mergemodulehash->{'hasmsiassemblies'} )
            {
                installer::logger::include_timestamp_into_logfile("\nPerformance Info: Changing MsiAssembly table");
                change_msiassembly_table($mergemodulehash, $workdir);
            }
            installer::logger::include_timestamp_into_logfile("\nPerformance Info: All tables edited");

            # Including tables into msi database

            installer::logger::include_timestamp_into_logfile("\nPerformance Info: Before including tables");

            $systemcall = $msidb . " -d " . $msifilename . " -f " . $workdir . " -i " . $workingtables;
            $returnvalue = system($systemcall);

            $infoline = "Systemcall: $systemcall\n";
            push( @installer::globals::logfileinfo, $infoline);

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

        installer::logger::include_timestamp_into_logfile("\nPerformance Info: MergeModule into msi database, stop");
    }
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
    my ($mediafile) = @_;

    my $diskid = 0;
    my $line;
    foreach $line ( keys %{$mediafile} )
    {
        if ( $mediafile->{$line}->{'DiskId'} > $diskid ) { $diskid = $mediafile->{$line}->{'DiskId'}; }
    }

    $diskid++;
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
    my ($mergemodulehash) = @_;

    my $lastsequence = $installer::globals::lastsequence_before_merge + $mergemodulehash->{'filenumber'};

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
    my ($mergemodulehash, $mediafile) = @_;

    my $diskid = get_diskid($mediafile);
    my $lastsequence = get_lastsequence($mergemodulehash);
    my $diskprompt = get_diskprompt($mediafile);
    my $cabinet = $mergemodulehash->{'cabfilename'};
    my $volumelabel = get_volumelabel($mediafile);
    my $source = get_source($mediafile);

    if ( $installer::globals::include_cab_in_msi ) { $cabinet = "\#" . $cabinet; }

    my $newline = "$diskid\t$lastsequence\t$diskprompt\t$cabinet\t$volumelabel\t$source\n";

    return $newline;
}

#########################################################################
# In the media table the new cabinet file has to be added.
#########################################################################

sub change_media_table
{
    my ( $mergemodulehash, $workdir, $mergemodulegid ) = @_;

    my $infoline = "Changing content of table \"Media\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = "Media.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" in \"$workdir\" !", "change_media_table"); }

    my $filecontent = installer::files::read_file($filename);
    my $mediafile = analyze_media_file($filecontent, $workdir);
    set_current_last_sequence($mediafile);

    # the new line is identical for all localized databases, but has to be created for each MergeModule ($mergemodulegid)
    if ( ! exists($installer::globals::merge_media_line{$mergemodulegid}) )
    {
        $installer::globals::merge_media_line{$mergemodulegid} = create_new_media_line($mergemodulehash, $mediafile);
    }

    $infoline = "Adding line: $installer::globals::merge_media_line{$mergemodulegid}\n";
    push( @installer::globals::logfileinfo, $infoline);

    # adding new line
    push(@{$filecontent}, $installer::globals::merge_media_line{$mergemodulegid});

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
# In the file table "Sequence" and "Attributes" have to be chagned.
#########################################################################

sub change_file_table
{
    my ($mergemodulehash, $workdir) = @_;

    my $infoline = "Changing content of table \"File\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $idtfilename = "File.idt";
    if ( ! -f $idtfilename ) { installer::exiter::exit_program("ERROR: Could not find file \"$idtfilename\" in \"$workdir\" !", "change_file_table"); }

    my $filecontent = installer::files::read_file($idtfilename);
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
        # Important saved data: $installer::globals::lastsequence_before_merge.
        # This mechanism keeps the correct order inside the new cabinet file.
        $filehash->{'Sequence'} = $filehash->{'Sequence'} + $installer::globals::lastsequence_before_merge;

        my $oldline = ${$filecontent}[$linenumber];
        my $newline = get_new_line_for_file_table($filehash);
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

1;
