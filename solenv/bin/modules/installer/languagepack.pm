#*************************************************************************
#
#   $RCSfile: languagepack.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2004-06-11 18:16:05 $
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

package installer::languagepack;

use installer::logger;

####################################################
# Selecting all files with the correct language
####################################################

sub select_language_items
{
    my ( $itemsref, $languagesarrayref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting languages for language pack. Item: $itemname");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        my $ismultilingual = $oneitem->{'ismultilingual'};

        if (!($ismultilingual))
        {
            next;   # single language files are not included into language pack
        }

        my $specificlanguage = "";
        if ( $oneitem->{'specificlanguage'} ) { $specificlanguage = $oneitem->{'specificlanguage'}; }

        for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ )   # iterating over all languages
        {
            my $onelanguage = ${$languagesarrayref}[$j];

            if ( $specificlanguage eq $onelanguage )
            {
                $oneitem->{'modules'} = "gid_Module_Root";   # all files in a language pack are root files
                push(@itemsarray, $oneitem);
            }
        }
    }

    return \@itemsarray;
}

sub replace_languagestring_variable
{
    my ($onepackageref, $languagestringref) = @_;

    my $key;

    foreach $key (keys %{$onepackageref})
    {
        my $value = $onepackageref->{$key};
        $value =~ s/\%LANGUAGESTRING/$$languagestringref/g;
        $onepackageref->{$key} = $value;
    }
}

1;