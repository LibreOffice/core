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

use Cwd;
use File::Copy;
use File::Temp qw/ :mktemp /;

#################################################################################
# Global settings
#################################################################################

BEGIN
{
    $prog = "msi installer";
    $targetdir = "";
    $databasepath = "";
    $starttime = "";
    $globaltempdirname = "ooopackagingXXXXXX";
    $savetemppath = "";
    $msiinfo_available = 0;
    $path_displayed = 0;
    $localmsidbpath = "";

    $plat = $^O;

    if ( $plat =~ /cygwin/i )
    {
        $separator = "/";
        $pathseparator = "\:";
    }
    else
    {
        $separator = "\\";
        $pathseparator = "\;";
    }
}

#################################################################################
# Program information
#################################################################################

sub usage
{
    print <<Ende;
----------------------------------------------------------------------
This program installs a Windows Installer installation set
without using msiexec.exe. The installation is comparable
with an administrative installation using the Windows Installer
service.
Required parameter:
-d Path to installation set or msi database
-t Target directory
---------------------------------------------------------------------
Ende
    exit(-1);
}

#################################################################################
# Collecting parameter
#################################################################################

sub getparameter
{
    if (( $#ARGV < 3 ) || ( $#ARGV > 3 )) { usage(); }

    while ( $#ARGV >= 0 )
    {
        my $param = shift(@ARGV);

        if ($param eq "-t") { $targetdir = shift(@ARGV); }
        elsif ($param eq "-d") { $databasepath = shift(@ARGV); }
        else
        {
            print "\n**********************************************\n";
            print "Error: Unknown parameter: $param";
            print "\n**********************************************\n";
            usage();
            exit(-1);
        }
    }
}

#################################################################################
# Checking content of parameter
#################################################################################

sub controlparameter
{
    if ( $targetdir eq "" )
    {
        print "\n******************************************************\n";
        print "Error: Target directory not defined (parameter -t)!";
        print "\n******************************************************\n";
        usage();
        exit(-1);
    }

    if ( $databasepath eq "" )
    {
        print "\n******************************************************\n";
        print "Error: Path to msi database not defined (parameter -d)!";
        print "\n******************************************************\n";
        usage();
        exit(-1);
    }

    if ( -d $databasepath )
    {
        $databasepath =~ s/\\\s*$//;
        $databasepath =~ s/\/\s*$//;

        my $msifiles = find_file_with_file_extension("msi", $databasepath);

        if ( $#{$msifiles} < 0 ) { exit_program("ERROR: Did not find msi database in directory $installationdir"); }
        if ( $#{$msifiles} > 0 ) { exit_program("ERROR: Did find more than one msi database in directory $installationdir"); }

        $databasepath = $databasepath . $separator . ${$msifiles}[0];
    }

    if ( ! -f $databasepath ) { exit_program("ERROR: Did not find msi database in directory $databasepath."); }

    if ( ! -d $targetdir ) { create_directories($targetdir); }
}

#############################################################################
# The program msidb.exe can be located next to the Perl program. Then it is
# not necessary to find it in the PATH variable.
#############################################################################

sub check_local_msidb
{
    my $msidbname = "msidb.exe";
    my $perlprogramm = $0;
    my $path = $perlprogramm;

    get_path_from_fullqualifiedname(\$path);

    $path =~ s/\\\s*$//;
    $path =~ s/\/\s*$//;

    my $msidbpath = "";
    if ( $path =~ /^\s*$/ ) { $msidbpath = $msidbname; }
    else { $msidbpath = $path . $separator . $msidbname; }

    if ( -f $msidbpath )
    {
        $localmsidbpath = $msidbpath;
        print "Using $msidbpath (next to \"admin.pl\")\n";
    }
}

#############################################################################
# Converting a string list with separator $listseparator
# into an array
#############################################################################

sub convert_stringlist_into_array
{
    my ( $includestringref, $listseparator ) = @_;

    my @newarray = ();
    my $first;
    my $last = ${$includestringref};

    while ( $last =~ /^\s*(.+?)\Q$listseparator\E(.+)\s*$/) # "$" for minimal matching
    {
        $first = $1;
        $last = $2;
        # Problem with two directly following listseparators. For example a path with two ";;" directly behind each other
        $first =~ s/^$listseparator//;
        push(@newarray, "$first\n");
    }

    push(@newarray, "$last\n");

    return \@newarray;
}

#########################################################
# Checking the local system
# Checking existence of needed files in include path
#########################################################

sub check_system_path
{
    my $onefile;
    my $error = 0;
    my $pathvariable = $ENV{'PATH'};
    my $local_pathseparator = $pathseparator;

    if( $^O =~ /cygwin/i )
    {   # When using cygwin's perl the PATH variable is POSIX style and ...
        $pathvariable = qx{cygpath -mp "$pathvariable"} ;
        # has to be converted to DOS style for further use.
        $local_pathseparator = ';';
    }
    my $patharrayref = convert_stringlist_into_array(\$pathvariable, $local_pathseparator);

    my @needed_files_in_path = ("expand.exe");
    if ( $localmsidbpath eq "" ) { push(@needed_files_in_path, "msidb.exe"); } # not found locally -> search in path
    my @optional_files_in_path = ("msiinfo.exe");

    print("\nChecking required files:\n");

    foreach $onefile ( @needed_files_in_path )
    {
        print("...... searching $onefile ...");

        my $fileref = get_sourcepath_from_filename_and_includepath(\$onefile, $patharrayref);

        if ( $$fileref eq "" )
        {
            $error = 1;
            print( "$onefile not found\n" );
        }
        else
        {
            print( "\tFound: $$fileref\n" );
        }
    }

    if ( $error ) { exit_program("ERROR: Could not find all needed files in path (using setsolar should help)!"); }

    print("\nChecking optional files:\n");

    foreach $onefile ( @optional_files_in_path )
    {
        print("...... searching $onefile ...");

        my $fileref = get_sourcepath_from_filename_and_includepath(\$onefile, $patharrayref);

        if ( $$fileref eq "" )
        {
            print( "$onefile not found\n" );
            if ( $onefile eq "msiinfo.exe" ) { $msiinfo_available = 0; }
        }
        else
        {
            print( "\tFound: $$fileref\n" );
            if ( $onefile eq "msiinfo.exe" ) { $msiinfo_available = 1; }
        }
    }

}

##########################################################################
# Searching a file in a list of paths
##########################################################################

sub get_sourcepath_from_filename_and_includepath
{
    my ($searchfilenameref, $includepatharrayref) = @_;

    my $onefile = "";
    my $foundsourcefile = 0;

    for ( my $j = 0; $j <= $#{$includepatharrayref}; $j++ )
    {
        my $includepath = ${$includepatharrayref}[$j];
        $includepath =~ s/^\s*//;
        $includepath =~ s/\s*$//;

        $onefile = $includepath . $separator . $$searchfilenameref;

        if ( -f $onefile )
        {
            $foundsourcefile = 1;
            last;
        }
    }

    if (!($foundsourcefile)) { $onefile = ""; }

    return \$onefile;
}

########################################################
# Finding all files with a specified file extension
# in a specified directory.
########################################################

sub find_file_with_file_extension
{
    my ($extension, $dir) = @_;

    my @allfiles = ();
    my @sourcefiles = ();

    $dir =~ s/\Q$separator\E\s*$//;

    opendir(DIR, $dir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( $onefile =~ /^\s*(\S.*?)\.$extension\s*$/ )
            {
                push(@allfiles, $onefile)
            }
        }
    }

    return \@allfiles;
}

##############################################################
# Creating a directory with all parent directories
##############################################################

sub create_directories
{
    my ($directory) = @_;

    if ( ! try_to_create_directory($directory) )
    {
        my $parentdir = $directory;
        get_path_from_fullqualifiedname(\$parentdir);
        create_directories($parentdir);   # recursive
    }

    create_directory($directory);   # now it has to succeed
}

##############################################################
# Creating one directory
##############################################################

sub create_directory
{
    my ($directory) = @_;

    if ( ! -d $directory ) { mkdir($directory, 0775); }
}

##############################################################
# Trying to create a directory, no error if this fails
##############################################################

sub try_to_create_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;
    my $created_directory = 0;

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, 0775);

        if ($returnvalue)
        {
            $created_directory = 1;

            my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
            system($localcall);
        }
        else
        {
            $created_directory = 0;
        }
    }
    else
    {
        $created_directory = 1;
    }

    return $created_directory;
}

###########################################
# Getting path from full file name
###########################################

sub get_path_from_fullqualifiedname
{
    my ($longfilenameref) = @_;

    if ( $$longfilenameref =~ /\Q$separator\E/ )    # Is there a separator in the path? Otherwise the path is empty.
    {
        if ( $$longfilenameref =~ /^\s*(\S.*\Q$separator\E)(\S.+\S?)/ )
        {
            $$longfilenameref = $1;
        }
    }
    else
    {
        $$longfilenameref = ""; # there is no path
    }
}

##############################################################
# Getting file name from full file name
##############################################################

sub make_absolute_filename_to_relative_filename
{
    my ($longfilenameref) = @_;

    # Either '/' or '\'.
    if ( $$longfilenameref =~ /^.*[\/\\](\S.+\S?)/ )
    {
        $$longfilenameref = $1;
    }
}

############################################
# Exiting the program with an error
# This function is used instead of "die"
############################################

sub exit_program
{
    my ($message) = @_;

    print "\n***************************************************************\n";
    print "$message\n";
    print "***************************************************************\n";
    remove_complete_directory($savetemppath, 1);
    print "\n" . get_time_string();
    exit(-1);
}

#################################################################################
# Unpacking cabinet files with expand
#################################################################################

sub unpack_cabinet_file
{
    my ($cabfilename, $unpackdir) = @_;

    my $expandfile = "expand.exe"; # has to be in the PATH

    # expand.exe has to be located in the system directory.
    # Cygwin has another tool expand.exe, that converts tabs to spaces. This cannot be used of course.
    # But this wrong expand.exe is typically in the PATH before this expand.exe, to unpack
    # cabinet files.

    if ( $^O =~ /cygwin/i )
    {
        $expandfile = $ENV{'SYSTEMROOT'} . "/system32/expand.exe"; # Has to be located in the systemdirectory
        $expandfile =~ s/\\/\//;
        if ( ! -f $expandfile ) { exit_program("ERROR: Did not find file $expandfile in the Windows system folder!"); }
    }

    my $expandlogfile = $unpackdir . $separator . "expand.log";

    # exclude cabinet file
    # my $systemcall = $cabarc . " -o X " . $mergemodulehash->{'cabinetfile'};

    my $systemcall = "";
    if ( $^O =~ /cygwin/i ) {
        my $localunpackdir = qx{cygpath -w "$unpackdir"};
        $localunpackdir =~ s/\\/\\\\/g;

        my $localcabfilename = qx{cygpath -w "$cabfilename"};
        $localcabfilename =~ s/\\/\\\\/g;
        $localcabfilename =~ s/\s*$//g;

        $systemcall = $expandfile . " " . $localcabfilename . " -F:\* " . $localunpackdir . " \>\/dev\/null 2\>\&1";
    }
    else
    {
        $systemcall = $expandfile . " " . $cabfilename . " -F:\* " . $unpackdir . " \> " . $expandlogfile;
    }

    my $returnvalue = system($systemcall);

    if ($returnvalue) { exit_program("ERROR: Could not execute $systemcall !"); }
}

#################################################################################
# Extracting tables from msi database
#################################################################################

sub extract_tables_from_database
{
    my ($fullmsidatabasepath, $workdir, $tablelist) = @_;

    my $msidb = "msidb.exe";    # Has to be in the path
    if ( $localmsidbpath ) { $msidb = $localmsidbpath; }
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";

    if ( $^O =~ /cygwin/i ) {
        chomp( $fullmsidatabasepath = qx{cygpath -w "$fullmsidatabasepath"} );
        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
        $fullmsidatabasepath =~ s/\\/\\\\/g;
        $workdir =~ s/\\/\\\\/g;
        # and if there are still slashes, they also need to be double backslash
        $fullmsidatabasepath =~ s/\//\\\\/g;
        $workdir =~ s/\//\\\\/g;
    }

    # Export of all tables by using "*"

    $systemcall = $msidb . " -d " . $fullmsidatabasepath . " -f " . $workdir . " -e $tablelist";
    print "\nAnalyzing msi database\n";
    $returnvalue = system($systemcall);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        exit_program($infoline);
    }
}

########################################################
# Check, if this installation set contains
# internal cabinet files included into the msi
# database.
########################################################

sub check_for_internal_cabfiles
{
    my ($cabfilehash) = @_;

    my $contains_internal_cabfiles = 0;
    my %allcabfileshash = ();

    foreach my $filename ( keys %{$cabfilehash} )
    {
        if ( $filename =~ /^\s*\#/ )     # starting with a hash
        {
            $contains_internal_cabfiles = 1;
            # setting real filename without hash as key and name with hash as value
            my $realfilename = $filename;
            $realfilename =~ s/^\s*\#//;
            $allcabfileshash{$realfilename} = $filename;
        }
    }

    return ( $contains_internal_cabfiles, \%allcabfileshash );
}

#################################################################
# Exclude all cab files from the msi database.
#################################################################

sub extract_cabs_from_database
{
    my ($msidatabase, $allcabfiles) = @_;

    my $infoline = "";
    my $fullsuccess = 1;
    my $msidb = "msidb.exe";    # Has to be in the path
    if ( $localmsidbpath ) { $msidb = $localmsidbpath; }

    my @all_excluded_cabfiles = ();

    if( $^O =~ /cygwin/i )
    {
        $msidatabase = qx{cygpath -w "$msidatabase"};
        $msidatabase =~ s/\\/\\\\/g;
        $msidatabase =~ s/\s*$//g;
    }
    else
    {
        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
        $msidatabase =~ s/\//\\\\/g;
    }

    foreach my $onefile ( keys %{$allcabfiles} )
    {
        my $systemcall = $msidb . " -d " . $msidatabase . " -x " . $onefile;
         system($systemcall);
         push(@all_excluded_cabfiles, $onefile);
    }

    \@all_excluded_cabfiles;
}

################################################################################
# Collect all DiskIds to the corresponding cabinet files from Media.idt.
################################################################################

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
            my $newdir = $fulldir . $separator . $dirname;
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

    print "Creating directories\n";

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
# Cygwin: Setting privileges for files
####################################################################################

sub change_privileges
{
    my ($destfile, $privileges) = @_;

    my $localcall = "chmod $privileges " . "\"" . $destfile . "\"";
    system($localcall);
}

####################################################################################
# Cygwin: Setting privileges for files recursively
####################################################################################

sub change_privileges_full
{
    my ($target) = @_;

    print "Changing privileges\n";

    my $localcall = "chmod -R 755 " . "\"" . $target . "\"";
    system($localcall);
}

######################################################
# Creating a new directory with defined privileges
######################################################

sub create_directory_with_privileges
{
    my ($directory, $privileges) = @_;

    my $returnvalue = 1;
    my $infoline = "";

    if (!(-d $directory))
    {
        my $localprivileges = oct("0".$privileges); # changes "777" to 0777
        $returnvalue = mkdir($directory, $localprivileges);

        if ($returnvalue)
        {
            my $localcall = "chmod $privileges $directory \>\/dev\/null 2\>\&1";
            system($localcall);
        }
    }
    else
    {
        my $localcall = "chmod $privileges $directory \>\/dev\/null 2\>\&1";
        system($localcall);
    }
}

######################################################
# Creating a unique directory with pid extension
######################################################

sub create_pid_directory
{
    my ($directory) = @_;

    $directory =~ s/\Q$separator\E\s*$//;
    my $pid = $$;           # process id
    my $time = time();      # time

    $directory = $directory . "_" . $pid . $time;

    if ( ! -d $directory ) { create_directory($directory); }
    else { exit_program("ERROR: Directory $directory already exists!"); }

    return $directory;
}

####################################################################################
# Copying files into installation set
####################################################################################

sub copy_files_into_directory_structure
{
    my ($fileorder, $filehash, $componenthash, $fullpathhash, $maxsequence, $unpackdir, $installdir, $dirhash) = @_;

    print "Copying files\n";

    for ( my $i = 1; $i <= $maxsequence; $i++ )
    {
        if ( exists($fileorder->{$i}) )
        {
            my $file = $fileorder->{$i};
            if ( ! exists($filehash->{$file}->{'Component'}) ) { exit_program("ERROR: Did not find component for file: \"$file\"."); }
            my $component = $filehash->{$file}->{'Component'};
            if ( ! exists($componenthash->{$component}) ) { exit_program("ERROR: Did not find directory for component: \"$component\"."); }
            my $dirname = $componenthash->{$component};
            if ( ! exists($fullpathhash->{$dirname}) ) { exit_program("ERROR: Did not find full directory path for dir: \"$dirname\"."); }
            my $destdir = $fullpathhash->{$dirname};
            if ( ! exists($filehash->{$file}->{'FileName'}) ) { exit_program("ERROR: Did not find \"FileName\" for file: \"$file\"."); }
            my $destfile = $filehash->{$file}->{'FileName'};

            $destfile = $destdir . $separator . $destfile;
            my $sourcefile = $unpackdir . $separator . $file;

            if ( ! -f $sourcefile )
            {
                # It is possible, that this was an unpacked file
                # Looking in the dirhash, to find the subdirectory in the installation set (the id is $dirname)
                # subdir is not recursively analyzed, only one directory.

                my $oldsourcefile = $sourcefile;
                my $subdir = "";
                if ( exists($dirhash->{$dirname}->{'DefaultDir'}) ) { $subdir = $dirhash->{$dirname}->{'DefaultDir'} . $separator; }
                my $realfilename = $filehash->{$file}->{'FileName'};
                my $localinstalldir = $installdir;

                $localinstalldir =~ s/\\\s*$//;
                $localinstalldir =~ s/\/\s*$//;

                $sourcefile = $localinstalldir . $separator . $subdir . $realfilename;

                if ( ! -f $sourcefile ) { exit_program("ERROR: File not found: \"$oldsourcefile\" (or \"$sourcefile\")."); }
            }

            my $copyreturn = copy($sourcefile, $destfile);

            if ( ! $copyreturn) { exit_program("ERROR: Could not copy $source to $dest\n"); }

            # if (( $^O =~ /cygwin/i ) && ( $destfile =~ /\.exe\s*$/ )) { change_privileges($destfile, "775"); }
        }
        # else  # allowing missing sequence numbers ?
        # {
        #   exit_program("ERROR: No file assigned to sequence $i");
        # }
    }
}

######################################################
# Removing a complete directory with subdirectories
######################################################

sub remove_complete_directory
{
    my ($directory, $start) = @_;

    my @content = ();
    my $infoline = "";

    $directory =~ s/\Q$separator\E\s*$//;

    if ( -d $directory )
    {
        if ( $start ) { print "Removing directory $directory\n"; }

        opendir(DIR, $directory);
        @content = readdir(DIR);
        closedir(DIR);

        my $oneitem;

        foreach $oneitem (@content)
        {
            if ((!($oneitem eq ".")) && (!($oneitem eq "..")))
            {
                my $item = $directory . $separator . $oneitem;

                if ( -f $item || -l $item )     # deleting files or links
                {
                    unlink($item);
                }

                if ( -d $item )     # recursive
                {
                    remove_complete_directory($item, 0);
                }
            }
        }

        # try to remove empty directory
        my $returnvalue = rmdir $directory;
        if ( ! $returnvalue ) { print "Warning: Problem with removing empty dir $directory\n"; }
    }
}

####################################################################################
# Defining a temporary path
####################################################################################

sub get_temppath
{
    my $temppath = "";

    if (( $ENV{'TMP'} ) || ( $ENV{'TEMP'} ))
    {
        if ( $ENV{'TMP'} ) { $temppath = $ENV{'TMP'}; }
        elsif ( $ENV{'TEMP'} )  { $temppath = $ENV{'TEMP'}; }

        $temppath =~ s/\Q$separator\E\s*$//;    # removing ending slashes and backslashes
        $temppath = $temppath . $separator . $globaltempdirname;
        $temppath = mkdtemp($temppath);

        my $dirsave = $temppath;

        $temppath = $temppath . $separator . "a";
        $temppath = create_pid_directory($temppath);

        if ( ! -d $temppath ) { exit_program("ERROR: Failed to create directory $temppath ! Possible reason: Wrong privileges in directory $dirsave."); }

        if ( $^O =~ /cygwin/i )
        {
            $temppath =~ s/\\/\\\\/g;
            chomp( $temppath = qx{cygpath -w "$temppath"} );
        }

        $savetemppath = $temppath;
    }
    else
    {
        exit_program("ERROR: Could not set temporary directory (TMP and TEMP not set!).");
    }

    return $temppath;
}

####################################################################################
# Reading one file
####################################################################################

sub read_file
{
    my ($localfile) = @_;

    my @localfile = ();

    open( IN, "<$localfile" ) || exit_program("ERROR: Cannot open file $localfile for reading");

    #   Don't use "my @localfile = <IN>" here, because
    #   perl has a problem with the internal "large_and_huge_malloc" function
    #   when calling perl using Mac OS X 10.5 with a perl built with Mac OS X 10.4
    while ( $line = <IN> ) {
        push @localfile, $line;
    }

    close( IN );

    return \@localfile;
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
    $month++;

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
    my ($msidatabase) = @_;

    print "Setting SIS in msi database\n";

    if ( ! -f $msidatabase ) { exit_program("ERROR: Cannot find file $msidatabase"); }

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

    $returnvalue = system($systemcall);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        exit_program($infoline);
    }
}

###############################################################
# Convert time string
###############################################################

sub convert_timestring
{
    my ($secondstring) = @_;

    my $timestring = "";

    if ( $secondstring < 60 )    # less than a minute
    {
        if ( $secondstring < 10 ) { $secondstring = "0" . $secondstring; }
        $timestring = "00\:$secondstring min\.";
    }
    elsif ( $secondstring < 3600 )
    {
        my $minutes = $secondstring / 60;
        my $seconds = $secondstring % 60;
        if ( $minutes =~ /(\d*)\.\d*/ ) { $minutes = $1; }
        if ( $minutes < 10 ) { $minutes = "0" . $minutes; }
        if ( $seconds < 10 ) { $seconds = "0" . $seconds; }
        $timestring = "$minutes\:$seconds min\.";
    }
    else    # more than one hour
    {
        my $hours = $secondstring / 3600;
        my $secondstring = $secondstring % 3600;
        my $minutes = $secondstring / 60;
        my $seconds = $secondstring % 60;
        if ( $hours =~ /(\d*)\.\d*/ ) { $hours = $1; }
        if ( $minutes =~ /(\d*)\.\d*/ ) { $minutes = $1; }
        if ( $hours < 10 ) { $hours = "0" . $hours; }
        if ( $minutes < 10 ) { $minutes = "0" . $minutes; }
        if ( $seconds < 10 ) { $seconds = "0" . $seconds; }
        $timestring = "$hours\:$minutes\:$seconds hours";
    }

    return $timestring;
}

###############################################################
# Returning time string for logging
###############################################################

sub get_time_string
{
    my $currenttime = time();
    $currenttime = $currenttime - $starttime;
    $currenttime = convert_timestring($currenttime);
    $currenttime = localtime() . " \(" . $currenttime . "\)\n";
    return $currenttime;
}

####################################################################################
# Simulating an administrative installation
####################################################################################

$starttime = time();

getparameter();
controlparameter();
check_local_msidb();
check_system_path();
my $temppath = get_temppath();

print("\nmsi database: $databasepath\n");
print("Destination directory: $targetdir\n" );

my $helperdir = $temppath . $separator . "installhelper";
create_directory($helperdir);

# Get File.idt, Component.idt and Directory.idt from database

my $tablelist = "File Directory Component Media CustomAction";
extract_tables_from_database($databasepath, $helperdir, $tablelist);

# Set unpackdir
my $unpackdir = $helperdir . $separator . "unpack";
create_directory($unpackdir);

# Reading media table to check for internal cabinet files
my $filename = $helperdir . $separator . "Media.idt";
if ( ! -f $filename ) { exit_program("ERROR: Could not find required file: $filename !"); }
my $filecontent = read_file($filename);
my $cabfilehash = analyze_media_file($filecontent);

# Check, if there are internal cab files
my ( $contains_internal_cabfiles, $all_internal_cab_files) = check_for_internal_cabfiles($cabfilehash);

if ( $contains_internal_cabfiles )
{
    # Set unpackdir
    my $cabdir = $helperdir . $separator . "internal_cabs";
    create_directory($cabdir);
    my $from = cwd();
    chdir($cabdir);
    # Exclude all cabinet files from database
    my $all_excluded_cabs = extract_cabs_from_database($databasepath, $all_internal_cab_files);
    print "Unpacking files from internal cabinet file(s)\n";
    foreach my $cabfile ( @{$all_excluded_cabs} ) { unpack_cabinet_file($cabfile, $unpackdir); }
    chdir($from);
}

# Unpack all cab files into $helperdir, cab files must be located next to msi database
my $installdir = $databasepath;

get_path_from_fullqualifiedname(\$installdir);

my $databasefilename = $databasepath;
make_absolute_filename_to_relative_filename(\$databasefilename);

my $cabfiles = find_file_with_file_extension("cab", $installdir);

if (( $#{$cabfiles} < 0 ) && ( ! $contains_internal_cabfiles )) { exit_program("ERROR: Did not find any cab file in directory $installdir"); }

print "Unpacking files from cabinet file(s)\n";
for ( my $i = 0; $i <= $#{$cabfiles}; $i++ )
{
    my $cabfile = $installdir . $separator . ${$cabfiles}[$i];
    unpack_cabinet_file($cabfile, $unpackdir);
}

# Reading tables
$filename = $helperdir . $separator . "Directory.idt";
$filecontent = read_file($filename);
my $dirhash = analyze_directory_file($filecontent);

$filename = $helperdir . $separator . "Component.idt";
$filecontent = read_file($filename);
my $componenthash = analyze_component_file($filecontent);

$filename = $helperdir . $separator . "File.idt";
$filecontent = read_file($filename);
my ( $filehash, $fileorder, $maxsequence ) = analyze_file_file($filecontent);

# Creating the directory structure
my $fullpathhash = create_directory_structure($dirhash, $targetdir);

# Copying files
copy_files_into_directory_structure($fileorder, $filehash, $componenthash, $fullpathhash, $maxsequence, $unpackdir, $installdir, $dirhash);
if ( $^O =~ /cygwin/i ) { change_privileges_full($targetdir); }

my $msidatabase = $targetdir . $separator . $databasefilename;
my $copyreturn = copy($databasepath, $msidatabase);
if ( ! $copyreturn) { exit_program("ERROR: Could not copy $source to $dest\n"); }

# Saving info in Summary Information Stream of msi database (required for following patches)
if ( $msiinfo_available ) { write_sis_info($msidatabase); }

# Removing the helper directory
remove_complete_directory($temppath, 1);

print "\nSuccessful installation: " . get_time_string();
