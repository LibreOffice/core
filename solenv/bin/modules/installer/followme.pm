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

package installer::followme;

use File::Spec;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::systemactions;

####################################################
# Creating a file, that contains all information
# to create a follow up process.
####################################################

sub save_followme_info
{
    my ($finalinstalldir, $includepatharrayref, $allvariableshashref, $downloadname, $languagestringref, $languagesarrayref, $current_install_number, $loggingdir, $installlogdir) = @_;

    my $downloadinfofilename = $installer::globals::logfilename;
    if ( $installer::globals::updatepack ) { $downloadinfofilename =~ s/log_/log_$current_install_number\_/; }
    $downloadinfofilename =~ s/log_/follow_me_/;

    # Creating directory
    my $downloadinfodir = installer::systemactions::create_directory_next_to_directory($finalinstalldir, "follow_me");

    my @filecontent = ();

    push(@filecontent, "finalinstalldir: $finalinstalldir\n");
    push(@filecontent, "downloadname: $downloadname\n");
    push(@filecontent, "currentinstallnumber: $current_install_number\n");
    push(@filecontent, "loggingdir: $loggingdir\n");
    push(@filecontent, "installlogdir: $installlogdir\n");
    push(@filecontent, "languagestring: $$languagestringref\n");
    foreach my $element ( @{$languagesarrayref} ) { push(@filecontent, "languagesarray: $element\n"); }
    foreach my $path ( @{$includepatharrayref} ) { push(@filecontent, "includepatharray: $path"); }
    foreach my $key ( sort keys %{$allvariableshashref} ) { push(@filecontent, "allvariableshash: $key : $allvariableshashref->{$key}\n"); }
    push(@filecontent, "globals:updatepack: $installer::globals::updatepack\n");
    push(@filecontent, "globals:added_english: $installer::globals::added_english\n");
    push(@filecontent, "globals:iswindowsbuild: $installer::globals::iswindowsbuild\n");
    push(@filecontent, "globals:issolarisbuild: $installer::globals::issolarisbuild\n");
    push(@filecontent, "globals:issolarispkgbuild: $installer::globals::issolarispkgbuild\n");
    push(@filecontent, "globals:issolarissparcbuild: $installer::globals::issolarissparcbuild\n");
    push(@filecontent, "globals:issolarisx86build: $installer::globals::issolarisx86build\n");
    push(@filecontent, "globals:isfreebsdpkgbuild: $installer::globals::isfreebsdpkgbuild\n");
    push(@filecontent, "globals:islinuxbuild: $installer::globals::islinuxbuild\n");
    push(@filecontent, "globals:isrpmbuild: $installer::globals::isrpmbuild\n");
    push(@filecontent, "globals:islinuxintelrpmbuild: $installer::globals::islinuxintelrpmbuild\n");
    push(@filecontent, "globals:islinuxppcrpmbuild: $installer::globals::islinuxppcrpmbuild\n");
    push(@filecontent, "globals:islinuxx86_64rpmbuild: $installer::globals::islinuxx86_64rpmbuild\n");
    push(@filecontent, "globals:isdebbuild: $installer::globals::isdebbuild\n");
    push(@filecontent, "globals:islinuxinteldebbuild: $installer::globals::islinuxinteldebbuild\n");
    push(@filecontent, "globals:islinuxppcdebbuild: $installer::globals::islinuxppcdebbuild\n");
    push(@filecontent, "globals:islinuxx86_64debbuild: $installer::globals::islinuxx86_64debbuild\n");
    push(@filecontent, "globals:issolaris: $installer::globals::issolaris\n");
    push(@filecontent, "globals:islinux: $installer::globals::islinux\n");
    push(@filecontent, "globals:unpackpath: $installer::globals::unpackpath\n");
    push(@filecontent, "globals:idttemplatepath: $installer::globals::idttemplatepath\n");
    push(@filecontent, "globals:idtlanguagepath: $installer::globals::idtlanguagepath\n");
    push(@filecontent, "globals:logfilename: $installer::globals::logfilename\n");
    push(@filecontent, "globals:product: $installer::globals::product\n");
    push(@filecontent, "globals:patch: $installer::globals::patch\n");
    push(@filecontent, "globals:languagepack: $installer::globals::languagepack\n");
    push(@filecontent, "globals:helppack: $installer::globals::helppack\n");
    push(@filecontent, "globals:installertypedir: $installer::globals::installertypedir\n");
    push(@filecontent, "globals:max_lang_length: $installer::globals::max_lang_length\n");
    push(@filecontent, "globals:compiler: $installer::globals::compiler\n");
    push(@filecontent, "globals:product: $installer::globals::product\n");
    push(@filecontent, "globals:minor: $installer::globals::minor\n");
    push(@filecontent, "globals:lastminor: $installer::globals::lastminor\n");
    push(@filecontent, "globals:nsisfilename: $installer::globals::nsisfilename\n");

    # Saving file
    installer::files::save_file($downloadinfodir . $installer::globals::separator . $downloadinfofilename, \@filecontent);
    installer::logger::print_message( "... creating \"follow me\" info file $downloadinfofilename.\n" );
}

####################################################
# Reading the file, that contains all information
# to create a follow up process.
####################################################

sub read_followme_info
{
    my ( $filename ) = @_;

    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file: $filename", "read_download_info"); }

    installer::logger::print_message( "\n... reading \"follow me\" info file $filename\n" );

    my %contenthash = ();

    my $finalinstalldir = "";
    my $downloadname = "";
    my $currentinstallnumber = "";
    my $loggingdir = "";
    my $installlogdir = "";
    my $languagestring = "";
    my @includepatharray = ();
    my @languagesarray = ();
    my %allvariableshash = ();

    # Global variables can be set directly

    my $filecontent = installer::files::read_file($filename);

    # First line has to contain the string "finalinstalldir:".
    # Otherwise this is not a correct file.


    if ( ! ( ${$filecontent}[0] =~ /s*finalinstalldir:\s*(.*?)\s*$/ )) { installer::exiter::exit_program("ERROR: Not a correct download info file: $filename", "read_download_info"); }

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $line = ${$filecontent}[$i];

        if ( $line =~ /^\s*finalinstalldir:\s*(.*?)\s*$/ ) { $finalinstalldir = $1; }
        if( $^O =~ /cygwin/i ) { $finalinstalldir =~ s/\\/\//; }
        if ( $line =~ /^\s*downloadname:\s*(.*?)\s*$/ ) { $downloadname = $1; }
        if ( $line =~ /^\s*currentinstallnumber:\s*(.*?)\s*$/ ) { $currentinstallnumber = $1; }
        if ( $line =~ /^\s*loggingdir:\s*(.*?)\s*$/ ) { $loggingdir = $1; }
        if( $^O =~ /cygwin/i ) { $loggingdir =~ s/\\/\//; }
        if ( $line =~ /^\s*installlogdir:\s*(.*?)\s*$/ ) { $installlogdir = $1; }
        if( $^O =~ /cygwin/i ) { $installlogdir =~ s/\\/\//; }
        if ( $line =~ /^\s*languagestring:\s*(.*?)\s*$/ ) { $languagestring = $1; }
        if ( $line =~ /^\s*languagesarray:\s*(.*?)\s*$/ ) { push(@languagesarray, $1); }
        if ( $line =~ /^\s*includepatharray:\s*(.*?)\s*$/ )
        {
            my $path = $1;
            if( $^O =~ /cygwin/i ) { $path  =~ s/\\/\//; }
            push(@includepatharray, $path . "\n");
        }
        if ( $line =~ /^\s*allvariableshash:\s*(.*?)\s*:\s*(.*?)\s*$/ ) { $allvariableshash{$1} = $2; }
        if ( $line =~ /^\s*globals:(.*?)\s*:\s*(.*?)\s*$/ )
        {
            my $name = $1;
            my $value = $2;
            if ( $name eq "updatepack" ) { $installer::globals::updatepack = $value; }
            if ( $name eq "added_english" ) { $installer::globals::added_english = $value; }
            if ( $name eq "iswindowsbuild" ) { $installer::globals::iswindowsbuild = $value; }
            if ( $name eq "issolarisbuild" ) { $installer::globals::issolarisbuild = $value; }
            if ( $name eq "issolarispkgbuild" ) { $installer::globals::issolarispkgbuild = $value; }
            if ( $name eq "issolarissparcbuild" ) { $installer::globals::issolarissparcbuild = $value; }
            if ( $name eq "issolarisx86build" ) { $installer::globals::issolarisx86build = $value; }
            if ( $name eq "isfreebsdpkgbuild" ) { $installer::globals::isfreebsdpkgbuild = $value; }
            if ( $name eq "islinuxbuild" ) { $installer::globals::islinuxbuild = $value; }
            if ( $name eq "isrpmbuild" ) { $installer::globals::isrpmbuild = $value; }
            if ( $name eq "islinuxintelrpmbuild" ) { $installer::globals::islinuxintelrpmbuild = $value; }
            if ( $name eq "islinuxppcrpmbuild" ) { $installer::globals::islinuxppcrpmbuild = $value; }
            if ( $name eq "islinuxx86_64rpmbuild" ) { $installer::globals::islinuxx86_64rpmbuild = $value; }
            if ( $name eq "isdebbuild" ) { $installer::globals::isdebbuild = $value; }
            if ( $name eq "islinuxinteldebbuild" ) { $installer::globals::islinuxinteldebbuild = $value; }
            if ( $name eq "islinuxppcdebbuild" ) { $installer::globals::islinuxppcdebbuild = $value; }
            if ( $name eq "islinuxx86_64debbuild" ) { $installer::globals::islinuxx86_64debbuild = $value; }
            if ( $name eq "issolaris" ) { $installer::globals::issolaris = $value; }
            if ( $name eq "islinux" ) { $installer::globals::islinux = $value; }
            if ( $name eq "unpackpath" ) { $installer::globals::unpackpath = $value; }
            if( $^O =~ /cygwin/i ) { $installer::globals::unpackpath =~ s/\\/\//; }
            if ( $name eq "idttemplatepath" ) { $installer::globals::idttemplatepath = $value; }
            if( $^O =~ /cygwin/i ) { $installer::globals::idttemplatepath =~ s/\\/\//; }
            if ( $name eq "idtlanguagepath" ) { $installer::globals::idtlanguagepath = $value; }
            if( $^O =~ /cygwin/i ) { $installer::globals::idtlanguagepath =~ s/\\/\//; }
            if ( $name eq "logfilename" ) { $installer::globals::logfilename = $value; }
            if ( $name eq "product" ) { $installer::globals::product = $value; }
            if ( $name eq "patch" ) { $installer::globals::patch = $value; }
            if ( $name eq "languagepack" ) { $installer::globals::languagepack = $value; }
            if ( $name eq "helppack" ) { $installer::globals::helppack = $value; }
            if ( $name eq "installertypedir" ) { $installer::globals::installertypedir = $value; }
            if ( $name eq "max_lang_length" ) { $installer::globals::max_lang_length = $value; }
            if ( $name eq "compiler" ) { $installer::globals::compiler = $value; }
            if ( $name eq "product" ) { $installer::globals::product = $value; }
            if ( $name eq "minor" ) { $installer::globals::minor = $value; }
            if ( $name eq "lastminor" ) { $installer::globals::lastminor = $value; }
            if ( $name eq "nsisfilename" ) { $installer::globals::nsisfilename = $value; }
        }
    }

    $contenthash{'finalinstalldir'} = $finalinstalldir;
    $contenthash{'downloadname'} = $downloadname;
    $contenthash{'currentinstallnumber'} = $currentinstallnumber;
    $contenthash{'loggingdir'} = $loggingdir;
    $contenthash{'installlogdir'} = $installlogdir;
    $contenthash{'languagestring'} = $languagestring;
    $contenthash{'languagesarray'} = \@languagesarray;
    $contenthash{'includepatharray'} = \@includepatharray;
    $contenthash{'allvariableshash'} = \%allvariableshash;

    return \%contenthash;
}

1;
