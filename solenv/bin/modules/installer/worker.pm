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

package installer::worker;

use Cwd;
use File::Copy;
use File::stat;
use File::Temp qw(tmpnam);
use File::Path;
use installer::control;
use installer::converter;
use installer::existence;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::mail;
use installer::pathanalyzer;
use installer::scpzipfiles;
use installer::scriptitems;
use installer::sorter;
use installer::systemactions;
use installer::windows::language;

#####################################################################
# Unpacking all files ending with tar.gz in a specified directory
#####################################################################

sub unpack_all_targzfiles_in_directory
{
    my ( $directory ) = @_;

    installer::logger::include_header_into_logfile("Unpacking tar.gz files:");

    installer::logger::print_message( "... unpacking tar.gz files ... \n" );

    my $localdirectory = $directory . $installer::globals::separator . "packages";
    my $alltargzfiles = installer::systemactions::find_file_with_file_extension("tar.gz", $localdirectory);

    for ( my $i = 0; $i <= $#{$alltargzfiles}; $i++ )
    {
        my $onefile = $localdirectory . $installer::globals::separator . ${$alltargzfiles}[$i];

        my $systemcall = "cd $localdirectory; cat ${$alltargzfiles}[$i] \| gunzip \| tar -xf -";
        $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Executed \"$systemcall\" successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

#########################################
# Copying installation sets to ship
#########################################

sub copy_install_sets_to_ship
{
    my ( $destdir, $shipinstalldir  ) = @_;

    installer::logger::include_header_into_logfile("Copying installation set to ship:");

    my $dirname = $destdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$dirname);
    $dirname = $dirname . "_inprogress";
    my $localshipinstalldir = $shipinstalldir . $installer::globals::separator . $dirname;
    if ( ! -d $localshipinstalldir ) { installer::systemactions::create_directory_structure($localshipinstalldir); }

    # copy installation set to /ship ($localshipinstalldir)
    installer::logger::print_message( "... copy installation set from " . $destdir . " to " . $localshipinstalldir . "\n" );
    installer::systemactions::copy_complete_directory($destdir, $localshipinstalldir);

    # unpacking the tar.gz file for Solaris
    if ( $installer::globals::issolarisbuild ) { unpack_all_targzfiles_in_directory($localshipinstalldir); }

    $localshipinstalldir = installer::systemactions::rename_string_in_directory($localshipinstalldir, "_inprogress", "");

    return $localshipinstalldir;
}

#########################################
# Copying installation sets to ship
#########################################

sub link_install_sets_to_ship
{
    my ( $destdir, $shipinstalldir  ) = @_;

    installer::logger::include_header_into_logfile("Linking installation set to ship:");

    my $infoline = "... destination directory: $shipinstalldir ...\n";
    installer::logger::print_message( $infoline );
    push( @installer::globals::logfileinfo, $infoline);

    if ( ! -d $shipinstalldir)
    {
        $infoline = "Creating directory: $shipinstalldir\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::systemactions::create_directory_structure($shipinstalldir);
        $infoline = "Created directory: $shipinstalldir\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    my $dirname = $destdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$dirname);

    my $localshipinstalldir = $shipinstalldir . $installer::globals::separator . $dirname;

    # link installation set to /ship ($localshipinstalldir)
    installer::logger::print_message( "... linking installation set from " . $destdir . " to " . $localshipinstalldir . "\n" );

    my $systemcall = "ln -s $destdir $localshipinstalldir";

    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not create link \"$localshipinstalldir\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Created link \"$localshipinstalldir\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $localshipinstalldir;
}

#########################################
# Create checksum file
#########################################

sub make_checksum_file
{
    my ( $filesref, $includepatharrayref ) = @_;

    my @checksum = ();

    my $checksumfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$installer::globals::checksumfile, $includepatharrayref, 1);
    if ( $$checksumfileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file $installer::globals::checksumfile !", "make_checksum_file"); }

    my $systemcall = "$$checksumfileref";

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        $systemcall = $systemcall . " " . $onefile->{'sourcepath'};     # very very long systemcall

        if ((( $i > 0 ) &&  ( $i%100 == 0 )) || ( $i == $#{$filesref} ))    # limiting to 100 files
        {
            $systemcall = $systemcall . " \|";

            my @localchecksum = ();
            open (CHECK, "$systemcall");
            @localchecksum = <CHECK>;
            close (CHECK);

            for ( my $j = 0; $j <= $#localchecksum; $j++ ) { push(@checksum, $localchecksum[$j]); }

            $systemcall = "$$checksumfileref";  # reset the system call
        }
    }

    return \@checksum;
}

#########################################
# Saving the checksum file
#########################################

sub save_checksum_file
{
    my ($current_install_number, $installchecksumdir, $checksumfile) = @_;

    my $numberedchecksumfilename = $installer::globals::checksumfilename;
    $numberedchecksumfilename =~ s/\./_$current_install_number\./;  # checksum.txt -> checksum_01.txt
    installer::files::save_file($installchecksumdir . $installer::globals::separator . $numberedchecksumfilename, $checksumfile);
}

#################################################
# Writing some global information into
# the list of files without flag PATCH
#################################################

sub write_nopatchlist_header
{
    my ( $content ) = @_;

    my @header = ();
    my $infoline = "This is a list of files, that are defined in scp-projects without\n";
    push(@header, $infoline);
    $infoline = "flag \"PATCH\". Important: This does not mean in any case, that \n";
    push(@header, $infoline);
    $infoline = "this files are included into or excluded from a patch. \n\n";
    push(@header, $infoline);
    $infoline = "Exception Linux: A patch rpm is a complete rpm. This means that all \n";
    push(@header, $infoline);
    $infoline = "files are included into a patch rpm, if only one file of the rpm has the \n";
    push(@header, $infoline);
    $infoline = "style \"PATCH\". \n\n";
    push(@header, $infoline);

    for ( my $i = 0; $i <= $#header; $i++ ) { push(@{$content},$header[$i]); }
}

#################################################
# Creating the content of the list of files
# without flag PATCH.
# All files are saved in
# @{$installer::globals::nopatchfilecollector}
#################################################

sub create_nopatchlist
{
    my @content =();

    write_nopatchlist_header(\@content);

    for ( my $i = 0; $i <= $#{$installer::globals::nopatchfilecollector}; $i++ )
    {
        my $onefile = ${$installer::globals::nopatchfilecollector}[$i];
        my $oneline = $onefile->{'destination'};
        if ( $onefile->{'zipfilename'} ) { $oneline = $oneline . " (" . $onefile->{'zipfilename'} . ")"; }
        $oneline = $oneline . "\n";
        push(@content, $oneline);
    }

    return \@content;
}

#########################################
# Saving the patchlist file
#########################################

sub save_patchlist_file
{
    my ($installlogdir, $patchlistfilename) = @_;

    my $installpatchlistdir = installer::systemactions::create_directory_next_to_directory($installlogdir, "patchlist");
    $patchlistfilename =~ s/log\_/patchfiles\_/;
    $patchlistfilename =~ s/\.log/\.txt/;
    installer::files::save_file($installpatchlistdir . $installer::globals::separator . $patchlistfilename, \@installer::globals::patchfilecollector);
    installer::logger::print_message( "... creating patchlist file $patchlistfilename \n" );

    if (( $installer::globals::patch ) && ( ! $installer::globals::creating_windows_installer_patch ))  # only for non-Windows patches
    {
        $patchlistfilename =~ s/patchfiles\_/nopatchfiles\_/;
        my $nopatchlist = create_nopatchlist();
        installer::files::save_file($installpatchlistdir . $installer::globals::separator . $patchlistfilename, $nopatchlist);
        installer::logger::print_message( "... creating patch exclusion file $patchlistfilename \n" );
    }

}

###############################################################
# Removing all directories of a special language
# in the directory $basedir
###############################################################

sub remove_old_installation_sets
{
    my ($basedir) = @_;

    installer::logger::print_message( "... removing old installation directories ...\n" );

    my $removedir = $basedir;

    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    # looking for non successful old installation sets

    $removedir = $basedir . "_witherror";
    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    $removedir = $basedir . "_inprogress";
    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    # finally the $basedir can be created empty

    if ( $installer::globals::localinstalldirset ) { installer::systemactions::create_directory_structure($basedir); }

    installer::systemactions::create_directory($basedir);
}

###############################################################
# Removing all non successful installation sets on ship
###############################################################

sub remove_old_ship_installation_sets
{
    my ($fulldir, $counter) = @_;

    installer::logger::print_message( "... removing old installation directories ...\n" );

    my $basedir = $fulldir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    # collecting all directories next to the new installation directory
    my $alldirs = installer::systemactions::get_all_directories($basedir);

    if ( $fulldir =~ /^\s*(.*?inprogress\-)(\d+)(.*?)\s*$/ )
    {
        my $pre_inprogress = $1;        # $pre still contains "inprogress"
        my $number = $2;
        my $post = $3;
        my $pre_witherror = $pre_inprogress;
        $pre_witherror =~ s/inprogress/witherror/;

        for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
        {
            if ( ${$alldirs}[$i] eq $fulldir ) { next; }    # do not delete the newly created directory

            if ( ${$alldirs}[$i] =~ /^\s*\Q$pre_inprogress\E\d+\Q$post\E\s*$/ ) # removing old "inprogress" directories
            {
                installer::systemactions::remove_complete_directory(${$alldirs}[$i], 1);
            }

            if ( ${$alldirs}[$i] =~ /^\s*\Q$pre_witherror\E\d+\Q$post\E\s*$/ )  # removing old "witherror" directories
            {
                installer::systemactions::remove_complete_directory(${$alldirs}[$i], 1);
            }
        }
    }
}

###############################################################
# Creating the installation directory structure
###############################################################

sub create_installation_directory
{
    my ($shipinstalldir, $languagestringref, $current_install_number_ref) = @_;

    my $installdir = "";

    my $languageref = $languagestringref;

    if ( $installer::globals::updatepack )
    {
        $installdir = $shipinstalldir;
        installer::systemactions::create_directory_structure($installdir);
        $$current_install_number_ref = installer::systemactions::determine_maximum_number($installdir, $languageref);
        $installdir = installer::systemactions::rename_string_in_directory($installdir, "number", $$current_install_number_ref);
        remove_old_ship_installation_sets($installdir);
    }
    else
    {
        $installdir = installer::systemactions::create_directories("install", $languageref);
        installer::logger::print_message( "... creating installation set in $installdir ...\n" );
        remove_old_installation_sets($installdir);
        my $inprogressinstalldir = $installdir . "_inprogress";
        installer::systemactions::rename_directory($installdir, $inprogressinstalldir);
        $installdir = $inprogressinstalldir;
    }

    $installer::globals::saveinstalldir = $installdir;  # saving directory globally, in case of exiting

    return $installdir;
}

###############################################################
# Analyzing and creating the log file
###############################################################

sub analyze_and_save_logfile
{
    my ($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number) = @_;

    my $is_success = 1;
    my $finalinstalldir = "";

    installer::logger::print_message( "... checking log file " . $loggingdir . $installer::globals::logfilename . "\n" );

    my $contains_error = installer::control::check_logfile(\@installer::globals::logfileinfo);

    # Dependent from the success, the installation directory can be renamed and mails can be send.

    if ( $contains_error )
    {
        my $errordir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "_witherror");
        if ( $installer::globals::updatepack ) { installer::mail::send_fail_mail($allsettingsarrayref, $languagestringref, $errordir); }
        # Error output to STDERR
        for ( my $j = 0; $j <= $#installer::globals::errorlogfileinfo; $j++ )
        {
            my $line = $installer::globals::errorlogfileinfo[$j];
            $line =~ s/\s*$//g;
            installer::logger::print_error( $line );
        }
        $is_success = 0;

        $finalinstalldir = $errordir;
    }
    else
    {
        my $destdir = "";

        if ( $installer::globals::updatepack )
        {
            if ( $installdir =~ /_download_inprogress/ ) { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_download_inprogress", "_download"); }
            elsif ( $installdir =~ /_jds_inprogress/ ) { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_jds_inprogress", "_jds"); }
            elsif ( $installdir =~ /_msp_inprogress/ ) { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_msp_inprogress", "_msp"); }
            else
            {
                if ( $installdir =~ /_packed/ ) { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", ""); }
                else { $destdir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "_packed"); }
            }
            installer::mail::send_success_mail($allsettingsarrayref, $languagestringref, $destdir);
        }
        else
        {
            $destdir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "");
        }

        $finalinstalldir = $destdir;
    }

    # Saving the logfile in the log file directory and additionally in a log directory in the install directory

    my $numberedlogfilename = $installer::globals::logfilename;
    if ( $installer::globals::updatepack ) { $numberedlogfilename =~ s /log_/log_$current_install_number\_/; }
    installer::logger::print_message( "... creating log file $numberedlogfilename \n" );
    installer::files::save_file($loggingdir . $numberedlogfilename, \@installer::globals::logfileinfo);
    installer::files::save_file($installlogdir . $installer::globals::separator . $numberedlogfilename, \@installer::globals::logfileinfo);

    # Saving the checksumfile in a checksum directory in the install directory
    # installer::worker::save_checksum_file($current_install_number, $installchecksumdir, $checksumfile);

    # Saving the list of patchfiles in a patchlist directory in the install directory
    if (( $installer::globals::patch ) || ( $installer::globals::creating_windows_installer_patch )) { installer::worker::save_patchlist_file($installlogdir, $numberedlogfilename); }

    if ( $installer::globals::creating_windows_installer_patch ) { $installer::globals::creating_windows_installer_patch = 0; }

    # Exiting the packaging process, if an error occurred.
    # This is important, to get an error code "-1", if an error was found in the log file,
    # that did not break the packaging process

    if ( ! $is_success) { installer::exiter::exit_program("ERROR: Found an error in the logfile. Packaging failed.", "analyze_and_save_logfile"); }

    return ($is_success, $finalinstalldir);
}

###############################################################
# Analyzing and creating the log file
###############################################################

sub save_logfile_after_linking
{
    my ($loggingdir, $installlogdir, $current_install_number) = @_;

    # Saving the logfile in the log file directory and additionally in a log directory in the install directory
    my $numberedlogfilename = $installer::globals::logfilename;
    if ( $installer::globals::updatepack ) { $numberedlogfilename =~ s /log_/log_$current_install_number\_/; }
    installer::logger::print_message( "... creating log file $numberedlogfilename \n" );
    installer::files::save_file($loggingdir . $numberedlogfilename, \@installer::globals::logfileinfo);
    installer::files::save_file($installlogdir . $installer::globals::separator . $numberedlogfilename, \@installer::globals::logfileinfo);
}

###############################################################
# Removing all directories that are saved in the
# global directory @installer::globals::removedirs
###############################################################

sub clean_output_tree
{
    installer::logger::print_message( "... cleaning the output tree ...\n" );

    for ( my $i = 0; $i <= $#installer::globals::removedirs; $i++ )
    {
        if ( -d $installer::globals::removedirs[$i] )
        {
            installer::logger::print_message( "... removing directory $installer::globals::removedirs[$i] ...\n" );
            installer::systemactions::remove_complete_directory($installer::globals::removedirs[$i], 1);
        }
    }

    # Last try to remove the ship test directory

    if ( $installer::globals::shiptestdirectory )
    {
        if ( -d $installer::globals::shiptestdirectory )
        {
            my $infoline = "Last try to remove $installer::globals::shiptestdirectory . \n";
            push(@installer::globals::logfileinfo, $infoline);
            my $systemcall = "rmdir $installer::globals::shiptestdirectory";
            my $returnvalue = system($systemcall);
        }
    }
}

###############################################################
# Removing all directories that are saved in the
# global directory @installer::globals::jdsremovedirs
###############################################################

sub clean_jds_temp_dirs
{
    installer::logger::print_message( "... cleaning jds directories ...\n" );

    for ( my $i = 0; $i <= $#installer::globals::jdsremovedirs; $i++ )
    {
        if ( -d $installer::globals::jdsremovedirs[$i] )
        {
            installer::logger::print_message( "... removing directory $installer::globals::jdsremovedirs[$i] ...\n" );
            installer::systemactions::remove_complete_directory($installer::globals::jdsremovedirs[$i], 1);
        }
    }
}

###########################################################
# Setting one language in the language independent
# array of include pathes with $(LANG)
###########################################################

sub get_language_specific_include_pathes
{
    my ( $patharrayref, $onelanguage ) = @_;

    my @patharray = ();

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        my $line = ${$patharrayref}[$i];
        $line =~ s/\$\(LANG\)/$onelanguage/g;
        push(@patharray ,$line);
    }

    return \@patharray;
}

##############################################################
# Returning the first item with a defined flag
##############################################################

sub return_first_item_with_special_flag
{
    my ($itemsref, $flag) = @_;

    my $firstitem = "";

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'} };

        if ( $styles =~ /\b$flag\b/ )
        {
            $firstitem = $oneitem;
            last;
        }
    }

    return $firstitem;
}

##############################################################
# Collecting all items with a defined flag
##############################################################

sub collect_all_items_with_special_flag
{
    my ($itemsref, $flag) = @_;

    my @allitems = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'} };

        if ( $styles =~ /\b$flag\b/ )
        {
            push( @allitems, $oneitem );
        }
    }

    return \@allitems;
}

##############################################################
# Collecting all files without patch flag in
# $installer::globals::nopatchfilecollector
##############################################################

sub collect_all_files_without_patch_flag
{
    my ($filesref) = @_;

    my $newfiles = collect_all_items_without_special_flag($filesref, "PATCH");

    for ( my $i = 0; $i <= $#{$newfiles}; $i++ ) { push(@{$installer::globals::nopatchfilecollector}, ${$newfiles}[$i]); }
}

##############################################################
# Collecting all items without a defined flag
##############################################################

sub collect_all_items_without_special_flag
{
    my ($itemsref, $flag) = @_;

    my @allitems = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'} };

        if ( !( $styles =~ /\b$flag\b/ ))
        {
            push( @allitems, $oneitem );
        }
    }

    return \@allitems;
}

##############################################################
# Removing all items with a defined flag from collector
##############################################################

sub remove_all_items_with_special_flag
{
    my ($itemsref, $flag) = @_;

    my @allitems = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'} };
        if ( $styles =~ /\b$flag\b/ )
        {
            my $infoline = "Attention: Removing from collector: $oneitem->{'Name'} !\n";
            push( @installer::globals::logfileinfo, $infoline);
            if ( $flag eq "BINARYTABLE_ONLY" ) { push(@installer::globals::binarytableonlyfiles, $oneitem); }
            next;
        }
        push( @allitems, $oneitem );
    }

    return \@allitems;
}

###########################################################
# Mechanism for simple installation without packing
###########################################################

sub install_simple ($$$$$$)
{
    my ($packagename, $languagestring, $directoriesarray, $filesarray, $linksarray, $unixlinksarray) = @_;

        # locate GNU cp on the system
        my $gnucp = 'cp';
        if ( $ENV{'GNUCOPY'} ) { $gnucp = $ENV{'GNUCOPY'}; }
    my $copyopts = '-af';
    $copyopts = '-PpRf' unless ( $ENV{'GNUCOPY'} ); # if not gnucopy, assume POSIX copy

    installer::logger::print_message( "... installing module $packagename ...\n" );

    my $destdir = $installer::globals::destdir;
    my @lines = ();

    installer::logger::print_message( "DestDir: $destdir \n" );
    installer::logger::print_message( "Rootpath: $installer::globals::rootpath \n" );

    `mkdir -p $destdir` if $destdir ne "";
    `mkdir -p $destdir$installer::globals::rootpath`;

    # Create Directories
    for ( my $i = 0; $i <= $#{$directoriesarray}; $i++ )
    {
        my $onedir = ${$directoriesarray}[$i];
        my $dir = "";

        if ( $onedir->{'Dir'} ) { $dir = $onedir->{'Dir'}; }

        if ((!($dir =~ /\bPREDEFINED_/ )) || ( $dir =~ /\bPREDEFINED_PROGDIR\b/ ))
        {
            my $hostname = $onedir->{'HostName'};

            # ignore '.' subdirectories
            next if ( $hostname =~ m/\.$/ );
            # remove './' from the path
            $hostname =~ s/\.\///g;

            # printf "mkdir $destdir$hostname\n";
            mkdir $destdir . $hostname;
            push @lines, "%dir " . $hostname . "\n";
        }
    }

    for ( my $i = 0; $i <= $#{$filesarray}; $i++ )
    {
        my $onefile = ${$filesarray}[$i];
        my $unixrights = $onefile->{'UnixRights'};
        my $destination = $onefile->{'destination'};
        my $sourcepath = $onefile->{'sourcepath'};

        # This is necessary to install SDK that includes files with $ in its name
        # Otherwise, the following shell commands does not work and the file list
        # is not correct
        $destination =~ s/\$\$/\$/;
        $sourcepath =~ s/\$\$/\$/;

        # remove './' from the path
        $sourcepath =~ s/\.\///g;
        $destination =~ s/\.\///g;

        push @lines, "$destination\n";
        if(-d  "$destdir$destination"){
            rmtree("$destdir$destination");
        }
        if(-e "$destdir$destination") {
            unlink "$destdir$destination";
        }

        copy ("$sourcepath", "$destdir$destination") || die "Can't copy file: $sourcepath -> $destdir$destination $!";
        my $sourcestat = stat($sourcepath);
        utime ($sourcestat->atime, $sourcestat->mtime, "$destdir$destination");
        chmod (oct($unixrights), "$destdir$destination") || die "Can't change permissions: $!";
        push @lines, "$destination\n";
    }

    for ( my $i = 0; $i <= $#{$linksarray}; $i++ )
    {
        my $onelink = ${$linksarray}[$i];
        my $destination = $onelink->{'destination'};
        my $destinationfile = $onelink->{'destinationfile'};

        if(-e "$destdir$destination") {
            unlink "$destdir$destination";
        }
        symlink ("$destinationfile", "$destdir$destination") || die "Can't create symlink: $!";
        push @lines, "$destination\n";
    }

    for ( my $i = 0; $i <= $#{$unixlinksarray}; $i++ )
    {
        my $onelink = ${$unixlinksarray}[$i];
        my $target = $onelink->{'Target'};
        my $destination = $onelink->{'destination'};

        `ln -sf '$target' '$destdir$destination'`;
        push @lines, "$destination\n";
    }

    if ( $destdir ne "" )
    {
        my $filelist;
        my $fname = $installer::globals::destdir . "/$packagename";
        open ($filelist, ">$fname") || die "Can't open $fname: $!";
        print $filelist @lines;
        close ($filelist);
    }

}

###########################################################
# Adding shellnew files into files collector for
# user installation
###########################################################

sub add_shellnewfile_into_filesarray
{
    my ($filesref, $onefile, $inffile) = @_;

    my %shellnewfile = ();
    my $shellnewfileref = \%shellnewfile;

    installer::converter::copy_item_object($inffile, $shellnewfileref);

    $shellnewfileref->{'Name'} = $onefile->{'Name'};
    $shellnewfileref->{'sourcepath'} = $onefile->{'sourcepath'};
    $shellnewfileref->{'gid'} = $onefile->{'gid'} . "_Userinstall";

    # the destination has to be adapted
    my $destination = $inffile->{'destination'};
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
    $destination = $destination . $onefile->{'Name'};
    $shellnewfileref->{'destination'} = $destination;

    # add language specific inffile into filesarray
    push(@{$filesref}, $shellnewfileref);
}

###########################################################
# Replacing one placehoder in template file
###########################################################

sub replace_in_template_file
{
    my ($templatefile, $placeholder, $newstring) = @_;

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        ${$templatefile}[$i] =~ s/\Q$placeholder\E/$newstring/g;
    }
}

###########################################################
# Replacing one placehoder with an array in template file
###########################################################

sub replace_array_in_template_file
{
    my ($templatefile, $placeholder, $arrayref) = @_;

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        if ( ${$templatefile}[$i] =~ /\Q$placeholder\E/ )
        {
            my @return = splice(@{$templatefile}, $i, 1, @{$arrayref});
        }
    }
}

###########################################################
# Collecting all modules from registry items
###########################################################

sub collect_all_modules
{
    my ($registryitemsref) = @_;

    my @allmodules = ();

    for ( my $i = 0; $i <= $#{$registryitemsref}; $i++ )
    {
        $registryitem = ${$registryitemsref}[$i];
        my $module = $registryitem->{'ModuleID'};

        if ( ! installer::existence::exists_in_array($module, \@allmodules) )
        {
            push(@allmodules, $module);
        }
    }

    return \@allmodules;
}

###########################################################
# Changing the content of the inf file
###########################################################

sub write_content_into_inf_file
{
    my ($templatefile, $filesref, $registryitemsref, $folderref, $folderitemsref, $modulesref, $onelanguage, $inffile, $firstlanguage, $allvariableshashref) = @_;

    # First part: Shellnew files
    # SHELLNEWFILESPLACEHOLDER

    my $rootmodule = 0;
    # inf files can be assigned to "gid_Module_Root_Files_2"
    if ( $inffile->{'modules'} =~ /Module_Root/i ) { $rootmodule = 1; }

    if ( $rootmodule )
    {
        my $shellnewstring = "";

        for ( my $i = 0; $i <= $#{$filesref}; $i++ )
        {
            my $onefile = ${$filesref}[$i];
            my $directory = $onefile->{'Dir'};

            if ( $directory =~ /\bPREDEFINED_OSSHELLNEWDIR\b/ )
            {
                $shellnewstring = $shellnewstring . $onefile->{'Name'} . "\n";
                if (( $firstlanguage ) && ( ! $installer::globals::shellnewfilesadded )) { add_shellnewfile_into_filesarray($filesref, $onefile, $inffile); }
            }
        }

        $shellnewstring =~ s/\s*$//;
        replace_in_template_file($templatefile, "SHELLNEWFILESPLACEHOLDER", $shellnewstring);

        $installer::globals::shellnewfilesadded = 1;
    }

    # Second part: Start menu entries

    # The OfficeMenuFolder is defined as: $productname . " " . $productversion;

    my $productname = $allvariableshashref->{'PRODUCTNAME'};
    my $productversion = $allvariableshashref->{'PRODUCTVERSION'};
    my $productkey = $productname . " " . $productversion;

    replace_in_template_file($templatefile, "OFFICEFOLDERPLACEHOLDER", $productkey);

    # Setting name target and infotip for all applications

    for ( my $i = 0; $i <= $#{$folderitemsref}; $i++ )
    {
        my $folderitem = ${$folderitemsref}[$i];

        my $styles = "";
        if ( $folderitem->{'Styles'} ) { $styles = $folderitem->{'Styles'}; }
        if ( $styles =~ /\bNON_ADVERTISED\b/ ) { next; }    # no entry for non-advertised shortcuts

        if (( ! $folderitem->{'ismultilingual'} ) || (( $folderitem->{'ismultilingual'} ) && ( $folderitem->{'specificlanguage'} eq $onelanguage )))
        {
            my $gid = $folderitem->{'gid'};
            my $app = $gid;
            $app =~ s/gid_Folderitem_//;
            $app = uc($app);

            my $name = $folderitem->{'Name'};
            my $placeholder = "PLACEHOLDER_FOLDERITEM_NAME_" . $app;
            replace_in_template_file($templatefile, $placeholder, $name);

            my $tooltip = $folderitem->{'Tooltip'};
            $placeholder = "PLACEHOLDER_FOLDERITEM_TOOLTIP_" . $app;
            replace_in_template_file($templatefile, $placeholder, $tooltip);

            my $executablegid = $folderitem->{'FileID'};
            my $exefile = installer::existence::get_specified_file($filesref, $executablegid);
            my $exefilename = $exefile->{'Name'};
            $placeholder = "PLACEHOLDER_FOLDERITEM_TARGET_" . $app;
            replace_in_template_file($templatefile, $placeholder, $exefilename);
        }
    }

    # Third part: Windows registry entries

    # collecting all modules

    my $allmodules = collect_all_modules($registryitemsref);

    my @registryitems = ();
    my $allsectionsstring = "";

    for ( my $j = 0; $j <= $#{$allmodules}; $j++ )
    {
        my $moduleid = ${$allmodules}[$j];

        my $inffilemodule = $inffile->{'modules'};
        # inf files can be assigned to "gid_Module_Root_Files_2", but RegistryItems to "gid_Module_Root"
        if ( $inffilemodule =~ /Module_Root/i ) { $inffilemodule = $installer::globals::rootmodulegid; }

        if ( ! ( $moduleid eq $inffilemodule )) { next; }

        my $shortmodulename = $moduleid;
        $shortmodulename =~ s/gid_Module_//;
        my $sectionname = "InstRegKeys." . $shortmodulename;
        $allsectionsstring = $allsectionsstring . $sectionname . ",";
        my $sectionheader = "\[" . $sectionname . "\]" . "\n";
        push(@registryitems, $sectionheader);

        for ( my $i = 0; $i <= $#{$registryitemsref}; $i++ )
        {
            my $registryitem = ${$registryitemsref}[$i];

            if ( ! ( $registryitem->{'ModuleID'} eq $moduleid )) { next; }

            if (( ! $registryitem->{'ismultilingual'} ) || (( $registryitem->{'ismultilingual'} ) && ( $registryitem->{'specificlanguage'} eq $onelanguage )))
            {
                # Syntax: HKCR,".bau",,,"soffice.StarConfigFile.6"

                my $regroot = "";
                my $parentid = "";
                if ( $registryitem->{'ParentID'} ) { $parentid = $registryitem->{'ParentID'}; }
                if ( $parentid eq "PREDEFINED_HKEY_CLASSES_ROOT" ) { $regroot = "HKCR"; }
                if ( $parentid eq "PREDEFINED_HKEY_LOCAL_MACHINE" ) { $regroot = "HKCU"; }

                my $subkey = "";
                if ( $registryitem->{'Subkey'} ) { $subkey = $registryitem->{'Subkey'}; }
                if ( $subkey ne "" ) { $subkey = "\"" . $subkey . "\""; }

                my $valueentryname = "";
                if ( $registryitem->{'Name'} ) { $valueentryname = $registryitem->{'Name'}; }
                if ( $valueentryname ne "" ) { $valueentryname = "\"" . $valueentryname . "\""; }

                my $flag = "";

                my $value = "";
                if ( $registryitem->{'Value'} ) { $value = $registryitem->{'Value'}; }
                if ( $value =~ /\<progpath\>/ ) { $value =~ s/\\\"/\"\"/g; } # Quoting for INF is done by double ""
                $value =~ s/\\\"/\"/g;  # no more masquerading of '"'
                $value =~ s/\<progpath\>/\%INSTALLLOCATION\%/g;
                if ( $value ne "" ) { $value = "\"" . $value . "\""; }

                my $oneline = $regroot . "," . $subkey . "," . $valueentryname . "," . $flag . "," . $value . "\n";

                push(@registryitems, $oneline);
            }
        }

        push(@registryitems, "\n"); # empty line after each section
    }

    # replacing the $allsectionsstring
    $allsectionsstring =~ s/\,\s*$//;
    replace_in_template_file($templatefile, "ALLREGISTRYSECTIONSPLACEHOLDER", $allsectionsstring);

    # replacing the placeholder for all registry keys
    replace_array_in_template_file($templatefile, "REGISTRYKEYSPLACEHOLDER", \@registryitems);

}

###########################################################
# Creating inf files for local user system integration
###########################################################

sub create_inf_file
{
    my ($filesref, $registryitemsref, $folderref, $folderitemsref, $modulesref, $languagesarrayref, $languagestringref, $allvariableshashref) = @_;

    # collecting all files with flag INFFILE

    my $inf_files = collect_all_items_with_special_flag($filesref ,"INFFILE");

    if ( $#{$inf_files} > -1 )
    {
        # create new language specific inffile
        installer::logger::include_header_into_logfile("Creating inf files:");

        my $infdirname = "inffiles";
        my $infdir = installer::systemactions::create_directories($infdirname, $languagestringref);

        my $infoline = "Number of inf files: $#{$inf_files} + 1 \n";
        push( @installer::globals::logfileinfo, $infoline);

        # there are inffiles for all modules

        for ( my $i = 0; $i <= $#{$inf_files}; $i++ )
        {
            my $inffile = ${$inf_files}[$i];
            my $inf_file_name = $inffile->{'Name'};

            my $templatefilename = $inffile->{'sourcepath'};

            if ( ! -f $templatefilename ) { installer::exiter::exit_program("ERROR: Could not find file $templatefilename !", "create_inf_file");  }

            # iterating over all languages

            for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ )   # iterating over all languages
            {
                my $firstlanguage = 0;
                if ( $j == 0 ) { $firstlanguage = 1; }

                my $onelanguage = ${$languagesarrayref}[$j];

                $infoline = "Templatefile: $inf_file_name, Language: $onelanguage \n";
                push( @installer::globals::logfileinfo, $infoline);

                my $templatefile = installer::files::read_file($templatefilename);

                my $linesbefore = $#{$templatefile};

                write_content_into_inf_file($templatefile, $filesref, $registryitemsref, $folderref, $folderitemsref, $modulesref, $onelanguage, $inffile, $firstlanguage, $allvariableshashref);

                $infoline = "Lines change: From $linesbefore to $#{$templatefile}.\n";
                push( @installer::globals::logfileinfo, $infoline);

                # rename language specific inffile
                my $language_inf_file_name = $inf_file_name;
                my $windowslanguage = installer::windows::language::get_windows_language($onelanguage);
                $language_inf_file_name =~ s/\.inf/_$windowslanguage\.inf/;

                my $sourcepath = $infdir . $installer::globals::separator . $language_inf_file_name;
                installer::files::save_file($sourcepath, $templatefile);

                $infoline = "Saving file: $sourcepath\n";
                push( @installer::globals::logfileinfo, $infoline);

                # creating new file object

                my %languageinffile = ();
                my $languageinifileref = \%languageinffile;

                if ( $j < $#{$languagesarrayref} ) { installer::converter::copy_item_object($inffile, $languageinifileref); }
                else { $languageinifileref = $inffile; }

                $languageinifileref->{'Name'} = $language_inf_file_name;
                $languageinifileref->{'sourcepath'} = $sourcepath;
                # destination and gid also have to be adapted
                $languageinifileref->{'gid'} = $languageinifileref->{'gid'} . "_" . $onelanguage;
                my $destination = $languageinifileref->{'destination'};
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
                $destination = $destination . $language_inf_file_name;
                $languageinifileref->{'destination'} = $destination;

                # add language specific inffile into filesarray
                if ( $j < $#{$languagesarrayref} ) { push(@{$filesref}, $languageinifileref); }
            }
        }
    }
}

###########################################################
# Selecting patch items
###########################################################

sub select_patch_items
{
    my ( $itemsref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting items for patches. Item: $itemname");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        my $name = $oneitem->{'Name'};
        if (( $name =~ /\bLICENSE/ ) || ( $name =~ /\bREADME/ ))
        {
            push(@itemsarray, $oneitem);
            next;
        }

        # Items with style "PATCH" have to be included into the patch
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if ( $styles =~ /\bPATCH\b/ ) { push(@itemsarray, $oneitem); }
    }

    return \@itemsarray;
}

###########################################################
# Selecting patch items
###########################################################

sub select_patch_items_without_name
{
    my ( $itemsref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting RegistryItems for patches");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        # Items with style "PATCH" have to be included into the patch
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if ( $styles =~ /\bPATCH\b/ ) { push(@itemsarray, $oneitem); }
    }

    return \@itemsarray;
}

###########################################################
# Selecting langpack items
###########################################################

sub select_langpack_items
{
    my ( $itemsref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting RegistryItems for Language Packs");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        # Items with style "LANGUAGEPACK" have to be included into the patch
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if (( $styles =~ /\bLANGUAGEPACK\b/ ) || ( $styles =~ /\bFORCELANGUAGEPACK\b/ )) { push(@itemsarray, $oneitem); }
    }

    return \@itemsarray;
}

###########################################################
# Selecting helppack items
###########################################################

sub select_helppack_items
{
    my ( $itemsref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting RegistryItems for Help Packs");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        # Items with style "HELPPACK" have to be included into the patch
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if (( $styles =~ /\bHELPPACK\b/ ) || ( $styles =~ /\bFORCEHELPPACK\b/ )) { push(@itemsarray, $oneitem); }
    }

    return \@itemsarray;
}

###########################################################
# Searching if LICENSE and README, which are not removed
# in select_patch_items are really needed for the patch.
# If not, they are removed now.
###########################################################

sub analyze_patch_files
{
    my ( $filesref ) = @_;

    installer::logger::include_header_into_logfile("Analyzing patch files");

    my @filesarray = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( !( $styles =~ /\bPATCH\b/) ) { next; } # removing all files without flag PATCH (LICENSE, README, ...)

        if ( $installer::globals::iswindowsbuild )
        {
            # all files of the Windows patch belong to the root module
            $onefile->{'modules'} = $installer::globals::rootmodulegid;
        }

        push(@filesarray, $onefile);
    }

    return \@filesarray;
}

###########################################################
# Sorting an array
###########################################################

sub sort_array
{
    my ( $arrayref ) = @_;

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        my $under = ${$arrayref}[$i];

        for ( my $j = $i + 1; $j <= $#{$arrayref}; $j++ )
        {
            my $over = ${$arrayref}[$j];

            if ( $under gt $over)
            {
                ${$arrayref}[$i] = $over;
                ${$arrayref}[$j] = $under;
                $under = $over;
            }
        }
    }
}

###########################################################
# Renaming linux files with flag LINUXLINK
###########################################################

sub prepare_linuxlinkfiles
{
    my ( $filesref ) = @_;

    @installer::globals::linuxlinks = (); # empty this array, because it could be already used
    @installer::globals::linuxpatchfiles = (); # empty this array, because it could be already used
    @installer::globals::allfilessav = (); # empty this array, because it could be already used. Required for forced links

    my @filesarray = ();

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my %linkfilehash = ();
        my $linkfile = \%linkfilehash;
        installer::converter::copy_item_object($onefile, $linkfile);

        my $ispatchfile = 0;
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( $styles =~ /\bPATCH\b/ ) { $ispatchfile = 1; }

        # Collecting all files for the mechanism with forced links
        # Saving a copy
        my %copyfilehash = ();
        my $copyfile = \%copyfilehash;
        installer::converter::copy_item_object($onefile, $copyfile);
        push( @installer::globals::allfilessav, $copyfile);

        my $original_destination = $onefile->{'destination'};
        # $onefile->{'destination'} is used in the epm list file. This value can be changed now!

        if ( $ispatchfile ) { $onefile->{'destination'} = $onefile->{'destination'} . "\.$installer::globals::linuxlibrarypatchlevel"; }
        else { $onefile->{'destination'} = $onefile->{'destination'} . "\.$installer::globals::linuxlibrarybaselevel"; }

        my $infoline = "LINUXLINK: Changing file destination from $original_destination to $onefile->{'destination'} !\n";
        push( @installer::globals::logfileinfo, $infoline);

        # all files without PATCH flag are included into the RPM
        if ( ! $ispatchfile ) { push( @filesarray, $onefile); }
        else { push( @installer::globals::linuxpatchfiles, $onefile); }

        # Preparing the collector for the links
        # Setting the new file name as destination of the link
        my $linkdestination = $linkfile->{'Name'};
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$linkdestination);
        if ( $ispatchfile ) { $linkfile->{'destinationfile'} = $linkdestination . "\.$installer::globals::linuxlibrarypatchlevel"; }
        else { $linkfile->{'destinationfile'} = $linkdestination . "\.$installer::globals::linuxlibrarybaselevel"; }
        push( @installer::globals::linuxlinks, $linkfile );

        $infoline = "LINUXLINK: Created link: $linkfile->{'destination'} pointing to $linkfile->{'destinationfile'} !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return \@filesarray;
}

###########################################################
# Adding links into "u-RPMs", that have the flag
# FORCE_INTO_UPDATE_PACKAGE
# This is only relevant for Linux
###########################################################

sub prepare_forced_linuxlinkfiles
{
    my ( $linksref ) = @_;

    my @linksarray = ();

    for ( my $i = 0; $i <= $#{$linksref}; $i++ )
    {
        my $onelink = ${$linksref}[$i];

        my $isforcedlink = 0;
        my $styles = "";
        if ( $onelink->{'Styles'} ) { $styles = $onelink->{'Styles'}; }
        if ( $styles =~ /\bFORCE_INTO_UPDATE_PACKAGE\b/ ) { $isforcedlink = 1; }

        if ( $isforcedlink )
        {
            my $fileid = "";

            if ( $onelink->{'ShortcutID'} )
            {
                $fileid = $onelink->{'ShortcutID'};

                my $searchedlinkfile = find_file_by_id($linksref, $fileid);

                # making a copy!
                my %linkfilehash = ();
                my $linkfile = \%linkfilehash;
                installer::converter::copy_item_object($searchedlinkfile, $linkfile);

                $linkfile->{'Name'} = $onelink->{'Name'};
                $linkfile->{'destinationfile'} = $linkfile->{'destination'};
                my $linkdestination = $linkfile->{'destinationfile'};
                installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$linkdestination);
                $linkfile->{'destinationfile'} = $linkdestination;

                my $localdestination = $linkfile->{'destination'};
                # Getting the path
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$localdestination);
                $localdestination =~ s/\Q$installer::globals::separator\E\s*$//;
                $linkfile->{'destination'} = $localdestination . $installer::globals::separator . $onelink->{'Name'};

                $infoline = "Forced link into update file: $linkfile->{'destination'} pointing to $linkfile->{'destinationfile'} !\n";
                push( @installer::globals::logfileinfo, $infoline);

                # The file, defined by the link, has to be included into the
                # link array @installer::globals::linuxlinks
                push( @installer::globals::linuxlinks, $linkfile );
            }

            if ( $onelink->{'FileID'} )
            {
                $fileid = $onelink->{'FileID'};

                my $searchedlinkfile = find_file_by_id(\@installer::globals::allfilessav, $fileid);

                # making a copy!
                my %linkfilehash = ();
                my $linkfile = \%linkfilehash;
                installer::converter::copy_item_object($searchedlinkfile, $linkfile);

                $linkfile->{'Name'} = $onelink->{'Name'};
                $linkfile->{'destinationfile'} = $linkfile->{'destination'};
                my $linkdestination = $linkfile->{'destinationfile'};
                installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$linkdestination);
                $linkfile->{'destinationfile'} = $linkdestination;

                my $localdestination = $linkfile->{'destination'};
                # Getting the path
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$localdestination);
                $localdestination =~ s/\Q$installer::globals::separator\E\s*$//;
                $linkfile->{'destination'} = $localdestination . $installer::globals::separator . $onelink->{'Name'};

                $infoline = "Forced link into update file: $linkfile->{'destination'} pointing to $linkfile->{'destinationfile'} !\n";
                push( @installer::globals::logfileinfo, $infoline);

                # The file, defined by the link, has to be included into the
                # link array @installer::globals::linuxlinks
                push( @installer::globals::linuxlinks, $linkfile );
             }

            if ( $fileid eq "" ) { installer::exiter::exit_program("ERROR: No FileID assigned to forced link $onelink->{'gid'} !", "prepare_forced_linuxlinkfiles"); }

        }
        else
        {
            # Links with flag FORCE_INTO_UPDATE_PACKAGE are forced into "u"-RPM. All other
            # links are included into the non-"u"-package.
            push( @linksarray, $onelink );
        }
    }

    return \@linksarray;
}

###########################################################
# reorganizing the patchfile content,
# sorting for directory to decrease the file size
###########################################################

sub reorg_patchfile
{
    my ($patchfiles, $patchfiledirectories) = @_;

    my @patchfilesarray = ();
    my $line = "";
    my $directory = "";

    # iterating over all directories, writing content into new patchfiles list

    for ( my $i = 0; $i <= $#{$patchfiledirectories}; $i++ )
    {
        $directory = ${$patchfiledirectories}[$i];
        $line = "[" . $directory . "]" . "\n";
        push(@patchfilesarray, $line);

        for ( my $j = 0; $j <= $#{$patchfiles}; $j++ )
        {
            if ( ${$patchfiles}[$j] =~ /^\s*(.*?)\s*\tXXXXX\t\Q$directory\E\s*$/ )
            {
                $line = $1 . "\n";
                push(@patchfilesarray, $line);
            }
        }
    }

    return \@patchfilesarray;
}

###########################################################
# One special file has to be the last in patchfile.txt.
# Controlling this file, guarantees, that all files were
# patch correctly. Using version.ini makes it easy to
# control this by looking into the about box
# -> shifting one section to the end
###########################################################

sub shift_section_to_end
{
    my ($patchfilelist) = @_;

    my @patchfile = ();
    my @lastsection = ();
    my $lastsection = "program";
    my $notlastsection = "Basis\\program";
    my $record = 0;

    for ( my $i = 0; $i <= $#{$patchfilelist}; $i++ )
    {
        my $line = ${$patchfilelist}[$i];

        if (( $record ) && ( $line =~ /^\s*\[/ )) { $record = 0; }

        if (( $line =~ /^\s*\[\Q$lastsection\E\\\]\s*$/ ) && ( ! ( $line =~ /\Q$notlastsection\E\\\]\s*$/ ))) { $record = 1; }

        if ( $record ) { push(@lastsection, $line); }
        else { push(@patchfile, $line); }
    }

    if ( $#lastsection > -1 )
    {
        for ( my $i = 0; $i <= $#lastsection; $i++ )
        {
            push(@patchfile, $lastsection[$i]);
        }
    }

    return \@patchfile;
}

###########################################################
# One special file has to be the last in patchfile.txt.
# Controlling this file, guarantees, that all files were
# patch correctly. Using version.ini makes it easy to
# control this by looking into the about box
# -> shifting one file of the last section to the end
###########################################################

sub shift_file_to_end
{
    my ($patchfilelist) = @_;

    my @patchfile = ();
    my $lastfilename = "version.ini";
    my $lastfileline = "";
    my $foundfile = 0;

    # Only searching this file in the last section
    my $lastsectionname = "";

    for ( my $i = 0; $i <= $#{$patchfilelist}; $i++ )
    {
        my $line = ${$patchfilelist}[$i];
        if ( $line =~ /^\s*\[(.*?)\]\s*$/ ) { $lastsectionname = $1; }
    }

    my $record = 0;
    for ( my $i = 0; $i <= $#{$patchfilelist}; $i++ )
    {
        my $line = ${$patchfilelist}[$i];

        if ( $line =~ /^\s*\[\Q$lastsectionname\E\]\s*$/ ) { $record = 1; }

        if (( $line =~ /^\s*\"\Q$lastfilename\E\"\=/ ) && ( $record ))
        {
            $lastfileline = $line;
            $foundfile = 1;
            $record = 0;
            next;
        }

        push(@patchfile, $line);
    }

    if ( $foundfile ) { push(@patchfile, $lastfileline); }

    return  \@patchfile;
}

###########################################################
# Putting hash content into array and sorting it
###########################################################

sub sort_hash
{
    my ( $hashref ) =  @_;

    my $item = "";
    my @sortedarray = ();

    foreach $item (keys %{$hashref}) { push(@sortedarray, $item); }
    installer::sorter::sorting_array_of_strings(\@sortedarray);

    return \@sortedarray;
}

###########################################################
# Renaming Windows files in Patch and creating file
# patchfiles.txt
###########################################################

sub prepare_windows_patchfiles
{
    my ( $filesref, $languagestringref, $allvariableshashref ) = @_;

    my @patchfiles = ();
    my %patchfiledirectories = ();
    my $patchfilename = "patchlist.txt";
    my $patchfilename2 = "patchmsi.dll";

    if ( ! $allvariableshashref->{'WINDOWSPATCHLEVEL'} ) { installer::exiter::exit_program("ERROR: No Windows patch level defined in list file (WINDOWSPATCHLEVEL) !", "prepare_windows_patchfiles"); }
    my $windowspatchlevel = $installer::globals::buildid;

    # the environment variable CWS_WORK_STAMP is set only in CWS
    if ( $ENV{'CWS_WORK_STAMP'} ) { $windowspatchlevel = $ENV{'CWS_WORK_STAMP'} . $windowspatchlevel; }

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        my $filename = $onefile->{'Name'};
        if (( $filename eq $patchfilename ) || ( $filename eq $patchfilename2 )) { next; }

        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( $styles =~ /\bDONTRENAMEINPATCH\b/ ) { next; }

        # special handling for files with flag DONTSHOW. This files get the extension ".dontshow" to be filtered by dialogs.
        my $localwindowspatchlevel = $windowspatchlevel;
        if ( $styles =~ /\bDONTSHOW\b/ ) { $localwindowspatchlevel = $localwindowspatchlevel . "\.dontshow"; }

        my $olddestination = $onefile->{'destination'};
        my $newdestination = $olddestination . "." . $localwindowspatchlevel;
        my $localfilename = $olddestination;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$localfilename);  # file name part
        my $line = "\"" . $localfilename . "\"" . "=" . "\"" . "\." . $localwindowspatchlevel . "\"";
        $onefile->{'destination'} = $newdestination;

        my $newfilename = $onefile->{'Name'} . "." . $localwindowspatchlevel;
        $onefile->{'Name'} = $newfilename;

        # adding section information (section is the directory)
        my $origolddestination = $olddestination;
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$olddestination); # directory part
        if ( ! $olddestination ) { $olddestination = "_root";  }
        if ( ! exists($patchfiledirectories{$olddestination}) ) { $patchfiledirectories{$olddestination} = 1; }
        $line = $line . "\tXXXXX\t" . $olddestination . "\n";

        push(@patchfiles, $line);

        # also collecting all files from patch in @installer::globals::patchfilecollector
        my $patchfileline = $origolddestination . "\n";
        push(@installer::globals::patchfilecollector, $patchfileline);
    }

    my $winpatchdirname = "winpatch";
    my $winpatchdir = installer::systemactions::create_directories($winpatchdirname, $languagestringref);

    my $patchlistfile = installer::existence::get_specified_file_by_name($filesref, $patchfilename);

    # reorganizing the patchfile content, sorting for directory to decrease the file size
    my $sorteddirectorylist = sort_hash(\%patchfiledirectories);
    my $patchfilelist = reorg_patchfile(\@patchfiles, $sorteddirectorylist);

    # shifting version.ini to the end of the list, to guarantee, that all files are patched
    # if the correct version is shown in the about box
    $patchfilelist = shift_section_to_end($patchfilelist);
    $patchfilelist = shift_file_to_end($patchfilelist);

    # saving the file
    $patchfilename = $winpatchdir . $installer::globals::separator . $patchfilename;
    installer::files::save_file($patchfilename, $patchfilelist);

    my $infoline = "\nCreated list of patch files: $patchfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    # and assigning the new source
    $patchlistfile->{'sourcepath'} = $patchfilename;

    # and finally checking the file size
    if ( -f $patchfilename )    # test of existence
    {
        my $filesize = ( -s $patchfilename );
        $infoline = "Size of patch file list: $filesize\n\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::logger::print_message( "... size of patch list file: $filesize Byte ... \n" );
    }

}

###########################################################
# Replacing %-variables with the content
# of $allvariableshashref
###########################################################

sub replace_variables_in_string
{
    my ( $string, $variableshashref ) = @_;

    if ( $string =~ /^.*\%\w+.*$/ )
    {
        my $key;

        # we want to substitute FOO_BR before FOO to avoid floating _BR suffixes
        foreach $key (sort { length ($b) <=> length ($a) } keys %{$variableshashref})
        {
            my $value = $variableshashref->{$key};
            $key = "\%" . $key;
            $string =~ s/\Q$key\E/$value/g;
        }
    }

    return $string;
}

###########################################################
# Replacing %-variables with the content
# of $allvariableshashref
###########################################################

sub replace_dollar_variables_in_string
{
    my ( $string, $variableshashref ) = @_;

    if ( $string =~ /^.*\$\{\w+\}.*$/ )
    {
        my $key;

        foreach $key (keys %{$variableshashref})
        {
            my $value = $variableshashref->{$key};
            $key = "\$\{" . $key . "\}";
            $string =~ s/\Q$key\E/$value/g;
        }
    }

    return $string;
}

###########################################################
# The list file contains the list of packages/RPMs that
# have to be copied.
###########################################################

sub get_all_files_from_filelist
{
    my ( $listfile, $section ) = @_;

    my @allpackages = ();

    for (@{$listfile}) {
        next unless /^\s*([^#].*?)\s*$/;
        push @allpackages, $1;
    }

    return \@allpackages;
}

###########################################################
# Getting one section from a file. Section begins with
# [xyz] and ends with file end or next [abc].
###########################################################

sub get_section_from_file
{
    my ($file, $sectionname) = @_;

    my @section = ();
    my $record = 0;

    for ( my $i = 0; $i <= $#{$file}; $i++ )
    {
        my $line = ${$file}[$i];

        if (( $record ) && ( $line =~ /^\s*\[/ ))
        {
            $record = 0;
            last;
        }

        if ( $line =~ /^\s*\[\Q$sectionname\E\]\s*$/ ) { $record = 1; }

        if ( $line =~ /^\s*\[/ ) { next; } # this is a section line
        if ( $line =~ /^\s*\#/ ) { next; } # this is a comment line
        if ( $line =~ /^\s*$/ ) { next; }  # empty line
        $line =~ s/^\s*//;
        $line =~ s/\s*$//;
        if ( $record ) { push(@section, $line); }
    }

    return \@section;

}

#######################################################
# Substituting one variable in the xml file
#######################################################

sub replace_one_dollar_variable
{
    my ($file, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$file}; $i++ )
    {
        ${$file}[$i] =~ s/\$\{$searchstring\}/$variable/g;
    }
}

#######################################################
# Substituting the variables in the xml file
#######################################################

sub substitute_dollar_variables
{
    my ($file, $variableshashref) = @_;

    my $key;

    foreach $key (keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};
        replace_one_dollar_variable($file, $value, $key);
    }
}

#############################################################################
# Collecting all packages or rpms located in the installation directory
#############################################################################

sub get_all_packages_in_installdir
{
    my ($directory) = @_;

    my $infoline = "";

    my @allpackages = ();
    my $allpackages = \@allpackages;

    if ( $installer::globals::isrpmbuild )
    {
        $allpackages = installer::systemactions::find_file_with_file_extension("rpm", $directory);
    }

    if ( $installer::globals::issolarisbuild )
    {
        $allpackages = installer::systemactions::get_all_directories($directory);
    }

    return $allpackages;
}

###############################################################
# The list of exclude packages can contain the
# beginning of the package name, not the complete name.
###############################################################

sub is_matching
{
    my ($onepackage, $allexcludepackages ) = @_;

    my $matches = 0;

    for ( my $i = 0; $i <= $#{$allexcludepackages}; $i++ )
    {
        my $oneexcludepackage = ${$allexcludepackages}[$i];

        if ( $onepackage =~ /^\s*$oneexcludepackage/ )
        {
            $matches = 1;
            last;
        }
    }

    return $matches;
}

###############################################################
# Copying all Solaris packages or RPMs from installation set
###############################################################

sub copy_all_packages
{
    my ($allexcludepackages, $sourcedir, $destdir) = @_;

    my $infoline = "";

    $sourcedir =~ s/\/\s*$//;
    $destdir =~ s/\/\s*$//;

    # $allexcludepackages is a list of RPMs and packages, that shall NOT be included into jds product
    my $allpackages = get_all_packages_in_installdir($sourcedir);

    for ( my $i = 0; $i <= $#{$allpackages}; $i++ )
    {
        my $onepackage = ${$allpackages}[$i];

        my $packagename = $onepackage;

        if ( $installer::globals::issolarispkgbuild )   # on Solaris $onepackage contains the complete path
        {
            installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packagename);
        }

        if ( ! installer::existence::exists_in_array($packagename, $allexcludepackages))
        {
            if ( ! is_matching($packagename, $allexcludepackages ) )
            {

                if ( $installer::globals::isrpmbuild )
                {
                    my $sourcepackage = $sourcedir . $installer::globals::separator . $packagename;
                    my $destfile = $destdir . $installer::globals::separator . $packagename;
                    if ( ! -f $sourcepackage ) { installer::exiter::exit_program("ERROR: Could not find RPM $sourcepackage!", "copy_all_packages"); }
                    installer::systemactions::hardlink_one_file($sourcepackage, $destfile);
                }

                if ( $installer::globals::issolarispkgbuild )
                {
                    my $destinationdir = $destdir . $installer::globals::separator . $packagename;
                    if ( ! -d $onepackage ) { installer::exiter::exit_program("ERROR: Could not find Solaris package $onepackage!", "copy_all_packages"); }

                    my $systemcall = "cp -p -R $onepackage $destinationdir";
                     make_systemcall($systemcall);
                }
            }
            else
            {
                $infoline = "Excluding package (matching): $onepackage\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
        else
        {
            $infoline = "Excluding package (precise name): $onepackage\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

######################################################
# Making systemcall
######################################################

sub make_systemcall
{
    my ($systemcall) = @_;

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

###########################################################
# Copying all Solaris packages or RPMs from solver
###########################################################

sub copy_additional_packages
{
    my ($allcopypackages, $destdir, $includepatharrayref) = @_;

    my $infoline = "Copy additional packages into installation set.\n";
    push( @installer::globals::logfileinfo, $infoline);

    $destdir =~ s/\/\s*$//;

    for ( my $i = 0; $i <= $#{$allcopypackages}; $i++ )
    {
        my $onepackage = ${$allcopypackages}[$i];
        $infoline = "Copy package: $onepackage\n";
        push( @installer::globals::logfileinfo, $infoline);

        # this package must be delivered into the solver

        my $packagesourceref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$onepackage, $includepatharrayref, 0);
        if ($$packagesourceref eq "") { installer::exiter::exit_program("ERROR: Could not find jds file $onepackage!", "copy_additional_packages"); }

        if ( $onepackage =~ /\.tar\.gz\s*$/ )
        {
            my $systemcall = "cd $destdir; cat $$packagesourceref | gunzip | tar -xf -";
            make_systemcall($systemcall);
        }
        else
        {
            my $destfile = $destdir . $installer::globals::separator . $onepackage;
            installer::systemactions::copy_one_file($$packagesourceref, $destfile);
        }
    }
}

###########################################################
# Creating jds installation sets
###########################################################

sub create_jds_sets
{
    my ($installationdir, $allvariableshashref, $languagestringref, $languagesarrayref, $includepatharrayref) = @_;

    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating jds installation set ...\n" );
    installer::logger::print_message( "******************************************\n" );

    installer::logger::include_header_into_logfile("Creating jds installation sets:");

    my $firstdir = $installationdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$firstdir);

    my $lastdir = $installationdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$lastdir);

    if ( $lastdir =~ /\./ ) { $lastdir =~ s/\./_jds_inprogress\./ }
    else { $lastdir = $lastdir . "_jds_inprogress"; }

    # removing existing directory "_native_packed_inprogress" and "_native_packed_witherror" and "_native_packed"

    my $jdsdir = $firstdir . $lastdir;
    if ( -d $jdsdir ) { installer::systemactions::remove_complete_directory($jdsdir); }

    my $olddir = $jdsdir;
    $olddir =~ s/_inprogress/_witherror/;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    $olddir = $jdsdir;
    $olddir =~ s/_inprogress//;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    # creating the new directory

    installer::systemactions::create_directory($jdsdir);

    $installer::globals::saveinstalldir = $jdsdir;

    # find and read jds files list
    my $filelistname = $installer::globals::jdsexcludefilename;

    my $filelistnameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filelistname, "", 0);
    if ($$filelistnameref eq "") { installer::exiter::exit_program("ERROR: Could not find jds list file $filelistname!", "create_jds_sets"); }

    my $listfile = installer::files::read_file($$filelistnameref);

    my $infoline = "Found jds list file: $$filelistnameref\n";
    push( @installer::globals::logfileinfo, $infoline);

    # substituting the variables
    substitute_dollar_variables($listfile, $allvariableshashref);

    # determining the packages/RPMs to copy
    my $allexcludepackages = get_section_from_file($listfile, "excludefiles");
    my $allcopypackages = get_section_from_file($listfile, "copyfiles");

    # determining the source directory
    my $alldirs = installer::systemactions::get_all_directories($installationdir);
    my $sourcedir = ${$alldirs}[0]; # there is only one directory

    if ( $installer::globals::issolarisbuild ) { $sourcedir = $installer::globals::saved_packages_path; }

    # copy all packages/RPMs
    copy_all_packages($allexcludepackages, $sourcedir, $jdsdir);
    copy_additional_packages($allcopypackages, $jdsdir, $includepatharrayref);

    return $jdsdir;
}

#############################################################################
# Checking, whether this installation set contains the correct languages
#############################################################################

sub check_jds_language
{
    my ($allvariableshashref, $languagestringref) = @_;

    my $infoline = "";

    if ( ! $allvariableshashref->{'JDSLANG'} ) { installer::exiter::exit_program("ERROR: For building JDS installation sets \"JDSLANG\" must be defined!", "check_jds_language"); }
    my $languagestring = $allvariableshashref->{'JDSLANG'};

    my $sortedarray1 = installer::converter::convert_stringlist_into_array(\$languagestring, ",");

    installer::sorter::sorting_array_of_strings($sortedarray1);

    my $sortedarray2 = installer::converter::convert_stringlist_into_array($languagestringref, "_");
    installer::sorter::sorting_array_of_strings($sortedarray2);

    my $string1 = installer::converter::convert_array_to_comma_separated_string($sortedarray1);
    my $string2 = installer::converter::convert_array_to_comma_separated_string($sortedarray2);

    my $arrays_are_equal = compare_arrays($sortedarray1, $sortedarray2);

    return $arrays_are_equal;
}

###################################################################################
# Comparing two arrays. The arrays are equal, if the complete content is equal.
###################################################################################

sub compare_arrays
{
    my ($array1, $array2) = @_;

    my $arrays_are_equal = 1;

    # checking the size

    if ( ! ( $#{$array1} == $#{$array2} )) { $arrays_are_equal = 0; }   # different size

    if ( $arrays_are_equal ) # only make further investigations if size is equal
    {
        for ( my $i = 0; $i <= $#{$array1}; $i++ )
        {
            # ingnoring whitespaces at end and beginning
            ${$array1}[$i] =~ s/^\s*//;
            ${$array2}[$i] =~ s/^\s*//;
            ${$array1}[$i] =~ s/\s*$//;
            ${$array2}[$i] =~ s/\s*$//;

            if ( ! ( ${$array1}[$i] eq ${$array2}[$i] ))
            {
                $arrays_are_equal = 0;
                last;
            }
        }
    }

    return $arrays_are_equal;
}

#################################################################
# Copying the files defined as ScpActions into the
# installation set.
#################################################################

sub put_scpactions_into_installset
{
    my ($installdir) = @_;

    installer::logger::include_header_into_logfile("Start: Copying scp action files into installation set");

    for ( my $i = 0; $i <= $#installer::globals::allscpactions; $i++ )
    {
        my $onescpaction = $installer::globals::allscpactions[$i];

        my $subdir = "";
        if ( $onescpaction->{'Subdir'} ) { $subdir = $onescpaction->{'Subdir'}; }

        if ( $onescpaction->{'Name'} eq "loader.exe" ) { next; }    # do not copy this ScpAction loader

        my $destdir = $installdir;
        $destdir =~ s/\Q$installer::globals::separator\E\s*$//;
        if ( $subdir ) { $destdir = $destdir . $installer::globals::separator . $subdir; }

        my $sourcefile = $onescpaction->{'sourcepath'};
        my $destfile = $destdir . $installer::globals::separator . $onescpaction->{'DestinationName'};

        my $styles = "";
        if ( $onescpaction->{'Styles'} ) { $styles = $onescpaction->{'Styles'}; }
        if (( $styles =~ /\bFILE_CAN_MISS\b/ ) && ( $sourcefile eq "" )) { next; }

        if (( $subdir =~ /\// ) || ( $subdir =~ /\\/ ))
        {
            installer::systemactions::create_directory_structure($destdir);
        }
        else
        {
            installer::systemactions::create_directory($destdir);
        }

        installer::systemactions::copy_one_file($sourcefile, $destfile);

        if ( $onescpaction->{'UnixRights'} )
        {
            chmod oct($onescpaction->{'UnixRights'}), $destfile;
        }

    }

    installer::logger::include_header_into_logfile("End: Copying scp action files into installation set");

}

#################################################################
# Collecting scp actions for all languages
#################################################################

sub collect_scpactions
{
    my ($allscpactions) = @_;

    for ( my $i = 0; $i <= $#{$allscpactions}; $i++ )
    {
        push(@installer::globals::allscpactions, ${$allscpactions}[$i]);
    }
}

#################################################################
# Setting the platform name for download
#################################################################

sub get_platform_name
{
    my $platformname = "";

    if (( $installer::globals::islinuxintelrpmbuild ) || ( $installer::globals::islinuxinteldebbuild ))
    {
        $platformname = "LinuxIntel";
    }
    elsif (( $installer::globals::islinuxppcrpmbuild ) || ( $installer::globals::islinuxppcdebbuild ))
    {
        $platformname = "LinuxPowerPC";
    }
    elsif (( $installer::globals::islinuxx86_64rpmbuild ) || ( $installer::globals::islinuxx86_64debbuild ))
    {
        $platformname = "LinuxX86-64";
    }
    elsif ( $installer::globals::issolarissparcbuild )
    {
        $platformname = "SolarisSparc";
    }
    elsif ( $installer::globals::issolarisx86build )
    {
        $platformname = "Solarisx86";
    }
    elsif ( $installer::globals::iswindowsbuild )
    {
        $platformname = "Win32Intel";
    }
    elsif ( $installer::globals::compiler =~ /^unxmacxi/ )
    {
        $platformname = "MacOSXIntel";
    }
    elsif ( $installer::globals::compiler =~ /^unxmacxp/ )
    {
        $platformname = "MacOSXPowerPC";
    }
    else
    {
        $platformname = $installer::globals::compiler;
    }

    return $platformname;
}

###########################################################
# Adding additional variables into the variableshashref,
# that are defined in include files in the solver. The
# names of the include files are stored in
# ADD_INCLUDE_FILES (comma separated list).
###########################################################

sub add_variables_from_inc_to_hashref
{
    my ($allvariables, $includepatharrayref) = @_;

    my $infoline = "";
    my $includefilelist = "";
    if ( $allvariables->{'ADD_INCLUDE_FILES'} ) { $includefilelist = $allvariables->{'ADD_INCLUDE_FILES'}; }

    my $includefiles = installer::converter::convert_stringlist_into_array_without_newline(\$includefilelist, ",");

    for ( my $i = 0; $i <= $#{$includefiles}; $i++ )
    {
        my $includefilename = ${$includefiles}[$i];
        $includefilename =~ s/^\s*//;
        $includefilename =~ s/\s*$//;
        $includefilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$includefilename, $includepatharrayref, 1);
        if ( $$includefilenameref eq "" ) { installer::exiter::exit_program("Include file $includefilename not found!\nADD_INCLUDE_FILES = $allvariables->{'ADD_INCLUDE_FILES'}", "add_variables_from_inc_to_hashref"); }

        $infoline = "Including inc file: $$includefilenameref \n";
        push( @installer::globals::globallogfileinfo, $infoline);

        my $includefile = installer::files::read_file($$includefilenameref);

        for ( my $j = 0; $j <= $#{$includefile}; $j++ )
        {
            # Analyzing all "key=value" lines
            my $oneline = ${$includefile}[$j];

            if ( $oneline =~ /^\s*(\S+)\s*\=\s*(.*?)\s*$/ ) # no white space allowed in key
            {
                my $key = $1;
                my $value = $2;
                $allvariables->{$key} = $value;
                $infoline = "Setting of variable: $key = $value\n";
                push( @installer::globals::globallogfileinfo, $infoline);
            }
        }
    }

    # Allowing different Java versions for Windows and Unix. Instead of "JAVAVERSION"
    # the property "WINDOWSJAVAVERSION" has to be used, if it is set.

    if ( $installer::globals::iswindowsbuild )
    {
        if (( exists($allvariables->{'WINDOWSJAVAVERSION'})) && ( $allvariables->{'WINDOWSJAVAVERSION'} ne "" ))
        {
            $allvariables->{'JAVAVERSION'} = $allvariables->{'WINDOWSJAVAVERSION'};
            $infoline = "Changing value of property \"JAVAVERSION\" to $allvariables->{'JAVAVERSION'} (property \"WINDOWSJAVAVERSION\").\n";
            push( @installer::globals::globallogfileinfo, $infoline);
        }
    }
}

##############################################
# Collecting all files from include pathes
##############################################

sub collect_all_files_from_includepathes
{
    my ($patharrayref) = @_;

    installer::logger::globallog("Reading all directories: Start");
    installer::logger::print_message( "... reading include pathes ...\n" );
    # empty the global

    @installer::globals::allincludepathes =();
    my $infoline;

    for ( my $i = 0; $i <= $#{$patharrayref}; $i++ )
    {
        $includepath = ${$patharrayref}[$i];
        installer::remover::remove_leading_and_ending_whitespaces(\$includepath);

        if ( ! -d $includepath )
        {
            $infoline = "$includepath does not exist. (Can be removed from include path list?)\n";
            push( @installer::globals::globallogfileinfo, $infoline);
            next;
        }

        my @sourcefiles = ();
        my $pathstring = "";
        installer::systemactions::read_full_directory($includepath, $pathstring, \@sourcefiles);

        if ( ! ( $#sourcefiles > -1 ))
        {
            $infoline = "$includepath is empty. (Can be removed from include path list?)\n";
            push( @installer::globals::globallogfileinfo, $infoline);
        }
        else
        {
            my $number = $#sourcefiles + 1;
            $infoline = "Directory $includepath contains $number files (including subdirs)\n";
            push( @installer::globals::globallogfileinfo, $infoline);

            my %allfileshash = ();
            $allfileshash{'includepath'} = $includepath;

            for ( my $j = 0; $j <= $#sourcefiles; $j++ )
            {
                $allfileshash{$sourcefiles[$j]} = 1;
            }

            push(@installer::globals::allincludepathes, \%allfileshash);
        }
    }

    $installer::globals::include_pathes_read = 1;

    installer::logger::globallog("Reading all directories: End");
    push( @installer::globals::globallogfileinfo, "\n");
}

##############################################
# Searching for a file with the gid
##############################################

sub find_file_by_id
{
    my ( $filesref, $gid ) = @_;

    my $foundfile = 0;
    my $onefile;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        $onefile = ${$filesref}[$i];
        my $filegid = $onefile->{'gid'};

        if ( $filegid eq $gid )
        {
            $foundfile = 1;
            last;
        }
    }

    if (! $foundfile ) { $onefile  = ""; }

    return $onefile;
}

##############################################
# Searching for an item with the gid
##############################################

sub find_item_by_gid
{
    my ( $itemsref, $gid ) = @_;

    my $founditem = 0;
    my $oneitem = "";

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $localitem = ${$itemsref}[$i];
        my $itemgid = $localitem->{'gid'};

        if ( $itemgid eq $gid )
        {
            $oneitem = $localitem;
            $founditem = 1;
            last;
        }
    }

    return $oneitem;
}

#########################################################
# Calling sum
#########################################################

sub call_sum
{
    my ($filename) = @_;

    $sumfile = "/usr/bin/sum";

    if ( ! -f $sumfile ) { installer::exiter::exit_program("ERROR: No file /usr/bin/sum", "call_sum"); }

    my $systemcall = "$sumfile $filename |";

    my $sumoutput = "";

    open (SUM, "$systemcall");
    $sumoutput = <SUM>;
    close (SUM);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $sumoutput;
}

#########################################################
# Calling wc
# wc -c pkginfo | cut -f6 -d' '
#########################################################

sub call_wc
{
    my ($filename) = @_;

    $wcfile = "/usr/bin/wc";

    if ( ! -f $wcfile ) { installer::exiter::exit_program("ERROR: No file /usr/bin/wc", "call_wc"); }

    my $systemcall = "$wcfile -c $filename |";

    my $wcoutput = "";

    open (WC, "$systemcall");
    $wcoutput = <WC>;
    close (WC);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $wcoutput;
}

##############################################
# Setting architecture ARCH=i86pc
# instead of ARCH=i386.
##############################################

sub set_old_architecture_string
{
    my ($pkginfofile) = @_;

    for ( my $i = 0; $i <= $#{$pkginfofile}; $i++ )
    {
        if ( ${$pkginfofile}[$i] =~ /^\s*ARCH=i386\s*$/ )
        {
            ${$pkginfofile}[$i] =~ s/i386/i86pc/;
            last;
        }
    }
}

##############################################
# For the new copied package, it is necessary
# that a value for the key SUNW_REQUIRES
# is set. Otherwise this copied package
# with ARCH=i86pc would be useless.
##############################################

sub check_requires_setting
{
    my ($pkginfofile) = @_;

    my $found = 0;
    my $patchid = "";

    for ( my $i = 0; $i <= $#{$pkginfofile}; $i++ )
    {
        if ( ${$pkginfofile}[$i] =~ /^\s*SUNW_REQUIRES=(\S*?)\s*$/ )
        {
            $patchid = $1;
            $found = 1;
            last;
        }
    }

    if (( ! $found ) || ( $patchid eq "" )) { installer::exiter::exit_program("ERROR: No patch id defined for SUNW_REQUIRES in patch pkginfo file!", "check_requires_setting"); }
}

##############################################
# Setting checksum and wordcount for changed
# pkginfo file into pkgmap.
##############################################

sub set_pkginfo_line
{
    my ($pkgmapfile, $pkginfofilename) = @_;

    # 1 i pkginfo 442 34577 1166716297
    # ->
    # 1 i pkginfo 443 34737 1166716297
    #
    # wc -c pkginfo | cut -f6 -d' '  -> 442  (variable)
    # sum pkginfo | cut -f1 -d' '  -> 34577  (variable)
    # grep 'pkginfo' pkgmap | cut -f6 -d' '  -> 1166716297  (fix)

    my $checksum = call_sum($pkginfofilename);
    if ( $checksum =~ /^\s*(\d+)\s+.*$/ ) { $checksum = $1; }

    my $wordcount = call_wc($pkginfofilename);
    if ( $wordcount =~ /^\s*(\d+)\s+.*$/ ) { $wordcount = $1; }

    for ( my $i = 0; $i <= $#{$pkgmapfile}; $i++ )
    {
        if ( ${$pkgmapfile}[$i] =~ /(^.*\bpkginfo\b\s+)(\d+)(\s+)(\d+)(\s+)(\d+)(\s*$)/ )
        {
            my $newline = $1 . $wordcount . $3 . $checksum . $5 . $6 . $7;
            ${$pkgmapfile}[$i] = $newline;
            last;
        }
    }
}

##############################################
# Setting time stamp of copied files to avoid
# errors from pkgchk.
##############################################

sub set_time_stamp
{
    my ($olddir, $newdir, $copyfiles) = @_;

    for ( my $i = 0; $i <= $#{$copyfiles}; $i++ )
    {
        my $sourcefile = $olddir . $installer::globals::separator . ${$copyfiles}[$i];
        my $destfile = $newdir . $installer::globals::separator . ${$copyfiles}[$i];

        my $systemcall = "touch -r $sourcefile $destfile";

        my $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: \"$systemcall\" failed!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: \"$systemcall\" !\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

############################################################
# Generating pathes for cygwin (first version)
# This function has problems with cygwin, if $tmpfilename
# contains many thousand files (OpenOffice SDK).
############################################################

sub generate_cygwin_pathes_old
{
    my ($filesref) = @_;

    my ($tmpfilehandle, $tmpfilename) = tmpnam();
    open SOURCEPATHLIST, ">$tmpfilename" or die "oops...\n";
    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        print SOURCEPATHLIST "${$filesref}[$i]->{'sourcepath'}\n";
    }
    close SOURCEPATHLIST;
    my @cyg_sourcepathlist = qx{cygpath -w -f "$tmpfilename"};
    chomp @cyg_sourcepathlist;
    unlink "$tmpfilename" or die "oops\n";
    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        ${$filesref}[$i]->{'cyg_sourcepath'} = $cyg_sourcepathlist[$i];
    }

}

#################################################
# Generating pathes for cygwin (second version)
# This function generates smaller files for
#################################################

sub generate_cygwin_pathes
{
    my ($filesref) = @_;

    installer::logger::include_timestamp_into_logfile("Starting generating cygwin pathes");

    my $infoline = "Generating cygwin pathes (generate_cygwin_pathes)\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $max = 5000;  # number of pathes in one file

    my @pathcollector = ();
    my $startnumber = 0;
    my $counter = 0;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $line = ${$filesref}[$i]->{'sourcepath'} . "\n";
        push(@pathcollector, $line);
        $counter++;

        if (( $i == $#{$filesref} ) || ((( $counter % $max ) == 0 ) && ( $i > 0 )))
        {
            my $tmpfilename = "cygwinhelper_" . $i . ".txt";
            my $temppath = $installer::globals::temppath;
            $temppath =~ s/\Q$installer::globals::separator\E\s*$//;
            $tmpfilename = $temppath . $installer::globals::separator . $tmpfilename;
            $infoline = "Creating temporary file for cygwin conversion: $tmpfilename (contains $counter pathes)\n";
            push( @installer::globals::logfileinfo, $infoline);
            if ( -f $tmpfilename ) { unlink $tmpfilename; }

            installer::files::save_file($tmpfilename, \@pathcollector);

            my $success = 0;
            my @cyg_sourcepathlist = qx{cygpath -w -f "$tmpfilename"};
            chomp @cyg_sourcepathlist;

            # Validating the array, it has to contain the correct number of values
            my $new_pathes = $#cyg_sourcepathlist + 1;
            if ( $new_pathes == $counter ) { $success = 1; }

            if ($success)
            {
                $infoline = "Success: Successfully converted to cygwin pathes!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
            else
            {
                $infoline = "ERROR: Failed to convert to cygwin pathes!\n";
                push( @installer::globals::logfileinfo, $infoline);
                installer::exiter::exit_program("ERROR: Failed to convert to cygwin pathes!", "generate_cygwin_pathes");
            }

            for ( my $j = 0; $j <= $#cyg_sourcepathlist; $j++ )
            {
                my $number = $startnumber + $j;
                ${$filesref}[$number]->{'cyg_sourcepath'} = $cyg_sourcepathlist[$j];
            }

            if ( -f $tmpfilename ) { unlink $tmpfilename; }

            @pathcollector = ();
            $startnumber = $startnumber + $max;
            $counter = 0;
        }
    }

    # Checking existence fo cyg_sourcepath for every file
    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        if (( ! exists(${$filesref}[$i]->{'cyg_sourcepath'}) ) || ( ${$filesref}[$i]->{'cyg_sourcepath'} eq "" ))
        {
            $infoline = "ERROR: No cygwin sourcepath defined for file ${$filesref}[$i]->{'sourcepath'}\n";
            push( @installer::globals::logfileinfo, $infoline);
            installer::exiter::exit_program("ERROR: No cygwin sourcepath defined for file ${$filesref}[$i]->{'sourcepath'}!", "generate_cygwin_pathes");
        }
    }

    installer::logger::include_timestamp_into_logfile("Ending generating cygwin pathes");
}

##############################################
# Include only files from install directory
# in pkgmap file.
##############################################

sub filter_pkgmapfile
{
    my ($pkgmapfile) = @_;

    my @pkgmap = ();

    my $line = ": 1 10\n";
    push(@pkgmap, $line);

    for ( my $i = 0; $i <= $#{$pkgmapfile}; $i++ )
    {
        $line = ${$pkgmapfile}[$i];
        if ( $line =~ /^\s*1\si\s/ ) { push(@pkgmap, $line); }
    }

    return \@pkgmap;
}

##############################################
# Creating double packages for Solaris x86.
# One package with ARCH=i386 and one with
# ARCH=i86pc.
##############################################

sub fix_solaris_x86_patch
{
    my ($packagename, $subdir) = @_;

    # changing into directory of packages, important for soft linking
    my $startdir = cwd();
    chdir($subdir);

    # $packagename is: "SUNWstaroffice-core01"
    # Current working directory is: "<path>/install/en-US_inprogress"

    # create new folder in "packages": $packagename . ".i"
    my $newpackagename = $packagename . "\.i";
    my $newdir = $newpackagename;
    installer::systemactions::create_directory($newdir);

    # collecting all directories in the package
    my $olddir = $packagename;
    my $allsubdirs = installer::systemactions::get_all_directories_without_path($olddir);

    # link all directories from $packagename to $packagename . ".i"
    for ( my $i = 0; $i <= $#{$allsubdirs}; $i++ )
    {
        my $sourcedir = $olddir . $installer::globals::separator . ${$allsubdirs}[$i];
        my $destdir = $newdir . $installer::globals::separator . ${$allsubdirs}[$i];
        my $directory_depth = 2; # important for soft links, two directories already exist
        installer::systemactions::softlink_complete_directory($sourcedir, $destdir, $directory_depth);
    }

    # copy "pkginfo" and "pkgmap" from $packagename to $packagename . ".i"
    my @allcopyfiles = ("pkginfo", "pkgmap");
    for ( my $i = 0; $i <= $#allcopyfiles; $i++ )
    {
        my $sourcefile = $olddir . $installer::globals::separator . $allcopyfiles[$i];
        my $destfile = $newdir . $installer::globals::separator . $allcopyfiles[$i];
        installer::systemactions::copy_one_file($sourcefile, $destfile);
    }

    # change in pkginfo in $packagename . ".i" the value for ARCH from i386 to i86pc
    my $pkginfofilename = "pkginfo";
    $pkginfofilename = $newdir . $installer::globals::separator . $pkginfofilename;

    my $pkginfofile = installer::files::read_file($pkginfofilename);
    set_old_architecture_string($pkginfofile);
    installer::files::save_file($pkginfofilename, $pkginfofile);

    # adapt the values in pkgmap for pkginfo file, because this file was edited
    my $pkgmapfilename = "pkgmap";
    $pkgmapfilename = $newdir . $installer::globals::separator . $pkgmapfilename;

    my $pkgmapfile = installer::files::read_file($pkgmapfilename);
    set_pkginfo_line($pkgmapfile, $pkginfofilename);
    installer::files::save_file($pkgmapfilename, $pkgmapfile);

    # changing back to startdir
    chdir($startdir);
}

###################################################
# Creating double core01 package for Solaris x86.
# One package with ARCH=i386 and one with
# ARCH=i86pc. This is necessary, to inform the
# user about the missing "small patch", if
# packages with ARCH=i86pc are installed.
###################################################

sub fix2_solaris_x86_patch
{
    my ($packagename, $subdir) = @_;

    if ( $packagename =~ /-core01\s*$/ )    # only this one package needs to be duplicated
    {
        my $startdir = cwd();
        chdir($subdir);

        # $packagename is: "SUNWstaroffice-core01"
        # Current working directory is: "<path>/install/en-US_inprogress"

        # create new package in "packages": $packagename . ".i"
        my $olddir = $packagename;
        my $newpackagename = $packagename . "\.i";
        my $newdir = $newpackagename;

        installer::systemactions::create_directory($newdir);

        my $oldinstalldir = $olddir . $installer::globals::separator . "install";
        my $newinstalldir = $newdir . $installer::globals::separator . "install";

        installer::systemactions::copy_complete_directory($oldinstalldir, $newinstalldir);

        # setting time stamp of all copied files to avoid errors from pkgchk
        my $allinstallfiles = installer::systemactions::get_all_files_from_one_directory_without_path($newinstalldir);
        set_time_stamp($oldinstalldir, $newinstalldir, $allinstallfiles);

        # copy "pkginfo" and "pkgmap" from $packagename to $packagename . ".i"
        my @allcopyfiles = ("pkginfo", "pkgmap");
        for ( my $i = 0; $i <= $#allcopyfiles; $i++ )
        {
            my $sourcefile = $olddir . $installer::globals::separator . $allcopyfiles[$i];
            my $destfile = $newdir . $installer::globals::separator . $allcopyfiles[$i];
            installer::systemactions::copy_one_file($sourcefile, $destfile);
        }

        # change in pkginfo in $packagename . ".i" the value for ARCH from i386 to i86pc
        my $pkginfofilename = "pkginfo";
        $pkginfofilename = $newdir . $installer::globals::separator . $pkginfofilename;

        my $pkginfofile = installer::files::read_file($pkginfofilename);
        set_old_architecture_string($pkginfofile);
        check_requires_setting($pkginfofile);
        installer::files::save_file($pkginfofilename, $pkginfofile);

        # adapt the values in pkgmap for pkginfo file, because this file was edited
        my $pkgmapfilename = "pkgmap";
        $pkgmapfilename = $newdir . $installer::globals::separator . $pkgmapfilename;

        my $pkgmapfile = installer::files::read_file($pkgmapfilename);
        set_pkginfo_line($pkgmapfile, $pkginfofilename);
        $pkgmapfile = filter_pkgmapfile($pkgmapfile);
        installer::files::save_file($pkgmapfilename, $pkgmapfile);

        # setting time stamp of all copied files to avoid errors from pkgchk
        set_time_stamp($olddir, $newdir, \@allcopyfiles);

        # changing back to startdir
        chdir($startdir);
    }
}

################################################
# Files with flag HIDDEN get a dot at the
# beginning of the file name. This cannot be
# defined in scp2 project, because tooling
# cannot handle files with beginning dot
# correctly.
################################################

sub resolving_hidden_flag
{
    my ($filesarrayref, $variableshashref, $item, $languagestringref) = @_;

    my $diritem = lc($item);
    my $infoline = "";

    my $hiddendirbase = installer::systemactions::create_directories("hidden_$diritem", $languagestringref);

    installer::logger::include_header_into_logfile("$item with flag HIDDEN:");

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bHIDDEN\b/ )
        {
            # Language specific subdirectory

            my $onelanguage = $onefile->{'specificlanguage'};

            if ($onelanguage eq "")
            {
                $onelanguage = "00";    # files without language into directory "00"
            }

            my $hiddendir = $hiddendirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator;
            installer::systemactions::create_directory($hiddendir); # creating language specific directories

            # copy files and edit them with the variables defined in the zip.lst

            my $onefilename = $onefile->{'Name'};
            my $newfilename = "\." . $onefilename;
            my $sourcefile = $onefile->{'sourcepath'};
            my $destfile = $hiddendir . $newfilename;

            my $copysuccess = installer::systemactions::copy_one_file($sourcefile, $destfile);

            if ( $copysuccess )
            {
                $onefile->{'sourcepath'} = $destfile;
                $destination = $onefile->{'destination'};
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
                if ( $destination eq "" ) { $onefile->{'destination'} = $newfilename; }
                else { $onefile->{'destination'} = $destination . $installer::globals::separator . $newfilename; }

                $infoline = "Success: Using file with flag HIDDEN from \"$onefile->{'sourcepath'}\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
            else
            {
                $infoline = "Error: Failed to copy HIDDEN file from \"$sourcefile\" to \"$destfile\"!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

################################################
# Controlling that all keys in hash A are
# also key in hash B.
################################################

sub key_in_a_is_also_key_in_b
{
    my ( $hashref_a, $hashref_b) = @_;

    my $returnvalue = 1;

    my $key;
    foreach $key ( keys %{$hashref_a} )
    {
        if ( ! exists($hashref_b->{$key}) )
        {
            print "*****\n";
            foreach $keyb ( keys %{$hashref_b} ) { print "$keyb : $hashref_b->{$keyb}\n"; }
            print "*****\n";
            $returnvalue = 0;
        }
    }

    return $returnvalue;
}

######################################################
# Getting the first entry from a list of languages
######################################################

sub get_first_from_list
{
    my ( $list ) = @_;

    my $first = $list;

    if ( $list =~ /^\s*(.+?),(.+)\s*$/) # "?" for minimal matching
    {
        $first = $1;
    }

    return $first;
}

################################################
# Setting all spellchecker languages
################################################

sub set_spellcheckerlanguages
{
    my ( $productlanguagesarrayref, $allvariables ) = @_;

    my %productlanguages = ();
    for ( my $i = 0; $i <= $#{$productlanguagesarrayref}; $i++ ) { $productlanguages{${$productlanguagesarrayref}[$i]} = 1;  }

    my $spellcheckfilename = $allvariables->{'SPELLCHECKERFILE'};

    my $spellcheckfileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$spellcheckfilename, "", 1);

    if ($$spellcheckfileref eq "") { installer::exiter::exit_program("ERROR: Could not find $spellcheckfilename!", "set_spellcheckerlanguages"); }

    my $infoline = "Using spellchecker file: $$spellcheckfileref \n";
    push( @installer::globals::globallogfileinfo, $infoline);

    my $spellcheckfile = installer::files::read_file($$spellcheckfileref);
    my %spellcheckhash = ();

    for ( my $j = 0; $j <= $#{$spellcheckfile}; $j++ )
    {
        # Analyzing all "key=value" lines
        my $oneline = ${$spellcheckfile}[$j];

        if ( $oneline =~ /^\s*(\S+)\s*\=\s*\"(.*?)\"\s*$/ ) # no white space allowed in key
        {
            my $onelang = $1;
            my $languagelist = $2;

            # Special handling for language packs. Only include the first language of the language list.
            # If no spellchecker shall be included, the keyword "EMPTY" can be used.

            if ( $installer::globals::languagepack )
            {
                my $first = get_first_from_list($languagelist);

                if ( $first eq "EMPTY" )     # no spellchecker into language pack
                {
                    $languagelist = "";
                }
                else
                {
                    $languagelist = $first;
                }
            }
            else  # no language pack, so EMPTY is not required
            {
                $languagelist =~ s/^\s*EMPTY\s*,//; # removing the entry EMPTY
            }

            $spellcheckhash{$onelang} = $languagelist;
        }
    }

    # Collecting all required languages in %installer::globals::spellcheckerlanguagehash

    foreach my $lang (keys %productlanguages)
    {
        my $languagelist = "";
        if ( exists($spellcheckhash{$lang}) ) { $languagelist = $spellcheckhash{$lang}; }
        else { $languagelist = $spellcheckhash{'en-US'}; }  # defaulting to English

        my $langlisthash = installer::converter::convert_stringlist_into_hash(\$languagelist, ",");
        foreach my $onelang ( keys %{$langlisthash} ) { $installer::globals::spellcheckerlanguagehash{$onelang} = 1; }
    }

    $installer::globals::analyze_spellcheckerlanguage = 1;

    # Logging

    my $langstring = "";
    foreach my $lang (sort keys %installer::globals::spellcheckerlanguagehash) { $langstring = $langstring . "," . $lang }
    $langstring =~ s/^\s*,//;

    $infoline = "Collected spellchecker languages for spellchecker: $langstring \n";
    push( @installer::globals::globallogfileinfo, $infoline);
}

################################################
# Including a license text into setup script
################################################

sub put_license_into_setup
{
    my ($installdir, $includepatharrayref) = @_;

    # find and read english license file
    my $licenselanguage = "en-US";                  # always english !
    # my $licensefilename = "LICENSE"; # _" . $licenselanguage;
    my $licensefilename = "license"; # . $licenselanguage . ".txt";
    my $licenseincludepatharrayref = get_language_specific_include_pathes($includepatharrayref, $licenselanguage);

    my $licenseref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, $licenseincludepatharrayref, 0);
    if ($$licenseref eq "") { installer::exiter::exit_program("ERROR: Could not find License file $licensefilename!", "put_license_into_setup"); }
    my $licensefile = installer::files::read_file($$licenseref);

    # Read setup
    my $setupfilename = $installdir . $installer::globals::separator . "setup";
    my $setupfile = installer::files::read_file($setupfilename);

    # Replacement
    my $infoline = "Adding licensefile into setup script\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $includestring = "";
    for ( my $i = 0; $i <= $#{$licensefile}; $i++ ) { $includestring = $includestring . ${$licensefile}[$i]; }
    for ( my $i = 0; $i <= $#{$setupfile}; $i++ ) { ${$setupfile}[$i] =~ s/LICENSEFILEPLACEHOLDER/$includestring/; }

    # Write setup
    installer::files::save_file($setupfilename, $setupfile);
}

################################################
# Setting global path to getuid.so library
################################################

sub set_getuid_path
{
    my ($includepatharrayref) = @_;

    my $getuidlibraryname = "getuid.so";
    my $getuidlibraryref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$getuidlibraryname, $includepatharrayref, 0);
    if ($$getuidlibraryref eq "") { installer::exiter::exit_program("ERROR: Could not find $getuidlibraryname!", "set_getuid_path"); }

    $installer::globals::getuidpath = $$getuidlibraryref;
    $installer::globals::getuidpathset = 1;
}

#########################################################
# Create a tar file from the binary package
#########################################################

sub tar_package
{
    my ( $installdir, $packagename, $tarfilename, $getuidlibrary) = @_;

    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    my $systemcall = "cd $installdir; $ldpreloadstring tar -cf - $packagename > $tarfilename";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    chmod 0775, $tarfilename;

    my $fulltarfile = $installdir . $installer::globals::separator . $tarfilename;
    my $filesize = ( -s $fulltarfile );

    return $filesize;
}

#########################################################
# Create a tar file from the binary package
#########################################################

sub untar_package
{
    my ( $installdir, $tarfilename, $getuidlibrary) = @_;

    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    my $systemcall = "cd $installdir; $ldpreloadstring tar -xf $tarfilename";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    chmod 0775, $tarfilename;
}

#########################################################
# Shuffle an array (Fisher Yates shuffle)
#########################################################

sub shuffle_array
{
    my ( $arrayref ) = @_;

    my $i = @$arrayref;
    while (--$i)
    {
        my $j = int rand ($i+1);
        @$arrayref[$i,$j] = @$arrayref[$j,$i];
    }
}

################################################
# Defining the English license text to add
# it into Solaris packages.
################################################

sub set_english_license
{
    my $additional_license_name = $installer::globals::englishsolarislicensename;   # always the English file
    my $licensefileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$additional_license_name, "" , 0);
    if ( $$licensefileref eq "" ) { installer::exiter::exit_program("ERROR: Could not find license file $additional_license_name!", "set_english_license"); }
    $installer::globals::englishlicenseset = 1;
    $installer::globals::englishlicense = installer::files::read_file($$licensefileref);
    installer::scpzipfiles::replace_all_ziplistvariables_in_file($installer::globals::englishlicense, $variableshashref);
}

##############################################
# Setting time stamp of copied files to avoid
# errors from pkgchk.
##############################################

sub set_time_stamp_for_file
{
    my ($sourcefile, $destfile) = @_;

    my $systemcall = "touch -r $sourcefile $destfile";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: \"$systemcall\" failed!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: \"$systemcall\" !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

##############################################
# Setting checksum and wordcount for changed
# pkginfo file into pkgmap.
##############################################

sub change_onefile_in_pkgmap
{
    my ($pkgmapfile, $fullfilename, $shortfilename) = @_;

    # 1 i pkginfo 442 34577 1166716297
    # ->
    # 1 i pkginfo 443 34737 1166716297
    #
    # wc -c pkginfo | cut -f6 -d' '  -> 442  (variable)
    # sum pkginfo | cut -f1 -d' '  -> 34577  (variable)
    # grep 'pkginfo' pkgmap | cut -f6 -d' '  -> 1166716297  (fix)

    my $checksum = call_sum($fullfilename);
    if ( $checksum =~ /^\s*(\d+)\s+.*$/ ) { $checksum = $1; }

    my $wordcount = call_wc($fullfilename);
    if ( $wordcount =~ /^\s*(\d+)\s+.*$/ ) { $wordcount = $1; }

    for ( my $i = 0; $i <= $#{$pkgmapfile}; $i++ )
    {
        if ( ${$pkgmapfile}[$i] =~ /(^.*\b\Q$shortfilename\E\b\s+)(\d+)(\s+)(\d+)(\s+)(\d+)(\s*$)/ )
        {
            my $newline = $1 . $wordcount . $3 . $checksum . $5 . $6 . $7;
            ${$pkgmapfile}[$i] = $newline;
            last;
        }
    }
}

################################################
# Adding the content of the English license
# file into the system integration packages.
################################################

sub add_license_into_systemintegrationpackages
{
    my ($destdir, $packages) = @_;

    for ( my $i = 0; $i <= $#{$packages}; $i++ )
    {
        my $copyrightfilename = ${$packages}[$i] . $installer::globals::separator . "install" . $installer::globals::separator . "copyright";
        if ( ! -f $copyrightfilename ) { installer::exiter::exit_program("ERROR: Could not find license file in system integration package: $copyrightfilename!", "add_license_into_systemintegrationpackages"); }
        my $copyrightfile = installer::files::read_file($copyrightfilename);

        # Saving time stamp of old copyrightfile
        my $savcopyrightfilename = $copyrightfilename . ".sav";
        installer::systemactions::copy_one_file($copyrightfilename, $savcopyrightfilename);
        set_time_stamp_for_file($copyrightfilename, $savcopyrightfilename); # now $savcopyrightfile has the time stamp of $copyrightfile

        # Adding license content to copyright file
        push(@{$copyrightfile}, "\n");
        for ( my $i = 0; $i <= $#{$installer::globals::englishlicense}; $i++ ) { push(@{$copyrightfile}, ${$installer::globals::englishlicense}[$i]); }
        installer::files::save_file($copyrightfilename, $copyrightfile);

        # Setting the old time stamp saved with $savcopyrightfilename
        set_time_stamp_for_file($savcopyrightfilename, $copyrightfilename); # now $copyrightfile has the time stamp of $savcopyrightfile
        unlink($savcopyrightfilename);

        # Changing content of copyright file in pkgmap
        my $pkgmapfilename = ${$packages}[$i] . $installer::globals::separator . "pkgmap";
        if ( ! -f $pkgmapfilename ) { installer::exiter::exit_program("ERROR: Could not find pkgmap in system integration package: $pkgmapfilename!", "add_license_into_systemintegrationpackages"); }
        my $pkgmap = installer::files::read_file($pkgmapfilename);
        change_onefile_in_pkgmap($pkgmap, $copyrightfilename, "copyright");
        installer::files::save_file($pkgmapfilename, $pkgmap);
    }
}

#########################################################
# Collecting all pkgmap files from an installation set
#########################################################

sub collectpackagemaps
{
    my ( $installdir, $languagestringref, $allvariables ) = @_;

    installer::logger::include_header_into_logfile("Collecing all packagemaps (pkgmap):");

    my $pkgmapdir = installer::systemactions::create_directories("pkgmap", $languagestringref);
    my $subdirname = $allvariables->{'UNIXPRODUCTNAME'} . "_pkgmaps";
    my $pkgmapsubdir = $pkgmapdir . $installer::globals::separator . $subdirname;
    if ( -d $pkgmapsubdir ) { installer::systemactions::remove_complete_directory($pkgmapsubdir); }
    if ( ! -d $pkgmapsubdir ) { installer::systemactions::create_directory($pkgmapsubdir); }

    $installdir =~ s/\/\s*$//;
    # Collecting all packages in $installdir and its sub package ("packages")
    my $searchdir = $installdir . $installer::globals::separator . $installer::globals::epmoutpath;

    my $allpackages = installer::systemactions::get_all_directories_without_path($searchdir);

    for ( my $i = 0; $i <= $#{$allpackages}; $i++ )
    {
        my $pkgmapfile = $searchdir . $installer::globals::separator . ${$allpackages}[$i] . $installer::globals::separator . "pkgmap";
        my $destfilename = $pkgmapsubdir . $installer::globals::separator . ${$allpackages}[$i] . "_pkgmap";
        installer::systemactions::copy_one_file($pkgmapfile, $destfilename);
    }

    # Create a tar gz file with all package maps
    my $tarfilename = $subdirname . ".tar";
    my $targzname = $tarfilename . ".gz";
    $systemcall = "cd $pkgmapdir; tar -cf - $subdirname | gzip > $targzname";
    make_systemcall($systemcall);
    installer::systemactions::remove_complete_directory($pkgmapsubdir, 1);
}

1;
