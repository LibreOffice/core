#*************************************************************************
#
#   $RCSfile: language.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hjs $ $Date: 2004-06-25 16:09:53 $
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

package pre2par::language;

use pre2par::existence;

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
            $newstring =~ s/\"/\\\"/g;  # masquerading all '"' in the string
            $newstring = "\"" . $newstring . "\"";
            last;
        }
    }

    # defaulting to english?

    # if ( $newstring eq "" )
    # {
    #   $language = "01";   # defaulting to english
    #
    #   for ( my $i = 0; $i <= $#{$language_block}; $i++ )
    #   {
    #       if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*(\".*\")\s*$/ )
    #       {
    #           $newstring = $1;
    #           last;
    #       }
    #   }
    # }

    return $newstring;
}

##############################################################
# Returning the complete block in all languages
# for a specified string
##############################################################

sub get_language_block_from_language_file
{
    my ($searchstring, $ulffile) = @_;

    my @language_block = ();

    for ( my $i = 0; $i <= $#{$ulffile}; $i++ )
    {
        if ( ${$ulffile}[$i] =~ /^\s*\[\s*$searchstring\s*\]\s*$/ )
        {
            my $counter = $i;

            push(@language_block, ${$ulffile}[$counter]);
            $counter++;

            while (( $counter <= $#{$ulffile} ) && (!( ${$ulffile}[$counter] =~ /^\s*\[/ )))
            {
                push(@language_block, ${$ulffile}[$counter]);
                $counter++;
            }

            last;
        }
    }

    return \@language_block;
}

############################################
# collecting all replace strings
# in a ulf file
############################################

sub get_all_replace_strings
{
    my ($ulffile) = @_;

    my @allstrings = ();

    for ( my $i = 0; $i <= $#{$ulffile}; $i++ )
    {
        if ( ${$ulffile}[$i] =~ /^\s*\[\s*(.*?)\s*\]\s*$/ )
        {
            my $replacestring = $1;
            if (! pre2par::existence::exists_in_array($replacestring, \@allstrings))
            {
                push(@allstrings, $replacestring);
            }
        }
    }

    return \@allstrings;
}

############################################
# localizing the par file with the
# corresponding ulf file
############################################

sub localize
{
    my ($parfile, $ulffile) = @_;

    my $allreplacestrings = get_all_replace_strings($ulffile);

    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        my $oneline = ${$parfile}[$i];

        for ( my $j = 0; $j <= $#{$allreplacestrings}; $j++ )
        {
            if ( $oneline =~ /\b${$allreplacestrings}[$j]\b/ ) # Not for basic scripts
            {
                my $oldstring = ${$allreplacestrings}[$j];

                if ( $oneline =~ /^\s*\w+\s*\(([\w-]+)\)\s*\=/ )
                {
                    my $language = $1;   # can be "01" or "en" or "en-US" or ...

                    my $languageblock = get_language_block_from_language_file($oldstring, $ulffile);
                    my $newstring = get_language_string_from_language_block($languageblock, $language);

                    if ( $newstring eq "" ) { $newstring = "\"" . $oldstring . "\""; }

                    $oneline =~ s/$oldstring/$newstring/g;

                    ${$parfile}[$i] = $oneline;
                }
            }

            if ( $oneline =~ /\b${$allreplacestrings}[$j]\#([\w-]+)\b/ ) # Only for basic scripts (STR_BASIC_ADABASINST1#01)
            {
                my $language = $1;
                my $oldstring = ${$allreplacestrings}[$j];

                my $languageblock = get_language_block_from_language_file($oldstring, $ulffile);
                my $newstring = get_language_string_from_language_block($languageblock, $language);

                $oldstring = $oldstring . "\#" . $language;

                if ( $newstring eq "" ) { $newstring = "\"" . $oldstring . "\""; }

                $oneline =~ s/\Q$oldstring\E/$newstring/g;

                ${$parfile}[$i] = $oneline;
            }
        }
    }
}

1;
