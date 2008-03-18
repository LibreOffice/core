#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: property.pm,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:04:37 $
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
    if ( $installer::globals::updatepack ) { $localminor = $installer::globals::lastminor; }
    else { $localminor = $installer::globals::minor; }

    my $buildidstring = "(" . $installer::globals::build . $localminor . "(Build:" . $installer::globals::buildid . "))";

    # the environment variable CWS_WORK_STAMP is set only in CWS
    if ( $ENV{'CWS_WORK_STAMP'} ) { $buildidstring = $buildidstring . "\[CWS\:" . $ENV{'CWS_WORK_STAMP'} . "\]"; }

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

sub get_productname_for_property_table
{
    my ( $allvariables ) = @_;

    my $name = $allvariables->{'PRODUCTNAME'};
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
        my $langstring = get_language_string(); # Example (English, Deutsch)
        $productname = $name . " " . $version . " Language Pack" . " " . $langstring;
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

sub get_quickstarterlinkname_for_property_table
{
    my ( $allvariables ) = @_;

    # no usage of POSTVERSIONEXTENSION for Quickstarter link name!

    my $name = $allvariables->{'PRODUCTNAME'};
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

    if (( $allvariables->{'PRODUCTNAME'} ) && ( $allvariables->{'PRODUCTVERSION'} ) && ( $allvariables->{'MANUFACTURER'} ) && ( $allvariables->{'PRODUCTCODE'} ))
    {
        my $onepropertyline = "FINDPRODUCT" . "\t" . "Software\\" . $allvariables->{'MANUFACTURER'} . "\\" . $allvariables->{'PRODUCTNAME'} . $allvariables->{'PRODUCTADDON'} . "\\" . $allvariables->{'PRODUCTVERSION'} . "\\" . $allvariables->{'PRODUCTCODE'} . "\n";
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

        $onepropertyline = "SETUP_USED" . "\t" . "0" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    if ( $installer::globals::languagepack )
    {
        my $onepropertyline = "ISLANGUAGEPACK" . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    my $languagesline = "PRODUCTALLLANGUAGES" . "\t" . $$languagestringref . "\n";
    push(@{$propertyfile}, $languagesline);

    # Adding also used tree conditions for multilayer products.
    # These are saved in %installer::globals::usedtreeconditions
    foreach my $treecondition (keys %installer::globals::usedtreeconditions)
    {
        my $onepropertyline = $treecondition . "\t" . "1" . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

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

    # Getting the new values
    # Some values (arpcomments, arpcontacts, ...) are inserted from the Property.mlf

    my $arpcomments = get_arpcomments_for_property_table($allvariables, $languagestringref);
    my $installlevel = get_installlevel_for_property_table();
    my $ischeckforproductupdates = get_ischeckforproductupdates_for_property_table();
    my $manufacturer = get_manufacturer_for_property_table();
    my $productlanguage = get_productlanguage_for_property_table($language);
    my $productname = get_productname_for_property_table($allvariables);
    my $productversion = get_productversion_for_property_table();
    my $quickstarterlinkname = get_quickstarterlinkname_for_property_table($allvariables);

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
    }

    # Setting variables into propertytable
    set_important_properties($propertyfile, $allvariables, $languagestringref);

    # Setting feature names as properties for Windows patch mechanism
    if ( $installer::globals::patch ) { set_featurename_properties_for_patch($propertyfile); }

    # Saving the file

    installer::files::save_file($properyfilename ,$propertyfile);
    my $infoline = "Updated idt file: $properyfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

####################################################################################
# Setting language specific Properties in file Property.idt dynamically
# Adding:
# is1033 = 1
# isMulti = 1
####################################################################################

sub set_languages_in_property_table
{
    my ($basedir, $languagesarrayref) = @_;

    my $properyfilename = $basedir . $installer::globals::separator . "Property.idt";
    my $propertyfile = installer::files::read_file($properyfilename);

    # Setting the component properties saved in %installer::globals::languageproperties
    foreach my $localproperty ( keys %installer::globals::languageproperties )
    {
        $onepropertyline =  $localproperty . "\t" . $installer::globals::languageproperties{$localproperty} . "\n";
        push(@{$propertyfile}, $onepropertyline);
    }

    # Setting the info about multilingual installation in property "isMulti"

    my $propertyname = "isMulti";
    my $ismultivalue = 0;

    if ( $installer::globals::ismultilingual ) { $ismultivalue = 1; }

    my $onepropertyline =  $propertyname . "\t" . $ismultivalue . "\n";
    push(@{$propertyfile}, $onepropertyline);

    # setting the ARPPRODUCTICON

    if ($installer::globals::sofficeiconadded)  # set in shortcut.pm
    {
        $onepropertyline =  "ARPPRODUCTICON" . "\t" . "soffice.exe" . "\n";
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
# Setting the variable REGKEYPRODPATH, that is used
# by the language packs.
############################################################

sub set_regkeyprodpath_in_property_table
{
    my ($basedir, , $allvariables) = @_;

    # Reading the property file

    my $properyfilename = $basedir . $installer::globals::separator . "Property.idt";
    my $propertyfile = installer::files::read_file($properyfilename);

    my $name = $allvariables->{'PRODUCTNAME'};
    my $version = $allvariables->{'PRODUCTVERSION'};

    my $onepropertyline = "REGKEYPRODPATH" . "\t" . "Software" . "\\" . $installer::globals::manufacturer . "\\". $name;

    push(@{$propertyfile}, $onepropertyline);

    # Saving the property file

    installer::files::save_file($properyfilename ,$propertyfile);
    my $infoline = "Added language content into idt file: $properyfilename\n";
    push(@installer::globals::logfileinfo, $infoline);

}

1;
