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
        sleep 5;
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
