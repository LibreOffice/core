#*************************************************************************
#
#   $RCSfile: worker.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:18:03 $
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

use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
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
            $infoline = "Error: Could not execute \"$systemcall\"!\n";
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
        $infoline = "Error: Could not create link \"$localshipinstalldir\"!\n";
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

1;
