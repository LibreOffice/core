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

package installer::download;

use strict;
use warnings;

use File::Spec;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::systemactions;

BEGIN { # This is needed so that cygwin's perl evaluates ACLs
    # (needed for correctly evaluating the -x test.)
    if( $^O =~ /cygwin/i ) {
        require filetest; import filetest "access";
    }
}

##################################################################
# Including the lowercase product name into the script template
##################################################################

sub put_productname_into_script
{
    my ($scriptfile, $variableshashref) = @_;

    my $productname = $variableshashref->{'PRODUCTNAME'};
    $productname = lc($productname);
    $productname =~ s/\.//g;    # openoffice.org -> openofficeorg
    $productname =~ s/\s*//g;

    my $infoline = "Adding productname $productname into download shell script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/PRODUCTNAMEPLACEHOLDER/$productname/;
    }
}

#########################################################
# Including the linenumber into the script template
#########################################################

sub put_linenumber_into_script
{
    my ( $scriptfile ) = @_;

    my $linenumber =  $#{$scriptfile} + 2;

    my $infoline = "Adding linenumber $linenumber into download shell script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/LINENUMBERPLACEHOLDER/$linenumber/;
    }
}

#########################################################
# Determining the name of the new scriptfile
#########################################################

sub determine_scriptfile_name
{
    my ( $filename ) = @_;

    $installer::globals::downloadfileextension = ".sh";
    $filename = $filename . $installer::globals::downloadfileextension;
    $installer::globals::downloadfilename = $filename;

    my $infoline = "Setting download shell script file name to $filename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $filename;
}

#########################################################
# Saving the script file in the installation directory
#########################################################

sub save_script_file
{
    my ($directory, $newscriptfilename, $scriptfile) = @_;

    $newscriptfilename = $directory . $installer::globals::separator . $newscriptfilename;
    installer::files::save_file($newscriptfilename, $scriptfile);

    my $infoline = "Saving script file $newscriptfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( ! $installer::globals::iswindowsbuild )
    {
        chmod 0775, $newscriptfilename;
    }

    return $newscriptfilename;
}

#########################################################
# Including checksum and size into script file
#########################################################

sub put_checksum_and_size_into_script
{
    my ($scriptfile, $sumout) = @_;

    my $checksum = "";
    my $size = "";

    if  ( $sumout =~ /^\s*(\d+)\s+(\d+)\s*$/ )
    {
        $checksum = $1;
        $size = $2;
    }
    else
    {
        installer::exiter::exit_program("ERROR: Incorrect return value from /usr/bin/sum: $sumout", "put_checksum_and_size_into_script");
    }

    my $infoline = "Adding checksum $checksum and size $size into download shell script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/CHECKSUMPLACEHOLDER/$checksum/;
        ${$scriptfile}[$i] =~ s/DISCSPACEPLACEHOLDER/$size/;
    }

}

#########################################################
# Determining checksum and size of tar file
#########################################################

sub call_sum
{
    my ($filename, $getuidlibrary) = @_;

    my $systemcall = "/usr/bin/sum $filename |";

    my $sumoutput = "";

    open (SUM, "$systemcall");
    $sumoutput = <SUM>;
    close (SUM);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

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

    $sumoutput =~ s/\s+$filename\s$//;
    return $sumoutput;
}

#########################################################
# Searching for the getuid.so in the solver
#########################################################

sub get_path_for_library
{
    my ($includepatharrayref) = @_;

    my $getuidlibraryname = "getuid.so";

    my $getuidlibraryref = "";

    if ( $installer::globals::include_paths_read )
    {
        $getuidlibraryref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$getuidlibraryname, $includepatharrayref, 0);
    }
    else
    {
        $getuidlibraryref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$getuidlibraryname, $includepatharrayref, 0);
    }

    if ($$getuidlibraryref eq "") { installer::exiter::exit_program("ERROR: Could not find $getuidlibraryname!", "get_path_for_library"); }

    return $$getuidlibraryref;
}

#########################################################
# Include the tar file into the script
#########################################################

sub include_tar_into_script
{
    my ($scriptfile, $temporary_tarfile) = @_;

    my $systemcall = "cat $temporary_tarfile >> $scriptfile && rm $temporary_tarfile";
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
    return $returnvalue;
}

#########################################################
# Create a tar file from the binary package
#########################################################

sub tar_package
{
    my ( $installdir, $tarfilename, $getuidlibrary) = @_;

    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    my $systemcall = "cd $installdir; $ldpreloadstring tar -cf - * > $tarfilename";

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

    chmod 0775, $tarfilename;

    return ( -s $tarfilename );
}

#########################################################
# Setting type of installation
#########################################################

sub get_installation_type
{
    my $type = "";

    if ( $installer::globals::languagepack ) { $type = "langpack"; }
    elsif ( $installer::globals::helppack ) { $type = "helppack"; }
    else { $type = "install"; }

    return $type;
}

#########################################################
# Setting installation languages
#########################################################

sub get_downloadname_language
{
    my ($languagestringref) = @_;

    my $languages = $$languagestringref;

    if ( $installer::globals::added_english )
    {
        $languages =~ s/en-US_//;
        $languages =~ s/_en-US//;
    }

    if ( length ($languages) > $installer::globals::max_lang_length )
    {
        $languages = 'multi';
    }

    return $languages;
}

#########################################################
# Setting download name
#########################################################

sub get_downloadname_productname
{
    my ($allvariables) = @_;

    my $start = "LibO";

    if ( $allvariables->{'PRODUCTNAME'} eq "LibreOffice" ) { $start = "LibO"; }

    if ( $allvariables->{'PRODUCTNAME'} eq "LOdev" ) { $start = "LibO-Dev"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "LibreOffice" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "SDK" )) { $start = "LibO-SDK"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "LOdev" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "SDK" )) { $start = "LibO-Dev-SDK"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "LibreOffice" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "TEST" )) { $start = "LibO-Test"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "LOdev" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "TEST" )) { $start = "LibO-Dev-Test"; }

    if ( $allvariables->{'PRODUCTNAME'} eq "URE" ) { $start = "LibO-URE"; }

    if ( $allvariables->{'PRODUCTNAME'} eq "OxygenOffice" ) { $start = "OOOP"; }



    return $start;
}

#########################################################
# Setting download version
#########################################################

sub get_download_version
{
    my ($allvariables) = @_;

    my $version = "";

    $version = $allvariables->{'PRODUCTVERSION'};
    if (( $allvariables->{'SHORT_PRODUCTEXTENSION'} ) && ( $allvariables->{'SHORT_PRODUCTEXTENSION'} ne "" )) { $version = $version . $allvariables->{'SHORT_PRODUCTEXTENSION'}; }

    return $version;
}

#################################################################
# Setting the platform name for download
#################################################################

sub get_download_platformname
{
    my $platformname = "";

    if ( $installer::globals::islinuxbuild )
    {
        $platformname = "Linux";
    }
    elsif ( $installer::globals::issolarisbuild )
    {
        $platformname = "Solaris";
    }
    elsif ( $installer::globals::iswindowsbuild )
    {
        $platformname = "Win";
    }
    elsif ( $installer::globals::isfreebsdbuild )
    {
        $platformname = "FreeBSD";
    }
    elsif ( $installer::globals::ismacbuild )
    {
        $platformname = "MacOS";
    }
    else
    {
        $platformname = $installer::globals::compiler;
    }

    return $platformname;
}

#########################################################
# Setting the architecture for the download name
#########################################################

sub get_download_architecture
{
    my $arch = "";

    if ( $installer::globals::compiler =~ /unxlngi/ )
    {
        $arch = "x86";
    }
    elsif ( $installer::globals::compiler =~ /unxlngppc/ )
    {
        $arch = "PPC";
    }
    elsif ( $installer::globals::compiler =~ /unxlngx/ )
    {
        $arch = "x86-64";
    }
    elsif ( $installer::globals::issolarissparcbuild )
    {
        $arch = "Sparc";
    }
    elsif ( $installer::globals::issolarisx86build )
    {
        $arch = "x86";
    }
    elsif ( $installer::globals::iswindowsbuild )
    {
        $arch = "x86";
    }
    elsif ( $installer::globals::compiler =~ /^unxmacxi/ )
    {
        $arch = "x86";
    }
    elsif ( $installer::globals::compiler =~ /^unxmacxp/ )
    {
        $arch = "PPC";
    }

    return $arch;
}

#########################################################
# Setting the installation type for the download name
#########################################################

sub get_install_type
{
    my ($allvariables) = @_;

    my $type = "";

    if ( $installer::globals::languagepack )
    {
        $type = "langpack";

        if ( $installer::globals::isrpmbuild )
        {
            $type = $type . "-rpm";
        }

        if ( $installer::globals::isdebbuild )
        {
            $type = $type . "-deb";
        }

        if ( $installer::globals::packageformat eq "archive" )
        {
            $type = $type . "-arc";
        }
    }
    elsif ( $installer::globals::helppack )
    {
        $type = "helppack";

        if ( $installer::globals::isrpmbuild )
        {
            $type = $type . "-rpm";
        }

        if ( $installer::globals::isdebbuild )
        {
            $type = $type . "-deb";
        }

        if ( $installer::globals::packageformat eq "archive" )
        {
            $type = $type . "-arc";
        }
    }
    else
    {
        $type = "install";

        if ( $installer::globals::isrpmbuild )
        {
            $type = $type . "-rpm";
        }

        if ( $installer::globals::isdebbuild )
        {
            $type = $type . "-deb";
        }

        if ( $installer::globals::packageformat eq "archive" )
        {
            $type = $type . "-arc";
        }

        if (( $allvariables->{'WITHJREPRODUCT'} ) && ( $allvariables->{'WITHJREPRODUCT'} == 1 ))
        {
            $type = $type . "-wJRE";
        }

    }

    return $type;
}

#########################################################
# Setting installation addons
#########################################################

sub get_downloadname_addon
{
    my $addon = "";

    if ( $installer::globals::isdebbuild ) { $addon = $addon . "_deb"; }

    if ( $installer::globals::product =~ /_wJRE\s*$/ ) { $addon = "_wJRE"; }

    return $addon;
}

#########################################################
# Looking for versionstring in version.info
# This has to be the only content of this file.
#########################################################

sub _get_versionstring
{
    my ( $versionfile ) = @_;

    my $versionstring = "";

    for ( @{$versionfile} )
    {
        next if /^\s*\#/; # comment line
        if ( /^\s*\"\s*(.*?)\s*\"\s*$/ )
        {
            $versionstring = $1;
            last;
        }
    }

    return $versionstring;
}

#########################################################
# Returning the current product version
# This has to be defined in file "version.info"
# in directory $installer::globals::ooouploaddir
#########################################################

sub get_current_version
{
    my $infoline = "";
    my $versionstring = "";
    my $filename = "version.info";

    if ( -f $filename )
    {
        $infoline = "File $filename exists. Trying to find current version.\n";
        push( @installer::globals::logfileinfo, $infoline);
        my $versionfile = installer::files::read_file($filename);
        $versionstring = _get_versionstring($versionfile);
        $infoline = "Setting version string: $versionstring\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "File $filename does not exist. No version setting in download file name.\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    $installer::globals::oooversionstring = $versionstring;

    return $versionstring;
}

###############################################################################################
# Setting the download file name
# Syntax:
# (PRODUCTNAME)_(VERSION)_(OS)_(ARCH)_(INSTALLTYPE)_(LANGUAGE).(FILEEXTENSION)
###############################################################################################

sub set_download_filename
{
    my ($languagestringref, $allvariables) = @_;

    my $start = get_downloadname_productname($allvariables);
    my $versionstring = get_download_version($allvariables);
    my $platform = get_download_platformname();
    my $architecture = get_download_architecture();
    my $type = get_install_type($allvariables);
    my $language = get_downloadname_language($languagestringref);

    # Setting the extension happens automatically

    my $filename = $start . "_" . $versionstring . "_" . "_" . $platform . "_" . $architecture . "_" . $type . "_" . $language;

    $filename =~ s/\_\_/\_/g;   # necessary, if $versionstring or $platform or $language are empty
    $filename =~ s/\_\s*$//;    # necessary, if $language and $addon are empty

    $installer::globals::ooodownloadfilename = $filename;

    return $filename;
}

#########################################################
# Creating a tar.gz file
#########################################################

sub create_tar_gz_file_from_directory
{
    my ($installdir, $getuidlibrary, $downloaddir, $downloadfilename) = @_;

    my $infoline = "";

    my $packdir = $installdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packdir);
    my $changedir = $installdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$changedir);

    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    $installer::globals::downloadfileextension = ".tar.gz";
    $installer::globals::downloadfilename = $downloadfilename . $installer::globals::downloadfileextension;
    my $targzname = $downloaddir . $installer::globals::separator . $installer::globals::downloadfilename;

    my $systemcall = "cd $changedir; $ldpreloadstring tar -cf - $packdir | gzip > $targzname";

    my $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
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

    return $targzname;
}

#########################################################
# Setting the variables in the download name
#########################################################

sub resolve_variables_in_downloadname
{
    my ($allvariables, $downloadname, $languagestringref) = @_;

    # Typical name: soa-{productversion}-{extension}-bin-{os}-{languages}

    my $productversion = "";
    if ( $allvariables->{'PRODUCTVERSION'} ) { $productversion = $allvariables->{'PRODUCTVERSION'}; }
    $downloadname =~ s/\{productversion\}/$productversion/;

    my $packageversion = "";
    if ( $allvariables->{'PACKAGEVERSION'} ) { $packageversion = $allvariables->{'PACKAGEVERSION'}; }
    $downloadname =~ s/\{packageversion\}/$packageversion/;

    my $extension = "";
    if ( $allvariables->{'SHORT_PRODUCTEXTENSION'} ) { $extension = $allvariables->{'SHORT_PRODUCTEXTENSION'}; }
    $extension = lc($extension);
    $downloadname =~ s/\{extension\}/$extension/;

    my $os = "";
    if ( $installer::globals::iswindowsbuild ) { $os = "windows"; }
    elsif ( $installer::globals::issolarissparcbuild ) { $os = "solsparc"; }
    elsif ( $installer::globals::issolarisx86build ) { $os = "solia"; }
    elsif ( $installer::globals::islinuxbuild ) { $os = "linux"; }
    elsif ( $installer::globals::compiler =~ /unxmacxi/ ) { $os = "macosxi"; }
    elsif ( $installer::globals::compiler =~ /unxmacxp/ ) { $os = "macosxp"; }
    else { $os = ""; }
    $downloadname =~ s/\{os\}/$os/;

    my $languages = $$languagestringref;
    $downloadname =~ s/\{languages\}/$languages/;

    $downloadname =~ s/\-\-\-/\-/g;
    $downloadname =~ s/\-\-/\-/g;
    $downloadname =~ s/\-\s*$//;

    return $downloadname;
}

##############################################################
# Returning the complete block in all languages
# for a specified string
##############################################################

sub get_language_block_from_language_file
{
    my ($searchstring, $languagefile) = @_;

    my @language_block = ();

    for ( my $i = 0; $i <= $#{$languagefile}; $i++ )
    {
        if ( ${$languagefile}[$i] =~ /^\s*\[\s*$searchstring\s*\]\s*$/ )
        {
            my $counter = $i;

            push(@language_block, ${$languagefile}[$counter]);
            $counter++;

            while (( $counter <= $#{$languagefile} ) && (!( ${$languagefile}[$counter] =~ /^\s*\[/ )))
            {
                push(@language_block, ${$languagefile}[$counter]);
                $counter++;
            }

            last;
        }
    }

    return \@language_block;
}

##############################################################
# Returning a specific language string from the block
# of all translations
##############################################################

sub get_language_string_from_language_block
{
    my ($language_block, $language) = @_;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$language_block}; $i++ )
    {
        if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
        {
            $newstring = $1;
            last;
        }
    }

    if ( $newstring eq "" )
    {
        $language = "en-US";    # defaulting to english

        for ( my $i = 0; $i <= $#{$language_block}; $i++ )
        {
            if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
            {
                $newstring = $1;
                last;
            }
        }
    }

    return $newstring;
}

#######################################################
# Setting supported platform for LibreOffice
# builds
#######################################################

sub is_supported_platform
{
    my $is_supported = 0;

    if (( $installer::globals::isrpmbuild ) ||
        ( $installer::globals::issolarissparcbuild ) ||
        ( $installer::globals::issolarisx86build ) ||
        ( $installer::globals::iswindowsbuild ))
    {
        $is_supported = 1;
    }

    return $is_supported;
}

####################################################
# Creating download installation sets
####################################################

sub create_download_sets
{
    my ($installationdir, $includepatharrayref, $allvariableshashref, $downloadname, $languagestringref, $languagesarrayref) = @_;

    my $infoline = "";

    my $force = 1; # print this message even in 'quiet' mode
    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating download installation set ...\n", $force );
    installer::logger::print_message( "******************************************\n" );

    installer::logger::include_header_into_logfile("Creating download installation sets:");

    my $firstdir = $installationdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$firstdir);

    my $lastdir = $installationdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$lastdir);

    if ( $installer::globals::iswindowsbuild && $lastdir =~ /\./ ) { $lastdir =~ s/\./_download_inprogress\./ }
    else { $lastdir = $lastdir . "_download_inprogress"; }

    # removing existing directory "_native_packed_inprogress" and "_native_packed_witherror" and "_native_packed"

    my $downloaddir = $firstdir . $lastdir;

    if ( -d $downloaddir ) { installer::systemactions::remove_complete_directory($downloaddir); }

    my $olddir = $downloaddir;
    $olddir =~ s/_inprogress/_witherror/;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    $olddir = $downloaddir;
    $olddir =~ s/_inprogress//;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    # creating the new directory

    installer::systemactions::create_directory($downloaddir);

    $installer::globals::saveinstalldir = $downloaddir;

    # evaluating the name of the download file

    if ( $allvariableshashref->{'OOODOWNLOADNAME'} ) { $downloadname = set_download_filename($languagestringref, $allvariableshashref); }
    else { $downloadname = resolve_variables_in_downloadname($allvariableshashref, $downloadname, $languagestringref); }

    if ( ! $installer::globals::iswindowsbuild )    # Unix specific part
    {

        # getting the path of the getuid.so (only required for Solaris and Linux)
        my $getuidlibrary = "";
        if (( $installer::globals::issolarisbuild ) || ( $installer::globals::islinuxbuild )) { $getuidlibrary = get_path_for_library($includepatharrayref); }

        if ( $allvariableshashref->{'OOODOWNLOADNAME'} )
        {
            my $downloadfile = create_tar_gz_file_from_directory($installationdir, $getuidlibrary, $downloaddir, $downloadname);
        }
        else
        {
            # find and read setup script template
            my $scriptfilename = "downloadscript.sh";

            my $scriptref = "";

            if ( $installer::globals::include_paths_read )
            {
                $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptfilename, $includepatharrayref, 0);
            }
            else
            {
                $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$scriptfilename, $includepatharrayref, 0);
            }

            if ($$scriptref eq "") { installer::exiter::exit_program("ERROR: Could not find script file $scriptfilename!", "create_download_sets"); }
            my $scriptfile = installer::files::read_file($$scriptref);

            $infoline = "Found  script file $scriptfilename: $$scriptref \n";
            push( @installer::globals::logfileinfo, $infoline);

            # add product name into script template
            put_productname_into_script($scriptfile, $allvariableshashref);

            # replace linenumber in script template
            put_linenumber_into_script($scriptfile);

            # create tar file
            my $temporary_tarfile_name = $downloaddir . $installer::globals::separator . 'installset.tar';
            my $size = tar_package($installationdir, $temporary_tarfile_name, $getuidlibrary);
            installer::exiter::exit_program("ERROR: Could not create tar file $temporary_tarfile_name!", "create_download_sets") unless $size;

            # calling sum to determine checksum and size of the tar file
            my $sumout = call_sum($temporary_tarfile_name);

            # writing checksum and size into scriptfile
            put_checksum_and_size_into_script($scriptfile, $sumout);

            # saving the script file
            my $newscriptfilename = determine_scriptfile_name($downloadname);
            $newscriptfilename = save_script_file($downloaddir, $newscriptfilename, $scriptfile);

            installer::logger::print_message( "... including installation set into $newscriptfilename ... \n" );
            # Append tar file to script
            include_tar_into_script($newscriptfilename, $temporary_tarfile_name);
        }
    }

    return $downloaddir;
}

1;
