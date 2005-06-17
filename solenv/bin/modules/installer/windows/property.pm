#*************************************************************************
#
#   $RCSfile: property.pm,v $
#
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
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

    # $comment = $comment . " [INSTALLLOCATION]";

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

    for ( my $i = 0; $i <= $#{$installer::globals::languagenames}; $i++ )
    {
        $langstring = $langstring . ${$installer::globals::languagenames}[$i] . ", ";
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

    if ( $installer::globals::languagepack )
    {
        my $langstring = get_language_string(); # Example (English, Deutsch)
        $productname = $name . " " . $version . " Language Pack" . " " . $langstring;
    }

    if ( $installer::globals::patch )
    {
        my $patchstring = "Product Update";
        $productname = $productname . " " . $patchstring;
    }

    return $productname;
}

sub get_productversion_for_property_table
{
    return $installer::globals::msiproductversion;
}

#######################################################
# Setting some important properties
# (for finding the product in deinstallation process)
#######################################################

sub set_important_properties
{
    my ($propertyfile, $allvariables) = @_;

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
    }

    # Setting variables into propertytable
    set_important_properties($propertyfile, $allvariables);

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

    # Setting the new properties for each language ( is1033 = 1 )

    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ )
    {
        my $language = ${$languagesarrayref}[$i];
        my $windowslanguage = installer::windows::language::get_windows_language($language);
        my $property = "IS" . $windowslanguage; # Capitol letter "IS" !
        my $value = 1;

        my $oneline = $property . "\t" . $value . "\n";
        push(@{$propertyfile}, $oneline);
    }

    # Setting the info about multilingual installation in property ISMULTI

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
