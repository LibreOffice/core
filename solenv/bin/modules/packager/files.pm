#*************************************************************************
#
#   $RCSfile: files.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2005-09-05 16:23:24 $
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


package packager::files;

use packager::exiter;

############################################
# File Operations
############################################

sub check_file
{
    my ($arg) = @_;

    if(!( -f $arg ))
    {
        packager::exiter::exit_program("ERROR: Cannot find file $arg", "check_file");
    }
}

sub read_file
{
    my ($localfile) = @_;

    if ( ! open( IN, $localfile ) ) {
        # try again - sometimes we get errors caused by race conditions in parallel builds
        sleep 1;
        open( IN, $localfile ) or packager::exiter::exit_program("ERROR: Cannot open file: $localfile", "read_file");
    }
    my @localfile = <IN>;
    close( IN );

    return \@localfile;
}

###########################################
# Saving files
###########################################

sub save_file
{
    my ($savefile, $savecontent) = @_;
    open( OUT, ">$savefile" );
    print OUT @{$savecontent};
    close( OUT);
    if (! -f $savefile) { packager::exiter::exit_program("ERROR: Cannot write file: $savefile", "save_file"); }
}

######################################################
# Creating a new direcotory
######################################################

sub create_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, 0775);

        if ($returnvalue)
        {
            $infoline = "\nCreated directory: $directory\n";
            push(@packager::globals::logfileinfo, $infoline);

            if ($packager::globals::isunix)
            {
                my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                system($localcall);
            }
        }
        else
        {
            packager::exiter::exit_program("ERROR: Could not create directory: $directory", "create_directory");
        }
    }
}

######################################################
# Creating a unique directory with number extension
######################################################

sub create_unique_directory
{
    my ($directory) = @_;

    $directory =~ s/\Q$packager::globals::separator\E\s*$//;
    $directory = $directory . "_INCREASINGNUMBER";

    my $counter = 1;
    my $created = 0;
    my $localdirectory = "";

    do
    {
        $localdirectory = $directory;
        $localdirectory =~ s/INCREASINGNUMBER/$counter/;
        $counter++;

        if ( ! -d $localdirectory )
        {
            create_directory($localdirectory);
            $created = 1;
        }
    }
    while ( ! $created );

    return $localdirectory;
}

######################################################
# Removing a complete directory with subdirectories
######################################################

sub remove_complete_directory
{
    my ($directory) = @_;

    my @content = ();

    $directory =~ s/\Q$packager::globals::separator\E\s*$//;

    if ( -d $directory )
    {
        opendir(DIR, $directory);
        @content = readdir(DIR);
        closedir(DIR);

        my $oneitem;

        foreach $oneitem (@content)
        {
            if ((!($oneitem eq ".")) && (!($oneitem eq "..")))
            {
                my $item = $directory . $packager::globals::separator . $oneitem;

                if ( -f $item )     # deleting files
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

        rmdir $directory;

    }
}

1;
