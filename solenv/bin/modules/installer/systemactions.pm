#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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
    my $infoline = "";

    if (!(-d $directory))
    {
        $returnvalue = mkdir($directory, 0775);

        if ($returnvalue)
        {
            $installer::logger::Lang->print("\n");
            $installer::logger::Lang->printf("Created directory: %s\n", $directory);

            my $localcall = "chmod 0775 $directory \>\/dev\/null 2\>\&1";
            system($localcall);

            # chmod 0775 is not sufficient on mac to remove sticky tag
            $localcall = "chmod a-s $directory \>\/dev\/null 2\>\&1";
            system($localcall);
        }
        else
        {
            # New solution in parallel packing: It is possible, that the directory now exists, although it
            # was not created in this process. There is only an important error, if the directory does not
            # exist now.

            $installer::logger::Lang->print("\n");
            $installer::logger::Lang->printf(
                "Did not succeed in creating directory: \"%s\". Further attempts will follow.\n",
                $directory);

            if (!(-d $directory))
            {
                # Problem with parallel packaging? -> Try a little harder, before exiting.
                # Did someone else remove the parent directory in the meantime?
                my $parentdir = $directory;
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$parentdir);
                if (!(-d $parentdir))
                {
                    $returnvalue = mkdir($parentdir, 0775);

                    if ($returnvalue)
                    {
                        $installer::logger::Lang->print("\n");
                        $installer::logger::Lang->printf(
                            "Attention: Successfully created parent directory (should already be created before): %s\n",
                            $parentdir);

                        my $localcall = "chmod 775 $parentdir \>\/dev\/null 2\>\&1";
                        system($localcall);
                    }
                    else
                    {
                        $infoline = "\Error: \"$directory\" could not be created. Even the parent directory \"$parentdir\" does not exist and could not be created.\n";
                        $installer::logger::Lang->print($infoline);
                        if ( -d $parentdir )
                        {
                            $installer::logger::Lang->print("\n");
                            $installer::logger::Lang->printf(
                                "Attention: Finally the parent directory \"%s\" exists, but I could not create it.\n",
                                $parentdir);
                        }
                        else
                        {
                            # Now it is time to exit, even the parent could not be created.
                            installer::exiter::exit_program("ERROR: Could not create parent directory \"$parentdir\"", "create_directory");
                        }
                    }
                }

                # At this point we have to assume, that the parent directory exist.
                # Trying once more to create the desired directory

                $returnvalue = mkdir($directory, 0775);

                if ($returnvalue)
                {
                    $installer::logger::Lang->print("\n");
                    $installer::logger::Lang->printf(
                        "Attention: Created directory \"\" in the second try.\n",
                        $directory);;

                    my $localcall = "chmod 775 $directory \>\/dev\/null 2\>\&1";
                    system($localcall);
                }
                else
                {
                    if ( -d $directory )
                    {
                        $installer::logger::Lang->print("\n");
                        $installer::logger::Lang->printf(
                            "Attention: Finally the directory \"%s\" exists, but I could not create it.\n",
                            $directory);
                    }
                    else
                    {
                        # It is time to exit, even the second try failed.
                        installer::exiter::exit_program("ERROR: Failed to create the directory: $directory", "create_directory");
                    }
                }
            }
            else
            {
                $installer::logger::Lang->print("\n");
                $installer::logger::Lang->printf(
                    "Another process created this directory in exactly this moment :-) : %s\n",
                    $directory);;
            }
        }
    }
    else
    {
        $installer::logger::Lang->printf("Already existing directory, did not create: %s\n", $directory);
    }
}

######################################################
# Creating a new direcotory with defined privileges
######################################################

sub create_directory_with_privileges
{
    my ($directory, $privileges) = @_;

    my $returnvalue = 1;
    my $infoline = "";

    if (!(-d $directory))
    {
        my $localprivileges = oct("0".$privileges); # changes "777" to 0777
        $returnvalue = mkdir($directory, $localprivileges);

        if ($returnvalue)
        {
            $installer::logger::Lang->print("\n");
            $installer::logger::Lang->printf("Created directory: %s\n", $directory);

            my $localcall = "chmod $privileges $directory \>\/dev\/null 2\>\&1";
            system($localcall);
        }
        else
        {
            # New solution in parallel packing: It is possible, that the directory now exists, although it
            # was not created in this process. There is only an important error, if the directory does not
            # exist now.

            $installer::logger::Lang->print("\n");
            $installer::logger::Lang->printf(
                "Did not succeed in creating directory: \"%s\". Further attempts will follow.\n",
                $directory);

            if (!(-d $directory))
            {
                # Problem with parallel packaging? -> Try a little harder, before exiting.
                # Did someone else remove the parent directory in the meantime?
                my $parentdir = $directory;
                installer::pathanalyzer::get_path_from_fullqualifiedname(\$parentdir);
                if (!(-d $parentdir))
                {
                    $returnvalue = mkdir($directory, $localprivileges);

                    if ($returnvalue)
                    {
                        $installer::logger::Lang->print("\n");
                        $installer::logger::Lang->printf(
                            "Attention: Successfully created parent directory (should already be created before): %s\n",
                            $parentdir);

                        my $localcall = "chmod $privileges $parentdir \>\/dev\/null 2\>\&1";
                        system($localcall);
                    }
                    else
                    {
                        $infoline = "\Error: \"$directory\" could not be created. Even the parent directory \"$parentdir\" does not exist and could not be created.\n";
                        $installer::logger::Lang->print($infoline);
                        if ( -d $parentdir )
                        {
                            $installer::logger::Lang->print("\n");
                            $installer::logger::Lang->printf(
                                "Attention: Finally the parent directory \"%s\" exists, but I could not create it.\n",
                                $parentdir);
                        }
                        else
                        {
                            # Now it is time to exit, even the parent could not be created.
                            installer::exiter::exit_program("ERROR: Could not create parent directory \"$parentdir\"", "create_directory_with_privileges");
                        }
                    }
                }

                # At this point we have to assume, that the parent directory exist.
                # Trying once more to create the desired directory

                $returnvalue = mkdir($directory, $localprivileges);

                if ($returnvalue)
                {
                    $installer::logger::Lang->print("\n");
                    $installer::logger::Lang->printf("Attention: Created directory \"%s\" in the second try.\n",
                        $directory);

                    my $localcall = "chmod $privileges $directory \>\/dev\/null 2\>\&1";
                    system($localcall);
                }
                else
                {
                    if ( -d $directory )
                    {
                        $installer::logger::Lang->print("\n");
                        $installer::logger::Lang->printf(
                            "Attention: Finally the directory \"\" exists, but I could not create it.\n",
                            $directory);
                    }
                    else
                    {
                        # It is time to exit, even the second try failed.
                        installer::exiter::exit_program("ERROR: Failed to create the directory: $directory", "create_directory_with_privileges");
                    }
                }
            }
            else
            {
                $installer::logger::Lang->print("\n");
                $installer::logger::Lang->printf(
                    "Another process created this directory in exactly this moment :-) : %s\n",
                    $directory);
            }
        }
    }
    else
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf("Already existing directory, did not create: %s\n", $directory);

        my $localcall = "chmod $privileges $directory \>\/dev\/null 2\>\&1";
        system($localcall);
    }
}




=item is_directory_empty ($path)
    Return
        1 if there are no files in the directory pointed to by $path
        0 if there are files
       -1 if there is an error accessing the directory.
=cut
sub is_directory_empty ($)
{
    my ($path) = @_;

    opendir my $dir, $path or return -1;

    my $result = 1;
    while (my $entry = readdir($dir))
    {
        if ($entry !~ /^\.+$/)
        {
            $result = 0;
            last;
        }
    }

    return $result;
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
        if ( ! is_directory_empty($directory))
        {
            $installer::logger::Lang->printf("directory '%s' is not empty and can not be removed\n", $directory);
            return;
        }
        else
        {
            my $systemcall = "rmdir $directory";

            $returnvalue = system($systemcall);

            $installer::logger::Lang->printf("Systemcall: %s\n", $systemcall);

            if ($returnvalue)
            {
                $installer::logger::Lang->printf("ERROR: Could not remove \"%s\"!\n", $directory);
            }
            else
            {
                $installer::logger::Lang->printf("Success: Removed \"%s\"!\n", $directory);
            }
        }
    }
}

#######################################################################
# Calculating the number of languages in the string
#######################################################################

sub get_number_of_langs
{
    my ($languagestring) = @_;

    my $number = 1;

    my $workstring = $languagestring;

    while ( $workstring =~ /^\s*(.*)_(.*?)\s*$/ )
    {
        $workstring = $1;
        $number++;
    }

    return $number;
}

#######################################################################
# Creating the directories, in which files are generated or unzipped
#######################################################################

sub create_directories
{
    my ($newdirectory, $languagesref) =@_;

    $installer::globals::unpackpath =~ s/\Q$installer::globals::separator\E\s*$//;  # removing ending slashes and backslashes

    my $path = "";

    if (( $newdirectory eq "uno" ) || ( $newdirectory eq "zip" ) || ( $newdirectory eq "cab" ) || ( $newdirectory =~ /rdb\s*$/i )) # special handling for zip files, cab files and services file because of performance reasons
    {
        if ( $installer::globals::temppathdefined ) { $path = $installer::globals::temppath; }
        else { $path = $installer::globals::unpackpath; }
        $path =~ s/\Q$installer::globals::separator\E\s*$//;    # removing ending slashes and backslashes
        $path = $path . $installer::globals::separator;
    }
    elsif ( ( $newdirectory eq "jds" ) )
    {
        if ( $installer::globals::jdstemppathdefined ) { $path = $installer::globals::jdstemppath; }
        else { $path = $installer::globals::unpackpath; }
        $path =~ s/\Q$installer::globals::separator\E\s*$//;    # removing ending slashes and backslashes
        $path = $path . $installer::globals::separator;
        installer::systemactions::create_directory($path);
    }
    else
    {
        $path = $installer::globals::unpackpath . $installer::globals::separator;

        # special handling, if LOCALINSTALLDIR is set
        if (( $installer::globals::localinstalldirset ) && ( $newdirectory eq "install" ))
        {
            $installer::globals::localinstalldir =~ s/\Q$installer::globals::separator\E\s*$//;
            $path = $installer::globals::localinstalldir . $installer::globals::separator;
        }
    }

    $infoline = "create_directories: Using $path for $newdirectory !\n";
    $installer::logger::Lang->print($infoline);

    if ($newdirectory eq "unzip" )  # special handling for common directory
    {
        $path = $path  . ".." . $installer::globals::separator . "common" . $installer::globals::productextension . $installer::globals::separator;
        create_directory($path);

        $path = $path . $newdirectory . $installer::globals::separator;
        create_directory($path);
    }
    else
    {
        my $localproductname = $installer::globals::product;
        my $localproductsubdir = "";

        if ( $installer::globals::product =~ /^\s*(.+?)\_\_(.+?)\s*$/ )
        {
            $localproductname = $1;
            $localproductsubdir = $2;
        }

        if ( $installer::globals::languagepack ) { $path = $path . $localproductname . "_languagepack" . $installer::globals::separator; }
        elsif ( $installer::globals::patch ) { $path = $path . $localproductname . "_patch" . $installer::globals::separator; }
        else { $path = $path . $localproductname . $installer::globals::separator; }

        create_directory($path);

        if ( $localproductsubdir )
        {
            $path = $path . $localproductsubdir . $installer::globals::separator;
            create_directory($path);
        }

        $path = $path . $installer::globals::installertypedir . $installer::globals::separator;
        create_directory($path);

        $path = $path . $newdirectory . $installer::globals::separator;
        create_directory($path);

        my $locallanguagesref = "";

        if ( $$languagesref ) { $locallanguagesref = $$languagesref; }

        if (!($locallanguagesref eq "" ))   # this will be a path like "01_49", for Profiles and ConfigurationFiles, idt-Files
        {
            my $languagestring = installer::languages::get_language_directory_name($$languagesref);

            $path = $path . $languagestring  . $installer::globals::separator;
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


    # copy returns 0 if files are identical :-(
    if ( $installer::globals::isos2 ) {
        unlink($dest);
    }

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

    $installer::logger::Lang->print($infoline);

    if ( !$returnvalue ) {
        return $returnvalue;
    }

    # taking care of file attributes
    if ($installer::globals::iswin && -f $dest) {
        my $mode = -x $source ? 0775 : 0664;
        my $mode_str = sprintf("%o", $mode);
        my $chmodreturn = chmod($mode, $dest);
        if ($chmodreturn)
        {
            $infoline = "chmod $mode_str, $dest\n";
            $returnvalue = 1;
        }
        else
        {
            $infoline = "WARNING: Could not chmod $dest: $!\n";
            $returnvalue = 0;
        }

        $installer::logger::Lang->print($infoline);
    }

    return $returnvalue;
}

##########################
# Hard linking one file
##########################

sub hardlink_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $copyreturn = link($source, $dest);

    if ($copyreturn)
    {
        $infoline = "Link: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not link $source to $dest\n";
        $returnvalue = 0;
    }

    $installer::logger::Lang->print($infoline);

    return $returnvalue;
}

##########################
# Soft linking one file
##########################

sub softlink_one_file
{
    my ($source, $dest) = @_;

    my ($returnvalue, $infoline);

    my $linkreturn = symlink($source, $dest);

    if ($linkreturn)
    {
        $infoline = "Symlink: $source to $dest\n";
        $returnvalue = 1;
    }
    else
    {
        $infoline = "ERROR: Could not symlink $source to $dest\n";
        $returnvalue = 0;
    }

    $installer::logger::Lang->print($infoline);

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

    $installer::logger::Lang->print($infoline);

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
    $installer::logger::Lang->print($infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    $installer::logger::Lang->print($infoline);

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

##########################################
# Copying all files from one directory
# to another directory
##########################################

sub is_empty_dir
{
    my ($dir) = @_;

    my $directory_is_empty = 1;
    my @sourcefiles = ();

    opendir(DIR, $dir);
    @sourcefiles = readdir(DIR);
    closedir(DIR);

    my $onefile;
    my @realcontent = ();

    foreach $onefile (@sourcefiles)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            push(@realcontent, $onefile);
        }
    }

    if ( $#realcontent > -1 ) { $directory_is_empty = 0; }

    return $directory_is_empty;
}

#####################################################################
# Creating hard links to a complete directory with sub directories.
#####################################################################

sub hardlink_complete_directory
{
    my ($sourcedir, $destdir) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    $installer::logger::Lang->print($infoline);
    $infoline = "Creating hard links for all files from directory $sourcedir to directory $destdir\n";
    $installer::logger::Lang->print($infoline);

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
                hardlink_one_file($source, $dest);
            }
            if ( -d $source )   # recursive
            {
                hardlink_complete_directory($source, $dest);
            }
        }
    }
}

#####################################################################
# Creating hard links to a complete directory with sub directories.
#####################################################################

sub softlink_complete_directory
{
    my ($sourcedir, $destdir, $depth) = @_;

    my @sourcefiles = ();

    $sourcedir =~ s/\Q$installer::globals::separator\E\s*$//;
    $destdir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( ! -d $destdir ) { create_directory($destdir); }

    my $infoline = "\n";
    $installer::logger::Lang->print($infoline);
    $infoline = "Creating soft links for all files from directory $sourcedir to directory $destdir\n";
    $installer::logger::Lang->print($infoline);

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
                my $localsource = $source;
                if ( $depth > 0 ) { for ( my $i = 1; $i <= $depth; $i++ ) { $localsource = "../" . $localsource; } }
                softlink_one_file($localsource, $dest);
            }
            if ( -d $source )   # recursive
            {
                my $newdepth = $depth + 1;
                softlink_complete_directory($source, $dest, $newdepth);
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
    $installer::logger::Lang->print($infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir\n";
    $installer::logger::Lang->print($infoline);

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
    $installer::logger::Lang->print($infoline);
    $infoline = "Copying files from directory $sourcedir to directory $destdir (without CVS)\n";
    $installer::logger::Lang->print($infoline);

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
    $installer::logger::Lang->print($infoline);
    $infoline = "Copying files with extension $extension from directory $sourcedir to directory $destdir\n";
    $installer::logger::Lang->print($infoline);

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
    $installer::logger::Lang->print($infoline);
    $infoline = "Copying files without extension $extension from directory $sourcedir to directory $destdir\n";
    $installer::logger::Lang->print($infoline);

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
    $installer::logger::Lang->print($infoline);
    $infoline = "Renaming files with extension \"$extension\" in the directory $dir\n";
    $installer::logger::Lang->print($infoline);

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
    $installer::logger::Lang->print($infoline);
    $infoline = "Searching files with extension \"$extension\" in the directory $dir\n";
    $installer::logger::Lang->print($infoline);

    opendir(DIR, $dir);
    @sourcefiles = sort readdir(DIR);
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
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf("Moved directory from %s to %s\n", $olddir, $newdir);
        $returndir = $newdir;
    }
    else
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf("ATTENTION: Could not move directory from %s to %s, \"make_numbered_dir\"\n",
            $olddir,
            $newdir);
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

    if (( -d $newdir ) && ( $olddir ne $newdir )) { remove_complete_directory($newdir, 1); }

    if ( move($olddir, $newdir) )
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf("Moved directory from $olddir to %s\n", $newdir);
    }
    else
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf(
            "ATTENTION: Could not move directory from %s to %s, \"rename_string_in_directory\"\n",
            $olddir, $newdir);
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

    # noticed problems under Windows from time to time that directories can't be moved, seems a timing issue
    # workaround with sleep, should be investigated with a new packaging mechanism
    sleep(2);
    if ( move($olddir, $newdir) )
    {
        $installer::logger::Lang->print("\n");
        $installer::logger::Lang->printf("Moved directory from $olddir to %s\n", $newdir);
    }
    else
    {
        installer::exiter::exit_program("ERROR: Could not move directory from $olddir to $newdir", "rename_directory");
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
# Collecting all directories inside a directory
# Returning without path
##############################################################

sub get_all_directories_without_path
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

        if ( -d $completeentry ) { push(@alldirs, $direntry); }
    }

    closedir(DIR);

    return \@alldirs;
}

##############################################################
# Collecting all files inside one directory
##############################################################

sub get_all_files_from_one_directory
{
    my ($basedir) = @_;

    my @allfiles = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if ( -f $completeentry ) { push(@allfiles, $completeentry); }
    }

    closedir(DIR);

    return \@allfiles;
}

##############################################################
# Collecting all files inside one directory
##############################################################

sub get_all_files_from_one_directory_without_path
{
    my ($basedir) = @_;

    my @allfiles = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if ( -f $completeentry ) { push(@allfiles, $direntry); }
    }

    closedir(DIR);

    return \@allfiles;
}

##############################################################
# Collecting all files and directories inside one directory
##############################################################

sub read_directory
{
    my ($basedir) = @_;

    my @allcontent = ();
    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if (( -f $completeentry ) || ( -d $completeentry )) { push(@allcontent, $completeentry); }
    }

    closedir(DIR);

    return \@allcontent;
}

##############################################################
# Finding the new content in a directory
##############################################################

sub find_new_content_in_directory
{
    my ( $basedir, $oldcontent ) = @_;

    my @newcontent = ();
    my @allcontent = ();

    my $direntry;

    $basedir =~ s/\Q$installer::globals::separator\E\s*$//;

    opendir(DIR, $basedir);

    foreach $direntry (readdir (DIR))
    {
        next if $direntry eq ".";
        next if $direntry eq "..";

        my $completeentry = $basedir . $installer::globals::separator . $direntry;

        if (( -f $completeentry ) || ( -d $completeentry ))
        {
            push(@allcontent, $completeentry);
            if (! installer::existence::exists_in_array($completeentry, $oldcontent))
            {
                push(@newcontent, $completeentry);
            }
        }
    }

    closedir(DIR);

    return (\@newcontent, \@allcontent);
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
            $installer::logger::Lang->print("\n");
            $installer::logger::Lang->printf("Created directory: %s\n", $directory);

            my $localcall = "chmod 0775 $directory \>\/dev\/null 2\>\&1";
            system($localcall);

            # chmod 0775 is not sufficient on mac to remove sticky tag
            $localcall = "chmod a-s $directory \>\/dev\/null 2\>\&1";
            system($localcall);
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

        $installer::logger::Lang->printf("INFO: Did not create directory %s\n", $directory);
        $installer::logger::Lang->printf("Now trying to create parent directory %s\n", $parentdir);

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
    my $infoline = "";

    $directory =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( -d $directory )
    {
        if ( $start )
        {
            $installer::logger::Lang->print("\n");
            $installer::logger::Lang->printf("Removing directory %s\n", $directory);
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

                if ( -f $item || -l $item )     # deleting files or links
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

        if ( ! -d $directory)
        {
            $installer::logger::Info->printf("trying to remove directory that doesn't exist: %s\n", $directory);
        }
        my $returnvalue = rmdir $directory;

        if ( ! $returnvalue )
        {
            $installer::logger::Lang->printf("Warning: Problem with removing empty dir %s\n", $directory);
        }

        # try a little bit harder (sometimes there is a performance problem)
        if ( -d $directory )
        {
            for ( my $j = 1; $j <= 3; $j++ )
            {
                if ( -d $directory )
                {
                    $installer::logger::Lang->print("\n");
                    $installer::logger::Lang->printf("Warning (Try %d): Problems with removing directory %s\n",
                        $j, $directory);

                    $returnvalue = rmdir $directory;

                    if ( $returnvalue )
                    {
                        $installer::logger::Lang->printf("Successfully removed empty dir %s\n", $directory);
                    }
                    else
                    {
                        $installer::logger::Lang->printf("Warning: rmdir %s failed.\n", $directory);
                    }
                }
            }
        }
    }
}

######################################################
# Creating a unique directory with number extension
######################################################

sub create_unique_directory
{
    my ($directory) = @_;

    $directory =~ s/\Q$installer::globals::separator\E\s*$//;
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
# Creating a unique directory with pid extension
######################################################

sub create_pid_directory
{
    my ($directory) = @_;

    $directory =~ s/\Q$installer::globals::separator\E\s*$//;
    my $pid = $$;           # process id
    my $time = time();      # time

    $directory = $directory . "_" . $pid . $time;

    if ( ! -d $directory ) { create_directory($directory); }
    else { installer::exiter::exit_program("ERROR: Directory $directory already exists!", "create_pid_directory"); }

    return $directory;
}

##############################################################
# Reading all files from a directory and its subdirectories
##############################################################

sub read_complete_directory
{
    my ($directory, $pathstring, $filecollector) = @_;

    my @content = ();
    opendir(DIR, $directory);
    @content = readdir(DIR);
    closedir(DIR);

    my $onefile;

    foreach $onefile (@content)
    {
        if ((!($onefile eq ".")) && (!($onefile eq "..")))
        {
            my $completefilename = $directory . $installer::globals::separator . $onefile;
            my $sep = "";
            if ( $pathstring ne "" ) { $sep = $installer::globals::separator; }

            if ( ! -d $completefilename )   # only files, no directories
            {
                my $content = $pathstring . $sep . $onefile;
                push(@{$filecollector}, $content);
            }
            else  # recursive for directories
            {
                my $newpathstring = $pathstring . $sep . $onefile;
                read_complete_directory($completefilename, $newpathstring, $filecollector);
            }
        }
    }
}

##############################################################
# Reading all files from a directory and its subdirectories
# Version 2
##############################################################

sub read_full_directory {
    my ( $currentdir, $pathstring, $collector ) = @_;
    my $item;
    my $fullname;
    local *DH;

    unless (opendir(DH, $currentdir))
    {
        return;
    }
    while (defined ($item = readdir(DH)))
    {
        next if($item eq "." or $item eq "..");
        $fullname = $currentdir . $installer::globals::separator . $item;
        my $sep = "";
        if ( $pathstring ne "" ) { $sep = $installer::globals::separator; }

        if( -d $fullname)
        {
            my $newpathstring = $pathstring . $sep . $item;
            read_full_directory($fullname, $newpathstring, $collector) if(-d $fullname);
        }
        else
        {
            my $content = $pathstring . $sep . $item;
            push(@{$collector}, $content);
        }
    }
    closedir(DH);
    return
}

##############################################################
# Removing all empty directories below a specified directory
##############################################################

sub remove_empty_dirs_in_folder
{
    my ( $dir ) = @_;

    my @content = ();
    my $infoline = "";

    $dir =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( -d $dir )
    {
        opendir(DIR, $dir);
        @content = readdir(DIR);
        closedir(DIR);

        my $oneitem;

        foreach $oneitem (@content)
        {
            if ((!($oneitem eq ".")) && (!($oneitem eq "..")))
            {
                my $item = $dir . $installer::globals::separator . $oneitem;

                if ( -d $item ) # recursive
                {
                    remove_empty_dirs_in_folder($item);
                }
            }
        }

        # try to remove empty directory
        $installer::logger::Info->printf("remove_empty_dirs_in_folder %s\n", $dir);
        my $returnvalue = rmdir $dir;

        if ( $returnvalue )
        {
            $installer::logger::Lang->printf("Successfully removed empty dir %s\n", $dir);
        }

    }

}

1;
