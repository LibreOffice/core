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

package installer::archivefiles;

use Archive::Zip qw( :ERROR_CODES :CONSTANTS );
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;
use Cwd;

#################################################################
# Changing the name for files with flag RENAME_TO_LANGUAGE
#################################################################

sub put_language_into_name
{
    my ( $oldname, $onelanguage ) = @_;

    my $newname = "";

    my $filename = "";
    my $extension = "";

    if ( $oldname =~ /en-US/ )  # files, that contain the language in the file name
    {
        $newname = $oldname;
        $newname =~ s/en-US/$onelanguage/;
    }
    else    # files, that do not contain the language in the file name
    {
        if ( $oldname =~ /^\s*(.*)(\..*?)\s*$/ )    # files with extension
        {
            $filename = $1;
            $extension = $2;
        }
        else
        {
            $filename = $oldname;
            $extension = "";
        }

        $newname = $1 . "_" . $onelanguage . $2;
    }

    return $newname;
}

#################################################################
# Converting patchfiles string into array
#################################################################

sub get_patch_file_list
{
    my ( $patchfilestring ) = @_;

    $patchfilestring =~ s/^\s*\(?//;
    $patchfilestring =~ s/\)?\s*$//;
    $patchfilestring =~ s/^\s*\///;
    $patchfilestring =~ s/^\s*\\//;

    my @patchfilesarray = split /,\s*/, $patchfilestring;

    return \@patchfilesarray;
}

#################################################################
# Reading all executables in the "manifest.xml"
#################################################################

sub get_all_executables_from_manifest
{
    my ($unzipdir, $manifestfile, $executable_files_in_extensions) = @_;

    my $is_executable = 0;

    for ( my $i = 0; $i <= $#{$manifestfile}; $i++ )
    {
        my $line = ${$manifestfile}[$i];

        if ( $line =~ /\"application\/vnd\.sun\.star\.executable\"/ ) { $is_executable = 1; }

        if (( $line =~ /manifest\:full\-path=\"(.*?)\"/ ) && ( $is_executable ))
        {
            my $filename = $unzipdir . $installer::globals::separator . $1;
            # making only slashes for comparison reasons
            $filename =~ s/\\/\//g;
            $executable_files_in_extensions->{$filename} = 1;
        }

        if ( $line =~ /\/\>/ ) { $is_executable = 0; }
    }
}

#################################################################
# Reading the "manifest.xml" in extensions and determine, if
# there are executable files
#################################################################

sub collect_all_executable_files_in_extensions
{
    my ($unzipdir, $executable_files_in_extensions) = @_;

    $unzipdir =~ s/\Q$installer::globals::separator\E\s*$//;

    my $manifestfilename = $unzipdir . $installer::globals::separator . "META-INF" . $installer::globals::separator . "manifest.xml";

    if ( -f $manifestfilename )
    {
        my $manifestfile = installer::files::read_file($manifestfilename);
        get_all_executables_from_manifest($unzipdir, $manifestfile, $executable_files_in_extensions);
    }
}

#################################################################
# Analyzing files with flag ARCHIVE
#################################################################

sub resolving_archive_flag
{
    my ($filesarrayref, $additionalpathsref, $languagestringref, $loggingdir) = @_;

    my @newallfilesarray = ();

    my ($systemcall, $returnvalue, $infoline);

    my $unziplistfile = $loggingdir . "unziplist_" . $installer::globals::build . "_" . $installer::globals::compiler . "_" . $$languagestringref . ".txt";

    my $platformunzipdirbase = installer::systemactions::create_directories("zip", $languagestringref);
    push(@installer::globals::removedirs, $platformunzipdirbase);

    installer::logger::include_header_into_logfile("Files with flag ARCHIVE:");

    my $repeat_unzip = 0;
    my $maxcounter = 0;

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        if ( $repeat_unzip ) { $i--; }  # decreasing the counter

        my $onefile = ${$filesarrayref}[$i];
        my $styles = "";

        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( $styles =~ /\bARCHIVE\b/ )     # copying, unzipping and changing the file list
        {
            my $iscommonfile = 0;
            my $sourcepath = $onefile->{'sourcepath'};

            if ( $sourcepath =~ /\Q$installer::globals::separator\E\bcommon$installer::globals::productextension\Q$installer::globals::separator\E/ )   # /common/ or /common.pro/
            {
                $iscommonfile = 1;
            }

            my $use_internal_rights = 0;
            if ( $styles =~ /\bUSE_INTERNAL_RIGHTS\b/ ) { $use_internal_rights = 1; }   # using the rights used inside the zip file

            my $rename_to_language = 0;
            if ( $styles =~ /\bRENAME_TO_LANGUAGE\b/ ) { $rename_to_language = 1; } # special handling for renamed files (scriptitems.pm)

            my %executable_files_in_extensions = ();
            my $set_executable_privileges = 0;  # setting privileges for exectables is required for oxt files
            if ( $onefile->{'Name'} =~ /\.oxt\s*$/ ) { $set_executable_privileges = 1; }

            # mechanism to select files from an archive files
            my $select_files = 0;
            my $selectlistfiles = "";
            my @keptfiles = ();
            if ( $onefile->{'Selectfiles'} )
            {
                $select_files = 1;
                $selectlistfiles = get_patch_file_list( $onefile->{'Selectfiles'} );
                $infoline = "Selected file list defined at file: $onefile->{'Name'} :\n";
                push( @installer::globals::logfileinfo, $infoline);
                for ( my $k = 0; $k <= $#{$selectlistfiles}; $k++ )
                {
                    $infoline = "\"${$selectlistfiles}[$k]\"\n";
                    push( @installer::globals::logfileinfo, $infoline);
                }
            }

            if ( $onefile->{'Selectfiles'} ) { $onefile->{'Selectfiles'} = ""; } # Selected files list no longer required

            # creating directories

            my $onelanguage = $onefile->{'specificlanguage'};

            # files without language into directory "00"

            if ($onelanguage eq "") { $onelanguage = "00"; }

            my $unzipdir;

            $unzipdir = $platformunzipdirbase . $installer::globals::separator . $onelanguage . $installer::globals::separator;

            installer::systemactions::create_directory($unzipdir);  # creating language specific subdirectories

            my $onefilename = $onefile->{'Name'};
            $onefilename =~ s/\./\_/g;      # creating new directory name
            $onefilename =~ s/\//\_/g;      # only because of /letter/fontunxpsprint.zip, the only zip file with path
            $unzipdir = $unzipdir . $onefilename . $installer::globals::separator;

            if ( $installer::globals::dounzip ) { installer::systemactions::create_directory($unzipdir); }  # creating subdirectories with the names of the zipfiles

            my $zip = Archive::Zip->new();
            if ( $zip->read($sourcepath) != AZ_OK )
            {
                $infoline = "ERROR: Could not unzip $sourcepath\n";
                push( @installer::globals::logfileinfo, $infoline);
            }

            my $counter = 0;
            my $contains_dll = 0;
            my @dllList = ();
            my @dirs = ();
            foreach my $member ( $zip->memberNames() )
            {
                $counter++;
                if ( $member =~ /.dll\s*$/i ) { $contains_dll = 1; push(@dllList, $member); }
                if ( $member =~ m/\/$/ ) { push(@dirs, $member); }
            }

            if (! ( $counter > 0 )) # the zipfile is empty
            {
                $infoline = "ERROR: Could not unzip $sourcepath\n";
                push( @installer::globals::logfileinfo, $infoline);

            }
            else
            {
                if ( $installer::globals::dounzip )         # really unpacking the files
                {
                    if ( $zip->extractTree("", $unzipdir) != AZ_OK ) { die "Could not unzip: $!"; }

                    if (( $^O =~ /cygwin/i ) && ( $contains_dll ))
                    {
                        my $dir = getcwd();
                        chdir($unzipdir);
                        my $changed = chmod(0775, @dllList);
                        $infoline = "Changed mode of $changed files (of ".scalar(@dllList).")\n";
                        push( @installer::globals::logfileinfo, $infoline);
                        chdir($dir);

                        if ($changed != scalar(@dllList))
                        {
                            $infoline = "ERROR: Could not chmod all files!\n";
                            push( @installer::globals::logfileinfo, $infoline);
                        }
                    }

                    if ( ! $installer::globals::iswindowsbuild && scalar(@dirs) > 0 )
                    {
                        my $dir = getcwd();
                        chdir($unzipdir);
                        # Setting unix rights to "775" for all created directories inside the package
                        my $changed = chmod(0775, @dirs);
                        $infoline = "Changed mode of : $changed; should be: ".scalar(@dirs)."\n";
                        chdir($dir);

                        push( @installer::globals::logfileinfo, $infoline);

                        if ($changed != scalar(@dirs))
                        {
                            $infoline = "ERROR: Could not chmod all files!\n";
                            push( @installer::globals::logfileinfo, $infoline);
                        }
                    }

                    # Selecting names of executable files in extensions
                    if ( $set_executable_privileges )
                    {
                        collect_all_executable_files_in_extensions($unzipdir, \%executable_files_in_extensions);
                    }
                }

                my $unziperror = 0;

                foreach my $zipname ( $zip->memberNames() )
                {
                    # Format from Archive:::Zip :
                    # dir1/
                    # dir1/so7drawing.desktop

                    # some directories and files (from the help) start with "./simpress.idx"

                    $zipname =~ s/^\s*\.\///;

                    if ($installer::globals::iswin and $^O =~ /MSWin/i) { $zipname =~ s/\//\\/g; }

                    if ( $zipname =~ /\Q$installer::globals::separator\E\s*$/ ) # slash or backslash at the end characterizes a directory
                    {
                        $zipname = $zipname . "\n";
                        push(@{$additionalpathsref}, $zipname);

                        # Also needed here:
                        # Name
                        # Language
                        # ismultilingual
                        # Basedirectory

                        # This is not needed, because the list of all directories for the
                        # epm list file is generated from the destination directories of the
                        # files included in the product!
                    }
                    else
                    {
                        my %newfile = ();
                        %newfile = %{$onefile};
                        $newfile{'Name'} = $zipname;
                        my $destination = $onefile->{'destination'};
                        installer::pathanalyzer::get_path_from_fullqualifiedname(\$destination);
                        $newfile{'destination'} = $destination . $zipname;
                        $newfile{'sourcepath'} = $unzipdir . $zipname;
                        $newfile{'zipfilename'} = $onefile->{'Name'};
                        $newfile{'zipfilesource'} = $onefile->{'sourcepath'};
                        $newfile{'zipfiledestination'} = $onefile->{'destination'};

                        if (( $use_internal_rights ) && ( ! $installer::globals::iswin ))
                        {
                            $newfile{'UnixRights'} = sprintf("%o", ($zip->memberNamed($zipname)->unixFileAttributes() & 07777));
                            $infoline = "Setting unix rights for \"$newfile{'sourcepath'}\" to \"$newfile{'UnixRights'}\"\n";
                            push( @installer::globals::logfileinfo, $infoline);
                        }

                        if ( $set_executable_privileges )
                        {
                            # All paths to executables are saved in the hash %executable_files_in_extensions
                            my $compare_path = $newfile{'sourcepath'};
                            $compare_path =~ s/\\/\//g;  # contains only slashes for comparison reasons
                            if ( exists($executable_files_in_extensions{$compare_path}) )
                            {
                                $newfile{'UnixRights'} = "775";
                                $infoline = "Executable in Extension: Setting unix rights for \"$newfile{'sourcepath'}\" to \"$newfile{'UnixRights'}\"\n";
                                push( @installer::globals::logfileinfo, $infoline);
                            }
                        }

                        if ( $select_files )
                        {
                            if ( ! grep {$_ eq $zipname} @{$selectlistfiles} )
                            {
                                $infoline = "Removing from ARCHIVE file $onefilename: $zipname\n";
                                push( @installer::globals::logfileinfo, $infoline);
                                next; # ignoring files, that are not included in $selectlistfiles
                            }
                            else
                            {
                                $infoline = "Keeping from ARCHIVE file $onefilename: $zipname\n";
                                push( @installer::globals::logfileinfo, $infoline);
                                push( @keptfiles, $zipname); # collecting all kept files
                            }
                        }

                        if ( $rename_to_language )
                        {
                            my $newzipname = put_language_into_name($zipname, $onelanguage);
                            my $oldfilename = $unzipdir . $zipname;
                            my $newfilename = $unzipdir . $newzipname;

                            installer::systemactions::copy_one_file($oldfilename, $newfilename);

                            $newfile{'Name'} = $newzipname;
                            $newfile{'destination'} = $destination . $newzipname;
                            $newfile{'sourcepath'} = $unzipdir . $newzipname;

                            $infoline = "RENAME_TO_LANGUAGE: Using $newzipname instead of $zipname!\n";
                            push( @installer::globals::logfileinfo, $infoline);
                        }

                        my $sourcefiletest = $unzipdir . $zipname;
                        if ( ! -f $sourcefiletest )
                        {
                            $infoline = "ATTENTION: Unzip failed for $sourcefiletest!\n";
                            push( @installer::globals::logfileinfo, $infoline);
                            $unziperror = 1;
                        }

                        # only adding the new line into the files array, if not in repeat modus

                        if ( ! $repeat_unzip ) { push(@newallfilesarray, \%newfile); }
                    }
                }

                # Comparing the content of @keptfiles and $selectlistfiles
                # Do all files from the list of selected files are stored in @keptfiles ?
                # @keptfiles contains only files included in $selectlistfiles. But are all
                # files from $selectlistfiles included in @keptfiles?

                if ( $select_files )
                {
                    my $number = $#{$selectlistfiles} + 1;
                    $infoline = "SELECTLIST: Number of files in file selection list: $number\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $number = $#keptfiles + 1;
                    $infoline = "SELECTLIST: Number of kept files: $number\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    for ( my $k = 0; $k <= $#keptfiles; $k++ )
                    {
                        $infoline = "KEPT FILES: $keptfiles[$k]\n";
                        push( @installer::globals::logfileinfo, $infoline);
                    }

                    my @warningfiles = ();

                    for ( my $k = 0; $k <= $#{$selectlistfiles}; $k++ )
                    {
                        if ( ! grep {$_ eq ${$selectlistfiles}[$k]} @keptfiles )
                        {
                            push(@warningfiles, ${$selectlistfiles}[$k]);
                        }
                    }

                    for ( my $k = 0; $k <= $#warningfiles; $k++ )
                    {
                        $infoline = "WARNING: $warningfiles[$k] not included in install set (does not exist in zip file)!\n";
                        push( @installer::globals::logfileinfo, $infoline);
                    }

                }

                if ( $unziperror )
                {
                    installer::logger::print_warning( "Repeating to unpack $sourcepath! \n" );
                    $infoline = "ATTENTION: Repeating to unpack $sourcepath !\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $repeat_unzip = 1;
                    $maxcounter++;

                    if ( $maxcounter == 5 ) # exiting the program
                    {
                        die "Failed to unzip $sourcepath !";
                    }
                }
                else
                {
                    $infoline = "Info: $sourcepath unpacked without problems !\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $repeat_unzip = 0;
                    $maxcounter = 0;
                }
            }
        }
        else        # nothing to do here, no zipped file (no ARCHIVE flag)
        {
            push(@newallfilesarray, $onefile);
        }
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

    return \@newallfilesarray;
}


1;
