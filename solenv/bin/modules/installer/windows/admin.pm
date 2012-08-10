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
    if ( $installer::globals::isunix )
    {
        $infoline = "ERROR: We need to change this to use cabextract instead of expand.exe\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

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
# Include tables into a msi database
#################################################################################

sub include_tables_into_pcpfile
{
    my ($fullmsidatabasepath, $workdir, $tables) = @_;

    my $msidb = "msidb.exe";    # Has to be in the path
    if ( $installer::globals::isunix )
    {
        $msidb = "$ENV{'OUTDIR_FOR_BUILD'}/bin/msidb.exe";
    }
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";

    # Make all table 8+3 conform
    my $alltables = installer::converter::convert_stringlist_into_array(\$tables, " ");

    for ( my $i = 0; $i <= $#{$alltables}; $i++ )
    {
        my $tablename = ${$alltables}[$i];
        $tablename =~ s/\s*$//;
        my $namelength = length($tablename);
        if ( $namelength > 8 )
        {
            my $newtablename = substr($tablename, 0, 8);    # name, offset, length
            my $oldfile = $workdir . $installer::globals::separator . $tablename . ".idt";
            my $newfile = $workdir . $installer::globals::separator . $newtablename . ".idt";
            if ( -f $newfile ) { unlink $newfile; }
            installer::systemactions::copy_one_file($oldfile, $newfile);
            my $savfile = $oldfile . ".orig";
            installer::systemactions::copy_one_file($oldfile, $savfile);
        }
    }

    # Import of tables

    $systemcall = $msidb . " -d " . $fullmsidatabasepath . " -f " . $workdir . " -i " . $tables;

    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program("ERROR: Could not include tables into msi database: $fullmsidatabasepath !", "include_tables_into_pcpfile");
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
    if ( $installer::globals::isunix )
    {
        $msidb = "$ENV{'OUTDIR_FOR_BUILD'}/bin/msidb.exe";
    }
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
# Analyzing the full content of Component.idt
#################################################################################

sub analyze_keypath_component_file
{
    my ($filecontent) = @_;

    my %keypathtable = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if (( $i == 0 ) || ( $i == 1 ) || ( $i == 2 )) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $component = $1;
            my $keypath = $6;

            $keypathtable{$keypath} = $component;
        }
    }

    return (\%keypathtable);

}

#################################################################################
# Analyzing the content of Registry.idt
#################################################################################

sub analyze_registry_file
{
    my ($filecontent) = @_;

    my %table = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if (( $i == 0 ) || ( $i == 1 ) || ( $i == 2 )) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $registry = $1;
            my $root = $2;
            my $key = $3;
            my $name = $4;
            my $value = $5;
            my $component = $6;

            my %helphash = ();
            $helphash{'Root'} = $root;
            $helphash{'Key'} = $key;
            $helphash{'Name'} = $name;
            $helphash{'Value'} = $value;
            $helphash{'Component'} = $component;

            $table{$registry} = \%helphash;
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
# Windows registry entries containing properties are not set
# correctly during msp patch process. The properties are
# empty or do get their default values. This destroys the
# values of many entries in Windows registry.
# This can be fixed by removing all entries in Registry table,
# containing a property before starting msimsp.exe.
###############################################################

sub remove_properties_from_registry_table
{
    my ($registryhash, $componentkeypathhash, $registryfilecontent) = @_;

    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Start remove_properties_from_registry_table");

    my @registrytable = ();

    # Registry hash
    # Collecting all RegistryItems with values containing a property: [...]
    # To which component do they belong
    # Is this after removal an empty component? Create a replacement, so that
    # no Component has to be removed.
    # Is this RegistryItem a KeyPath of a component. Then it cannot be removed.

    my %problemitems = ();
    my %problemcomponents = ();
    my %securecomponents = ();
    my $changevalue = "";
    my $changeroot = "";
    my $infoline = "";

    my $newitemcounter = 0;
    my $olditemcounter = 0;

    foreach my $regitem ( keys %{$registryhash} )
    {
        my $value = "";
        if ( exists($registryhash->{$regitem}->{'Value'}) ) { $value = $registryhash->{$regitem}->{'Value'}; }

        if ( $value =~ /^.*(\[.*?\]).*$/ )
        {
            my $property = $1;

            # Collecting registry item
            $problemitems{$regitem} = 1;    # "1" -> can be removed
            if ( exists($componentkeypathhash->{$regitem}) ) { $problemitems{$regitem} = 2; }   # "2" -> cannot be removed, KeyPath

            # Collecting component (and number of problematic registry items
            # my $component = $registryhash->{$regitem}->{'Component'};
            # if ( exists($problemcomponents{$regitem}) ) { $problemcomponents{$regitem} = $problemcomponents{$regitem} + 1; }
            # else { $problemcomponents{$regitem} = 1; }
        }
        else
        {
            # Collecting all components with secure regisry items
            my $component = "";
            if ( exists($registryhash->{$regitem}->{'Component'}) ) { $component = $registryhash->{$regitem}->{'Component'}; }
            if ( $component eq "" ) { installer::exiter::exit_program("ERROR: Did not find component for registry item \"$regitem\".", "remove_properties_from_registry_table"); }
            $securecomponents{$component} = 1;
        }

        # Searching for change value
        my $localkey = "";
        if ( exists($registryhash->{$regitem}->{'Key'}) ) { $localkey = $registryhash->{$regitem}->{'Key'}; }
        if (( $localkey =~ /^\s*(Software\\.*\\)StartMenu\s*$/ ) && ( $changevalue eq "" ))
        {
            $changevalue = $1;
            $changeroot = $registryhash->{$regitem}->{'Root'};
        }

        $olditemcounter++;
    }

    my $removecounter = 0;
    my $renamecounter = 0;

    foreach my $regitem ( keys %{$registryhash} )
    {
        my $value = "";
        if ( exists($registryhash->{$regitem}->{'Value'}) ) { $value = $registryhash->{$regitem}->{'Value'}; }

        if ( $value =~ /^.*(\[.*?\]).*$/ )
        {
            # Removing registry items, that are no KeyPath and that belong to components,
            # that have other secure registry items.

            my $component = "";
            if ( exists($registryhash->{$regitem}->{'Component'}) ) { $component = $registryhash->{$regitem}->{'Component'}; }
            if ( $component eq "" ) { installer::exiter::exit_program("ERROR: Did not find component for registry item (2) \"$regitem\".", "remove_properties_from_registry_table"); }

            if (( $problemitems{$regitem} == 1 ) && ( exists($securecomponents{$component}) ))
            {
                # remove complete registry item
                delete($registryhash->{$regitem});
                $removecounter++;
                $infoline = "Removing registry item: $regitem : $value\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
            else
            {
                # Changing values of registry items, that are KeyPath or that contain to
                # components with only unsecure registry items.

                if (( $problemitems{$regitem} == 2 ) || ( ! exists($securecomponents{$component}) ))
                {
                    # change value of registry item
                    if ( $changevalue eq "" ) { installer::exiter::exit_program("ERROR: Did not find good change value for registry items", "remove_properties_from_registry_table"); }

                    my $oldkey = "";
                    if ( exists($registryhash->{$regitem}->{'Key'}) ) { $oldkey = $registryhash->{$regitem}->{'Key'}; };
                    my $oldname = "";
                    if ( exists($registryhash->{$regitem}->{'Name'}) ) { $oldname = $registryhash->{$regitem}->{'Name'}; }
                    my $oldvalue = "";
                    if ( exists($registryhash->{$regitem}->{'Value'}) ) { $oldvalue = $registryhash->{$regitem}->{'Value'}; }

                    $registryhash->{$regitem}->{'Key'} = $changevalue . "RegistryItem";
                    $registryhash->{$regitem}->{'Root'} = $changeroot;
                    $registryhash->{$regitem}->{'Name'} = $regitem;
                    $registryhash->{$regitem}->{'Value'} = 1;
                    $renamecounter++;

                    $infoline = "Changing registry item: $regitem\n";
                    $infoline = "Old: $oldkey : $oldname : $oldvalue\n";
                    $infoline = "New: $registryhash->{$regitem}->{'Key'} : $registryhash->{$regitem}->{'Name'} : $registryhash->{$regitem}->{'Value'}\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
            }
        }
    }

    $infoline = "Number of removed registry items: $removecounter\n";
    push( @installer::globals::logfileinfo, $infoline);
    $infoline = "Number of changed registry items: $renamecounter\n";
    push( @installer::globals::logfileinfo, $infoline);

    # Creating the new content of Registry table
    # First three lines from $registryfilecontent
    # All further files from changed $registryhash

    for ( my $i = 0; $i <= 2; $i++ ) { push(@registrytable, ${$registryfilecontent}[$i]); }

    foreach my $regitem ( keys %{$registryhash} )
    {
        my $root = "";
        if ( exists($registryhash->{$regitem}->{'Root'}) ) { $root = $registryhash->{$regitem}->{'Root'}; }
        else { installer::exiter::exit_program("ERROR: Did not find root in registry table for item: \"$regitem\".", "remove_properties_from_registry_table"); }
        my $localkey = "";
        if ( exists($registryhash->{$regitem}->{'Key'}) ) { $localkey = $registryhash->{$regitem}->{'Key'}; }
        my $name = "";
        if ( exists($registryhash->{$regitem}->{'Name'}) ) { $name = $registryhash->{$regitem}->{'Name'}; }
        my $value = "";
        if ( exists($registryhash->{$regitem}->{'Value'}) ) { $value = $registryhash->{$regitem}->{'Value'}; }
        my $comp = "";
        if ( exists($registryhash->{$regitem}->{'Component'}) ) { $comp = $registryhash->{$regitem}->{'Component'}; }

        my $oneline = $regitem . "\t" . $root . "\t" . $localkey . "\t" . $name . "\t" . $value . "\t" . $comp . "\n";
        push(@registrytable, $oneline);

        $newitemcounter++;
    }

    $infoline = "Number of registry items: $newitemcounter. Old value: $olditemcounter.\n";
    push( @installer::globals::logfileinfo, $infoline);

    installer::logger::include_timestamp_into_logfile("\nPerformance Info: End remove_properties_from_registry_table");

    return (\@registrytable);
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
    if ( $installer::globals::isunix )
    {
        $msiinfo = "$ENV{'OUTDIR_FOR_BUILD'}/bin/msiinfo.exe";
    }
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

    # Editing registry table because of wrong Property value
    #   my $registryfilename = $helperdir . $installer::globals::separator . "Registry.idt";
    #   my $componentfilename = $helperdir . $installer::globals::separator . "Component.idt";
    #   my $componentkeypathhash = analyze_keypath_component_file($componentfilecontent);

    #   my $registryfilecontent = installer::files::read_file($registryfilename);
    #   my $registryhash = analyze_registry_file($registryfilecontent);

    #   $registryfilecontent = remove_properties_from_registry_table($registryhash, $componentkeypathhash, $registryfilecontent);

    #   installer::files::save_file($registryfilename, $registryfilecontent);
    #   $tablelist = "Registry";
    #   include_tables_into_pcpfile($msidatabase, $helperdir, $tablelist);

    # Saving info in Summary Information Stream of msi database (required for following patches)
    write_sis_info($msidatabase);

    return $msidatabase;
}

1;
