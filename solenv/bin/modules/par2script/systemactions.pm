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


package par2script::systemactions;

use File::Copy;
use par2script::exiter;
use par2script::globals;

######################################################
# Creating a new directory
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
    }
    else
    {
        $path = $path . $par2script::globals::platformid . $par2script::globals::separator;
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
