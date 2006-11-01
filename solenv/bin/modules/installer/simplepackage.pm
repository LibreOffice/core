#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: simplepackage.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: vg $ $Date: 2006-11-01 13:48:50 $
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

package installer::simplepackage;

use Cwd;
use installer::download;
use installer::globals;
use installer::logger;
use installer::strip;
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

    if (( $installer::globals::packageformat eq "installed" ) ||
        ( $installer::globals::packageformat eq "archive" ))
    {
        $installer::globals::is_simple_packager_project = 1;
    }
}

#############################################
# Creating the "simple" package.
# "zip" for Windows
# "tar.gz" for all other platforms
#############################################

sub create_package
{
    my ( $installdir, $packagename, $includepatharrayref ) = @_;

    # moving dir into temporary directory
    my $pid = $$; # process id
    my $tempdir = $installdir . "_temp" . "." . $pid;
    my $systemcall = "";
    my $from = "";
    my $return_to_start = 0;
    installer::systemactions::rename_directory($installdir, $tempdir);

    # creating new directory with original name
    installer::systemactions::create_directory($installdir);

    if ( $installer::globals::iswindowsbuild )  # Windows specific part
    {
        $fileextension = ".zip";
        my $zipfilename = $packagename . $fileextension;
        $zipfilename = $installdir . $installer::globals::separator . $zipfilename;

        $from = cwd();
        $return_to_start = 1;
        chdir($tempdir);
        $systemcall = "$installer::globals::zippath -qr $zipfilename .";
        # $systemcall = "$installer::globals::zippath -r $zipfilename .";
    }
    else    # Non-Windows specific part
    {
        $fileextension = ".tar.gz";
        my $targzname = $packagename . $fileextension;
        $targzname = $installdir . $installer::globals::separator . $targzname;

        # getting the path of the getuid.so (only required for Solaris and Linux)
        my $getuidlibrary = "";
        my $ldpreloadstring = "";
        if (( $installer::globals::issolarisbuild ) || ( $installer::globals::islinuxbuild ))
        {
            $getuidlibrary = installer::download::get_path_for_library($includepatharrayref);
            if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }
        }

        $systemcall = "cd $tempdir; $ldpreloadstring tar -cf - . | gzip > $targzname";
    }

    print "... $systemcall ...\n";
    my $returnvalue = system($systemcall);
    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
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
    my ( $filesref, $dirsref, $scpactionsref, $linksref, $loggingdir, $languagestringref, $shipinstalldir, $allsettingsarrayref, $allvariables, $includepatharrayref ) = @_;

    # Creating directories

    installer::logger::print_message( "... creating installation directory ...\n" );

    installer::logger::include_header_into_logfile("Creating installation directory");

    my $current_install_number = "";
    my $infoline = "";

    my $installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);

    my $installlogdir = installer::systemactions::create_directory_next_to_directory($installdir, "log");

    # Setting package name (similar to the download name)
    my $packagename = "";

    if ( $installer::globals::packageformat eq "archive" )
    {
        if ( $allvariables->{'OOODOWNLOADNAME'} )
        {
            $packagename = installer::download::set_download_filename($languagestringref, $allvariables);
        }
        else
        {
            $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "downloadname");
            $packagename = installer::download::resolve_variables_in_downloadname($allvariables, $$downloadname, $languagestringref);
        }
    }

    # Creating subfolder in installdir, which shall become the root of package or zip file
    my $subfolderdir = "";
    if ( $packagename ne "" ) { $subfolderdir = $installdir . $installer::globals::separator . $packagename; }
    else { $subfolderdir = $installdir; }

    if ( ! -d $subfolderdir ) { installer::systemactions::create_directory($subfolderdir); }

    # Create directories, copy files and ScpActions

    installer::logger::print_message( "... creating directories ...\n" );
    installer::logger::include_header_into_logfile("Creating directories:");

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];

        if ( $onedir->{'HostName'} )
        {
            my $destdir = $subfolderdir . $installer::globals::separator . $onedir->{'HostName'};
            if ( ! -d $destdir ) { installer::systemactions::create_directory_structure($destdir); }
        }
    }

    # stripping files ?!
    if (( $installer::globals::strip ) && ( ! $installer::globals::iswindowsbuild )) { installer::strip::strip_libraries($filesref, $languagestringref); }

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

        installer::systemactions::copy_one_file($source, $destination);

        if ( ! $installer::globals::iswindowsbuild )
        {
            my $unixrights = "";
            if ( $onefile->{'UnixRights'} )
            {
                $unixrights = $onefile->{'UnixRights'};
                my $localcall = "chmod $unixrights $destination \>\/dev\/null 2\>\&1";
                system($localcall);
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

        my $localcall = "ln -sf $destinationfile $destination \>\/dev\/null 2\>\&1";
        system($localcall);

        $infoline = "Creating link: \"ln -sf $destinationfile $destination\"\n";
        push(@installer::globals::logfileinfo, $infoline);
    }

    if ( $installer::globals::packageformat eq "archive" )
    {
        # creating a package
        # -> zip for Windows
        # -> tar.gz for all other platforms
        create_package($installdir, $packagename, $includepatharrayref);
    }

    # Analyzing the log file

    installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);

}

1;