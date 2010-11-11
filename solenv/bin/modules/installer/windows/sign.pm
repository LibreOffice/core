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
    # Windows : "msicert.exe", "diff.exe", "msidb.exe", "signtool.exe"

    my @needed_files_in_path = ("msicert.exe", "msidb.exe", "signtool.exe", "diff.exe");
    if ( $installer::globals::internal_cabinet_signing )
    {
        push(@needed_files_in_path, "cabarc.exe");
        push(@needed_files_in_path, "makecab.exe");
    }

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

######################################################
# Making systemcall with warning
######################################################

sub make_systemcall_with_warning
{
    my ($systemcall, $displaysystemcall) = @_;

    installer::logger::print_message( "... $displaysystemcall ...\n" );

    my $success = 1;
    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $displaysystemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "WARNING: Could not execute \"$displaysystemcall\"!\n";
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

######################################################
# Making systemcall with more return data
######################################################

sub execute_open_system_call
{
    my ( $systemcall ) = @_;

    my @openoutput = ();
    my $success = 1;

    my $comspec = $ENV{COMSPEC};
    $comspec = $comspec . " -c ";

    if( $^O =~ /cygwin/i )
    {
        $comspec = "";
    }

    my $localsystemcall = "$comspec $systemcall 2>&1 |";

    open( OPN, "$localsystemcall") or warn "Can't execute $localsystemcall\n";
    while (<OPN>) { push(@openoutput, $_); }
    close (OPN);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
        $success = 0;
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return ($success, \@openoutput);
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
# Collect all last files in a cabinet file. This is
# necessary to control, if the cabinet file was damaged
# by calling signtool.exe.
############################################################

sub analyze_file_file
{
    my ($filecontent) = @_;

    my %filenamehash = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i < 3 ) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $name = $1;
            my $sequence = $8;

            $filenamehash{$sequence} = $name;
        }
    }

    return ( \%filenamehash );
}

############################################################
# Collect all DiskIds to the corresponding cabinet files.
############################################################

sub analyze_media_file
{
    my ($filecontent) = @_;

    my %diskidhash = ();
    my %lastsequencehash = ();

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( $i < 3 ) { next; }

        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $diskid = $1;
            my $lastsequence = $2;
            my $cabfile = $4;

            $diskidhash{$cabfile} = $diskid;
            $lastsequencehash{$cabfile} = $lastsequence;
        }
    }

    return ( \%diskidhash, \%lastsequencehash );
}

########################################################
# Collect all DiskIds from database table "Media".
########################################################

sub collect_diskid_from_media_table
{
    my ($msidatabase, $languagestring) = @_;

    # creating working directory
    my $workdir = installer::systemactions::create_directories("media", \$languagestring);
    installer::windows::admin::extract_tables_from_pcpfile($msidatabase, $workdir, "Media File");

    # Reading tables
    my $filename = $workdir . $installer::globals::separator . "Media.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find required file: $filename !", "collect_diskid_from_media_table"); }
    my $filecontent = installer::files::read_file($filename);
    my ( $diskidhash, $lastsequencehash ) = analyze_media_file($filecontent);

    $filename = $workdir . $installer::globals::separator . "File.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find required file: $filename !", "collect_diskid_from_media_table"); }
    $filecontent = installer::files::read_file($filename);
    my $filenamehash = analyze_file_file($filecontent);

    return ( $diskidhash, $filenamehash, $lastsequencehash );
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
    my $contains_external_cabfiles = 0;
    my $msidatabase = "";
    my $contains_msidatabase = 0;

    for ( my $j = 0; $j <= $#sourcefiles; $j++ )
    {
        if ( $sourcefiles[$j] =~ /\.cab\s*$/ ) { $allcabfileshash{$sourcefiles[$j]} = 1; }
        else
        {
            if ( $sourcefiles[$j] =~ /\.txt\s*$/ ) { next; }
            if ( $sourcefiles[$j] =~ /\.html\s*$/ ) { next; }
            if ( $sourcefiles[$j] =~ /\.ini\s*$/ ) { next; }
            if ( $sourcefiles[$j] =~ /\.bmp\s*$/ ) { next; }
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
    if ( $cabcounter > 0 ) { $contains_external_cabfiles = 1; }

    # How about a cab file without a msi database?
    if (( $cabcounter > 0 ) && ( $msidatabase eq "" )) { installer::exiter::exit_program("ERROR: There is no msi database in the installation set, but an external cabinet file. Path: $installsetpath !", "collect_installset_content"); }

    if ( $msidatabase ne "" ) { $contains_msidatabase = 1; }

    return (\%allcabfileshash, \%allfileshash, $msidatabase, $contains_external_cabfiles, $contains_msidatabase, \@sourcefiles);
}

########################################################
# Adding content of external cabinet files into the
# msi database
########################################################

sub msicert_database
{
    my ($msidatabase, $allcabfiles, $cabfilehash, $internalcabfile) = @_;

    my $fullsuccess = 1;

    foreach my $cabfile ( keys %{$allcabfiles} )
    {
        my $origfilesize = -s $cabfile;

        my $mediacabfilename = $cabfile;
        if ( $internalcabfile ) { $mediacabfilename = "\#" . $mediacabfilename; }
        if ( ! exists($cabfilehash->{$mediacabfilename}) ) { installer::exiter::exit_program("ERROR: Could not determine DiskId from media table for cabinet file \"$cabfile\" !", "msicert_database"); }
        my $diskid = $cabfilehash->{$mediacabfilename};

        my $systemcall = "msicert.exe -d $msidatabase -m $diskid -c $cabfile -h";
         $success = make_systemcall($systemcall, $systemcall);
        if ( ! $success ) { $fullsuccess = 0; }

        # size of cabinet file must not change
        my $finalfilesize = -s $cabfile;

        if ( $origfilesize != $finalfilesize ) { installer::exiter::exit_program("ERROR: msicert.exe changed size of cabinet file !", "msicert_database"); }
    }

    return $fullsuccess;
}

########################################################
# Checking if cabinet file was broken by signtool.
########################################################

sub cabinet_cosistency_check
{
    my ( $onefile, $followmeinfohash, $filenamehash, $lastsequencehash, $temppath ) = @_;

    my $infoline = "Making consistency check of $onefile\n";
    push( @installer::globals::logfileinfo, $infoline);
    my $expandfile = "expand.exe";  # Has to be in the path

    if ( $^O =~ /cygwin/i )
    {
        $expandfile = qx(cygpath -u "$ENV{WINDIR}"/System32/expand.exe);
        chomp $expandfile;
    }

    if ( $filenamehash == 0 )
    {
        $infoline = "Warning: Stopping consistency check: Important hash of filenames is empty!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    elsif  ( $lastsequencehash == 0 )
    {
        $infoline = "Warning: Stopping consistency check; Important hash of last sequences is empty!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else # both hashes are available
    {
        # $onefile contains only the name of the cabinet file without path
         my $sequence = $lastsequencehash->{$onefile};
         my $lastfile = $filenamehash->{$sequence};
        $infoline = "Check of $onefile: Sequence: $sequence is file: $lastfile\n";
        push( @installer::globals::logfileinfo, $infoline);

         # Therefore the file $lastfile need to be binary compared.
         # It has to be expanded from the cabinet file
         # of the original installation set and from the
         # newly signed cabinet file.

        # How about cabinet files extracted from msi database?
        my $finalinstalldir = $followmeinfohash->{'finalinstalldir'};

        $finalinstalldir =~ s/\\\s*$//;
        $finalinstalldir =~ s/\/\s*$//;
        my $sourcecabfile = $finalinstalldir . $installer::globals::separator . $onefile;
        my $currentpath = cwd();
        my $destcabfile = $currentpath . $installer::globals::separator . $onefile;

        if ( $^O =~ /cygwin/i )
        {
            chomp( $destcabfile = qx{cygpath -w "$destcabfile"} );
            $destcabfile =~ s/\\/\//g;
        }

        if ( ! -f $sourcecabfile )
        {
            $infoline = "WARNING: Check of cab file cannot happen, because source cabinet file was not found: $sourcecabfile\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        elsif ( ! -f $destcabfile )
        {
            $infoline = "WARNING: Check of cab file cannot happen, because destination cabinet file was not found: $sourcecabfile\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else # everything is okay for the check
        {
            my $diffpath = get_diff_path($temppath);

            my $origdiffpath = $diffpath . $installer::globals::separator . "orig";
            my $newdiffpath = $diffpath . $installer::globals::separator . "new";

            if ( ! -d $origdiffpath ) { mkdir($origdiffpath); }
            if ( ! -d $newdiffpath ) { mkdir($newdiffpath); }

            my $systemcall = "$expandfile $sourcecabfile $origdiffpath -f:$lastfile ";
            $infoline = $systemcall . "\n";
            push( @installer::globals::logfileinfo, $infoline);

            my $success = make_systemcall($systemcall, $systemcall);
            if ( ! $success ) { installer::exiter::exit_program("ERROR: Could not successfully execute: $systemcall !", "cabinet_cosistency_check"); }

            $systemcall = "$expandfile $destcabfile $newdiffpath -f:$lastfile ";
            $infoline = $systemcall . "\n";
            push( @installer::globals::logfileinfo, $infoline);

            $success = make_systemcall($systemcall, $systemcall);
            if ( ! $success ) { installer::exiter::exit_program("ERROR: Could not successfully execute: $systemcall !", "cabinet_cosistency_check"); }

            # and finally the two files can be diffed.
            my $origfile = $origdiffpath . $installer::globals::separator . $lastfile;
            my $newfile = $newdiffpath . $installer::globals::separator . $lastfile;

            if ( ! -f $origfile ) { installer::exiter::exit_program("ERROR: Unpacked original file not found: $origfile !", "cabinet_cosistency_check"); }
            if ( ! -f $newfile ) { installer::exiter::exit_program("ERROR: Unpacked new file not found: $newfile !", "cabinet_cosistency_check"); }

            my $origsize = -s $origfile;
            my $newsize = -s $newfile;

            if ( $origsize != $newsize ) # This shows an error!
            {
                $infoline = "ERROR: Different filesize after signtool.exe was used. Original: $origsize Bytes, new: $newsize. File: $lastfile\n";
                push( @installer::globals::logfileinfo, $infoline);
                installer::exiter::exit_program("ERROR: The cabinet file $destcabfile is broken after signtool.exe signed this file !", "cabinet_cosistency_check");
            }
            else
            {
                $infoline = "Same size of last file in cabinet file after usage of signtool.exe: $newsize (File: $lastfile)\n";
                push( @installer::globals::logfileinfo, $infoline);

                # Also making a binary diff?

                my $difffile = "diff.exe";  # has to be in the path
                $systemcall = "$difffile $origfile $newfile";
                $infoline = $systemcall . "\n";
                $returnvalue = make_systemcall($systemcall, $systemcall);

                my $success = $?;

                if ( $success == 0 )
                {
                    $infoline = "Last files are identical after signing cabinet file (File: $lastfile)\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
                elsif ( $success == 1 )
                {
                    $infoline = "ERROR: Last files are different after signing cabinet file (File: $lastfile)\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    installer::exiter::exit_program("ERROR: Last files are different after signing cabinet file (File: $lastfile)!", "cabinet_cosistency_check");
                }
                else
                {
                    $infoline = "ERROR: Problem occured calling diff.exe (File: $lastfile)\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    installer::exiter::exit_program("ERROR: Problem occured calling diff.exe (File: $lastfile) !", "cabinet_cosistency_check");
                }
            }
        }
    }

}

########################################################
# Signing a list of files
########################################################

sub sign_files
{
    my ( $followmeinfohash, $allfiles, $pw, $cabinternal, $filenamehash, $lastsequencehash, $temppath ) = @_;

    my $infoline = "";
    my $fullsuccess = 1;
    my $maxcounter = 3;

    my $productname = "";
    if ( $followmeinfohash->{'allvariableshash'}->{'PRODUCTNAME'} ) { $productname = "/d " . "\"$followmeinfohash->{'allvariableshash'}->{'PRODUCTNAME'}\""; }
    my $url = "";
    if (( ! exists($followmeinfohash->{'allvariableshash'}->{'OPENSOURCE'}) ) || ( $followmeinfohash->{'allvariableshash'}->{'OPENSOURCE'} == 0 )) { $url = "/du " . "\"http://www.sun.com\""; }
    else { $url = "/du " . "\"http://www.openoffice.org\""; }
    my $timestampurl = "http://timestamp.verisign.com/scripts/timestamp.dll";

    my $pfxfilepath = $installer::globals::pfxfile;

    if( $^O =~ /cygwin/i )
    {
        $pfxfilepath = qx{cygpath -w "$pfxfilepath"};
        $pfxfilepath =~ s/\\/\\\\/g;
        $pfxfilepath =~ s/\s*$//g;
    }

    foreach my $onefile ( reverse sort keys %{$allfiles} )
    {
        if ( already_certified($onefile) )
        {
            $infoline = "Already certified: Skipping file $onefile\n";
            push( @installer::globals::logfileinfo, $infoline);
            next;
        }

        my $counter = 1;
        my $success = 0;

        while (( $counter <= $maxcounter ) && ( ! $success ))
        {
            if ( $counter > 1 ) { installer::logger::print_message( "\n\n... repeating file $onefile ...\n" ); }
            if ( $cabinternal ) { installer::logger::print_message("    Signing: $onefile\n"); }
            my $systemcall = "signtool.exe sign /f \"$pfxfilepath\" /p $pw $productname $url /t \"$timestampurl\" \"$onefile\"";
            my $displaysystemcall = "signtool.exe sign /f \"$pfxfilepath\" /p ***** $productname $url /t \"$timestampurl\" \"$onefile\"";
             $success = make_systemcall_with_warning($systemcall, $displaysystemcall);
             $counter++;
         }

         # Special check for cabinet files, that sometimes get damaged by signtool.exe
         if (( $success ) && ( $onefile =~ /\.cab\s*$/ ) && ( ! $cabinternal ))
         {
             cabinet_cosistency_check($onefile, $followmeinfohash, $filenamehash, $lastsequencehash, $temppath);
        }

         if ( ! $success )
         {
             $fullsuccess = 0;
            installer::exiter::exit_program("ERROR: Could not sign file: $onefile!", "sign_files");
        }
    }

    return $fullsuccess;
}

##########################################################################
# Lines in ddf files must not contain more than 256 characters
##########################################################################

sub check_ddf_file
{
    my ( $ddffile, $ddffilename ) = @_;

    my $maxlength = 0;
    my $maxline = 0;
    my $linelength = 0;
    my $linenumber = 0;

    for ( my $i = 0; $i <= $#{$ddffile}; $i++ )
    {
        my $oneline = ${$ddffile}[$i];

        $linelength = length($oneline);
        $linenumber = $i + 1;

        if ( $linelength > 256 )
        {
            installer::exiter::exit_program("ERROR \"$ddffilename\" line $linenumber: Lines in ddf files must not contain more than 256 characters!", "check_ddf_file");
        }

        if ( $linelength > $maxlength )
        {
            $maxlength = $linelength;
            $maxline = $linenumber;
        }
    }

    my $infoline = "Check of ddf file \"$ddffilename\": Maximum length \"$maxlength\" in line \"$maxline\" (allowed line length: 256 characters)\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#################################################################
# Setting the path, where the cab files are unpacked.
#################################################################

sub get_cab_path
{
    my ($temppath) = @_;

    my $cabpath = "cabs_" . $$;
    $cabpath = $temppath . $installer::globals::separator . $cabpath;
    if ( ! -d $cabpath ) { installer::systemactions::create_directory($cabpath); }

    return $cabpath;
}

#################################################################
# Setting the path, where the diff can happen.
#################################################################

sub get_diff_path
{
    my ($temppath) = @_;

    my $diffpath = "diff_" . $$;
    $diffpath = $temppath . $installer::globals::separator . $diffpath;
    if ( ! -d $diffpath ) { installer::systemactions::create_directory($diffpath); }

    return $diffpath;
}

#################################################################
# Exclude all cab files from the msi database.
#################################################################

sub extract_cabs_from_database
{
    my ($msidatabase, $allcabfiles) = @_;

    installer::logger::include_header_into_logfile("Extracting cabs from msi database");

    my $infoline = "";
    my $fullsuccess = 1;
    my $msidb = "msidb.exe";    # Has to be in the path

    # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
    $msidatabase =~ s/\//\\\\/g;

    foreach my $onefile ( keys %{$allcabfiles} )
    {
        my $systemcall = $msidb . " -d " . $msidatabase . " -x " . $onefile;
         my $success = make_systemcall($systemcall, $systemcall);
        if ( ! $success ) { $fullsuccess = 0; }

        # and removing the stream from the database
        $systemcall = $msidb . " -d " . $msidatabase . " -k " . $onefile;
         $success = make_systemcall($systemcall, $systemcall);
        if ( ! $success ) { $fullsuccess = 0; }
    }

    return $fullsuccess;
}

#################################################################
# Include cab files into the msi database.
#################################################################

sub include_cabs_into_database
{
    my ($msidatabase, $allcabfiles) = @_;

    installer::logger::include_header_into_logfile("Including cabs into msi database");

    my $infoline = "";
    my $fullsuccess = 1;
    my $msidb = "msidb.exe";    # Has to be in the path

    # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
    $msidatabase =~ s/\//\\\\/g;

    foreach my $onefile ( keys %{$allcabfiles} )
    {
        my $systemcall = $msidb . " -d " . $msidatabase . " -a " . $onefile;
         my $success = make_systemcall($systemcall, $systemcall);
        if ( ! $success ) { $fullsuccess = 0; }
    }

    return $fullsuccess;
}

########################################################
# Reading the order of the files inside the
# cabinet files.
########################################################

sub read_cab_file
{
    my ($cabfilename) = @_;

    installer::logger::print_message( "\n... reading cabinet file $cabfilename ...\n" );
    my $infoline = "Reading cabinet file $cabfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $systemcall = "cabarc.exe" . " L " . $cabfilename;
    push(@logfile, "$systemcall\n");

    my ($success, $fileorder) = execute_open_system_call($systemcall);

    my @allfiles = ();

    for ( my $i = 0; $i <= $#{$fileorder}; $i++ )
    {
        my $line = ${$fileorder}[$i];
        if ( $line =~ /^\s*(.*?)\s+\d+\s+\d+\/\d+\/\d+\s+\d+\:\d+\:\d+\s+[\w-]+\s*$/ )
        {
            my $filename = $1;
            push(@allfiles, $filename);
        }
    }

    return \@allfiles;
}

########################################################
# Unpacking a cabinet file.
########################################################

sub unpack_cab_file
{
    my ($cabfilename, $temppath) = @_;

    installer::logger::print_message( "\n... unpacking cabinet file $cabfilename ...\n" );
    my $infoline = "Unpacking cabinet file $cabfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $dirname = $cabfilename;
    $dirname =~ s/\.cab\s*$//;
    my $workingpath = $temppath . $installer::globals::separator . "unpack_". $dirname . "_" . $$;
    if ( ! -d $workingpath ) { installer::systemactions::create_directory($workingpath); }

    # changing into unpack directory
    my $from = cwd();
    chdir($workingpath);

    my $fullcabfilename = $from . $installer::globals::separator . $cabfilename;

    if( $^O =~ /cygwin/i )
    {
        $fullcabfilename = qx{cygpath -w "$fullcabfilename"};
        $fullcabfilename =~ s/\\/\\\\/g;
        $fullcabfilename =~ s/\s*$//g;
    }

    my $systemcall = "cabarc.exe" . " -p X " . $fullcabfilename;
    $success = make_systemcall($systemcall, $systemcall);
    if ( ! $success ) { installer::exiter::exit_program("ERROR: Could not unpack cabinet file: $fullcabfilename!", "unpack_cab_file"); }

    # returning to directory
    chdir($from);

    return $workingpath;
}

########################################################
# Returning the header of a ddf file.
########################################################

sub get_ddf_file_header
{
    my ($ddffileref, $cabinetfile, $installdir) = @_;

    my $oneline;
    my $compressionlevel = 2;

    if( $^O =~ /cygwin/i )
    {
        $installdir = qx{cygpath -w "$installdir"};
        $installdir =~ s/\s*$//g;
    }

    $oneline = ".Set CabinetName1=" . $cabinetfile . "\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set ReservePerCabinetSize=128\n";  # This reserves space for a digital signature.
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set MaxDiskSize=2147483648\n";     # This allows the .cab file to get a size of 2 GB.
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set CompressionType=LZX\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set Compress=ON\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set CompressionLevel=$compressionlevel\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set Cabinet=ON\n";
    push(@{$ddffileref} ,$oneline);
    $oneline = ".Set DiskDirectoryTemplate=" . $installdir . "\n";
    push(@{$ddffileref} ,$oneline);
}

########################################################
# Writing content into ddf file.
########################################################

sub put_all_files_into_ddffile
{
    my ($ddffile, $allfiles, $workingpath) = @_;

    $workingpath =~ s/\//\\/g;

    for ( my $i = 0; $i <= $#{$allfiles}; $i++ )
    {
        my $filename = ${$allfiles}[$i];
        if( $^O =~ /cygwin/i ) { $filename =~ s/\//\\/g; } # Backslash for Cygwin!
        if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file: $filename!", "put_all_files_into_ddffile"); }
        my $infoline = "\"" . $filename . "\"" . " " . ${$allfiles}[$i] . "\n";
        push( @{$ddffile}, $infoline);
    }
}

########################################################
# Packing a cabinet file.
########################################################

sub do_pack_cab_file
{
    my ($cabfilename, $allfiles, $workingpath, $temppath) = @_;

    installer::logger::print_message( "\n... packing cabinet file $cabfilename ...\n" );
    my $infoline = "Packing cabinet file $cabfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( -f $cabfilename ) { unlink($cabfilename); } # removing cab file
    if ( -f $cabfilename ) { installer::exiter::exit_program("ERROR: Failed to remove file: $cabfilename!", "do_pack_cab_file"); }

    # generate ddf file for makecab.exe
    my @ddffile = ();

    my $dirname = $cabfilename;
    $dirname =~ s/\.cab\s*$//;
    my $ddfpath = $temppath . $installer::globals::separator . "ddf_". $dirname . "_" . $$;

    my $ddffilename = $cabfilename;
    $ddffilename =~ s/.cab/.ddf/;
    $ddffilename = $ddfpath . $installer::globals::separator . $ddffilename;

    if ( ! -d $ddfpath ) { installer::systemactions::create_directory($ddfpath); }

    my $from = cwd();

    chdir($workingpath); # changing into the directory with the unpacked files

    get_ddf_file_header(\@ddffile, $cabfilename, $from);
    put_all_files_into_ddffile(\@ddffile, $allfiles, $workingpath);
    # lines in ddf files must not be longer than 256 characters
    check_ddf_file(\@ddffile, $ddffilename);

    installer::files::save_file($ddffilename, \@ddffile);

    if( $^O =~ /cygwin/i )
    {
        $ddffilename = qx{cygpath -w "$ddffilename"};
        $ddffilename =~ s/\\/\\\\/g;
        $ddffilename =~ s/\s*$//g;
    }

    my $systemcall = "makecab.exe /V1 /F " . $ddffilename;
    my $success = make_systemcall($systemcall, $systemcall);
    if ( ! $success ) { installer::exiter::exit_program("ERROR: Could not pack cabinet file!", "do_pack_cab_file"); }

    chdir($from);

    return ($success);
}

########################################################
# Extraction the file extension from a file
########################################################

sub get_extension
{
    my ( $file ) = @_;

    my $extension = "";

    if ( $file =~ /^\s*(.*)\.(\w+?)\s*$/ ) { $extension = $2; }

    return $extension;
}

########################################################
# Checking, if a file already contains a certificate.
# This must not be overwritten.
########################################################

sub already_certified
{
    my ( $filename ) = @_;

    my $success = 1;
    my $is_certified = 0;

    my $systemcall = "signtool.exe verify /q /pa \"$filename\"";
    my $returnvalue = system($systemcall);

    if ( $returnvalue ) { $success = 0; }

     if ( $success )
     {
         $is_certified = 1;
        installer::logger::print_message( "... already certified -> skipping $filename ...\n" );
    }

    return $is_certified;
}

########################################################
# Signing the files, that are included into
# cabinet files.
########################################################

sub sign_files_in_cabinet_files
{
    my ( $followmeinfohash, $allcabfiles, $pw, $temppath ) = @_;

    my $complete_success = 1;
    my $from = cwd();

    foreach my $cabfilename ( keys %{$allcabfiles} )
    {
        my $success = 1;

        # saving order of files in cab file
        my $fileorder = read_cab_file($cabfilename);

        # unpack into $working path
        my $workingpath = unpack_cab_file($cabfilename, $temppath);

        chdir($workingpath);

        # sign files
        my %allfileshash = ();
        foreach my $onefile ( @{$fileorder} )
        {
            my $extension = get_extension($onefile);
            if ( exists( $installer::globals::sign_extensions{$extension} ) )
            {
                $allfileshash{$onefile} = 1;
            }
        }
         $success = sign_files($followmeinfohash, \%allfileshash, $pw, 1, 0, 0, $temppath);
        if ( ! $success ) { $complete_success = 0; }

        chdir($from);

        # pack into new directory
        do_pack_cab_file($cabfilename, $fileorder, $workingpath, $temppath);
    }

    return $complete_success;
}

########################################################
# Comparing the content of two directories.
# Only filesize is compared.
########################################################

sub compare_directories
{
    my ( $dir1, $dir2, $files ) = @_;

    $dir1 =~ s/\\\s*//;
    $dir2 =~ s/\\\s*//;
    $dir1 =~ s/\/\s*//;
    $dir2 =~ s/\/\s*//;

    my $infoline = "Comparing directories: $dir1 and $dir2\n";
    push( @installer::globals::logfileinfo, $infoline);

    foreach my $onefile ( @{$files} )
    {
        my $file1 = $dir1 . $installer::globals::separator . $onefile;
        my $file2 = $dir2 . $installer::globals::separator . $onefile;

        if ( ! -f $file1 ) { installer::exiter::exit_program("ERROR: Missing file : $file1!", "compare_directories"); }
        if ( ! -f $file2 ) { installer::exiter::exit_program("ERROR: Missing file : $file2!", "compare_directories"); }

        my $size1 = -s $file1;
        my $size2 = -s $file2;

        $infoline = "Comparing files: $file1 ($size1) and $file2 ($size2)\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ( $size1 != $size2 )
        {
            installer::exiter::exit_program("ERROR: File defect after copy (different size) $file1 ($size1 bytes) and $file2 ($size2 bytes)!", "compare_directories");
        }
    }
}

########################################################
# Signing an existing Windows installation set.
########################################################

sub sign_install_set
{
    my ($followmeinfohash, $make_copy, $temppath) = @_;

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
    my ($allcabfiles, $allfiles, $msidatabase, $contains_external_cabfiles, $contains_msidatabase, $sourcefiles) = analyze_installset_content($installsetpath);

    if ( $make_copy ) { compare_directories($installsetpath, $followmeinfohash->{'finalinstalldir'}, $sourcefiles); }

    # changing into installation set
    my $from = cwd();
    my $fullmsidatabase = $installsetpath . $installer::globals::separator . $msidatabase;

    if( $^O =~ /cygwin/i )
    {
        $fullmsidatabase = qx{cygpath -w "$fullmsidatabase"};
        $fullmsidatabase =~ s/\\/\\\\/g;
        $fullmsidatabase =~ s/\s*$//g;
    }

    chdir($installsetpath);

    if ( $contains_msidatabase )
    {
        # exclude media table from msi database and get all diskids.
        my ( $cabfilehash, $filenamehash, $lastsequencehash ) = collect_diskid_from_media_table($msidatabase, $followmeinfohash->{'languagestring'});

        # Check, if there are internal cab files
        my ( $contains_internal_cabfiles, $all_internal_cab_files) = check_for_internal_cabfiles($cabfilehash);

        if ( $contains_internal_cabfiles )
        {
            my $cabpath = get_cab_path($temppath);
            chdir($cabpath);

            # Exclude all cabinet files from database
            $success = extract_cabs_from_database($fullmsidatabase, $all_internal_cab_files);
            if ( ! $success ) { $complete_success = 0; }

            if ( $installer::globals::internal_cabinet_signing ) { sign_files_in_cabinet_files($followmeinfohash, $all_internal_cab_files, $pw, $temppath); }

            $success = sign_files($followmeinfohash, $all_internal_cab_files, $pw, 0, $filenamehash, $lastsequencehash, $temppath);
            if ( ! $success ) { $complete_success = 0; }
            $success = msicert_database($fullmsidatabase, $all_internal_cab_files, $cabfilehash, 1);
            if ( ! $success ) { $complete_success = 0; }

            # Include all cabinet files into database
            $success = include_cabs_into_database($fullmsidatabase, $all_internal_cab_files);
            if ( ! $success ) { $complete_success = 0; }
            chdir($installsetpath);
        }

        # Warning: There might be a problem with very big cabinet files
        # signing all external cab files first
        if ( $contains_external_cabfiles )
        {
            if ( $installer::globals::internal_cabinet_signing ) { sign_files_in_cabinet_files($followmeinfohash, $allcabfiles, $pw, $temppath); }

            $success = sign_files($followmeinfohash, $allcabfiles, $pw, 0, $filenamehash, $lastsequencehash, $temppath);
            if ( ! $success ) { $complete_success = 0; }
            $success = msicert_database($msidatabase, $allcabfiles, $cabfilehash, 0);
            if ( ! $success ) { $complete_success = 0; }
        }
    }

    # finally all other files can be signed
    $success = sign_files($followmeinfohash, $allfiles, $pw, 0, 0, 0, $temppath);
    if ( ! $success ) { $complete_success = 0; }

    # and changing back
    chdir($from);

    installer::logger::include_header_into_logfile("End: Signing installation set $installsetpath");

    return ($installsetpath);
}

1;
