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

package installer::windows::msp;

use File::Copy;
use installer::control;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::logger;
use installer::pathanalyzer;
use installer::systemactions;
use installer::windows::admin;
use installer::windows::idtglobal;
use installer::windows::update;

#################################################################################
# Making all required administrative installations
#################################################################################

sub install_installation_sets
{
    my ($installationdir) = @_;

    # Finding the msi database in the new installation set, that is located in $installationdir

    my $msifiles = installer::systemactions::find_file_with_file_extension("msi", $installationdir);

    if ( $#{$msifiles} < 0 ) { installer::exiter::exit_program("ERROR: Did not find msi database in directory $installationdir", "create_msp_patch"); }
    if ( $#{$msifiles} > 0 ) { installer::exiter::exit_program("ERROR: Did find more than one msi database in directory $installationdir", "create_msp_patch"); }

    my $newinstallsetdatabasepath = $installationdir . $installer::globals::separator . ${$msifiles}[0];
    my $oldinstallsetdatabasepath = $installer::globals::updatedatabasepath;

    # Creating temp directory again
    installer::systemactions::create_directory_structure($installer::globals::temppath);

    # Creating old installation directory
    my $dirname = "admin";
    my $installpath = $installer::globals::temppath . $installer::globals::separator . $dirname;
    if ( ! -d $installpath) { installer::systemactions::create_directory($installpath); }

    my $oldinstallpath = $installpath . $installer::globals::separator . "old";
    my $newinstallpath = $installpath . $installer::globals::separator . "new";

    if ( ! -d $oldinstallpath) { installer::systemactions::create_directory($oldinstallpath); }
    if ( ! -d $newinstallpath) { installer::systemactions::create_directory($newinstallpath); }

    my $olddatabase = installer::windows::admin::make_admin_install($oldinstallsetdatabasepath, $oldinstallpath);
    my $newdatabase = installer::windows::admin::make_admin_install($newinstallsetdatabasepath, $newinstallpath);

    if ( $^O =~ /cygwin/i ) {
        $olddatabase = qx{cygpath -w "$olddatabase"};
        $olddatabase =~ s/\s*$//g;
        $newdatabase = qx{cygpath -w "$newdatabase"};
        $newdatabase =~ s/\s*$//g;
    }

    return ($olddatabase, $newdatabase);
}

#################################################################################
# Extracting all tables from a pcp file
#################################################################################

sub extract_all_tables_from_pcpfile
{
    my ($fullpcpfilepath, $workdir) = @_;

    my $msidb = "msidb.exe";    # Has to be in the path
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";
    my $extraslash = "";        # Has to be set for non-ActiveState perl

    my $localfullpcpfile = $fullpcpfilepath;
    my $localworkdir = $workdir;

    if ( $^O =~ /cygwin/i ) {
        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
        $localfullpcpfile =~ s/\//\\\\/g;
        $localworkdir =~ s/\//\\\\/g;
        $extraslash = "\\";
    }
    if ( $^O =~ /linux/i ) {
        $extraslash = "\\";
    }

    # Export of all tables by using "*"

    $systemcall = $msidb . " -d " . $localfullpcpfile . " -f " . $localworkdir . " -e " . $extraslash . "*";
    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program("ERROR: Could not exclude tables from pcp file: $fullpcpfilepath !", "extract_all_tables_from_msidatabase");
    }
    else
    {
        $infoline = "Success: Executed $systemcall successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#################################################################################
# Include tables into a pcp file
#################################################################################

sub include_tables_into_pcpfile
{
    my ($fullpcpfilepath, $workdir, $tables) = @_;

    my $msidb = "msidb.exe";    # Has to be in the path
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";

    # Make all table 8+3 conform
    my $alltables = installer::converter::convert_stringlist_into_array(\$tables, " ");

    for ( my $i = 0; $i <= $#{$alltables}; $i++ )
    {
        my $tablename = ${$alltables}[$i];
        $tablename =~ s/\s*$//;
        my $namelength = length($tablename);
        if ( $namelength > 8 )
        {
            my $newtablename = substr($tablename, 0, 8);    # name, offset, length
            my $oldfile = $workdir . $installer::globals::separator . $tablename . ".idt";
            my $newfile = $workdir . $installer::globals::separator . $newtablename . ".idt";
            if ( -f $newfile ) { unlink $newfile; }
            installer::systemactions::copy_one_file($oldfile, $newfile);
        }
    }

    # Import of tables

    my $localworkdir = $workdir;
    my $localfullpcpfilepath = $fullpcpfilepath;

    if ( $^O =~ /cygwin/i ) {
        # msidb.exe really wants backslashes. (And double escaping because system() expands the string.)
        $localfullpcpfilepath =~ s/\//\\\\/g;
        $localworkdir =~ s/\//\\\\/g;
    }

    my @tables = split(' ', $tables); # I found that msidb from Windows SDK 7.1 did not accept more than one table.
    foreach my $table (@tables)
    {
        $systemcall = $msidb . " -d " . $localfullpcpfilepath . " -f " . $localworkdir . " -i " . $table;

        $returnvalue = system($systemcall);

        $infoline = "Systemcall: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ($returnvalue)
        {
            $infoline = "ERROR: Could not execute $systemcall !\n";
            push( @installer::globals::logfileinfo, $infoline);
            installer::exiter::exit_program("ERROR: Could not include tables into pcp file: $fullpcpfilepath !", "include_tables_into_pcpfile");
        }
        else
        {
            $infoline = "Success: Executed $systemcall successfully!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }
}

#################################################################################
# Calling msimsp.exe
#################################################################################

sub execute_msimsp
{
    my ($fullpcpfilename, $mspfilename, $localmspdir) = @_;

    my $msimsp = "msimsp.exe";  # Has to be in the path
    my $infoline = "";
    my $systemcall = "";
    my $returnvalue = "";
    my $logfilename = $localmspdir . $installer::globals::separator . "msimsp.log";

    # Using a specific temp for each msimsp.exe process
    # Creating temp directory again (should already have happened)
    installer::systemactions::create_directory_structure($installer::globals::temppath);

    # Creating old installation directory
    my $dirname = "msimsptemp";
    my $msimsptemppath = $installer::globals::temppath . $installer::globals::separator . $dirname;
    if ( ! -d $msimsptemppath) { installer::systemactions::create_directory($msimsptemppath); }

    # r:\msvc9p\PlatformSDK\v6.1\bin\msimsp.exe -s c:\patch\hotfix_qfe1.pcp -p c:\patch\patch_ooo3_m2_m3.msp -l c:\patch\patch_ooo3_m2_m3.log

    if ( -f $logfilename ) { unlink $logfilename; }

    my $localfullpcpfilename = $fullpcpfilename;
    my $localmspfilename = $mspfilename;
    my $locallogfilename = $logfilename;
    my $localmsimsptemppath = $msimsptemppath;

    if ( $^O =~ /cygwin/i ) {
        # msimsp.exe really wants backslashes. (And double escaping because system() expands the string.)
        $localfullpcpfilename =~ s/\//\\\\/g;
        $locallogfilename =~ s/\//\\\\/g;

        $localmspfilename =~ s/\\/\\\\/g; # path already contains backslash

        $localmsimsptemppath = qx{cygpath -w "$localmsimsptemppath"};
        $localmsimsptemppath =~ s/\\/\\\\/g;
        $localmsimsptemppath =~ s/\s*$//g;
    }

    $systemcall = $msimsp . " -s " . $localfullpcpfilename . " -p " . $localmspfilename . " -l " . $locallogfilename . " -f " . $localmsimsptemppath;
    installer::logger::print_message( "... $systemcall ...\n" );

    $returnvalue = system($systemcall);

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute $systemcall !\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program("ERROR: Could not execute $systemcall !", "execute_msimsp");
    }
    else
    {
        $infoline = "Success: Executed $systemcall successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $logfilename;
}

####################################################################
# Checking existence and saving all tables, that need to be edited
####################################################################

sub check_and_save_tables
{
    my ($tablelist, $workdir) = @_;

    my $tables = installer::converter::convert_stringlist_into_array(\$tablelist, " ");

    for ( my $i = 0; $i <= $#{$tables}; $i++ )
    {
        my $filename = ${$tables}[$i];
        $filename =~ s/\s*$//;
        my $fullfilename = $workdir . $installer::globals::separator . $filename . ".idt";

        if ( ! -f $fullfilename ) { installer::exiter::exit_program("ERROR: Required idt file could not be found: \"$fullfilename\"!", "check_and_save_tables"); }

        my $savfilename = $fullfilename . ".sav";
        installer::systemactions::copy_one_file($fullfilename, $savfilename);
    }
}

####################################################################
# Setting the languages for the service packs
####################################################################

sub create_langstring
{
    my ( $languagesarrayref ) = @_;

    my $langstring = "";
    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ ) { $langstring = $langstring . "_" . ${$languagesarrayref}[$i]; }

    return $langstring;
}

####################################################################
# Setting the name of the msp database
####################################################################

sub set_mspfilename
{
    my ($allvariables, $mspdir, $languagesarrayref) = @_;

    my $databasename = $allvariables->{'PRODUCTNAME'};
    $databasename = lc($databasename);
    $databasename =~ s/\.//g;
    $databasename =~ s/\-//g;
    $databasename =~ s/\s//g;

    if ( $allvariables->{'MSPPRODUCTVERSION'} ) { $databasename = $databasename . $allvariables->{'MSPPRODUCTVERSION'}; }

    # possibility to overwrite the name with variable DATABASENAME
    # if ( $allvariables->{'DATABASENAME'} ) { $databasename = $allvariables->{'DATABASENAME'}; }

    # Adding patch info to database name
    # if ( $installer::globals::buildid ) { $databasename = $databasename . "_" . $installer::globals::buildid; }

    # if ( $allvariables->{'VENDORPATCHVERSION'} ) { $databasename = $databasename . "_" . $allvariables->{'VENDORPATCHVERSION'}; }


    if (( $allvariables->{'SERVICEPACK'} ) && ( $allvariables->{'SERVICEPACK'} == 1 ))
    {
        my $windowspatchlevel = 0;
        if ( $allvariables->{'MSPPATCHLEVEL'} ) { $windowspatchlevel = $allvariables->{'MSPPATCHLEVEL'}; }
        $databasename = $databasename . "_servicepack_" . $windowspatchlevel;
        my $languagestring = create_langstring($languagesarrayref);
        $databasename = $databasename . $languagestring;
    }
    else
    {
        my $hotfixaddon = "hotfix_";
        $hotfixaddon = $hotfixaddon . $installer::globals::buildid;
        my $cwsname = "";
        if ( $allvariables->{'OVERWRITE_CWSNAME'} ) { $hotfixaddon = $allvariables->{'OVERWRITE_CWSNAME'}; }
        $databasename = $databasename . "_" . $hotfixaddon;
    }

    $databasename = $databasename . ".msp";

    my $fullmspname = $mspdir . $installer::globals::separator . $databasename;

    if ( $^O =~ /cygwin/i ) { $fullmspname =~ s/\//\\/g; }

    return $fullmspname;
}

####################################################################
# Editing table Properties
####################################################################

sub change_properties_table
{
    my ($localmspdir, $mspfilename) = @_;

    my $infoline = "Changing content of table \"Properties\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = $localmspdir . $installer::globals::separator . "Properties.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" !", "change_properties_table"); }

    my $filecontent = installer::files::read_file($filename);


    my $guidref = installer::windows::msiglobal::get_guid_list(1, 1);
    ${$guidref}[0] =~ s/\s*$//;     # removing ending spaces
    my $patchcode = "\{" . ${$guidref}[0] . "\}";

    # Setting "PatchOutputPath"
    my $found_patchoutputpath = 0;
    my $found_patchguid = 0;

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( ${$filecontent}[$i] =~ /^\s*PatchOutputPath\t(.*?)\s*$/ )
        {
            my $oldvalue = $1;
            ${$filecontent}[$i] =~ s/\Q$oldvalue\E/$mspfilename/;
            $found_patchoutputpath = 1;
        }

        if ( ${$filecontent}[$i] =~ /^\s*PatchGUID\t(.*?)\s*$/ )
        {
            my $oldvalue = $1;
            ${$filecontent}[$i] =~ s/\Q$oldvalue\E/$patchcode/;
            $found_patchguid = 1;
        }
    }

    if ( ! $found_patchoutputpath )
    {
        my $newline = "PatchOutputPath\t$mspfilename\n";
        push(@{$filecontent}, $newline);
    }

    if ( ! $found_patchguid )
    {
        my $newline = "PatchGUID\t$patchcode\n";
        push(@{$filecontent}, $newline);
    }

    # saving file
    installer::files::save_file($filename, $filecontent);
}

####################################################################
# Editing table TargetImages
####################################################################

sub change_targetimages_table
{
    my ($localmspdir, $olddatabase) = @_;

    my $infoline = "Changing content of table \"TargetImages\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = $localmspdir . $installer::globals::separator . "TargetImages.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" !", "change_targetimages_table"); }

    my $filecontent = installer::files::read_file($filename);
    my @newcontent = ();

    # Copying the header
    for ( my $i = 0; $i <= $#{$filecontent}; $i++ ) { if ( $i < 3 ) { push(@newcontent, ${$filecontent}[$i]); } }

    #Adding all targets
    my $newline = "T1\t$olddatabase\t\tU1\t1\t0x00000922\t1\n";
    push(@newcontent, $newline);

    # saving file
    installer::files::save_file($filename, \@newcontent);
}

####################################################################
# Editing table UpgradedImages
####################################################################

sub change_upgradedimages_table
{
    my ($localmspdir, $newdatabase) = @_;

    my $infoline = "Changing content of table \"UpgradedImages\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = $localmspdir . $installer::globals::separator . "UpgradedImages.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" !", "change_upgradedimages_table"); }

    my $filecontent = installer::files::read_file($filename);
    my @newcontent = ();

    # Copying the header
    for ( my $i = 0; $i <= $#{$filecontent}; $i++ ) { if ( $i < 3 ) { push(@newcontent, ${$filecontent}[$i]); } }

    # Syntax: Upgraded MsiPath PatchMsiPath SymbolPaths Family

    # default values
    my $upgraded = "U1";
    my $msipath = $newdatabase;
    my $patchmsipath = "";
    my $symbolpaths = "";
    my $family = "22334455";

    if ( $#{$filecontent} >= 3 )
    {
        my $line = ${$filecontent}[3];
        if ( $line =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            $upgraded = $1;
            $patchmsipath = $3;
            $symbolpaths = $4;
            $family = $5;
        }
    }

    #Adding sequence line, saving PatchFamily
    my $newline = "$upgraded\t$msipath\t$patchmsipath\t$symbolpaths\t$family\n";
    push(@newcontent, $newline);

    # saving file
    installer::files::save_file($filename, \@newcontent);
}

####################################################################
# Editing table ImageFamilies
####################################################################

sub change_imagefamilies_table
{
    my ($localmspdir) = @_;

    my $infoline = "Changing content of table \"ImageFamilies\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = $localmspdir . $installer::globals::separator . "ImageFamilies.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" !", "change_imagefamilies_table"); }

    my $filecontent = installer::files::read_file($filename);
    my @newcontent = ();

    # Copying the header
    for ( my $i = 0; $i <= $#{$filecontent}; $i++ ) { if ( $i < 3 ) { push(@newcontent, ${$filecontent}[$i]); } }

    # Syntax: Family MediaSrcPropName MediaDiskId FileSequenceStart DiskPrompt VolumeLabel
    # "FileSequenceStart has to be set

    # Default values:

    my $family = "22334455";
    my $mediasrcpropname = "MediaSrcPropName";
    my $mediadiskid = "2";
    my $filesequencestart = get_filesequencestart();
    my $diskprompt = "";
    my $volumelabel = "";

    if ( $#{$filecontent} >= 3 )
    {
        my $line = ${$filecontent}[3];
        if ( $line =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            $family = $1;
            $mediasrcpropname = $2;
            $mediadiskid = $3;
            $diskprompt = $5;
            $volumelabel = $6;
        }
    }

    #Adding sequence line
    my $newline = "$family\t$mediasrcpropname\t$mediadiskid\t$filesequencestart\t$diskprompt\t$volumelabel\n";
    push(@newcontent, $newline);

    # saving file
    installer::files::save_file($filename, \@newcontent);
}

####################################################################
# Setting start sequence for patch
####################################################################

sub get_filesequencestart
{
    my $sequence = 1000;  # default

    if ( $installer::globals::updatelastsequence ) { $sequence = $installer::globals::updatelastsequence + 500; }

    return $sequence;
}

####################################################################
# Setting time value into pcp file
# Format mm/dd/yyyy hh:mm
####################################################################

sub get_patchtime_value
{
    # Syntax: 8/8/2008 11:55
    my $minute = (localtime())[1];
    my $hour = (localtime())[2];
    my $day = (localtime())[3];
    my $month = (localtime())[4];
    my $year = 1900 + (localtime())[5];

    $month++; # zero based month
    if ( $minute  < 10 ) { $minute = "0" . $minute; }
    if ( $hour  < 10 ) { $hour = "0" . $hour; }

    my $timestring = $month . "/" . $day . "/" . $year . " " . $hour . ":" . $minute;

    return $timestring;
}

#################################################################################
# Checking, if this is the correct database.
#################################################################################

sub correct_langs
{
    my ($langs, $languagestringref) = @_;

    my $correct_langs = 0;

    # Comparing $langs with $languagestringref

    my $langlisthash = installer::converter::convert_stringlist_into_hash(\$langs, ",");
    my $langstringhash = installer::converter::convert_stringlist_into_hash($languagestringref, "_");

    my $not_included = 0;
    foreach my $onelang ( keys %{$langlisthash} )
    {
        if ( ! exists($langstringhash->{$onelang}) )
        {
            $not_included = 1;
            last;
        }
    }

    if ( ! $not_included )
    {
        foreach my $onelanguage ( keys %{$langstringhash} )
        {
            if ( ! exists($langlisthash->{$onelanguage}) )
            {
                $not_included = 1;
                last;
            }
        }

        if ( ! $not_included ) { $correct_langs = 1; }
    }

    return $correct_langs;
}

#################################################################################
# Searching for the path to the reference database for this special product.
#################################################################################

sub get_patchid_from_list
{
    my ($filecontent, $languagestringref, $filename) = @_;

    my $patchid = "";

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $line = ${$filecontent}[$i];
        if ( $line =~ /^\s*$/ ) { next; } # empty line
        if ( $line =~ /^\s*\#/ ) { next; } # comment line

        if ( $line =~ /^\s*(.+?)\s*=\s*(.+?)\s*$/ )
        {
            my $langs = $1;
            my $localpatchid = $2;

            if ( correct_langs($langs, $languagestringref) )
            {
                $patchid = $localpatchid;
                last;
            }
        }
        else
        {
            installer::exiter::exit_program("ERROR: Wrong syntax in file: $filename! Line: \"$line\"", "get_patchid_from_list");
        }
    }

    return $patchid;
}

####################################################################
# Editing table PatchMetadata
####################################################################

sub change_patchmetadata_table
{
    my ($localmspdir, $allvariables, $languagestringref) = @_;

    my $infoline = "Changing content of table \"PatchMetadata\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = $localmspdir . $installer::globals::separator . "PatchMetadata.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" !", "change_patchmetadata_table"); }

    my $filecontent = installer::files::read_file($filename);
    my @newcontent = ();

    # Syntax: Company Property Value
    # Interesting properties: "Classification" and "CreationTimeUTC"

    my $classification_set = 0;
    my $creationtime_set = 0;
    my $targetproductname_set = 0;
    my $manufacturer_set = 0;
    my $displayname_set = 0;
    my $description_set = 0;
    my $allowremoval_set = 0;

    my $defaultcompany = "";

    my $classificationstring = "Classification";
    my $classificationvalue = "Hotfix";
    if (( $allvariables->{'SERVICEPACK'} ) && ( $allvariables->{'SERVICEPACK'} == 1 )) { $classificationvalue = "ServicePack"; }

    my $allowremovalstring = "AllowRemoval";
    my $allowremovalvalue = "1";
    if (( exists($allvariables->{'MSPALLOWREMOVAL'}) ) && ( $allvariables->{'MSPALLOWREMOVAL'} == 0 )) { $allowremovalvalue = 0; }

    my $timestring = "CreationTimeUTC";
    # Syntax: 8/8/2008 11:55
    my $timevalue = get_patchtime_value();

    my $targetproductnamestring = "TargetProductName";
    my $targetproductnamevalue = $allvariables->{'PRODUCTNAME'};
    if ( $allvariables->{'PROPERTYTABLEPRODUCTNAME'} ) { $targetproductnamevalue = $allvariables->{'PROPERTYTABLEPRODUCTNAME'}; }

    my $manufacturerstring = "ManufacturerName";
    my $manufacturervalue = $ENV{'OOO_VENDOR'};
    if ( $installer::globals::longmanufacturer ) { $manufacturervalue = $installer::globals::longmanufacturer; }

    my $displaynamestring = "DisplayName";
    my $descriptionstring = "Description";
    my $displaynamevalue = "";
    my $descriptionvalue = "";

    my $base = $allvariables->{'PRODUCTNAME'} . " " . $allvariables->{'PRODUCTVERSION'};
    if ( $installer::globals::languagepack || $installer::globals::helppack ) { $base = $targetproductnamevalue; }

    my $windowspatchlevel = 0;
    if ( $allvariables->{'WINDOWSPATCHLEVEL'} ) { $windowspatchlevel = $allvariables->{'WINDOWSPATCHLEVEL'}; }

    my $displayaddon = "";
    if ( $allvariables->{'PATCHDISPLAYADDON'} ) { $displayaddon = $allvariables->{'PATCHDISPLAYADDON'}; }

    my $patchsequence = get_patchsequence($allvariables);

    if (( $allvariables->{'SERVICEPACK'} ) && ( $allvariables->{'SERVICEPACK'} == 1 ))
    {
        $displaynamevalue = $base . " ServicePack " . $windowspatchlevel . " " . $patchsequence . " Build: " . $installer::globals::buildid;
        $descriptionvalue = $base . " ServicePack " . $windowspatchlevel . " " . $patchsequence . " Build: " . $installer::globals::buildid;
    }
    else
    {
        $displaynamevalue = $base . " Hotfix " . $displayaddon . " " . $patchsequence . " Build: " . $installer::globals::buildid;
        $descriptionvalue = $base . " Hotfix " . $displayaddon . " " . $patchsequence . " Build: " . $installer::globals::buildid;
        $displaynamevalue =~ s/    / /g;
        $descriptionvalue =~ s/    / /g;
        $displaynamevalue =~ s/   / /g;
        $descriptionvalue =~ s/   / /g;
        $displaynamevalue =~ s/  / /g;
        $descriptionvalue =~ s/  / /g;
    }

    if ( $allvariables->{'MSPPATCHNAMELIST'} )
    {
        my $patchnamelistfile = $allvariables->{'MSPPATCHNAMELIST'};
        $patchnamelistfile = $installer::globals::idttemplatepath  . $installer::globals::separator . $patchnamelistfile;
        if ( ! -f $patchnamelistfile ) { installer::exiter::exit_program("ERROR: Could not find file \"$patchnamelistfile\".", "change_patchmetadata_table"); }
        my $filecontent = installer::files::read_file($patchnamelistfile);

        # Get name and path of reference database
        my $patchid = get_patchid_from_list($filecontent, $languagestringref, $patchnamelistfile);

        if ( $patchid eq "" ) { installer::exiter::exit_program("ERROR: Could not find file patchid in file \"$patchnamelistfile\" for language(s) \"$$languagestringref\".", "change_patchmetadata_table"); }

        # Setting language specific patch id
    }

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        if ( ${$filecontent}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $company = $1;
            my $property = $2;
            my $value = $3;

            if ( $property eq $classificationstring )
            {
                ${$filecontent}[$i] = "$company\t$property\t$classificationvalue\n";
                $classification_set = 1;
            }

            if ( $property eq $allowremovalstring )
            {
                ${$filecontent}[$i] = "$company\t$property\t$allowremovalvalue\n";
                $allowremoval_set = 1;
            }

            if ( $property eq $timestring )
            {
                ${$filecontent}[$i] = "$company\t$property\t$timevalue\n";
                $creationtime_set = 1;
            }

            if ( $property eq $targetproductnamestring )
            {
                ${$filecontent}[$i] = "$company\t$property\t$targetproductnamevalue\n";
                $targetproductname_set = 1;
            }

            if ( $property eq $manufacturerstring )
            {
                ${$filecontent}[$i] = "$company\t$property\t$manufacturervalue\n";
                $manufacturer_set = 1;
            }

            if ( $property eq $displaynamestring )
            {
                ${$filecontent}[$i] = "$company\t$property\t$displaynamevalue\n";
                $displayname_set = 1;
            }

            if ( $property eq $descriptionstring )
            {
                ${$filecontent}[$i] = "$company\t$property\t$descriptionvalue\n";
                $description_set = 1;
            }
        }

        push(@newcontent, ${$filecontent}[$i]);
    }

    if ( ! $classification_set )
    {
        my $line = "$defaultcompany\t$classificationstring\t$classificationvalue\n";
        push(@newcontent, $line);
    }

    if ( ! $allowremoval_set )
    {
        my $line = "$defaultcompany\t$classificationstring\t$allowremovalvalue\n";
        push(@newcontent, $line);
    }

    if ( ! $allowremoval_set )
    {
        my $line = "$defaultcompany\t$classificationstring\t$allowremovalvalue\n";
        push(@newcontent, $line);
    }

    if ( ! $creationtime_set )
    {
        my $line = "$defaultcompany\t$timestring\t$timevalue\n";
        push(@newcontent, $line);
    }

    if ( ! $targetproductname_set )
    {
        my $line = "$defaultcompany\t$targetproductnamestring\t$targetproductnamevalue\n";
        push(@newcontent, $line);
    }

    if ( ! $manufacturer_set )
    {
        my $line = "$defaultcompany\t$manufacturerstring\t$manufacturervalue\n";
        push(@newcontent, $line);
    }

    if ( ! $displayname_set )
    {
        my $line = "$defaultcompany\t$displaynamestring\t$displaynamevalue\n";
        push(@newcontent, $line);
    }

    if ( ! $description_set )
    {
        my $line = "$defaultcompany\t$descriptionstring\t$descriptionvalue\n";
        push(@newcontent, $line);
    }

    # saving file
    installer::files::save_file($filename, \@newcontent);
}

####################################################################
# Editing table PatchSequence
####################################################################

sub change_patchsequence_table
{
    my ($localmspdir, $allvariables) = @_;

    my $infoline = "Changing content of table \"PatchSequence\"\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $filename = $localmspdir . $installer::globals::separator . "PatchSequence.idt";
    if ( ! -f $filename ) { installer::exiter::exit_program("ERROR: Could not find file \"$filename\" !", "change_patchsequence_table"); }

    my $filecontent = installer::files::read_file($filename);
    my @newcontent = ();

    # Copying the header
    for ( my $i = 0; $i <= $#{$filecontent}; $i++ ) { if ( $i < 3 ) { push(@newcontent, ${$filecontent}[$i]); } }

    # Syntax: PatchFamily Target Sequence Supersede

    my $patchfamily = "SO";
    my $target = "";
    my $patchsequence = get_patchsequence($allvariables);
    my $supersede = get_supersede($allvariables);

    if ( $#{$filecontent} >= 3 )
    {
        my $line = ${$filecontent}[3];
        if ( $line =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\s$/ )
        {
            $patchfamily = $1;
            $target = $2;
        }
    }

    #Adding sequence line, saving PatchFamily
    my $newline = "$patchfamily\t$target\t$patchsequence\t$supersede\n";
    push(@newcontent, $newline);

    # saving file
    installer::files::save_file($filename, \@newcontent);
}

####################################################################
# Setting supersede, "0" for Hotfixes, "1" for ServicePack
####################################################################

sub get_supersede
{
    my ( $allvariables ) = @_;

    my $supersede = 0;  # if not defined, this is a Hotfix

    if (( $allvariables->{'SERVICEPACK'} ) && ( $allvariables->{'SERVICEPACK'} == 1 )) { $supersede = 1; }

    return $supersede;
}

####################################################################
# Setting the sequence of the patch
####################################################################

sub get_patchsequence
{
    my ( $allvariables ) = @_;

    my $patchsequence = "1.0";

    if ( ! $allvariables->{'PACKAGEVERSION'} ) { installer::exiter::exit_program("ERROR: PACKAGEVERSION must be set for msp patch creation!", "get_patchsequence"); }

    my $packageversion = $allvariables->{'PACKAGEVERSION'};

    if ( $packageversion =~ /^\s*(\d+)\.(\d+)\.(\d+)\s*$/ )
    {
        my $major = $1;
        my $minor = $2;
        my $micro = $3;
        my $concat = 100 * $minor + $micro;
        $packageversion = $major . "\." . $concat;
    }
    my $vendornumber = 0;
    if ( $allvariables->{'VENDORPATCHVERSION'} ) { $vendornumber = $allvariables->{'VENDORPATCHVERSION'}; }
    $patchsequence = $packageversion . "\." . $installer::globals::buildid . "\." . $vendornumber;

    if ( $allvariables->{'PATCHSEQUENCE'} ) { $patchsequence = $allvariables->{'PATCHSEQUENCE'}; }

    return $patchsequence;
}

####################################################################
# Editing all tables from pcp file, that need to be edited
####################################################################

sub edit_tables
{
    my ($tablelist, $localmspdir, $olddatabase, $newdatabase, $mspfilename, $allvariables, $languagestringref) = @_;

    # table list contains:  my $tablelist = "Properties TargetImages UpgradedImages ImageFamilies PatchMetadata PatchSequence";

    change_properties_table($localmspdir, $mspfilename);
    change_targetimages_table($localmspdir, $olddatabase);
    change_upgradedimages_table($localmspdir, $newdatabase);
    change_imagefamilies_table($localmspdir);
    change_patchmetadata_table($localmspdir, $allvariables, $languagestringref);
    change_patchsequence_table($localmspdir, $allvariables);
}

#################################################################################
# Checking, if this is the correct database.
#################################################################################

sub correct_patch
{
    my ($product, $pro, $langs, $languagestringref) = @_;

    my $correct_patch = 0;

    # Comparing $product with $installer::globals::product and
    # $pro with $installer::globals::pro and
    # $langs with $languagestringref

    my $product_is_good = 0;

    my $localproduct = $installer::globals::product;
    if ( $installer::globals::languagepack ) { $localproduct = $localproduct . "LanguagePack"; }
    elsif ( $installer::globals::helppack ) { $localproduct = $localproduct . "HelpPack"; }

    if ( $product eq $localproduct ) { $product_is_good = 1; }

    if ( $product_is_good )
    {
        my $pro_is_good = 0;

        if ((( $pro eq "pro" ) && ( $installer::globals::pro )) || (( $pro eq "nonpro" ) && ( ! $installer::globals::pro ))) { $pro_is_good = 1; }

        if ( $pro_is_good )
        {
            my $langlisthash = installer::converter::convert_stringlist_into_hash(\$langs, ",");
            my $langstringhash = installer::converter::convert_stringlist_into_hash($languagestringref, "_");

            my $not_included = 0;
            foreach my $onelang ( keys %{$langlisthash} )
            {
                if ( ! exists($langstringhash->{$onelang}) )
                {
                    $not_included = 1;
                    last;
                }
            }

            if ( ! $not_included )
            {
                foreach my $onelanguage ( keys %{$langstringhash} )
                {
                    if ( ! exists($langlisthash->{$onelanguage}) )
                    {
                        $not_included = 1;
                        last;
                    }
                }

                if ( ! $not_included ) { $correct_patch = 1; }
            }
        }
    }

    return $correct_patch;
}

#################################################################################
# Searching for the path to the required patch for this special product.
#################################################################################

sub get_requiredpatchfile_from_list
{
    my ($filecontent, $languagestringref, $filename) = @_;

    my $patchpath = "";

    for ( my $i = 0; $i <= $#{$filecontent}; $i++ )
    {
        my $line = ${$filecontent}[$i];
        if ( $line =~ /^\s*$/ ) { next; } # empty line
        if ( $line =~ /^\s*\#/ ) { next; } # comment line

        if ( $line =~ /^\s*(.+?)\s*\t+\s*(.+?)\s*\t+\s*(.+?)\s*\t+\s*(.+?)\s*$/ )
        {
            my $product = $1;
            my $pro = $2;
            my $langs = $3;
            my $path = $4;

            if (( $pro ne "pro" ) && ( $pro ne "nonpro" )) { installer::exiter::exit_program("ERROR: Wrong syntax in file: $filename. Only \"pro\" or \"nonpro\" allowed in column 1! Line: \"$line\"", "get_databasename_from_list"); }

            if ( correct_patch($product, $pro, $langs, $languagestringref) )
            {
                $patchpath = $path;
                last;
            }
        }
        else
        {
            installer::exiter::exit_program("ERROR: Wrong syntax in file: $filename! Line: \"$line\"", "get_requiredpatchfile_from_list");
        }
    }

    return $patchpath;
}

##################################################################
# Converting unicode file to ascii
# to be more precise: uft-16 little endian to ascii
##################################################################

sub convert_unicode_to_ascii
{
    my ( $filename ) = @_;

    my @localfile = ();

    my $savfilename = $filename . "_before.unicode";
    installer::systemactions::copy_one_file($filename, $savfilename);

    open( IN, "<:encoding(UTF16-LE)", $filename ) || installer::exiter::exit_program("ERROR: Cannot open file $filename for reading", "convert_unicode_to_ascii");
    while ( $line = <IN> ) {
        push @localfile, $line;
    }
    close( IN );

    if ( open( OUT, ">", $filename ) )
    {
        print OUT @localfile;
        close(OUT);
    }
}

####################################################################
# Analyzing the log file created by msimsp.exe to find all
# files included into the patch.
####################################################################

sub analyze_msimsp_logfile
{
    my ($logfile, $filesarray) = @_;

    # Reading log file after converting from utf-16 (LE) to ascii
    convert_unicode_to_ascii($logfile);
    my $logfilecontent = installer::files::read_file($logfile);

    # Creating hash from $filesarray: unique file name -> destination of file
    my %filehash = ();
    my %destinationcollector = ();

    for ( my $i = 0; $i <= $#{$filesarray}; $i++ )
    {
        my $onefile = ${$filesarray}[$i];

        # Only collecting files with "uniquename" and "destination"
        if (( exists($onefile->{'uniquename'}) ) && ( exists($onefile->{'uniquename'}) ))
        {
            my $uniquefilename = $onefile->{'uniquename'};
            my $destpath = $onefile->{'destination'};
            $filehash{$uniquefilename} = $destpath;
        }
    }

    # Analyzing log file of msimsp.exe, finding all changed files
    # and searching all destinations of unique file names.
    # Content in log file: "INFO File Key: <file key> is modified"
    # Collecting content in @installer::globals::patchfilecollector

    for ( my $i = 0; $i <= $#{$logfilecontent}; $i++ )
    {
        if ( ${$logfilecontent}[$i] =~ /Key\:\s*(.*?) is modified\s*$/ )
        {
            my $filekey = $1;
            if ( exists($filehash{$filekey}) ) { $destinationcollector{$filehash{$filekey}} = 1; }
            else { installer::exiter::exit_program("ERROR: Could not find file key \"$filekey\" in file collector.", "analyze_msimsp_logfile"); }
        }
    }

    foreach my $onedest ( sort keys %destinationcollector ) { push(@installer::globals::patchfilecollector, "$onedest\n"); }

}

####################################################################
# Creating msp patch files for Windows
####################################################################

sub create_msp_patch
{
    my ($installationdir, $includepatharrayref, $allvariables, $languagestringref, $languagesarrayref, $filesarray) = @_;

    my $force = 1; # print this message even in 'quiet' mode
    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating msp installation set ...\n", $force );
    installer::logger::print_message( "******************************************\n" );

    $installer::globals::creating_windows_installer_patch = 1;

    my @needed_files = ("msimsp.exe");  # only required for patch creation process
    installer::control::check_needed_files_in_path(\@needed_files);

    installer::logger::include_header_into_logfile("Creating msp installation sets:");

    my $firstdir = $installationdir;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$firstdir);

    my $lastdir = $installationdir;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$lastdir);

    if ( $lastdir =~ /\./ ) { $lastdir =~ s/\./_msp_inprogress\./ }
    else { $lastdir = $lastdir . "_msp_inprogress"; }

    # Removing existing directory "_native_packed_inprogress" and "_native_packed_witherror" and "_native_packed"

    my $mspdir = $firstdir . $lastdir;
    if ( -d $mspdir ) { installer::systemactions::remove_complete_directory($mspdir); }

    my $olddir = $mspdir;
    $olddir =~ s/_inprogress/_witherror/;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    $olddir = $mspdir;
    $olddir =~ s/_inprogress//;
    if ( -d $olddir ) { installer::systemactions::remove_complete_directory($olddir); }

    # Creating the new directory for new installation set
    installer::systemactions::create_directory($mspdir);

    $installer::globals::saveinstalldir = $mspdir;

    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Starting product installation");

    # Installing both installation sets
    installer::logger::print_message( "... installing products ...\n" );
    my ($olddatabase, $newdatabase) = install_installation_sets($installationdir);

    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Starting pcp file creation");

    # Create pcp file
    installer::logger::print_message( "... creating pcp file ...\n" );

    my $localmspdir = installer::systemactions::create_directories("msp", $languagestringref);

    if ( ! $allvariables->{'PCPFILENAME'} ) { installer::exiter::exit_program("ERROR: Property \"PCPFILENAME\" has to be defined.", "create_msp_patch"); }
    my $pcpfilename = $allvariables->{'PCPFILENAME'};

    if ( $installer::globals::languagepack ) { $pcpfilename =~ s/.pcp\s*$/languagepack.pcp/; }
    elsif ( $installer::globals::helppack ) { $pcpfilename =~ s/.pcp\s*$/helppack.pcp/; }

    # Searching the pcp file in the include paths
    my $fullpcpfilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$pcpfilename, $includepatharrayref, 1);
    if ( $$fullpcpfilenameref eq "" ) { installer::exiter::exit_program("ERROR: pcp file not found: $pcpfilename !", "create_msp_patch"); }
    my $fullpcpfilenamesource = $$fullpcpfilenameref;

    # Copying pcp file
    my $fullpcpfilename = $localmspdir . $installer::globals::separator . $pcpfilename;
    installer::systemactions::copy_one_file($fullpcpfilenamesource, $fullpcpfilename);

    #  a. Extracting tables from msi database: msidb.exe -d <msifile> -f <directory> -e File Media, ...
    #  b. Changing content of msi database in tables: File, Media, Directory, FeatureComponent
    #  c. Including tables into msi database: msidb.exe -d <msifile> -f <directory> -i File Media, ...

    # Unpacking tables from pcp file
    extract_all_tables_from_pcpfile($fullpcpfilename, $localmspdir);

    # Tables, that need to be edited
    my $tablelist = "Properties TargetImages UpgradedImages ImageFamilies PatchMetadata PatchSequence"; # required tables

    # Saving all tables
    check_and_save_tables($tablelist, $localmspdir);

    # Setting the name of the new msp file
    my $mspfilename = set_mspfilename($allvariables, $mspdir, $languagesarrayref);

    # Editing tables
    edit_tables($tablelist, $localmspdir, $olddatabase, $newdatabase, $mspfilename, $allvariables, $languagestringref);

    # Adding edited tables into pcp file
    include_tables_into_pcpfile($fullpcpfilename, $localmspdir, $tablelist);

    # Start msimsp.exe
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Starting msimsp.exe");
    my $msimsplogfile = execute_msimsp($fullpcpfilename, $mspfilename, $localmspdir);

    # Copy final installation set next to msp file
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Copying installation set");
    installer::logger::print_message( "... copying installation set ...\n" );

    my $oldinstallationsetpath = $installer::globals::updatedatabasepath;

    if ( $^O =~ /cygwin/i ) { $oldinstallationsetpath =~ s/\\/\//g; }

    installer::pathanalyzer::get_path_from_fullqualifiedname(\$oldinstallationsetpath);
    installer::systemactions::copy_complete_directory($oldinstallationsetpath, $mspdir);

    # Copying additional patches into the installation set, if required
    if (( $allvariables->{'ADDITIONALREQUIREDPATCHES'} ) && ( $allvariables->{'ADDITIONALREQUIREDPATCHES'} ne "" ) && ( ! $installer::globals::languagepack ) && ( ! $installer::globals::helppack ))
    {
        my $filename = $allvariables->{'ADDITIONALREQUIREDPATCHES'};

        my $fullfilenameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$filename, $includepatharrayref, 1);
        if ( $$fullfilenameref eq "" ) { installer::exiter::exit_program("ERROR: Could not find file with required patches, although it is defined: $filename !", "create_msp_patch"); }
        my $fullfilename = $$fullfilenameref;

        # Reading list file
        my $listfile = installer::files::read_file($fullfilename);

        # Get name and path of reference database
        my $requiredpatchfile = get_requiredpatchfile_from_list($listfile, $languagestringref, $fullfilename);
        if ( $requiredpatchfile eq "" ) { installer::exiter::exit_program("ERROR: Could not find path to required patch in file $fullfilename for language(s) $$languagestringref!", "create_msp_patch"); }

        # Copying patch file
        installer::systemactions::copy_one_file($requiredpatchfile, $mspdir);
        # my $infoline = "Copy $requiredpatchfile to $mspdir\n";
        # push( @installer::globals::logfileinfo, $infoline);
    }

    # Find all files included into the patch
    # Analyzing the msimsp log file $msimsplogfile
    analyze_msimsp_logfile($msimsplogfile, $filesarray);

    # Done
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: msp creation done");

    return $mspdir;
}

1;
