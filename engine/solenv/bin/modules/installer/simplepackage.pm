#
# This file is part of the Collabora Office project.
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

package installer::simplepackage;

use strict;
use warnings;

use Cwd;
use File::Copy;
use installer::download;
use installer::exiter;
use installer::globals;
use installer::logger;
use installer::strip qw(strip_libraries);
use installer::systemactions;
use installer::worker;

####################################################
# Checking if the simple packager is required.
# This can be achieved by setting the global
# variable SIMPLE_PACKAGE in *.lst file or by
# setting the environment variable SIMPLE_PACKAGE.
####################################################

sub check_simple_packager_project
{
    my ( $allvariables ) = @_;

    if ( $installer::globals::packageformat eq "installed" )
    {
        $installer::globals::is_simple_packager_project = 1;
        $installer::globals::patch_user_dir = 1;
    }
    elsif ( $installer::globals::packageformat eq "archive" )
    {
        $installer::globals::is_simple_packager_project = 1;
    }
}

####################################################
# Detecting the directory with extensions
####################################################

sub get_extensions_dir
{
    my ( $subfolderdir ) = @_;

    my $extensiondir = $subfolderdir . $installer::globals::separator;
    if ( $installer::globals::officedirhostname ne "" ) { $extensiondir = $extensiondir . $installer::globals::officedirhostname . $installer::globals::separator; }
    my $extensionsdir = $extensiondir . "share" . $installer::globals::separator . "extensions";

    return $extensionsdir;
}

#############################################
# Creating the "simple" package.
# "zip" for Windows
# "tar.gz" for all other platforms
#############################################

sub create_package
{
    my ( $installdir, $archivedir, $packagename, $allvariables, $includepatharrayref, $languagestringref, $format ) = @_;

    installer::logger::print_message( "... creating $installer::globals::packageformat file ...\n" );
    installer::logger::include_header_into_logfile("Creating $installer::globals::packageformat file:");

    # moving dir into temporary directory
    my $pid = $$; # process id
    my $tempdir = $installdir . "_temp" . "." . $pid;
    my $systemcall = "";
    my $from = "";
    my $makesystemcall = 1;
    my $return_to_start = 0;
    installer::systemactions::rename_directory($installdir, $tempdir);

    # creating new directory with original name
    installer::systemactions::create_directory($archivedir);

    my $archive = $archivedir . $installer::globals::separator . $packagename . $format;

    if ( $archive =~ /zip$/ )
    {
        $from = cwd();
        $return_to_start = 1;
        chdir($tempdir);
        $systemcall = "$installer::globals::zippath -qr $archive .";

        # Using Archive::Zip fails because of very long path names below "share/uno_packages/cache"
        # my $packzip = Archive::Zip->new();
        # $packzip->addTree(".");   # after changing into $tempdir
        # $packzip->writeToFileNamed($archive);
        # $makesystemcall = 0;
    }
    else
    {
        # use fakeroot (only required for Linux)
        my $fakerootstring = "";
        if ( $installer::globals::islinuxbuild )
        {
            $fakerootstring = "fakeroot";
        }

        $systemcall = "cd $tempdir; $fakerootstring tar -cf - . | $installer::globals::packertool > $archive";
    }

    if ( $makesystemcall )
    {
        print "... $systemcall ...\n";
        my $systemcall_output = `$systemcall`;
        my $returnvalue = $? >> 8;
        my $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute \"$systemcall\" - exitcode: $returnvalue - output:\n$systemcall_output\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            $infoline = "Success: Executed \"$systemcall\" successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    if ( $return_to_start ) { chdir($from); }

    print "... removing $tempdir ...\n";
    installer::systemactions::remove_complete_directory($tempdir);
}

####################################################
# Main method for creating the simple package
# installation sets
####################################################

sub create_simple_package
{
    my ( $filesref, $dirsref, $linksref, $unixlinksref, $loggingdir, $languagestringref, $shipinstalldir, $allsettingsarrayref, $allvariables, $includepatharrayref ) = @_;

    # Creating directories

    my $current_install_number = "";
    my $infoline = "";

    installer::logger::print_message( "... creating installation directory ...\n" );
    installer::logger::include_header_into_logfile("Creating installation directory");

    $installer::globals::csp_installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);
    $installer::globals::csp_installlogdir = installer::systemactions::create_directory_next_to_directory($installer::globals::csp_installdir, "log");

    my $installdir = $installer::globals::csp_installdir;
    my $installlogdir = $installer::globals::csp_installlogdir;

    # Setting package name (similar to the download name)
    my $packagename = "";

    if ( $installer::globals::packageformat eq "archive" )
    {
        $installer::globals::csp_languagestring = $$languagestringref;

        my $locallanguage = $installer::globals::csp_languagestring;

        $packagename = installer::download::set_download_filename(\$locallanguage, $allvariables);
    }

    my $tempinstalldir = $installdir;

    # Creating subfolder in installdir, which shall become the root of package or zip file
    my $subfolderdir = "";
    if ( $packagename ne "" ) { $subfolderdir = $tempinstalldir . $installer::globals::separator . $packagename; }
    else { $subfolderdir = $tempinstalldir; }

    if ( ! -d $subfolderdir ) { installer::systemactions::create_directory($subfolderdir); }

    # Create directories and copy files

    installer::logger::print_message( "... creating directories ...\n" );
    installer::logger::include_header_into_logfile("Creating directories:");

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];

        if ( $onedir->{'HostName'} )
        {
            my $destdir = $subfolderdir . $installer::globals::separator . $onedir->{'HostName'};

            if ( ! -d $destdir )
            {
                if ( $^O =~ /cygwin/i ) # Cygwin performance check
                {
                    $infoline = "Try to create directory $destdir\n";
                    push(@installer::globals::logfileinfo, $infoline);
                    # Directories in $dirsref are sorted and all parents were added -> "mkdir" works without parent creation!
                    if ( ! ( -d $destdir )) { mkdir($destdir, 0775); }
                }
                else
                {
                    installer::systemactions::create_directory_structure($destdir);
                }
            }
        }
    }

    # stripping files ?!
    if ( $installer::globals::strip ) { strip_libraries($filesref, $languagestringref); }

    # copy Files
    installer::logger::print_message( "... copying files ...\n" );
    installer::logger::include_header_into_logfile("Copying files:");

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];

        if (( $onefile->{'Styles'} ) && ( $onefile->{'Styles'} =~ /\bBINARYTABLE_ONLY\b/ )) { next; }

        my $source = $onefile->{'sourcepath'};
        my $destination = $onefile->{'destination'};
        $destination = $subfolderdir . $installer::globals::separator . $destination;

        # Replacing $$ by $ is necessary to install files with $ in its name (back-masquerading)
        # Otherwise, the following shell command does not work and the file list is not correct
        $source =~ s/\$\$/\$/;
        $destination =~ s/\$\$/\$/;

        if ( $^O =~ /cygwin/i ) # Cygwin performance, do not use copy_one_file. "chmod -R" at the end
        {
            my $copyreturn = copy($source, $destination);

            if ($copyreturn)
            {
                $infoline = "Copy: $source to $destination\n";
            }
            else
            {
                $infoline = "ERROR: Could not copy $source to $destination $!\n";
            }

            push(@installer::globals::logfileinfo, $infoline);
        }
        else
        {
            installer::systemactions::copy_one_file($source, $destination);

            # see issue 102274
            if ( $onefile->{'UnixRights'} )
            {
                if ( ! -l $destination ) # that would be rather pointless
                {
                    chmod oct($onefile->{'UnixRights'}), $destination;
                }
            }
        }
    }

    # creating Links

    installer::logger::print_message( "... creating links ...\n" );
    installer::logger::include_header_into_logfile("Creating links:");

    for ( my $i = 0; $i <= $#{$linksref}; $i++ )
    {
        my $onelink = ${$linksref}[$i];

        my $destination = $onelink->{'destination'};
        $destination = $subfolderdir . $installer::globals::separator . $destination;
        my $destinationfile = $onelink->{'destinationfile'};

        my $localcall = "ln -sf \'$destinationfile\' \'$destination\' \>\/dev\/null 2\>\&1";
        system($localcall);

        $infoline = "Creating link: \"ln -sf $destinationfile $destination\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    for ( my $i = 0; $i <= $#{$unixlinksref}; $i++ )
    {
        my $onelink = ${$unixlinksref}[$i];

        my $target = $onelink->{'Target'};
        my $destination = $subfolderdir . $installer::globals::separator . $onelink->{'destination'};

        my @localcall = ('ln', '-sf', $target, $destination);
        system(@localcall) == 0 or die "system @localcall failed: $?";

        $infoline = "Creating Unix link: \"@localcall\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    # Setting privileges for cygwin globally

    if ( $^O =~ /cygwin/i )
    {
        installer::logger::print_message( "... changing privileges in $subfolderdir ...\n" );
        installer::logger::include_header_into_logfile("Changing privileges in $subfolderdir:");

        my $localcall = "chmod -R 755 " . "\"" . $subfolderdir . "\"";
        system($localcall);
    }

    installer::logger::print_message( "... removing superfluous directories ...\n" );
    installer::logger::include_header_into_logfile("Removing superfluous directories:");

    my $extensionfolder = get_extensions_dir($subfolderdir);
    installer::systemactions::remove_empty_dirs_in_folder($extensionfolder);

    if ( $installer::globals::ismacbuild )
    {
    }

    # Creating archive file
    if ( $installer::globals::packageformat eq "archive" )
    {
        create_package($tempinstalldir, $installdir, $packagename, $allvariables, $includepatharrayref, $languagestringref, $installer::globals::archiveformat);
    }

    # Analyzing the log file

    installer::worker::clean_output_tree(); # removing directories created in the output tree
    installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
}

1;

# vim: set shiftwidth=4 softtabstop=4 expandtab:
