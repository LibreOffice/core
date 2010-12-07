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


package par2script::systemactions;

use File::Copy;
use par2script::exiter;
use par2script::globals;

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
            $infoline = "Created directory: $directory\n";
            push(@par2script::globals::logfileinfo, $infoline);

            if ($par2script::globals::isunix)
            {
                my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                system($localcall);
            }
        }
        else
        {
            par2script::exiter::exit_program("Error: Could not create directory: $directory", "create_directory");
        }
    }
}

#######################################################################
# Creating the directories, in which files are generated or unzipped
#######################################################################

sub create_directories
{
    my ($directory, $languagesref) =@_;

    $par2script::globals::unpackpath =~ s/\Q$par2script::globals::separator\E\s*$//;    # removing ending slashes and backslashes

    my $path = $par2script::globals::unpackpath;     # this path already exists

    $path = $path . $par2script::globals::separator . $par2script::globals::build . $par2script::globals::separator;
    create_directory($path);

    $path = $path . $par2script::globals::minor . $par2script::globals::separator;
    create_directory($path);

    if ($directory eq "unzip" )
    {
        $path = $path . "common" . $par2script::globals::productextension . $par2script::globals::separator;
        create_directory($path);

        $path = $path . $directory . $par2script::globals::separator;
        create_directory($path);
    }
    else
    {
        $path = $path . $par2script::globals::compiler . $par2script::globals::productextension . $par2script::globals::separator;
        create_directory($path);

        $path = $path . $par2script::globals::product . $par2script::globals::separator;
        create_directory($path);

        $path = $path . $directory . $par2script::globals::separator;
        create_directory($path);

        if (!($$languagesref eq "" ))   # this will be a path like "01_49", for Profiles and ConfigurationFiles, idt-Files
        {
            $path = $path . $$languagesref . $par2script::globals::separator;
            create_directory($path);
        }
    }

    $path =~ s/\Q$par2script::globals::separator\E\s*$//;

    return $path;
}

########################
# Copying one file
########################

sub copy_one_file
{
    my ($source, $dest) = @_;

    my ($copyreturn, $returnvalue);
    my $infoline;

    $copyreturn = copy($source, $dest);

    if ($copyreturn)
    {
        $infoline = "Copy: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "Error: Could not copy $source to $dest $!\n";
        $returnvalue = 0;
    }

    push(@par2script::globals::logfileinfo, $infoline);

    return $returnvalue;
}

##########################################
# Copying all files from one directory
# to another directory
##########################################

sub copy_directory
{
    my ($sourcedir, $destdir) = @_;

    my ($onefile, $sourcefile, $destfile);
    my @sourcefiles = ();

    $sourcedir =~ s/\Q$par2script::globals::separator\E\s*$//;
    $destdir =~ s/\Q$par2script::globals::separator\E\s*$//;

    $infoline = "\n";
    push(@par2script::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    push(@par2script::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            $sourcefile = $sourcedir . $par2script::globals::separator . $onefile;
            $destfile = $destdir . $par2script::globals::separator . $onefile;
            if ( -f $sourcefile )   # only files, no directories
            {
                copy_one_file($sourcefile, $destfile);
            }
        }
    }
}


1;
