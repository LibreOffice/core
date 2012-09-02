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

package installer::windows::property;

use installer::exiter;
use installer::files;
use installer::globals;
use installer::windows::idtglobal;
use installer::windows::language;

#############################################
# Setting the properties dynamically
# for the table Property.idt
#############################################

sub get_arpcomments_for_property_table
{
    my ( $allvariables, $languagestringref ) = @_;

    my $name = $allvariables->{'PRODUCTNAME'};
    my $version = $allvariables->{'PRODUCTVERSION'};
    my $comment = $name . " " . $version;

    my $postversionextension = "";
    if ( $allvariables->{'POSTVERSIONEXTENSION'} )
    {
        $postversionextension = $allvariables->{'POSTVERSIONEXTENSION'};
        $comment = $comment . " " . $postversionextension;
    }

    if ( $installer::globals::languagepack ) { $comment = $comment . " " . "Language Pack"; }
    elsif ( $installer::globals::helppack ) { $comment = $comment . " " . "Help Pack"; }

    if ( $installer::globals::patch )
    {
        if ( ! $allvariables->{'WINDOWSPATCHLEVEL'} ) { installer::exiter::exit_program("ERROR: No Patch level defined for Windows patch: WINDOWSPATCHLEVEL", "get_arpcomments_for_property_table"); }
        my $patchstring = "Product Update" . " " . $allvariables->{'WINDOWSPATCHLEVEL'};
        $comment = $comment . " " . $patchstring;
    }

    my $languagestring = $$languagestringref;
    $languagestring =~ s/\_/\,/g;

    $comment = $comment . " ($languagestring)";

    my $localminor = "";
    $localminor = $installer::globals::minor;

    my $buildidstring = "(" . $installer::globals::build . $localminor . "(Build:" . $installer::globals::buildid . "))";

    $comment = $comment . " " . $buildidstring;

    return $comment;
}

sub get_installlevel_for_property_table
{
    my $installlevel = "100";
    return $installlevel;
}

sub get_ischeckforproductupdates_for_property_table
{
    my $ischeckforproductupdates = "1";
    return $ischeckforproductupdates;
}

sub get_manufacturer_for_property_table
{
    return $installer::globals::manufacturer;
}

sub get_productlanguage_for_property_table
{
    my ($language) = @_;
    my $windowslanguage = installer::windows::language::get_windows_language($language);
    return $windowslanguage;
}

sub get_language_string
{
    my $langstring = "";

    for ( my $i = 0; $i <= $#installer::globals::languagenames; $i++ )
    {
        $langstring = $langstring . $installer::globals::languagenames[$i] . ", ";
    }

    $langstring =~ s/\,\s*$//;
    $langstring = "(" . $langstring . ")";

    return $langstring;
}

sub get_english_language_string
{
    my $langstring = "";

    # Sorting value not keys, therefore collecting all values
    my %helper = ();
    foreach my $lang ( keys %installer::globals::all_required_english_languagestrings )
    {
        $helper{$installer::globals::all_required_english_languagestrings{$lang}} = 1;
    }

    foreach my $lang ( sort keys %helper )
    {
        $langstring = $langstring . $lang . ", ";
    }

    $langstring =~ s/\,\s*$//;
    $langstring = "(" . $langstring . ")";

    return $langstring;
}

sub get_productname($$)
{
    my ( $language, $allvariables ) = @_;

    my $name = $allvariables->{'PRODUCTNAME'};

    return $name;
}

sub get_productname_for_property_table($$)
{
    my ( $language, $allvariables ) = @_;

    my $name = get_productname ($language, $allvariables);
    my $version = $allvariables->{'PRODUCTVERSION'};
    my $productname = $name . " " . $version;

    my $postversionextension = "";
    if ( $allvariables->{'POSTVERSIONEXTENSION'} )
    {
        $postversionextension = $allvariables->{'POSTVERSIONEXTENSION'};
        $productname = $productname . " " . $postversionextension;
    }

    my $productextension = "";
    if ( $allvariables->{'PRODUCTEXTENSION'} )
    {
        $productextension = $allvariables->{'PRODUCTEXTENSION'};
        $productname = $productname . " " . $productextension;
    }

    if ( $installer::globals::languagepack )
    {
        my $langstring = get_english_language_string(); # Example: (English, German)
        $productname = $name . " " . $version . " Language Pack" . " " . $langstring;
    }
    elsif ( $installer::globals::helppack )
    {
        my $langstring = get_english_language_string(); # New: (English, German)
        $productname = $name . " " . $version . " Help Pack" . " " . $langstring;
    }

    if ( $installer::globals::patch )
    {
        if ( ! $allvariables->{'WINDOWSPATCHLEVEL'} ) { installer::exiter::exit_program("ERROR: No Patch level defined for Windows patch: WINDOWSPATCHLEVEL", "get_productname_for_property_table"); }
        my $patchstring = "Product Update" . " " . $allvariables->{'WINDOWSPATCHLEVEL'};
        $productname = $productname . " " . $patchstring;
    }

    # Saving this name in hash $allvariables for further usage
    $allvariables->{'PROPERTYTABLEPRODUCTNAME'} = $productname;
    my $infoline = "Defined variable PROPERTYTABLEPRODUCTNAME: $productname\n";
    push(@installer::globals::logfileinfo, $infoline);

    return $productname;
}

sub get_quickstarterlinkname_for_property_table($$)
{
    my ( $language, $allvariables ) = @_;

    # no usage of POSTVERSIONEXTENSION for Quickstarter link name!
    my $name = get_productname ($language, $allvariables);
    my $version = $allvariables->{'PRODUCTVERSION'};
    my $quickstartername = $name . " " . $version;

    my $infoline = "Defined Quickstarter Link name: $quickstartername\n";
    push(@installer::globals::logfileinfo, $infoline);

    return $quickstartername;
}

sub get_productversion_for_property_table
{
    return $installer::globals::msiproductversion;
}

#######################################################
# Setting all feature names as Properties. This is
# required for the Windows patch process.
#######################################################

sub set_featurename_properties_for_patch
{
    ($propertyfile) = @_;

    for ( my $i = 0; $i <= $#installer::globals::featurecollector; $i++ )
    {
        my $onepropertyline =  $installer::globals::featurecollector[$i] . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

}

#######################################################
# Setting some important properties
# (for finding the product in deinstallation process)
#######################################################

sub set_important_properties
{
    my ($propertyfile, $allvariables, $languagestringref) = @_;

    # Setting new variables with the content of %PRODUCTNAME and %PRODUCTVERSION
    if ( $allvariables->{'PRODUCTNAME'} )
    {
        my $onepropertyline =  "DEFINEDPRODUCT" . "\t" . $allvariables->{'PRODUCTNAME'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'PRODUCTVERSION'} )
    {
        my $onepropertyline = "DEFINEDVERSION" . "\t" . $allvariables->{'PRODUCTVERSION'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if (( $allvariables->{'PRODUCTNAME'} ) && ( $allvariables->{'PRODUCTVERSION'} ) && ( $allvariables->{'REGISTRYLAYERNAME'} ))
    {
        my $onepropertyline = "FINDPRODUCT" . "\t" . "Software\\LibreOffice" . "\\" . $allvariables->{'REGISTRYLAYERNAME'} . "\\" . $allvariables->{'PRODUCTNAME'} . "\\" . $allvariables->{'PRODUCTVERSION'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'PRODUCTMAJOR'} )
    {
        my $onepropertyline = "PRODUCTMAJOR" . "\t" . $allvariables->{'PRODUCTMAJOR'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'PRODUCTMINOR'} )
    {
        my $onepropertyline = "PRODUCTMINOR" . "\t" . $allvariables->{'PRODUCTMINOR'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'PRODUCTBUILDID'} )
    {
        my $onepropertyline = "PRODUCTBUILDID" . "\t" . $allvariables->{'PRODUCTBUILDID'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'URELAYERVERSION'} )
    {
        my $onepropertyline = "URELAYERVERSION" . "\t" . $allvariables->{'URELAYERVERSION'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'BRANDPACKAGEVERSION'} )
    {
        my $onepropertyline = "BRANDPACKAGEVERSION" . "\t" . $allvariables->{'BRANDPACKAGEVERSION'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'EXCLUDE_FROM_REBASE'} )
    {
        my $onepropertyline =  "EXCLUDE_FROM_REBASE" . "\t" . $allvariables->{'EXCLUDE_FROM_REBASE'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $allvariables->{'PREREQUIREDPATCH'} )
    {
        my $onepropertyline = "PREREQUIREDPATCH" . "\t" . $allvariables->{'PREREQUIREDPATCH'} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    my $onepropertyline = "IGNOREPREREQUIREDPATCH" . "\t" . "1" . "\n";
    push(@{$propertyfile}, $onepropertyline);

    $onepropertyline = "DONTOPTIMIZELIBS" . "\t" . "0" . "\n";
    push(@{$propertyfile}, $onepropertyline);

    if ( $installer::globals::officedirhostname )
    {
        my $onepropertyline = "OFFICEDIRHOSTNAME" . "\t" . $installer::globals::officedirhostname . "\n";
        push(@{$propertyfile}, $onepropertyline);

        my $localofficedirhostname = $installer::globals::officedirhostname;
        $localofficedirhostname =~ s/\//\\/g;
        $onepropertyline = "OFFICEDIRHOSTNAME_" . "\t" . $localofficedirhostname . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $installer::globals::desktoplinkexists )
    {
        my $onepropertyline = "DESKTOPLINKEXISTS" . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);

        $onepropertyline = "CREATEDESKTOPLINK" . "\t" . "1" . "\n"; # Setting the default
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $installer::globals::patch )
    {
        my $onepropertyline = "ISPATCH" . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $installer::globals::languagepack )
    {
        my $onepropertyline = "ISLANGUAGEPACK" . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }
    elsif ( $installer::globals::helppack )
    {
        my $onepropertyline = "ISHELPPACK" . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    my $languagesline = "PRODUCTALLLANGUAGES" . "\t" . $$languagestringref . "\n";
    push(@{$propertyfile}, $languagesline);

    if (( $allvariables->{'PRODUCTEXTENSION'} ) && ( $allvariables->{'PRODUCTEXTENSION'}  eq "Beta" ))
    {
        # my $registryline = "WRITE_REGISTRY" . "\t" . "0" . "\n";
        # push(@{$propertyfile}, $registryline);
        my $betainfoline = "BETAPRODUCT" . "\t" . "1" . "\n";
        push(@{$propertyfile}, $betainfoline);
    }
    elsif ( $allvariables->{'DEVELOPMENTPRODUCT'} )
    {
        my $registryline = "WRITE_REGISTRY" . "\t" . "0" . "\n";
        push(@{$propertyfile}, $registryline);
    }
    else
    {
        my $registryline = "WRITE_REGISTRY" . "\t" . "1" . "\n";    # Default: Write complete registry
        push(@{$propertyfile}, $registryline);
    }

    # Adding also used tree conditions for multilayer products.
    # These are saved in %installer::globals::usedtreeconditions
    foreach my $treecondition (keys %installer::globals::usedtreeconditions)
    {
        my $onepropertyline = $treecondition . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    # No more license dialog for selected products
    if ( $allvariables->{'HIDELICENSEDIALOG'} )
    {
        my $onepropertyline = "HIDEEULA" . "\t" . "1" . "\n";

        my $already_defined = 0;

        for ( my $i = 0; $i <= $#{$propertyfile}; $i++ )
        {
            if ( ${$propertyfile}[$i] =~ /^\s*HIDEEULA\t/ )
            {
                ${$propertyfile}[$i] = $onepropertyline;
                $already_defined = 1;
                last;
            }
        }

        if ( ! $already_defined )
        {
            push(@{$propertyfile}, $onepropertyline);
        }
    }

    # Setting .NET requirements
    push @{$propertyfile}, "REQUIRED_DOTNET_VERSION" . "\t" . "2.0.0.0" . "\n";
    push @{$propertyfile}, "DOTNET_SUFFICIENT" . "\t" . "1" . "\n"; # default value for found .NET
}

#######################################################
# Setting properties needed for ms file type registration
#######################################################

sub set_ms_file_types_properties
{
    my ($propertyfile) = @_;

    push(@{$propertyfile}, "REGISTER_PPS"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_PPSX" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_PPSM" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_PPAM" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_PPT"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_PPTX" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_PPTM" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_POT"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_POTX" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_POTM" . "\t" . "0" . "\n");

    push(@{$propertyfile}, "REGISTER_DOC"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_DOCX" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_DOCM" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_DOT"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_DOTX" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_DOTM" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_RTF"  . "\t" . "0" . "\n");

    push(@{$propertyfile}, "REGISTER_XLS"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_XLSX" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_XLSM" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_XLSB" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_XLAM" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_XLT"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_XLTX" . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_XLTM" . "\t" . "0" . "\n");

    push(@{$propertyfile}, "REGISTER_NO_MSO_TYPES"  . "\t" . "0" . "\n");
    push(@{$propertyfile}, "REGISTER_ALL_MSO_TYPES"  . "\t" . "0" . "\n");
}

####################################################################################
# Updating the file Property.idt dynamically
# Content:
# Property Value
####################################################################################

sub update_property_table
{
    my ($basedir, $language, $allvariables, $languagestringref) = @_;

    my $properyfilename = $basedir . $installer::globals::separator . "Property.idt";

    my $propertyfile = installer::files::read_file($properyfilename);

    my $hasarpnomodify = 0;

    # Getting the new values
    # Some values (arpcomments, arpcontacts, ...) are inserted from the Property.mlf

    my $arpcomments = get_arpcomments_for_property_table($allvariables, $languagestringref);
    my $installlevel = get_installlevel_for_property_table();
    my $ischeckforproductupdates = get_ischeckforproductupdates_for_property_table();
    my $manufacturer = get_manufacturer_for_property_table();
    my $productlanguage = get_productlanguage_for_property_table($language);
    my $productname = get_productname_for_property_table($language, $allvariables);
    my $productversion = get_productversion_for_property_table();
    my $quickstarterlinkname = get_quickstarterlinkname_for_property_table($language, $allvariables);

    # Updating the values

    for ( my $i = 0; $i <= $#{$propertyfile}; $i++ )
    {
        ${$propertyfile}[$i] =~ s/\bARPCOMMENTSTEMPLATE\b/$arpcomments/;
        ${$propertyfile}[$i] =~ s/\bINSTALLLEVELTEMPLATE\b/$installlevel/;
        ${$propertyfile}[$i] =~ s/\bISCHECKFORPRODUCTUPDATESTEMPLATE\b/$ischeckforproductupdates/;
        ${$propertyfile}[$i] =~ s/\bMANUFACTURERTEMPLATE\b/$manufacturer/;
        ${$propertyfile}[$i] =~ s/\bPRODUCTLANGUAGETEMPLATE\b/$productlanguage/;
        ${$propertyfile}[$i] =~ s/\bPRODUCTNAMETEMPLATE\b/$productname/;
        ${$propertyfile}[$i] =~ s/\bPRODUCTVERSIONTEMPLATE\b/$productversion/;
        ${$propertyfile}[$i] =~ s/\bQUICKSTARTERLINKNAMETEMPLATE\b/$quickstarterlinkname/;
        if ( ${$propertyfile}[$i] =~ m/\bARPNOMODIFY\b/ ) { $hasarpnomodify = 1; }
    }

    # Check if are building silent MSI
    if ( $ENV{ENABLE_SILENT_MSI} eq "TRUE" )
    {
        push(@{$propertyfile}, "LIMITUI" . "\t" . "1" . "\n");
        if ( !($hasarpnomodify) )
        {
            push(@{$propertyfile}, "ARPNOMODIFY" . "\t" . "1" . "\n");
        }
    }

    # Setting variables into propertytable
    set_important_properties($propertyfile, $allvariables, $languagestringref);

    # Setting feature names as properties for Windows patch mechanism
    if ( $installer::globals::patch ) { set_featurename_properties_for_patch($propertyfile); }

    # Setting variables for register for ms file types
    set_ms_file_types_properties($propertyfile);

    # Saving the file

    installer::files::save_file($properyfilename ,$propertyfile);
    my $infoline = "Updated idt file: $properyfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

####################################################################################
# Setting language specific Properties in file Property.idt dynamically
# Adding:
# isMulti = 1
####################################################################################

sub set_languages_in_property_table
{
    my ($basedir, $languagesarrayref) = @_;

    my $properyfilename = $basedir . $installer::globals::separator . "Property.idt";
    my $propertyfile = installer::files::read_file($properyfilename);

    # Setting the info about multilingual installation in property "isMulti"

    my $propertyname = "isMulti";
    my $ismultivalue = 0;

    if ( $installer::globals::ismultilingual ) { $ismultivalue = 1; }

    my $onepropertyline =  $propertyname . "\t" . $ismultivalue . "\n";
    push(@{$propertyfile}, $onepropertyline);

    # setting the ARPPRODUCTICON

    if ($installer::globals::sofficeiconadded)  # set in shortcut.pm
    {
        $onepropertyline =  "ARPPRODUCTICON" . "\t" . "soffice.ico" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    # Saving the file

    installer::files::save_file($properyfilename ,$propertyfile);
    my $infoline = "Added language content into idt file: $properyfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

############################################################
# Setting the ProductCode and the UpgradeCode
# into the Property table. Both have to be stored
# in the global file $installer::globals::codefilename
############################################################

sub set_codes_in_property_table
{
    my ($basedir) = @_;

    # Reading the property file

    my $properyfilename = $basedir . $installer::globals::separator . "Property.idt";
    my $propertyfile = installer::files::read_file($properyfilename);

    # Updating the values

    for ( my $i = 0; $i <= $#{$propertyfile}; $i++ )
    {
        ${$propertyfile}[$i] =~ s/\bPRODUCTCODETEMPLATE\b/$installer::globals::productcode/;
        ${$propertyfile}[$i] =~ s/\bUPGRADECODETEMPLATE\b/$installer::globals::upgradecode/;
    }

    # Saving the property file

    installer::files::save_file($properyfilename ,$propertyfile);
    my $infoline = "Added language content into idt file: $properyfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

############################################################
# Changing default for MS file type registration
# in Beta products.
############################################################

sub update_checkbox_table
{
    my ($basedir, $allvariables) = @_;

    if (( $allvariables->{'PRODUCTEXTENSION'} ) && ( $allvariables->{'PRODUCTEXTENSION'}  eq "Beta" ))
    {
        my $checkboxfilename = $basedir . $installer::globals::separator . "CheckBox.idt";

        if ( -f $checkboxfilename )
        {
            my $checkboxfile = installer::files::read_file($checkboxfilename);

            my $checkboxline = "SELECT_WORD" . "\t" . "0" . "\n";
            push(@{$checkboxfile}, $checkboxline);
            $checkboxline = "SELECT_EXCEL" . "\t" . "0" . "\n";
            push(@{$checkboxfile}, $checkboxline);
            $checkboxline = "SELECT_POWERPOINT" . "\t" . "0" . "\n";
            push(@{$checkboxfile}, $checkboxline);
            $checkboxline = "SELECT_VISIO" . "\t" . "0" . "\n";
            push(@{$checkboxfile}, $checkboxline);

            # Saving the property file
            installer::files::save_file($checkboxfilename ,$checkboxfile);
            my $infoline = "Added ms file type defaults into idt file: $checkboxfilename\n";
            push(@installer::globals::logfileinfo, $infoline);
        }
    }
}

1;
