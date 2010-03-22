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

    # defaulting to english!

    if ( $newstring eq "" )
    {
        $language = "en-US";    # defaulting to english

        for ( my $i = 0; $i <= $#{$language_block}; $i++ )
        {
            if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*(\".*\")\s*$/ )
            {
                $newstring = $1;
                last;
            }
        }
    }

    return $newstring;
}

##############################################################
# Returning the complete block in all languages
# for a specified string
##############################################################

sub get_language_block_from_language_file
{
    my ($searchstring, $langfile) = @_;

    my @language_block = ();

    for ( my $i = 0; $i <= $#{$langfile}; $i++ )
    {
        if ( ${$langfile}[$i] =~ /^\s*\[\s*$searchstring\s*\]\s*$/ )
        {
            my $counter = $i;

            push(@language_block, ${$langfile}[$counter]);
            $counter++;

            while (( $counter <= $#{$langfile} ) && (!( ${$langfile}[$counter] =~ /^\s*\[/ )))
            {
                push(@language_block, ${$langfile}[$counter]);
                $counter++;
            }

            last;
        }
    }

    return \@language_block;
}

############################################
# collecting all replace strings
# in a language file
############################################

sub get_all_replace_strings
{
    my ($langfile) = @_;

    my @allstrings = ();

    for ( my $i = 0; $i <= $#{$langfile}; $i++ )
    {
        if ( ${$langfile}[$i] =~ /^\s*\[\s*(.*?)\s*\]\s*$/ )
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
# corresponding language file
############################################

sub localize
{
    my ($parfile, $langfile) = @_;

    my $allreplacestrings = get_all_replace_strings($langfile);

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

                    my $languageblock = get_language_block_from_language_file($oldstring, $langfile);
                    my $newstring = get_language_string_from_language_block($languageblock, $language);

                    if ( $newstring eq "" ) { $newstring = "\"" . $oldstring . "\""; }

                    $oneline =~ s/$oldstring/$newstring/g;

                    ${$parfile}[$i] = $oneline;
                }
            }
        }
    }
}

1;
