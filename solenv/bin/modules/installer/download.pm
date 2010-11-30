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
# Calling md5sum
#########################################################

sub call_md5sum
{
    my ($filename) = @_;

    $md5sumfile = "/usr/bin/md5sum";

    if ( ! -f $md5sumfile ) { installer::exiter::exit_program("ERROR: No file /usr/bin/md5sum", "call_md5sum"); }

    my $systemcall = "$md5sumfile $filename |";

    my $md5sumoutput = "";

    open (SUM, "$systemcall");
    $md5sumoutput = <SUM>;
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

    return $md5sumoutput;
}

#########################################################
# Calling md5sum
#########################################################

sub get_md5sum
{
    ($md5sumoutput) = @_;

    my $md5sum;

    if  ( $md5sumoutput =~ /^\s*(\w+?)\s+/ )
    {
        $md5sum = $1;
    }
    else
    {
        installer::exiter::exit_program("ERROR: Incorrect return value from /usr/bin/md5sum: $md5sumoutput", "get_md5sum");
    }

    my $infoline = "Setting md5sum: $md5sum\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $md5sum;
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

    if ( $installer::globals::include_pathes_read )
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
# Creating a tar.gz file
#########################################################

sub create_tar_gz_file_from_package
{
    my ($installdir, $getuidlibrary) = @_;

    my $infoline = "";
    my $alldirs = installer::systemactions::get_all_directories($installdir);
    my $onedir = ${$alldirs}[0];
    $installdir = $onedir;

    my $allfiles = installer::systemactions::get_all_files_from_one_directory($installdir);

    for ( my $i = 0; $i <= $#{$allfiles}; $i++ )
    {
        my $onefile = ${$allfiles}[$i];
        my $systemcall = "cd $installdir; rm $onefile";
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
    }

    $alldirs = installer::systemactions::get_all_directories($installdir);
    $packagename = ${$alldirs}[0]; # only taking the first Solaris package
    if ( $packagename eq "" ) { installer::exiter::exit_program("ERROR: Could not find package in directory $installdir!", "determine_packagename"); }

    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packagename);

    $installer::globals::downloadfileextension = ".tar.gz";
    my $targzname = $packagename . $installer::globals::downloadfileextension;
    $installer::globals::downloadfilename = $targzname;
    my $ldpreloadstring = "";
    if ( $getuidlibrary ne "" ) { $ldpreloadstring = "LD_PRELOAD=" . $getuidlibrary; }

    $systemcall = "cd $installdir; $ldpreloadstring tar -cf - $packagename | gzip > $targzname";
    print "... $systemcall ...\n";

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
}

#########################################################
# Setting type of installation
#########################################################

sub get_installation_type
{
    my $type = "";

    if ( $installer::globals::languagepack ) { $type = "langpack"; }
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

    if ( $allvariables->{'PRODUCTNAME'} eq "LibO-dev" ) { $start = "LibO-Dev"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "LibreOffice" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "SDK" )) { $start = "LibO-SDK"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "LibO-dev" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "SDK" )) { $start = "LibO-Dev-SDK"; }

    if ( $allvariables->{'PRODUCTNAME'} eq "URE" ) { $start = "LibO-URE"; }

    if ( $allvariables->{'PRODUCTNAME'} eq "BrOffice.org" ) { $start = "BrOo"; }

    if ( $allvariables->{'PRODUCTNAME'} eq "BrOo-dev" ) { $start = "BrOo-Dev"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "BrOffice.org" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "SDK" )) { $start = "BrOo-SDK"; }

    if (( $allvariables->{'PRODUCTNAME'} eq "BrOo-dev" ) && ( $allvariables->{'POSTVERSIONEXTENSION'} eq "SDK" )) { $start = "BrOo-Dev-SDK"; }

    return $start;
}

#########################################################
# Setting download version
#########################################################

sub get_download_version
{
    my ($allvariables) = @_;

    my $version = "";

    my $devproduct = 0;
    if (( $allvariables->{'DEVELOPMENTPRODUCT'} ) && ( $allvariables->{'DEVELOPMENTPRODUCT'} == 1 )) { $devproduct = 1; }

    my $cwsproduct = 0;
    # the environment variable CWS_WORK_STAMP is set only in CWS
    if ( $ENV{'CWS_WORK_STAMP'} ) { $cwsproduct = 1; }

    if (( $cwsproduct ) || ( $devproduct ))  # use "DEV300m75"
    {
        my $source = uc($installer::globals::build); # DEV300
        my $localminor = "";
        if ( $installer::globals::minor ne "" ) { $localminor = $installer::globals::minor; }
        else { $localminor = $installer::globals::lastminor; }
        $version = $source . $localminor;
    }
    else  # use 3.2.0rc1
    {
        $version = $allvariables->{'PRODUCTVERSION'};
        if (( $allvariables->{'ABOUTBOXPRODUCTVERSION'} ) && ( $allvariables->{'ABOUTBOXPRODUCTVERSION'} ne "" )) { $version = $allvariables->{'ABOUTBOXPRODUCTVERSION'}; }
        if (( $allvariables->{'SHORT_PRODUCTEXTENSION'} ) && ( $allvariables->{'SHORT_PRODUCTEXTENSION'} ne "" )) { $version = $version . $allvariables->{'SHORT_PRODUCTEXTENSION'}; }
    }

    return $version;
}

###############################################################
# Set date string, format: yymmdd
###############################################################

sub set_date_string
{
    my ($allvariables) = @_;

    my $datestring = "";

    my $devproduct = 0;
    if (( $allvariables->{'DEVELOPMENTPRODUCT'} ) && ( $allvariables->{'DEVELOPMENTPRODUCT'} == 1 )) { $devproduct = 1; }

    my $cwsproduct = 0;
    # the environment variable CWS_WORK_STAMP is set only in CWS
    if ( $ENV{'CWS_WORK_STAMP'} ) { $cwsproduct = 1; }

    my $releasebuild = 1;
    if (( $allvariables->{'SHORT_PRODUCTEXTENSION'} ) && ( $allvariables->{'SHORT_PRODUCTEXTENSION'} ne "" )) { $releasebuild = 0; }

    if (( ! $devproduct ) && ( ! $cwsproduct ) && ( ! $releasebuild ))
    {
        my @timearray = localtime(time);

        my $day = $timearray[3];
        my $month = $timearray[4] + 1;
        my $year = $timearray[5] + 1900;

        if ( $month < 10 ) { $month = "0" . $month; }
        if ( $day < 10 ) { $day = "0" . $day; }

        $datestring = $year . $month . $day;
    }

    return $datestring;
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

sub get_versionstring
{
    my ( $versionfile ) = @_;

    my $versionstring = "";

    for ( my $i = 0; $i <= $#{$versionfile}; $i++ )
    {
        my $oneline = ${$versionfile}[$i];

        if ( $oneline =~ /^\s*\#/ ) { next; } # comment line
        if ( $oneline =~ /^\s*\"\s*(.*?)\s*\"\s*$/ )
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
        $versionstring = get_versionstring($versionfile);
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

    $systemcall = "cd $changedir; $ldpreloadstring tar -cf - $packdir | gzip > $targzname";

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

    my $ppackageversion = "";
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

##################################################################
# Windows: Replacing one placeholder with the specified value
##################################################################

sub replace_one_variable
{
    my ($templatefile, $placeholder, $value) = @_;

    my $infoline = "Replacing $placeholder by $value in nsi file\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        ${$templatefile}[$i] =~ s/$placeholder/$value/g;
    }

}

########################################################################################
# Converting a string to a unicode string
########################################################################################

sub convert_to_unicode
{
    my ($string) = @_;

    my $unicodestring = "";

    my $stringlength = length($string);

    for ( my $i = 0; $i < $stringlength; $i++ )
    {
        $unicodestring = $unicodestring . substr($string, $i, 1);
        $unicodestring = $unicodestring . chr(0);
    }

    return $unicodestring;
}

##################################################################
# Windows: Setting nsis version is necessary because of small
# changes in nsis from version 2.0.4 to 2.3.1
##################################################################

sub set_nsis_version
{
    my ($nshfile) = @_;

    my $searchstring = "\$\{LangFileString\}"; # occurs only in nsis 2.3.1 or similar

    for ( my $i = 0; $i <= $#{$nshfile}; $i++ )
    {
        if ( ${$nshfile}[$i] =~ /\Q$searchstring\E/ )
        {
            # this is nsis 2.3.1 or similar
            $installer::globals::nsis231 = 1;
            $installer::globals::unicodensis = 0;
            last;
        }
    }

    # checking unicode version
    $searchstring = convert_to_unicode($searchstring);

    for ( my $i = 0; $i <= $#{$nshfile}; $i++ )
    {
        if ( ${$nshfile}[$i] =~ /\Q$searchstring\E/ )
        {
            # this is nsis 2.3.1 or similar
            $installer::globals::nsis231 = 1;
            $installer::globals::unicodensis = 1;
            last;
        }
    }

    if ( ! $installer::globals::nsis231 ) { $installer::globals::nsis204 = 1; }
}

##################################################################
# Windows: Including the product name into nsi template
##################################################################

sub put_windows_productname_into_template
{
    my ($templatefile, $variableshashref) = @_;

    my $productname = $variableshashref->{'PRODUCTNAME'};
    $productname =~ s/\.//g;    # OpenOffice.org -> OpenOfficeorg

    replace_one_variable($templatefile, "PRODUCTNAMEPLACEHOLDER", $productname);
}

##################################################################
# Windows: Substituting the path to a file into the nsi template
##################################################################

sub substitute_path_into_template
{
    my ($templatefile, $includepatharrayref, $allvariables, $var, $subst) = @_;

    if ( ! $allvariables->{$var} ) { $var =~ s/_.*$//; } # _BR suffix ?
    if ( ! $allvariables->{$var} ) { installer::exiter::exit_program("ERROR: $var not defined in product definition!", "substitute_path_into_template"); }
    my $filename = $allvariables->{$var};

    my $completefilenameref = "";

    if ( $installer::globals::include_pathes_read )
    {
        $completefilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 0);
    }
    else
    {
        $completefilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$filename, $includepatharrayref, 0);
    }

    if ($$completefilenameref eq "") { installer::exiter::exit_program("ERROR: Could not find download file $filename!", "substitute_path_into_template"); }

    if ( $^O =~ /cygwin/i ) { $$completefilenameref =~ s/\//\\/g; }

    replace_one_variable($templatefile, $subst, $$completefilenameref);
}

##################################################################
# Windows: substitute a variable into the nsi template
##################################################################
sub substitute_variable_into_template($$$$)
{
    my ($templatefile, $variableshashref, $varname, $subst) = @_;

    my $var = "";
    if ( $variableshashref->{$varname} ) { $var = $variableshashref->{$varname}; }

    replace_one_variable($templatefile, $subst, $var);
}

##################################################################
# Windows: Including the product version into nsi template
##################################################################

sub put_windows_productpath_into_template
{
    my ($templatefile, $variableshashref, $languagestringref, $localnsisdir) = @_;

    my $productpath = $variableshashref->{'PROPERTYTABLEPRODUCTNAME'};

    if (length($locallangs) > $installer::globals::max_lang_length) { $locallangs = "multi lingual"; }

    if ( ! $installer::globals::languagepack ) { $productpath = $productpath . " (" . sprintf('%x', time()) . ")"; }

    replace_one_variable($templatefile, "PRODUCTPATHPLACEHOLDER", $productpath);
}

##################################################################
# Windows: Including download file name into nsi template
##################################################################

sub put_outputfilename_into_template
{
    my ($templatefile, $downloadname) = @_;

    $installer::globals::downloadfileextension = ".exe";
    $downloadname = $downloadname . $installer::globals::downloadfileextension;
    $installer::globals::downloadfilename = $downloadname;

    replace_one_variable($templatefile, "DOWNLOADNAMEPLACEHOLDER", $downloadname);
}

##################################################################
# Windows: Generating the file list in nsi file format
##################################################################

sub get_file_list
{
    my ( $basedir ) = @_;

    my @filelist = ();

    my $alldirs = installer::systemactions::get_all_directories($basedir);
    unshift(@{$alldirs}, $basedir); # $basedir is the first directory in $alldirs

    for ( my $i = 0; $i <= $#{$alldirs}; $i++ )
    {
        my $onedir = ${$alldirs}[$i];

        # Syntax:
        # SetOutPath "$INSTDIR"

        my $relativedir = $onedir;
        $relativedir =~ s/\Q$basedir\E//;

        my $oneline = " " . "SetOutPath" . " " . "\"\$INSTDIR" . $relativedir . "\"" . "\n";

        if ( $^O =~ /cygwin/i ) {
            $oneline =~ s/\//\\/g;
        }
        push(@filelist, $oneline);

        # Collecting all files in the specific directory

        my $files = installer::systemactions::get_all_files_from_one_directory($onedir);

        for ( my $j = 0; $j <= $#{$files}; $j++ )
        {
            my $onefile = ${$files}[$j];

            my $fileline = "  " . "File" . " " . "\"" . $onefile . "\"" . "\n";

            if ( $^O =~ /cygwin/i ) {
                $fileline =~ s/\//\\/g;
            }
            push(@filelist, $fileline);
        }
    }

    return \@filelist;
}

##################################################################
# Windows: Including list of all files into nsi template
##################################################################

sub put_filelist_into_template
{
    my ($templatefile, $installationdir) = @_;

    my $filelist = get_file_list($installationdir);

    my $filestring = "";

    for ( my $i = 0; $i <= $#{$filelist}; $i++ )
    {
        $filestring = $filestring . ${$filelist}[$i];
    }

    $filestring =~ s/\s*$//;

    replace_one_variable($templatefile, "ALLFILESPLACEHOLDER", $filestring);
}

##################################################################
# Windows: NSIS uses specific language names
##################################################################

sub nsis_language_converter
{
    my ($language) = @_;

    my $nsislanguage = "";

    # Assign language used by NSIS.
    # The files "$nsislanguage.nsh" and "$nsislanguage.nlf"
    # are needed in the NSIS environment.
    # Directory: <NSIS-Dir>/Contrib/Language files
    if ( $language eq "en-US" ) { $nsislanguage = "English"; }
    elsif ( $language eq "sq" ) { $nsislanguage = "Albanian"; }
    elsif ( $language eq "ar" ) { $nsislanguage = "Arabic"; }
    elsif ( $language eq "bg" ) { $nsislanguage = "Bulgarian"; }
    elsif ( $language eq "ca" ) { $nsislanguage = "Catalan"; }
    elsif ( $language eq "hr" ) { $nsislanguage = "Croatian"; }
    elsif ( $language eq "cs" ) { $nsislanguage = "Czech"; }
    elsif ( $language eq "da" ) { $nsislanguage = "Danish"; }
    elsif ( $language eq "nl" ) { $nsislanguage = "Dutch"; }
    elsif ( $language eq "de" ) { $nsislanguage = "German"; }
    elsif ( $language eq "de-LU" ) { $nsislanguage = "Luxembourgish"; }
    elsif ( $language eq "et" ) { $nsislanguage = "Estonian"; }
    elsif ( $language eq "fa" ) { $nsislanguage = "Farsi"; }
    elsif ( $language eq "el" ) { $nsislanguage = "Greek"; }
    elsif ( $language eq "fi" ) { $nsislanguage = "Finnish"; }
    elsif ( $language eq "fr" ) { $nsislanguage = "French"; }
    elsif ( $language eq "hu" ) { $nsislanguage = "Hungarian"; }
    elsif ( $language eq "he" ) { $nsislanguage = "Hebrew"; }
    elsif ( $language eq "is" ) { $nsislanguage = "Icelandic"; }
    elsif ( $language eq "id" ) { $nsislanguage = "Indonesian"; }
    elsif ( $language eq "it" ) { $nsislanguage = "Italian"; }
    elsif ( $language eq "lv" ) { $nsislanguage = "Latvian"; }
    elsif ( $language eq "lt" ) { $nsislanguage = "Lithuanian"; }
    elsif ( $language eq "mk" ) { $nsislanguage = "Macedonian"; }
    elsif ( $language eq "mn" ) { $nsislanguage = "Mongolian"; }
    elsif ( $language eq "no" ) { $nsislanguage = "Norwegian"; }
    elsif ( $language eq "no-NO" ) { $nsislanguage = "Norwegian"; }
    elsif ( $language eq "es" ) { $nsislanguage = "Spanish"; }
    elsif ( $language eq "sl" ) { $nsislanguage = "Slovenian"; }
    elsif ( $language eq "sv" ) { $nsislanguage = "Swedish"; }
    elsif ( $language eq "sk" ) { $nsislanguage = "Slovak"; }
    elsif ( $language eq "pl" ) { $nsislanguage = "Polish"; }
    elsif ( $language eq "pt-BR" ) { $nsislanguage = "PortugueseBR"; }
    elsif ( $language eq "pt" ) { $nsislanguage = "Portuguese"; }
    elsif ( $language eq "ro" ) { $nsislanguage = "Romanian"; }
    elsif ( $language eq "ru" ) { $nsislanguage = "Russian"; }
    elsif ( $language eq "sh" ) { $nsislanguage = "SerbianLatin"; }
    elsif ( $language eq "sr" ) { $nsislanguage = "Serbian"; }
    elsif ( $language eq "sr-SP" ) { $nsislanguage = "Serbian"; }
    elsif ( $language eq "uk" ) { $nsislanguage = "Ukrainian"; }
    elsif ( $language eq "tr" ) { $nsislanguage = "Turkish"; }
    elsif ( $language eq "ja" ) { $nsislanguage = "Japanese"; }
    elsif ( $language eq "ko" ) { $nsislanguage = "Korean"; }
    elsif ( $language eq "th" ) { $nsislanguage = "Thai"; }
    elsif ( $language eq "zh-CN" ) { $nsislanguage = "SimpChinese"; }
    elsif ( $language eq "zh-TW" ) { $nsislanguage = "TradChinese"; }
    else {
        my $infoline = "NSIS language_converter : Could not find nsis language for $language!\n";
        push( @installer::globals::logfileinfo, $infoline);
        $nsislanguage = "English";
    }

    return $nsislanguage;
}

##################################################################
# Windows: Including list of all languages into nsi template
##################################################################

sub put_language_list_into_template
{
    my ($templatefile, $languagesarrayref) = @_;

    my $alllangstring = "";
    my %nsislangs;

    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ )
    {
        my $onelanguage = ${$languagesarrayref}[$i];
        my $nsislanguage = nsis_language_converter($onelanguage);
        $nsislangs{$nsislanguage}++;
    }

    foreach my $nsislanguage ( keys(%nsislangs) )
    {
        # Syntax: !insertmacro MUI_LANGUAGE "English"
        my $langstring = "\!insertmacro MUI_LANGUAGE_PACK " . $nsislanguage . "\n";
        if ( $nsislanguage eq "English" )
        {
            $alllangstring = $langstring . $alllangstring;
        }
        else
        {
            $alllangstring = $alllangstring . $langstring;
        }
    }

    $alllangstring =~ s/\s*$//;

    replace_one_variable($templatefile, "ALLLANGUAGESPLACEHOLDER", $alllangstring);
}

##################################################################
# Windows: Collecting all identifier from mlf file
##################################################################

sub get_identifier
{
    my ( $mlffile ) = @_;

    my @identifier = ();

    for ( my $i = 0; $i <= $#{$mlffile}; $i++ )
    {
        my $oneline = ${$mlffile}[$i];

        if ( $oneline =~ /^\s*\[(.+)\]\s*$/ )
        {
            my $identifier = $1;
            push(@identifier, $identifier);
        }
    }

    return \@identifier;
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

##################################################################
# Windows: Replacing strings in NSIS nsh file
# nsh file syntax:
# !define MUI_TEXT_DIRECTORY_TITLE "Zielverzeichnis auswählen"
##################################################################

sub replace_identifier_in_nshfile
{
    my ( $nshfile, $identifier, $newstring, $nshfilename, $onelanguage ) = @_;

    if ( $installer::globals::nsis231 )
    {
        $newstring =~ s/\\r/\$\\r/g;    # \r -> $\r  in modern nsis versions
        $newstring =~ s/\\n/\$\\n/g;    # \n -> $\n  in modern nsis versions
    }

    for ( my $i = 0; $i <= $#{$nshfile}; $i++ )
    {
        if ( ${$nshfile}[$i] =~ /\s+\Q$identifier\E\s+\"(.+)\"\s*$/ )
        {
            my $oldstring = $1;
            ${$nshfile}[$i] =~ s/\Q$oldstring\E/$newstring/;
            my $infoline = "NSIS replacement in $nshfilename ($onelanguage): $oldstring \-\> $newstring\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

##################################################################
# Windows: Replacing strings in NSIS nlf file
# nlf file syntax (2 lines):
# # ^DirSubText
# Zielverzeichnis
##################################################################

sub replace_identifier_in_nlffile
{
    my ( $nlffile, $identifier, $newstring, $nlffilename, $onelanguage ) = @_;

    for ( my $i = 0; $i <= $#{$nlffile}; $i++ )
    {
        if ( ${$nlffile}[$i] =~ /^\s*\#\s+\^\s*\Q$identifier\E\s*$/ )
        {
            my $next = $i+1;
            my $oldstring = ${$nlffile}[$next];
            ${$nlffile}[$next] = $newstring . "\n";
            $oldstring =~ s/\s*$//;
            my $infoline = "NSIS replacement in $nlffilename ($onelanguage): $oldstring \-\> $newstring\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

##################################################################
# Windows: Translating the NSIS nsh and nlf file
##################################################################

sub translate_nsh_nlf_file
{
    my ($nshfile, $nlffile, $mlffile, $onelanguage, $nshfilename, $nlffilename, $nsislanguage) = @_;

    # Analyzing the mlf file, collecting all Identifier
    my $allidentifier = get_identifier($mlffile);

    $onelanguage = "en-US" if ( $nsislanguage eq "English" && $onelanguage ne "en-US");
    for ( my $i = 0; $i <= $#{$allidentifier}; $i++ )
    {
        my $identifier = ${$allidentifier}[$i];
        my $language_block = get_language_block_from_language_file($identifier, $mlffile);
        my $newstring = get_language_string_from_language_block($language_block, $onelanguage);

        # removing mask
        $newstring =~ s/\\\'/\'/g;

        replace_identifier_in_nshfile($nshfile, $identifier, $newstring, $nshfilename, $onelanguage);
        replace_identifier_in_nlffile($nlffile, $identifier, $newstring, $nlffilename, $onelanguage);
    }
}

##################################################################
# Converting utf 16 file to utf 8
##################################################################

sub convert_utf16_to_utf8
{
    my ( $filename ) = @_;

    my @localfile = ();

    my $savfilename = $filename . "_before.utf16";
    installer::systemactions::copy_one_file($filename, $savfilename);

    open( IN, "<:encoding(UTF16-LE)", $filename ) || installer::exiter::exit_program("ERROR: Cannot open file $filename for reading", "convert_utf16_to_utf8");
    while ( $line = <IN> ) {
        push @localfile, $line;
    }
    close( IN );

    if ( open( OUT, ">:utf8", $filename ) )
    {
        print OUT @localfile;
        close(OUT);
    }

    $savfilename = $filename . "_before.utf8";
    installer::systemactions::copy_one_file($filename, $savfilename);
}

##################################################################
# Converting utf 8 file to utf 16
##################################################################

sub convert_utf8_to_utf16
{
    my ( $filename ) = @_;

    my @localfile = ();

    my $savfilename = $filename . "_after.utf8";
    installer::systemactions::copy_one_file($filename, $savfilename);

    open( IN, "<:utf8", $filename ) || installer::exiter::exit_program("ERROR: Cannot open file $filename for reading", "convert_utf8_to_utf16");
    while ( $line = <IN> ) {
        push @localfile, $line;
    }
    close( IN );

    if ( open( OUT, ">:raw:encoding(UTF16-LE):crlf:utf8", $filename ) )
    {
        print OUT @localfile;
        close(OUT);
    }

    $savfilename = $filename . "_after.utf16";
    installer::systemactions::copy_one_file($filename, $savfilename);
}

##################################################################
# Converting text string to utf 16
##################################################################

sub convert_textstring_to_utf16
{
    my ( $textstring, $localnsisdir, $shortfilename ) = @_;

    my $filename = $localnsisdir . $installer::globals::separator . $shortfilename;
    my @filecontent = ();
    push(@filecontent, $textstring);
    installer::files::save_file($filename, \@filecontent);
    convert_utf8_to_utf16($filename);
    my $newfile = installer::files::read_file($filename);
    my $utf16string = "";
    if ( ${$newfile}[0] ne "" ) { $utf16string = ${$newfile}[0]; }

    return $utf16string;
}

##################################################################
# Windows: Copying NSIS language files to local nsis directory
##################################################################

sub copy_and_translate_nsis_language_files
{
    my ($nsispath, $localnsisdir, $languagesarrayref, $allvariables) = @_;

    my $nlffilepath = $nsispath . $installer::globals::separator . "Contrib" . $installer::globals::separator . "Language\ files" . $installer::globals::separator;
    my $nshfilepath = $nsispath . $installer::globals::separator . "Contrib" . $installer::globals::separator . "Modern\ UI" . $installer::globals::separator . "Language files" . $installer::globals::separator;

    my $infoline = "";

    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ )
    {
        my $onelanguage = ${$languagesarrayref}[$i];
        my $nsislanguage = nsis_language_converter($onelanguage);

        # Copying the nlf file
        my $sourcepath = $nlffilepath . $nsislanguage . "\.nlf";
        if ( ! -f $sourcepath ) { installer::exiter::exit_program("ERROR: Could not find nsis file: $sourcepath!", "copy_and_translate_nsis_language_files"); }
        my $nlffilename = $localnsisdir . $installer::globals::separator . $nsislanguage . "_pack.nlf";
        if ( $^O =~ /cygwin/i ) { $nlffilename =~ s/\//\\/g; }
        installer::systemactions::copy_one_file($sourcepath, $nlffilename);

        # Copying the nsh file
        # In newer nsis versions, the nsh file is located next to the nlf file
        $sourcepath = $nshfilepath . $nsislanguage . "\.nsh";
        if ( ! -f $sourcepath )
        {
            # trying to find the nsh file next to the nlf file
            $sourcepath = $nlffilepath . $nsislanguage . "\.nsh";
            if ( ! -f $sourcepath )
            {
                installer::exiter::exit_program("ERROR: Could not find nsis file: $sourcepath!", "copy_and_translate_nsis_language_files");
            }
        }
        my $nshfilename = $localnsisdir . $installer::globals::separator . $nsislanguage . "_pack.nsh";
        if ( $^O =~ /cygwin/i ) { $nshfilename =~ s/\//\\/g; }
        installer::systemactions::copy_one_file($sourcepath, $nshfilename);

        # Changing the macro name in nsh file: MUI_LANGUAGEFILE_BEGIN -> MUI_LANGUAGEFILE_PACK_BEGIN
        my $nshfile = installer::files::read_file($nshfilename);
        set_nsis_version($nshfile);

        if ( $installer::globals::unicodensis )
        {
            $infoline = "This is Unicode NSIS!\n";
            push( @installer::globals::logfileinfo, $infoline);
            convert_utf16_to_utf8($nshfilename);
            convert_utf16_to_utf8($nlffilename);
            $nshfile = installer::files::read_file($nshfilename);   # read nsh file again
        }

        replace_one_variable($nshfile, "MUI_LANGUAGEFILE_BEGIN", "MUI_LANGUAGEFILE_PACK_BEGIN");

        # find the ulf file for translation
        my $mlffile = get_translation_file($allvariables);

        # Translate the files
        my $nlffile = installer::files::read_file($nlffilename);
        translate_nsh_nlf_file($nshfile, $nlffile, $mlffile, $onelanguage, $nshfilename, $nlffilename, $nsislanguage);

        installer::files::save_file($nshfilename, $nshfile);
        installer::files::save_file($nlffilename, $nlffile);

        if ( $installer::globals::unicodensis )
        {
            convert_utf8_to_utf16($nshfilename);
            convert_utf8_to_utf16($nlffilename);
        }
    }

}

##################################################################
# Windows: Including the nsis path into the nsi template
##################################################################

sub put_nsis_path_into_template
{
    my ($templatefile, $nsisdir) = @_;

    replace_one_variable($templatefile, "NSISPATHPLACEHOLDER", $nsisdir);
}

##################################################################
# Windows: Including the output path into the nsi template
##################################################################

sub put_output_path_into_template
{
    my ($templatefile, $downloaddir) = @_;

    if ( $^O =~ /cygwin/i ) { $downloaddir =~ s/\//\\/g; }

    replace_one_variable($templatefile, "OUTPUTDIRPLACEHOLDER", $downloaddir);
}

##################################################################
# Windows: Only allow specific code for nsis 2.0.4 or nsis 2.3.1
##################################################################

sub put_version_specific_code_into_template
{
    my ($templatefile) = @_;

    my $subst204 = "";
    my $subst231 = "";

    if ( $installer::globals::nsis204 )
    {
        $subst231 = ";";
    }
    else
    {
        $subst204 = ";";
    }

    replace_one_variable($templatefile, "\#204\#", $subst204);
    replace_one_variable($templatefile, "\#231\#", $subst231);
}

##################################################################
# Windows: Finding the path to the nsis SDK
##################################################################

sub get_path_to_nsis_sdk
{
    my $vol;
    my $dir;
    my $file;
    my $nsispath = "";

    if ( $ENV{'NSIS_PATH'} ) {
        $nsispath = $ENV{'NSIS_PATH'};
    } elsif ( $ENV{'SOLARROOT'} ) {
        $nsispath = $ENV{'SOLARROOT'} . $installer::globals::separator . "NSIS";
    } else {
        # do we have nsis already in path ?
        @paths = split(/:/, $ENV{'PATH'});
        foreach $paths (@paths) {
            $paths =~ s/[\/\\]+$//; # remove trailing slashes;
            $nsispath = $paths . "/nsis";

            if ( -x $nsispath ) {
                $nsispath = $paths;
                last;
            }
            else {
                $nsispath = "";
            }
        }
    }
    if ( $ENV{'NSISSDK_SOURCE'} ) {
        installer::logger::print_warning( "NSISSDK_SOURCE is deprecated. use NSIS_PATH instead.\n" );
        $nsispath = $ENV{'NSISSDK_SOURCE'}; # overriding the NSIS SDK with NSISSDK_SOURCE
    }

    if ( $nsispath eq "" )
    {
        installer::logger::print_message( "... no Environment variable \"SOLARROOT\", \"NSIS_PATH\" or \"NSISSDK_SOURCE\" found and NSIS not found in path!", "get_path_to_nsis_sdk");
    } elsif ( ! -d $nsispath )
    {
        installer::exiter::exit_program("ERROR: NSIS path $nsispath does not exist!", "get_path_to_nsis_sdk");
    }

    return $nsispath;
}

##################################################################
# Windows: Executing NSIS to create the installation set
##################################################################

sub call_nsis
{
    my ( $nsispath, $nsifile ) = @_;

    my $makensisexe = $nsispath . $installer::globals::separator . "makensis.exe";

    installer::logger::print_message( "... starting $makensisexe ... \n" );

    if( $^O =~ /cygwin/i ) { $nsifile =~ s/\\/\//g; }

    my $systemcall = "$makensisexe /V4 /DMUI_VERBOSE=4 $nsifile |";

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    my @nsisoutput = ();

    open (NSI, "$systemcall");
    while (<NSI>) {push(@nsisoutput, $_); }
    close (NSI);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    if ($returnvalue)
    {
        $infoline = "ERROR: $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    for ( my $i = 0; $i <= $#nsisoutput; $i++ ) { push( @installer::globals::logfileinfo, "$nsisoutput[$i]"); }

}

#################################################################################
# Replacing one variable in one files
#################################################################################

sub replace_one_variable_in_translationfile
{
    my ($translationfile, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$translationfile}; $i++ )
    {
        ${$translationfile}[$i] =~ s/\%$searchstring/$variable/g;
    }
}

#################################################################################
# Replacing the variables in the translation file
#################################################################################

sub replace_variables
{
    my ($translationfile, $variableshashref) = @_;

    # we want to substitute FOO_BR before FOO to avoid floating _BR suffixes
    foreach $key (sort { length ($b) <=> length ($a) } keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};

        # special handling for PRODUCTVERSION, if $allvariables->{'POSTVERSIONEXTENSION'}
        if (( $key eq "PRODUCTVERSION" ) && ( $variableshashref->{'POSTVERSIONEXTENSION'} )) { $value = $value . " " . $variableshashref->{'POSTVERSIONEXTENSION'}; }

        replace_one_variable_in_translationfile($translationfile, $value, $key);
    }
}

#########################################################
# Getting the translation file for the nsis installer
#########################################################

sub get_translation_file
{
    my ($allvariableshashref) = @_;
    my $translationfilename = $installer::globals::idtlanguagepath . $installer::globals::separator . $installer::globals::nsisfilename;
    if ( $installer::globals::unicodensis ) { $translationfilename = $translationfilename . ".uulf"; }
    else { $translationfilename = $translationfilename . ".mlf"; }
    if ( ! -f $translationfilename ) { installer::exiter::exit_program("ERROR: Could not find language file $translationfilename!", "get_translation_file"); }
    my $translationfile = installer::files::read_file($translationfilename);
    replace_variables($translationfile, $allvariableshashref);

    my $infoline = "Reading translation file: $translationfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $translationfile;
}

####################################################
# Removing english, if it was added before
####################################################

sub remove_english_for_nsis_installer
{
    my ($languagestringref, $languagesarrayref) = @_;

    @{$languagesarrayref} = ("en-US");  # only english for NSIS installer!
}

####################################################
# Creating link tree for upload
####################################################

sub create_link_tree
{
    my ($sourcedownloadfile, $destfilename, $versionstring) = @_;

    if ( ! $installer::globals::ooouploaddir ) { installer::exiter::exit_program("ERROR: Directory for LO upload not defined!", "create_link_tree"); }
    my $versiondir = $installer::globals::ooouploaddir . $installer::globals::separator . $versionstring;
    my $infoline = "Directory for the link: $versiondir\n";
    push(@installer::globals::logfileinfo, $infoline);

    if ( ! -d $versiondir ) { installer::systemactions::create_directory_structure($versiondir); }

    # inside directory $versiondir all links have to be created
    my $linkdestination = $versiondir . $installer::globals::separator . $destfilename;

    # If there is an older version of this file (link), it has to be removed
    if ( -f $linkdestination ) { unlink($linkdestination); }

    $infoline = "Creating hard link from $sourcedownloadfile to $linkdestination\n";
    push(@installer::globals::logfileinfo, $infoline);
    installer::systemactions::hardlink_one_file($sourcedownloadfile, $linkdestination);
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

    # special handling for installation sets, to which english was added automatically
    if ( $installer::globals::added_english ) { remove_english_for_nsis_installer($languagestringref, $languagesarrayref); }

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

            if ( $installer::globals::include_pathes_read )
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
    else    # Windows specific part
    {
        my $localnsisdir = installer::systemactions::create_directories("nsis", $languagestringref);
        # push(@installer::globals::removedirs, $localnsisdir);

        # find nsis in the system
        my $nsispath = get_path_to_nsis_sdk();

        if ( $nsispath eq "" ) {
            # If nsis is not found just skip the rest of this function
            # and do not create the NSIS file.
            $infoline = "\nNo NSIS SDK found. Skipping the generation of NSIS file.\n";
            push(@installer::globals::logfileinfo, $infoline);
            installer::logger::print_message( "... no NSIS SDK found. Skipping the generation of NSIS file ... \n" );
            return $downloaddir;
        }

        # copy language files into nsis directory and translate them
        copy_and_translate_nsis_language_files($nsispath, $localnsisdir, $languagesarrayref, $allvariableshashref);

        # find and read the nsi file template
        my $templatefilename = "downloadtemplate.nsi";

        my $templateref = "";

        if ( $installer::globals::include_pathes_read )
        {
            $templateref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$templatefilename, $includepatharrayref, 0);
        }
        else
        {
            $templateref = installer::scriptitems::get_sourcepath_from_filename_and_includepath_classic(\$templatefilename, $includepatharrayref, 0);
        }

        if ($$templateref eq "") { installer::exiter::exit_program("ERROR: Could not find nsi template file $templatefilename!", "create_download_sets"); }
        my $templatefile = installer::files::read_file($$templateref);

        # add product name into script template
        put_windows_productname_into_template($templatefile, $allvariableshashref);
        @path_substs = (
            'DOWNLOADBANNER_BR' => 'BANNERBMPPLACEHOLDER_BR',
            'DOWNLOADBITMAP_BR' => 'WELCOMEBMPPLACEHOLDER_BR',
            'DOWNLOADBANNER' => 'BANNERBMPPLACEHOLDER',
            'DOWNLOADBITMAP' => 'WELCOMEBMPPLACEHOLDER',
            'DOWNLOADSETUPICO' => 'SETUPICOPLACEHOLDER'
        );
        while (@path_substs) {
            my $var= shift @path_substs;
            my $val = shift @path_substs;
            substitute_path_into_template($templatefile, $includepatharrayref,
                                          $allvariableshashref, $var, $val);
        }
        %var_substs = (
            'OOOVENDOR' => 'PUBLISHERPLACEHOLDER',
            'STARTCENTER_INFO_URL' => 'WEBSITEPLACEHOLDER',
            'WINDOWSJAVAFILENAME' => 'WINDOWSJAVAFILENAMEPLACEHOLDER',
            'PRODUCTVERSION' => 'PRODUCTVERSIONPLACEHOLDER'
        );
        for $var (keys %var_substs) {
            substitute_variable_into_template($templatefile, $allvariableshashref,
                                              $var, $var_substs{$var});
        }
        put_windows_productpath_into_template($templatefile, $allvariableshashref, $languagestringref, $localnsisdir);
        put_outputfilename_into_template($templatefile, $downloadname);
        put_filelist_into_template($templatefile, $installationdir);
        put_language_list_into_template($templatefile, $languagesarrayref);
        put_nsis_path_into_template($templatefile, $localnsisdir);
        put_output_path_into_template($templatefile, $downloaddir);
        put_version_specific_code_into_template($templatefile);

        my $nsifilename = save_script_file($localnsisdir, $templatefilename, $templatefile);

        installer::logger::print_message( "... created NSIS file $nsifilename ... \n" );

        # starting the NSIS SDK to create the download file
        call_nsis($nsispath, $nsifilename);
    }

    return $downloaddir;
}

####################################################
# Creating LO upload tree
####################################################

sub create_download_link_tree
{
    my ($downloaddir, $languagestringref, $allvariableshashref) = @_;

    my $infoline;

    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating download hard link ...\n" );
    installer::logger::print_message( "******************************************\n" );

    installer::logger::include_header_into_logfile("Creating download hard link:");
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Creating hard link, start");

    if ( is_supported_platform() )
    {
        my $versionstring = "";
        # Already defined $installer::globals::oooversionstring and $installer::globals::ooodownloadfilename ?

        if ( ! $installer::globals::oooversionstring ) { $versionstring = get_current_version(); }
        else { $versionstring = $installer::globals::oooversionstring; }

        # Is $versionstring empty? If yes, there is nothing to do now.

        $infoline = "Version string is set to: $versionstring\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ( $versionstring )
        {
            # Now the downloadfilename has to be set (if not already done)
            my $destdownloadfilename = "";
            if ( ! $installer::globals::ooodownloadfilename ) { $destdownloadfilename = set_download_filename($languagestringref, $versionstring, $allvariableshashref); }
            else { $destdownloadfilename = $installer::globals::ooodownloadfilename; }

            if ( $destdownloadfilename )
            {
                $destdownloadfilename = $destdownloadfilename . $installer::globals::downloadfileextension;

                $infoline = "Setting destination download file name: $destdownloadfilename\n";
                push( @installer::globals::logfileinfo, $infoline);

                my $sourcedownloadfile = $downloaddir . $installer::globals::separator . $installer::globals::downloadfilename;

                $infoline = "Setting source download file name: $sourcedownloadfile\n";
                push( @installer::globals::logfileinfo, $infoline);

                create_link_tree($sourcedownloadfile, $destdownloadfilename, $versionstring);
            }
        }
        else
        {
            $infoline = "Version string is empty. Nothing to do!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
    else
    {
        $infoline = "Platform not used for hard linking. Nothing to do!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    installer::logger::include_timestamp_into_logfile("Performance Info: Creating hard link, stop");
}

1;
