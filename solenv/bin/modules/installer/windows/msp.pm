#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: msp.pm,v $
#
# $Revision: 1.1.2.4 $
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

    # Export of all tables by using "*"

    $systemcall = $msidb . " -d " . $fullpcpfilepath . " -f " . $workdir . " -e \*";
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

    $systemcall = $msidb . " -d " . $fullpcpfilepath . " -f " . $workdir . " -i " . $tables;

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

    # r:\msvc9p\PlatformSDK\v6.1\bin\msimsp.exe -s c:\patch\hotfix_qfe1.pcp -p c:\patch\patch_ooo3_m2_m3.msp -l c:\patch\patch_ooo3_m2_m3.log

    $systemcall = $msimsp . " -s " . $fullpcpfilename . " -p " . $mspfilename . " -l " . $logfilename;
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
# Setting the name of the msp database
####################################################################

sub set_mspfilename
{
    my ($allvariables, $mspdir) = @_;

    my $databasename = $allvariables->{'PRODUCTNAME'} . $allvariables->{'PRODUCTVERSION'};
    $databasename = lc($databasename);
    $databasename =~ s/\.//g;
    $databasename =~ s/\-//g;
    $databasename =~ s/\s//g;

    # possibility to overwrite the name with variable DATABASENAME
    if ( $allvariables->{'DATABASENAME'} ) { $databasename = $allvariables->{'DATABASENAME'}; }

    # Adding patch info to database name
    if ( $installer::globals::buildid ) { $databasename = $databasename . "_" . $installer::globals::buildid; }

    if ( $allvariables->{'VENDORPATCHVERSION'} ) { $databasename = $databasename . "_" . $allvariables->{'VENDORPATCHVERSION'}; }

    $databasename = $databasename . ".msp";

    my $fullmspname = $mspdir . $installer::globals::separator . $databasename;

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

    my $timestring = $month . "/" . $day . "/" . $year . " " . $hour . ":" . $minute;

    return $timestring;
}

####################################################################
# Editing table PatchMetadata
####################################################################

sub change_patchmetadata_table
{
    my ($localmspdir, $allvariables) = @_;

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

    my $defaultcompany = "";

    my $classificationstring = "Classification";
    my $classificationvalue = "Hotfix";
    if (( $allvariables->{'SERVICEPACK'} ) && ( $allvariables->{'SERVICEPACK'} == 1 )) { $classificationvalue = "ServicePack"; }

    my $timestring = "CreationTimeUTC";
    # Syntax: 8/8/2008 11:55
    my $timevalue = get_patchtime_value();

    my $targetproductnamestring = "TargetProductName";
    my $targetproductnamevalue = $allvariables->{'PRODUCTNAME'};
    if ( $allvariables->{'PROPERTYTABLEPRODUCTNAME'} ) { $targetproductnamevalue = $allvariables->{'PROPERTYTABLEPRODUCTNAME'}; }

    my $manufacturerstring = "ManufacturerName";
    my $manufacturervalue = "OpenOffice.org";
    if ( $installer::globals::longmanufacturer ) { $manufacturervalue = $installer::globals::longmanufacturer; }

    my $displaynamestring = "DisplayName";
    my $descriptionstring = "Description";
    my $displaynamevalue = "";
    my $descriptionvalue = "";

    my $base = $allvariables->{'PRODUCTNAME'} . " " . $allvariables->{'PRODUCTVERSION'};
    if ( $installer::globals::languagepack ) { $base = $targetproductnamevalue; }

    my $windowspatchlevel = 0;
    if ( $allvariables->{'WINDOWSPATCHLEVEL'} ) { $windowspatchlevel = $allvariables->{'WINDOWSPATCHLEVEL'}; }

    if (( $allvariables->{'SERVICEPACK'} ) && ( $allvariables->{'SERVICEPACK'} == 1 ))
    {
        $displaynamevalue = $base . " Product Update " . $windowspatchlevel;
        $descriptionvalue = $base . " Product Update " . $windowspatchlevel . " Build: " . $installer::globals::buildid;
    }
    else
    {
        $displaynamevalue = $base . " Hotfix " . " Build: " . $installer::globals::buildid;
        $descriptionvalue = $base . " Hotfix " . " Build: " . $installer::globals::buildid;
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

    return $patchsequence;
}

####################################################################
# Editing all tables from pcp file, that need to be edited
####################################################################

sub edit_tables
{
    my ($tablelist, $localmspdir, $olddatabase, $newdatabase, $mspfilename, $allvariables) = @_;

    # table list contains:  my $tablelist = "Properties TargetImages UpgradedImages ImageFamilies PatchMetadata PatchSequence";

    change_properties_table($localmspdir, $mspfilename);
    change_targetimages_table($localmspdir, $olddatabase);
    change_upgradedimages_table($localmspdir, $newdatabase);
    change_imagefamilies_table($localmspdir);
    change_patchmetadata_table($localmspdir, $allvariables);
    change_patchsequence_table($localmspdir, $allvariables);
}

####################################################################
# Creating msp patch files for Windows
####################################################################

sub create_msp_patch
{
    my ($installationdir, $includepatharrayref, $allvariables, $languagestringref) = @_;

    my $force = 1; # print this message even in 'quiet' mode
    installer::logger::print_message( "\n******************************************\n" );
    installer::logger::print_message( "... creating msp installation set ...\n", $force );
    installer::logger::print_message( "******************************************\n" );

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

    # Searching the pcp file in the include pathes
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
    my $mspfilename = set_mspfilename($allvariables, $mspdir);

    # Editing tables
    edit_tables($tablelist, $localmspdir, $olddatabase, $newdatabase, $mspfilename, $allvariables);

    # Adding edited tables into pcp file
    include_tables_into_pcpfile($fullpcpfilename, $localmspdir, $tablelist);

    # Start msimsp.exe
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Starting msimsp.exe");
    execute_msimsp($fullpcpfilename, $mspfilename, $localmspdir);

    # Copy final installation set next to msp file
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: Copying installation set");
    installer::logger::print_message( "... copying installation set ...\n" );

    my $oldinstallationsetpath = $installer::globals::updatedatabasepath;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$oldinstallationsetpath);
    installer::systemactions::copy_complete_directory($oldinstallationsetpath, $mspdir);

    # Done
    installer::logger::include_timestamp_into_logfile("\nPerformance Info: msp creation done");

    return $mspdir;
}

1;
