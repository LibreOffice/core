#*************************************************************************
#
#   $RCSfile: property.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2004-07-13 09:10:54 $
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

#   sub get_arpcomments_for_property_table
#   {
#       my ( $allvariables ) = @_;
#
#       my $name = $allvariables->{'PRODUCTNAME'};
#       my $version = $allvariables->{'PRODUCTVERSION'};
#       my $comment = $name . " " . $version;
#
#       return $comment;
#   }

#   sub get_arpcontact_for_property_table
#   {
#       my $contact   = "";
#       if ( ! $installer::globals::isopensourceproduct ) { $contact   = "Abteilung für Technischen Support"; }
#       return $contact;
#   }

#   sub get_arphelplink_for_property_table
#   {
#       my $helplink  = "";
#       if ( ! $installer::globals::isopensourceproduct ) { $helplink  = 'http://www.sun.com/help'; }
#       return $helplink;
#   }

#   sub get_arphelptelephone_for_property_table
#   {
#       my $telephone = "";
#       if ( ! $installer::globals::isopensourceproduct ) { $telephone = '1-555-555-4505'; }
#       return $telephone;
#   }

#   sub get_arpurlinfoabout_for_property_table
#   {
#       my $urlinfoabout = "";
#       if ( ! $installer::globals::isopensourceproduct ) { $urlinfoabout = 'http://www.sun.com'; }
#       return $urlinfoabout;
#   }

#   sub get_arpurlupdateinfo_for_property_table
#   {
#       my $urlupdateinfo = "";
#       if ( ! $installer::globals::isopensourceproduct ) { $urlupdateinfo = 'http://www.sun.com/updateinfo'; }
#       return $urlupdateinfo;
#   }

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

#   sub get_productcode_for_property_table
#   {
#       my ( $allvariables ) = @_;
#
#       my $productcode = '{CBF271AE-179C-4601-BAA8-D0E728FA70DE}';
#       return $productcode;
#   }

sub get_productlanguage_for_property_table
{
    my ($language) = @_;
    my $windowslanguage = installer::windows::language::get_windows_language($language);
    return $windowslanguage;
}

sub get_productname_for_property_table
{
    my ( $allvariables ) = @_;

    my $name = $allvariables->{'PRODUCTNAME'};
    my $version = $allvariables->{'PRODUCTVERSION'};
    my $productname = $name . " " . $version;

    return $productname;
}

sub get_productversion_for_property_table
{
    my ( $allvariables ) = @_;
    my $productversion = $allvariables->{'PRODUCTVERSION'} . ".00.0000";
    return $productversion;
}

#   sub get_upgradecode_for_property_table
#   {
#       my ( $allvariables ) = @_;
#       my $upgradecode = '{826F42D3-1493-4D0D-BB64-5A040DD3AFD7}';
#       return $upgradecode;
#   }

####################################################################################
# Updating the file Property.idt dynamically
# Content:
# Property Value
####################################################################################

sub update_property_table
{
    my ($basedir, $language, $allvariables) = @_;

    my $properyfilename = $basedir . $installer::globals::separator . "Property.idt";

    my $propertyfile = installer::files::read_file($properyfilename);

    # Getting the new values
    # Some values (arpcomments, arpcontacts, ...) are inserted from the Property.ulf

    #   my $arpcomments = get_arpcomments_for_property_table($allvariables);
    #   my $arpcontact = get_arpcontact_for_property_table();
    #   my $arphelplink = get_arphelplink_for_property_table();
    #   my $arphelptelephone = get_arphelptelephone_for_property_table();
    #   my $arpurlinfoabout = get_arpurlinfoabout_for_property_table();
    #   my $arpurlupdateinfo = get_arpurlupdateinfo_for_property_table();
    my $installlevel = get_installlevel_for_property_table();
    my $ischeckforproductupdates = get_ischeckforproductupdates_for_property_table();
    my $manufacturer = get_manufacturer_for_property_table();
    #   my $productcode = get_productcode_for_property_table($allvariables);
    my $productlanguage = get_productlanguage_for_property_table($language);
    my $productname = get_productname_for_property_table($allvariables);
    my $productversion = get_productversion_for_property_table($allvariables);
    #   my $upgradecode = get_upgradecode_for_property_table($allvariables);

    # Updating the values

    for ( my $i = 0; $i <= $#{$propertyfile}; $i++ )
    {
        #   ${$propertyfile}[$i] =~ s/\bARPCOMMENTSTEMPLATE\b/$arpcomments/;
        #   ${$propertyfile}[$i] =~ s/\bARPCONTACTTEMPLATE\b/$arpcontact/;
        #   ${$propertyfile}[$i] =~ s/\bARPHELPLINKTEMPLATE\b/$arphelplink/;
        #   ${$propertyfile}[$i] =~ s/\bARPHELPTELEPHONETEMPLATE\b/$arphelptelephone/;
        #   ${$propertyfile}[$i] =~ s/\bARPURLINFOABOUTTEMPLATE\b/$arpurlinfoabout/;
        #   ${$propertyfile}[$i] =~ s/\bARPURLUPDATEINFOTEMPLATE\b/$arpurlupdateinfo/;
        ${$propertyfile}[$i] =~ s/\bINSTALLLEVELTEMPLATE\b/$installlevel/;
        ${$propertyfile}[$i] =~ s/\bISCHECKFORPRODUCTUPDATESTEMPLATE\b/$ischeckforproductupdates/;
        ${$propertyfile}[$i] =~ s/\bMANUFACTURERTEMPLATE\b/$manufacturer/;
        #   ${$propertyfile}[$i] =~ s/\bPRODUCTCODETEMPLATE\b/$productcode/;
        ${$propertyfile}[$i] =~ s/\bPRODUCTLANGUAGETEMPLATE\b/$productlanguage/;
        ${$propertyfile}[$i] =~ s/\bPRODUCTNAMETEMPLATE\b/$productname/;
        ${$propertyfile}[$i] =~ s/\bPRODUCTVERSIONTEMPLATE\b/$productversion/;
        #   ${$propertyfile}[$i] =~ s/\bUPGRADECODETEMPLATE\b/$upgradecode/;
    }

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

1;
