#*************************************************************************
#
#   $RCSfile: worker.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2004-08-12 08:30:00 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

package installer::worker;

use installer::control;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::mail;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::systemactions;

#####################################################################
# Unpacking all files ending with tar.gz in a specified directory
#####################################################################

sub unpack_all_targzfiles_in_directory
{
    my ( $directory ) = @_;

    installer::logger::include_header_into_logfile("Unpacking tar.gz files:");

    print "... unpacking tar.gz files ... \n";

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
    print "... copy installation set from " . $destdir . " to " . $localshipinstalldir . "\n";
    installer::systemactions::copy_complete_directory($destdir, $localshipinstalldir);

    if (( ! $installer::globals::iswindowsbuild ) && ( ! $installer::globals::javafilespath eq "" ))
    {
        # Setting Unix rights for Java starter ("setup")
        my $localcall = "chmod 775 $localshipinstalldir/setup \>\/dev\/null 2\>\&1";
        system($localcall);
    }

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
    print $infoline;
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
    print "... linking installation set from " . $destdir . " to " . $localshipinstalldir . "\n";

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

#   # very slow on Windows
#   for ( my $i = 0; $i <= $#{$filesref}; $i++ )
#   {
#       my $onefile = ${$filesref}[$i];
#       my $systemcall = "$$checksumfileref $onefile->{'sourcepath'} |";
#       open (CHECK, "$systemcall");
#       my $localchecksum = <CHECK>;
#       close (CHECK);
#       push(@checksum, $localchecksum);
#   }

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

###############################################################
# Removing all directories of a special language
# in the directory $basedir
###############################################################

sub remove_old_installation_sets
{
    my ($basedir) = @_;

    print "... removing old installation directories ...\n";

    my $removedir = $basedir;

    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    # looking for non successful old installation sets

    $removedir = $basedir . "_witherror";
    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    $removedir = $basedir . "_inprogress";
    if ( -d $removedir ) { installer::systemactions::remove_complete_directory($removedir, 1); }

    # finally the $basedir can be created empty

    installer::systemactions::create_directory($basedir);
}

###############################################################
# Removing all non successful installation sets on ship
###############################################################

sub remove_old_ship_installation_sets
{
    my ($fulldir, $counter) = @_;

    print "... removing old installation directories ...\n";

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

    if ( $installer::globals::updatepack )
    {
        $installdir = $shipinstalldir;
        installer::systemactions::create_directory_structure($installdir);
        $$current_install_number_ref = installer::systemactions::determine_maximum_number($installdir, $languagestringref);
        $installdir = installer::systemactions::rename_string_in_directory($installdir, "number", $$current_install_number_ref);
        remove_old_ship_installation_sets($installdir);
    }
    else
    {
        $installdir = installer::systemactions::create_directories("install", $languagestringref);
        print "... creating installation set in $installdir ...\n";
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

    print "... checking log file " . $loggingdir . $installer::globals::logfilename . "\n";

    my $contains_error = installer::control::check_logfile(\@installer::globals::logfileinfo);

    # Dependent from the success, the installation directory can be renamed and mails can be send.

    if ( $contains_error )
    {
        my $errordir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "_witherror");
        if ( $installer::globals::updatepack ) { installer::mail::send_fail_mail($allsettingsarrayref, $languagestringref, $errordir); }
    }
    else
    {
        my $destdir = "";

        if ( $installer::globals::updatepack )
        {
            $destdir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "_packed");
            installer::mail::send_success_mail($allsettingsarrayref, $languagestringref, $destdir);
        }
        else
        {
            $destdir = installer::systemactions::rename_string_in_directory($installdir, "_inprogress", "");
        }
    }

    # Saving the logfile in the log file directory and additionally in a log directory in the install directory

    my $numberedlogfilename = $installer::globals::logfilename;
    if ( $installer::globals::updatepack ) { $numberedlogfilename =~ s /log_/log_$current_install_number\_/; }
    print "... creating log file $numberedlogfilename \n";
    installer::files::save_file($loggingdir . $numberedlogfilename, \@installer::globals::logfileinfo);
    installer::files::save_file($installlogdir . $installer::globals::separator . $numberedlogfilename, \@installer::globals::logfileinfo);

    # Saving the checksumfile in a checksum directory in the install directory
    # installer::worker::save_checksum_file($current_install_number, $installchecksumdir, $checksumfile);
}

###############################################################
# Removing all directories that are saved in the
# global directory @installer::globals::removedirs
###############################################################

sub clean_output_tree
{
    print "... cleaning the output tree ...\n";

    for ( my $i = 0; $i <= $#installer::globals::removedirs; $i++ )
    {
        if ( -d $installer::globals::removedirs[$i] )
        {
            print "... removing directory $installer::globals::removedirs[$i] ...\n";
            installer::systemactions::remove_complete_directory($installer::globals::removedirs[$i], 1);
        }
    }
}

1;
