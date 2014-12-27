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
# Searching for the getuid.so
#########################################################

sub get_path_for_library
{
    my $getuidlibrary = $ENV{'WORKDIR'} . '/LinkTarget/Library/libgetuid.so';
    if ( ! -e $getuidlibrary ) { installer::exiter::exit_program("File $getuidlibrary does not exist!", "get_path_for_library"); }
    return $getuidlibrary;
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
# Get product’s default language
#########################################################

sub get_language_default_for_product
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
# Get the name of product
#########################################################

sub get_name_of_product
{
    my ($allvariables) = @_;

    my $start = "";

    $start = $allvariables->{'PRODUCTNAME'};

    return $start;
}

#########################################################
# Get version for the download name
#########################################################

sub get_version_for_download_name
{
    my ($allvariables) = @_;

    my $version = "";

    $version = $allvariables->{'PRODUCTVERSION'};
    if (( $allvariables->{'PRODUCTEXTENSION'} ) && ( $allvariables->{'PRODUCTEXTENSION'} ne "" ))
    {
        $version = $version . $allvariables->{'PRODUCTEXTENSION'};
    }

    return $version;
}

#########################################################
# Get the type of functionality for the download name
#########################################################

sub get_type_of_functionality_for_download_name
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
# Get the download file name
# Syntax:
# (PRODUCTNAME)_(VERSION)_(OS)_(ARCH)_(INSTALLTYPE)_(LANGUAGE).(FILEEXTENSION)
###############################################################################################

sub get_download_file_name
{
    my ($languagestringref, $allvariables) = @_;

    my $language = get_language_default_for_product($languagestringref);
    my $productname = get_name_of_product($allvariables);
    my $versionstring = get_version_for_download_name($allvariables);
    my $functionality = get_type_of_functionality_for_download_name($allvariables);

    # operating system for the download name
    my $osname = "";
    if ( $installer::globals::islinuxbuild )
    {
        $osname = "Linux";
    }
    elsif ( $installer::globals::issolarisbuild )
    {
        $osname = "Solaris";
    }
    elsif ( $installer::globals::iswindowsbuild )
    {
        $osname = "Win";
    }
    elsif ( $installer::globals::isfreebsdbuild )
    {
        $osname = "FreeBSD";
    }
    elsif ( $installer::globals::ismacbuild )
    {
        $osname = "MacOSX";
    }
    else
    {
        $osname = $installer::globals::os;
    }

    # architecture for the download name
    my $architecture = "";
    if ( $installer::globals::issolarissparcbuild )
    {
        $architecture = "Sparc";
    }
    elsif ( $installer::globals::issolarisx86build )
    {
        $architecture = "x86";
    }
    elsif ( $installer::globals::iswindowsbuild )
    {
        if ( $installer::globals::iswin64build )
        {
            $architecture = "x64";
        }
        else
        {
            $architecture = "x86";
        }
    }
    elsif ( $installer::globals::cpuname eq 'INTEL' )
    {
        $architecture = "x86";
    }
    elsif ( $installer::globals::cpuname eq 'POWERPC' )
    {
        $architecture = "PPC";
    }
    elsif ( $installer::globals::cpuname eq 'POWERPC64' )
    {
        $architecture = "PPC64";
    }
    elsif ( $installer::globals::cpuname eq 'X86_64' )
    {
        $architecture = "x86-64";
    }

    # type of content for the download name
    my $contenttype = "";
    if ( $installer::globals::isrpmbuild )
    {
        $contenttype = "rpm";
    }
    elsif ( $installer::globals::isdebbuild )
    {
        $contenttype = "deb";
    }
    elsif ( $installer::globals::packageformat eq "archive" )
    {
        $contenttype = "archive";
    }

    my $filename = $productname . "_" . $versionstring . "_" . $osname . "_" . $architecture . "_" . $contenttype . "_" . $functionality . "_" . $language;

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

    # fdo#67060 - install script is for RPM only
    if ( -e "$installdir/install" && !$installer::globals::isrpmbuild )
    {
        unlink("$installdir/install");
    }

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
    if ( $allvariables->{'PRODUCTEXTENSION'} ) { $extension = $allvariables->{'PRODUCTEXTENSION'}; }
    $extension = lc($extension);
    $downloadname =~ s/\{extension\}/$extension/;

    my $os = "";
    if ( $installer::globals::iswindowsbuild ) { $os = "windows"; }
    elsif ( $installer::globals::issolarissparcbuild ) { $os = "solsparc"; }
    elsif ( $installer::globals::issolarisx86build ) { $os = "solia"; }
    elsif ( $installer::globals::islinuxbuild ) { $os = "linux"; }
    elsif ( $installer::globals::ismacbuild ) { $os = "macosx"; }
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

    if ( $allvariableshashref->{'OOODOWNLOADNAME'} ) { $downloadname = get_download_file_name($languagestringref, $allvariableshashref); }
    else { $downloadname = resolve_variables_in_downloadname($allvariableshashref, $downloadname, $languagestringref); }

    if ( ! $installer::globals::iswindowsbuild )    # Unix specific part
    {

        # getting the path of the getuid.so (only required for Solaris and Linux)
        my $getuidlibrary = "";
        if (( $installer::globals::issolarisbuild ) || ( $installer::globals::islinuxbuild )) { $getuidlibrary = get_path_for_library(); }

        if ( $allvariableshashref->{'OOODOWNLOADNAME'} )
        {
            my $downloadfile = create_tar_gz_file_from_directory($installationdir, $getuidlibrary, $downloaddir, $downloadname);
        }
        else
        {
            # find and read setup script template
            my $scriptfilename = $ENV{'SRCDIR'} . "/setup_native/scripts/downloadscript.sh";

            if (! -f $scriptfilename) { installer::exiter::exit_program("ERROR: Could not find script file $scriptfilename!", "create_download_sets"); }
            my $scriptfile = installer::files::read_file($scriptfilename);

            $infoline = "Found  script file $scriptfilename \n";
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
