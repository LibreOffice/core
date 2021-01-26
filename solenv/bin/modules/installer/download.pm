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
    my ($filename) = @_;

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
    my ( $installdir, $tarfilename, $usefakeroot) = @_;

    my $fakerootstring = "";
    if ( $usefakeroot ) { $fakerootstring = "fakeroot"; }

    my $systemcall = "cd $installdir; $fakerootstring tar -cf - * > $tarfilename";

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

    # do not list languages if there are too many
    if ( length ($languages) > $installer::globals::max_lang_length )
    {
        $languages = '';
    }

    # do not list pure en-US, except for helppack and langpack
    if ( ( $languages eq "en-US" ) &&
         ( ! $installer::globals::languagepack ) &&
         ( ! $installer::globals::helppack ) )
    {
        $languages = '';
    }

    return $languages;
}

#########################################################
# Setting download name
#########################################################

sub get_downloadname_productname
{
    my ($allvariables) = @_;

    my $start = "";

    $start = $allvariables->{'PRODUCTNAME'};

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
    if (( $allvariables->{'PRODUCTEXTENSION'} ) && ( $allvariables->{'PRODUCTEXTENSION'} ne "" )) { $version = $version . $allvariables->{'PRODUCTEXTENSION'}; }

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
        $platformname = $installer::globals::os;
    }

    return $platformname;
}

#########################################################
# Setting the architecture for the download name
#########################################################

sub get_download_architecture
{
    my $arch = "";

    if ( $installer::globals::issolarissparcbuild )
    {
        $arch = "Sparc";
    }
    elsif ( $installer::globals::issolarisx86build )
    {
        $arch = "x86";
    }
    elsif ( $installer::globals::iswindowsbuild )
    {
        if ( $installer::globals::iswin64build )
        {
            $arch = "x64";
        }
        else
        {
            $arch = "x86";
        }
    }
    elsif ( $installer::globals::cpuname eq 'INTEL' )
    {
        $arch = "x86";
    }
    elsif ( $installer::globals::cpuname eq 'POWERPC' )
    {
        $arch = "PPC";
    }
    elsif ( $installer::globals::cpuname eq 'POWERPC64' )
    {
        $arch = "PPC";
    }
    elsif ( $installer::globals::cpuname eq 'X86_64' )
    {
        $arch = "x86-64";
    }
    elsif ( $installer::globals::cpuname eq 'AARCH64' )
    {
        $arch = "aarch64";
    }

    return $arch;
}

#########################################################
# Setting the content type for the download name
#########################################################

sub get_download_content
{
    my ($allvariables) = @_;

    my $content = "";

    # content type included in the installer
    if ( $installer::globals::isrpmbuild )
    {
        $content = "rpm";
    }
    elsif ( $installer::globals::isdebbuild )
    {
        $content = "deb";
    }
    elsif ( $installer::globals::packageformat eq "archive" )
    {
        $content = "archive";
    }

    return $content;
}

#########################################################
# Setting the functionality type for the download name
#########################################################

sub get_download_functionality
{
    my ($allvariables) = @_;

    my $functionality = "";

    if ( $installer::globals::languagepack )
    {
        $functionality = "langpack";
    }
    elsif ( $installer::globals::helppack )
    {
        $functionality = "helppack";
    }
    elsif ( $allvariables->{'POSTVERSIONEXTENSION'} eq "SDK" )
    {
        $functionality = "sdk";
    }
    elsif ( $allvariables->{'POSTVERSIONEXTENSION'} eq "TEST" )
    {
        $functionality = "test";
    }
    elsif ( $allvariables->{'PRODUCTNAME'} eq "URE" )
    {
        $functionality = "ure";
    }

    return $functionality;
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
    my $content = get_download_content($allvariables);
    my $functionality = get_download_functionality($allvariables);
    my $language = get_downloadname_language($languagestringref);

    # Setting the extension happens automatically

    my $filename = $start . "_" . $versionstring . "_" . $platform . "_" . $architecture . "_" . $content . "_" . $functionality . "_" . $language;

    # get rid of duplicit "_" delimiters when some strings are empty
    $filename =~ s/\_\_\_/\_/g;
    $filename =~ s/\_\_/\_/g;
    $filename =~ s/\_\s*$//;

    $installer::globals::ooodownloadfilename = $filename;

    return $filename;
}


#########################################################
# Creating a tar.gz file
#########################################################

sub create_tar_gz_file_from_directory
{
    my ($installdir, $usefakeroot, $downloaddir, $downloadfilename) = @_;

    my $infoline = "";

    my $packdir = $installdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packdir);
    my $changedir = $installdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$changedir);

    my $fakerootstring = "";
    if ( $usefakeroot ) { $fakerootstring = "fakeroot"; }

    $installer::globals::downloadfileextension = ".tar.gz";
    $installer::globals::downloadfilename = $downloadfilename . $installer::globals::downloadfileextension;
    my $targzname = $downloaddir . $installer::globals::separator . $installer::globals::downloadfilename;

    # fdo#67060 - install script is for RPM only
    if ( -e "$installdir/install" && !$installer::globals::isrpmbuild )
    {
        unlink("$installdir/install");
    }

    my $systemcall = "cd $changedir; $fakerootstring tar -cf - $packdir | gzip > $targzname";

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

    $downloadname = set_download_filename($languagestringref, $allvariableshashref);

    if ( ! $installer::globals::iswindowsbuild )    # Unix specific part
    {

        # whether to use fakeroot (only required for Solaris and Linux)
        my $usefakeroot = 0;
        if (( $installer::globals::issolarisbuild ) || ( $installer::globals::islinuxbuild )) { $usefakeroot = 1; }

        my $downloadfile = create_tar_gz_file_from_directory($installationdir, $usefakeroot, $downloaddir, $downloadname);
    }

    return $downloaddir;
}

1;
