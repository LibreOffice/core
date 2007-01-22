#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: systemactions.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2007-01-22 14:47:37 $
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


package pre2par::systemactions;

use File::Copy;
use pre2par::exiter;
use pre2par::globals;

######################################################
# Creating a new direcotory
######################################################

sub create_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;
    my $infoline = "";

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, 0775);

        if ($returnvalue)
        {
            $infoline = "Created directory: $directory\n";
            push(@pre2par::globals::logfileinfo, $infoline);

            if ($pre2par::globals::isunix)
            {
                my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                system($localcall);
            }
        }
        else
        {
            # New solution in parallel packing: It is possible, that the directory now exists, although it
            # was not created in this process. There is only an important error, if the directory does not
            # exist now.

            if (!(-d $directory))
            {
                pre2par::exiter::exit_program("Error: Could not create directory: $directory", "create_directory");
            }
            else
            {
                $infoline = "\nAnother process created this directory in exactly this moment :-) : $directory\n";
                push(@pre2par::globals::logfileinfo, $infoline);
            }
        }
    }
    else
    {
        $infoline = "\nAlready existing directory, did not create: $directory\n";
        push(@pre2par::globals::logfileinfo, $infoline);
    }
}

#######################################################################
# Creating the directories, in which files are generated or unzipped
#######################################################################

sub create_directories
{
    my ($directory, $languagesref) =@_;

    $pre2par::globals::unpackpath =~ s/\Q$pre2par::globals::separator\E\s*$//;  # removing ending slashes and backslashes

    my $path = $pre2par::globals::unpackpath;    # this path already exists

    $path = $path . $pre2par::globals::separator . $pre2par::globals::build . $pre2par::globals::separator;
    create_directory($path);

    $path = $path . $pre2par::globals::minor . $pre2par::globals::separator;
    create_directory($path);

    if ($directory eq "unzip" )
    {
        $path = $path . "common" . $pre2par::globals::productextension . $pre2par::globals::separator;
        create_directory($path);

        $path = $path . $directory . $pre2par::globals::separator;
        create_directory($path);
    }
    else
    {
        $path = $path . $pre2par::globals::compiler . $pre2par::globals::productextension . $pre2par::globals::separator;
        create_directory($path);

        $path = $path . $pre2par::globals::product . $pre2par::globals::separator;
        create_directory($path);

        $path = $path . $directory . $pre2par::globals::separator;
        create_directory($path);

        if (!($$languagesref eq "" ))   # this will be a path like "01_49", for Profiles and ConfigurationFiles, idt-Files
        {
            $path = $path . $$languagesref . $pre2par::globals::separator;
            create_directory($path);
        }
    }

    $path =~ s/\Q$pre2par::globals::separator\E\s*$//;

    return $path;
}

########################
# Copying one file
########################

sub copy_one_file
{
    my ($source, $dest) = @_;

    my ($copyreturn, $returnvalue, $infoline);

    $copyreturn = copy($source, $dest);

    if ($copyreturn)
    {
        $infoline = "Copy: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "Error: Could not copy $source to $dest\n";
        $returnvalue = 0;
    }

    push(@pre2par::globals::logfileinfo, $infoline);

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

    $sourcedir =~ s/\Q$pre2par::globals::separator\E\s*$//;
    $destdir =~ s/\Q$pre2par::globals::separator\E\s*$//;

    $infoline = "\n";
    push(@pre2par::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    push(@pre2par::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            $sourcefile = $sourcedir . $pre2par::globals::separator . $onefile;
            $destfile = $destdir . $pre2par::globals::separator . $onefile;
            if ( -f $sourcefile )   # only files, no directories
            {
                copy_one_file($sourcefile, $destfile);
            }
        }
    }
}


1;
