#*************************************************************************
#
#   $RCSfile: systemactions.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2004-08-12 08:29:46 $
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

package installer::systemactions;

use Cwd;
use File::Copy;
use installer::converter;
use installer::exiter;
use installer::globals;
use installer::pathanalyzer;
use installer::remover;

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
            push(@installer::globals::logfileinfo, $infoline);

            if ($installer::globals::isunix)
            {
                my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                system($localcall);
            }
        }
        else
        {
            installer::exiter::exit_program("ERROR: Could not create directory: $directory", "create_directory");
        }
    }
}

######################################################
# Removing a new direcotory
######################################################

sub remove_empty_directory
{
    my ($directory) = @_;

    my $returnvalue = 1;

    if (-d $directory)
    {
        my $systemcall = "rmdir $directory";

        $returnvalue = system($systemcall);

        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not remove \"$directory\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Removed \"$directory\"!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

#######################################################################
# Creating the directories, in which files are generated or unzipped
#######################################################################

sub create_directories
{
    my ($newdirectory, $languagesref) =@_;

    $installer::globals::unpackpath =~ s/\Q$installer::globals::separator\E\s*$//;  # removing ending slashes and backslashes

    my $path = $installer::globals::unpackpath . $installer::globals::separator;     # this path already exists

    if ($newdirectory eq "unzip" )  # special handling for common directory
    {
        $path = $path  . ".." . $installer::globals::separator . "common" . $installer::globals::productextension . $installer::globals::separator;
        create_directory($path);

        $path = $path . $newdirectory . $installer::globals::separator;
        create_directory($path);
    }
    else
    {
        if ( ! $installer::globals::languagepack ) { $path = $path . $installer::globals::product . $installer::globals::separator; }
        else { $path = $path . $installer::globals::product . "_languagepack" . $installer::globals::separator; }

        create_directory($path);

        $path = $path . $newdirectory . $installer::globals::separator;
        create_directory($path);

        my $locallanguagesref = "";

        if ( $$languagesref ) { $locallanguagesref = $$languagesref; }

        if (!($locallanguagesref eq "" ))   # this will be a path like "01_49", for Profiles and ConfigurationFiles, idt-Files
        {
            $path = $path . $$languagesref . $installer::globals::separator;
            create_directory($path);
        }
    }

    installer::remover::remove_ending_pathseparator(\$path);

    $path = installer::converter::make_path_conform($path);

    return $path;
}

########################
# Copying one file
########################

sub copy_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $copyreturn = copy($source, $dest);

    if ($copyreturn)
    {
        $infoline = "Copy: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not copy $source to $dest\n";
        $returnvalue = 0;
    }

    push(@installer::globals::logfileinfo, $infoline);

    return $returnvalue;
}

########################
# Renaming one file
########################

sub rename_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $renamereturn = rename($source, $dest);

    if ($renamereturn)
    {
        $infoline = "Rename: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not rename $source to $dest\n";
        $returnvalue = 0;
    }

    push(@installer::globals::logfileinfo, $infoline);

    return $returnvalue;
}

##########################################
# Copying all files from one directory
# to another directory
##########################################

sub copy_directory
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $sourcefile = $sourcedir . $installer::globals::separator . $onefile;
            my $destfile = $destdir . $installer::globals::separator . $onefile;
            if ( -f $sourcefile )   # only files, no directories
            {
                copy_one_file($sourcefile, $destfile);
            }
        }
    }
}

#####################################################
# Copying a complete directory with sub directories.
#####################################################

sub copy_complete_directory
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $source = $sourcedir . $installer::globals::separator . $onefile;
            my $dest = $destdir . $installer::globals::separator . $onefile;
            if ( -f $source )   # only files, no directories
            {
                copy_one_file($source, $dest);
            }
            if ( -d $source )   # recursive
            {
                if ((!( $source =~ /packages\/SUNW/ )) && (!( $source =~ /packages\/OOO/ )))    # do not copy complete Solaris packages!
                {
                    copy_complete_directory($source, $dest);
                }
            }
        }
    }
}

#####################################################################################
# Copying a complete directory with sub directories, but not the CVS directories.
#####################################################################################

sub copy_complete_directory_without_cvs
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir (without CVS)\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")) && (!($onefile eq "CVS")))
        {
            my $source = $sourcedir . $installer::globals::separator . $onefile;
            my $dest = $destdir . $installer::globals::separator . $onefile;
            if ( -f $source )   # only files, no directories
            {
                copy_one_file($source, $dest);
            }
            if ( -d $source )   # recursive
            {
                copy_complete_directory_without_cvs($source, $dest);
            }
        }
    }
}

#####################################################
# Copying all files with a specified file extension
# from one directory to another directory.
#####################################################

sub copy_directory_with_fileextension
{
    my ($sourcedir, $destdir, $extension) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files with extension $extension from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( $onefile =~ /\.$extension\s*$/ )   # only copying specified files
            {
                my $sourcefile = $sourcedir . $installer::globals::separator . $onefile;
                my $destfile = $destdir . $installer::globals::separator . $onefile;
                if ( -f $sourcefile )   # only files, no directories
                {
                    copy_one_file($sourcefile, $destfile);
                }
            }
        }
    }
}

#########################################################
# Copying all files without a specified file extension
# from one directory to another directory.
#########################################################

sub copy_directory_except_fileextension
{
    my ($sourcedir, $destdir, $extension) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Copying files without extension $extension from directory $sourcedir to directory $destdir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $sourcedir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( ! ( $onefile =~ /\.$extension\s*$/ ))  # only copying not having the specified extension
            {
                my $sourcefile = $sourcedir . $installer::globals::separator . $onefile;
                my $destfile = $destdir . $installer::globals::separator . $onefile;
                if ( -f $sourcefile )   # only files, no directories
                {
                    copy_one_file($sourcefile, $destfile);
                }
            }
        }
    }
}

########################################################
# Renaming all files with a specified file extension
# in a specified directory.
# Example: "Feature.idt.01" -> "Feature.idt"
########################################################

sub rename_files_with_fileextension
{
    my ($dir, $extension) = @_;

    my @sourcefiles = ();

    $dir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Renaming files with extension \"$extension\" in the directory $dir\n";
    push(@installer::globals::logfileinfo, $infoline);

    opendir(DIR, $dir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            if ( $onefile =~ /^\s*(\S.*?)\.$extension\s*$/ )    # only renaming specified files
            {
                my $destfile = $1;
                my $sourcefile = $dir . $installer::globals::separator . $onefile;
                $destfile = $dir . $installer::globals::separator . $destfile;
                if ( -f $sourcefile )   # only files, no directories
                {
                    rename_one_file($sourcefile, $destfile);
                }
            }
        }
    }
}

########################################################
# Finding all files with a specified file extension
# in a specified directory.
########################################################

sub find_file_with_file_extension
{
    my ($extension, $dir) = @_;

    my @allfiles = ();

    $dir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $infoline = "\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Searching files with extension \"$extension\" in the directory $dir\n";
    push(@installer::globals::logfileinfo, $infoline);

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
# Creating a unique directory, for example "01_inprogress_7"
# in the install directory.
##############################################################

sub make_numbered_dir
{
    my ($newstring, $olddir) = @_;

    my $basedir = $olddir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    my $alldirs = get_all_directories($basedir);

    # searching for the highest number extension

    my $maxnumber = 0;

    for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
    {
        if ( ${$alldirs}[$i] =~ /\_(\d+)\s*$/ )
        {
            my $number = $1;
            if ( $number > $maxnumber ) { $maxnumber = $number; }
        }
    }

    my $newnumber = $maxnumber + 1;

    my $newdir = $olddir . "_" . $newstring . "_" . $newnumber;

    my $returndir = "";

    if ( move($olddir, $newdir) )
    {
        $infoline = "\nMoved directory from $olddir to $newdir\n";
        push(@installer::globals::logfileinfo, $infoline);
        $returndir = $newdir;
    }
    else
    {
        $infoline = "\nATTENTION: Could not move directory from $olddir to $newdir, \"make_numbered_dir\"\n";
        push(@installer::globals::logfileinfo, $infoline);
        $returndir = $olddir;
    }

    return $returndir;
}

##############################################################
# Determining the highest number in the install directory.
##############################################################

sub determine_maximum_number
{
    my ($dir, $languagestringref) = @_;

    my $basedir = $dir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    my $alldirs = get_all_directories($basedir);

    my $maxnumber = 1;

    # In control.pm the installation directory is determined as:
    # $installer::globals::build . "_" . $installer::globals::lastminor . "_" .
    # "native_inprogress-number_" . $$languagesref . "\." . $installer::globals::buildid;

    # searching for the highest number extension after the first "-", which belongs to
    # $installer::globals::build, $installer::globals::lastminor and $installer::globals::buildid
    # In this step not looking for the language!

    my @correctbuildiddirs = ();

    for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
    {
        my $onedir = ${$alldirs}[$i];
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$onedir);

        if ( $onedir =~ /^\s*\Q$installer::globals::build\E\_\Q$installer::globals::lastminor\E\_(.*?)\-(\d+)\_(.*?)\.\Q$installer::globals::buildid\E\s*$/ )
        {
            my $number = $2;
            if ( $number > $maxnumber ) { $maxnumber = $number; }
            push(@correctbuildiddirs, $onedir);
        }
    }

    # From all directories with correct $installer::globals::build, $installer::globals::lastminor
    # and $installer::globals::buildid, those directories, which already have the maximum number
    # have to be selected

    my @maximumnumberdirs = ();

    for ( my $i = 0; $i <= $#correctbuildiddirs; $i++ )
    {
        my $onedir = $correctbuildiddirs[$i];

        if ( $onedir =~ /^\s*(.*?)\-(\d+)\_(.*?)\.(.*?)\s*$/ )
        {
            my $number = $2;

            if ( $number == $maxnumber )
            {
                push(@maximumnumberdirs, $onedir);
            }
        }
    }

    # @maximumnumberdirs contains only those directories with correct $installer::globals::build,
    # $installer::globals::lastminor and $installer::globals::buildid, which already have the maximum number.
    # If the current language is part of this directory, the number has to be increased.

    my $increase_counter = 0;

    for ( my $i = 0; $i <= $#maximumnumberdirs; $i++ )
    {
        my $onedir = $maximumnumberdirs[$i];

        if ( $onedir =~ /^\s*(.*?)\-(\d+)\_(.*?)\.(.*?)\s*$/ )
        {
            my $number = $2;
            my $languagestring = $3;

            if ( $languagestring eq $$languagestringref )
            {
                $increase_counter = 1;
            }
        }
    }

    if ( $increase_counter )
    {
        $maxnumber = $maxnumber + 1;
    }

    return $maxnumber;
}

#####################################################################################
# Renaming a directory by exchanging a string, for example from "01_inprogress_7"
# to "01_witherror_7".
#####################################################################################

sub rename_string_in_directory
{
    my ($olddir, $oldstring, $newstring) = @_;

    my $newdir = $olddir;
    my $infoline = "";

    $newdir =~ s/$oldstring/$newstring/g;

    if ( move($olddir, $newdir) )
    {
        $infoline = "\nMoved directory from $olddir to $newdir\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "\nATTENTION: Could not move directory from $olddir to $newdir, \"rename_string_in_directory\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    return $newdir;
}

######################################################
# Returning the complete directory name,
# input is the first part of the directory name.
######################################################

sub get_directoryname
{
    my ($searchdir, $startstring) = @_;

    my $dirname = "";
    my $founddir = 0;
    my $direntry;

    opendir(DIR, $searchdir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        if (( -d $direntry ) && ( $direntry =~ /^\s*\Q$startstring\E/ ))
        {
            $dirname = $direntry;
            $founddir = 1;
            last;
        }
    }

    closedir(DIR);

    if ( ! $founddir ) { installer::exiter::exit_program("ERROR: Did not find directory beginning with $startstring in directory $searchdir", "get_directoryname"); }

    return $dirname;
}


###################################
# Renaming a directory
###################################

sub rename_directory
{
    my ($olddir, $newdir) = @_;

    my $infoline = "";

    if ( move($olddir, $newdir) )
    {
        $infoline = "\nMoved directory from $olddir to $newdir\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "\nATTENTION: Could not move directory from $olddir to $newdir, \"rename_directory\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    return $newdir;
}

##############################################################
# Creating a directory next to an existing directory
##############################################################

sub create_directory_next_to_directory
{
    my ($topdir, $dirname) = @_;

    my $basedir = $topdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$basedir);

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $newdir = $basedir . $installer::globals::separator . $dirname;

    create_directory($newdir);

    return $newdir;
}

##############################################################
# Collecting all directories inside a directory
##############################################################

sub get_all_directories
{
    my ($basedir) = @_;

    my @alldirs = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if ( -d $completeentry ) { push(@alldirs, $completeentry); }
    }

    closedir(DIR);

    return \@alldirs;
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
            $infoline = "\nCreated directory: $directory\n";
            push(@installer::globals::logfileinfo, $infoline);

            if ($installer::globals::isunix)
            {
                my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                system($localcall);
            }
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

##############################################################
# Creating a complete directory structure
##############################################################

sub create_directory_structure
{
    my ($directory) = @_;

    if ( ! try_to_create_directory($directory) )
    {
        my $parentdir = $directory;
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$parentdir);
        create_directory_structure($parentdir);                                 # recursive
    }

    create_directory($directory);   # now it has to succeed
}

######################################################
# Removing a complete directory with subdirectories
######################################################

sub remove_complete_directory
{
    my ($directory, $start) = @_;

    my @content = ();

    $directory =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( -d $directory )
    {
        if ( $start )
        {
            my $infoline = "\n";
            push(@installer::globals::logfileinfo, $infoline);
            $infoline = "Removing directory $directory\n";
            push(@installer::globals::logfileinfo, $infoline);
        }

        opendir(DIR, $directory);
        @content = readdir(DIR);
        closedir(DIR);

        my $oneitem;

        foreach $oneitem (@content)
        {
            if ((!($oneitem eq ".")) && (!($oneitem eq "..")))
            {
                my $item = $directory . $installer::globals::separator . $oneitem;

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
