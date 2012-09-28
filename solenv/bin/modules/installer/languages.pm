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

package installer::languages;

use installer::converter;
use installer::exiter;
use installer::globals;
use installer::remover;
use installer::ziplist;

#############################################################################
# Analyzing the laguage list parameter and language list from zip list file
#############################################################################

sub analyze_languagelist
{
    my $first = $installer::globals::languagelist;

    $first =~ s/\_/\,/g;    # substituting "_" by ",", in case of dmake definition 01_49

    # Products are separated by a "#", if defined in zip-list by a "|". But "get_info_about_languages"
    # substitutes already "|" to "#". This procedure only knows "#" as product separator.
    # Different languages for one product are separated by ",". But on the command line the "_" is used.
    # Therefore "_" is replaced by "," at the beginning of this procedure.

    while ($first =~ /^(\S+)\#(\S+?)$/) # Minimal matching, to keep the order of languages
    {
        $first = $1;
        my $last = $2;
        unshift(@installer::globals::languageproducts, $last);
    }

    unshift(@installer::globals::languageproducts, $first);
}

####################################################
# Reading languages from zip list file
####################################################

sub get_info_about_languages
{
    my ( $allsettingsarrayref ) = @_;

    my $languagelistref;

    $languagelistref = installer::ziplist::getinfofromziplist($allsettingsarrayref, "languages");
    $installer::globals::languagelist = $$languagelistref;

    if ( $installer::globals::languagelist eq "" )  # not defined on command line and not in product list
    {
        installer::exiter::exit_program("ERROR: Languages not defined on command line (-l) and not in product list!", "get_info_about_languages");
    }

    # Adapting the separator format from zip list.
    # | means new product, , (comma) means more than one language in one product
    # On the command line, | is difficult to use. Therefore this script uses hashes

    $installer::globals::languagelist =~ s/\|/\#/g;

    analyze_languagelist();
}

#############################################################################
# Checking whether all elements of an array A are also member of array B
#############################################################################

sub all_elements_of_array1_in_array2
{
    my ($array1, $array2) = @_;

    my $array2_contains_all_elements_of_array1 = 1;

    for ( my $i = 0; $i <= $#{$array1}; $i++ )
    {
        if (! grep {$_ eq ${$array1}[$i]} @{$array2})
        {
            $array2_contains_all_elements_of_array1 = 0;
            last;
        }
    }

    return $array2_contains_all_elements_of_array1;
}

#############################################
# All languages defined for one product
#############################################

sub get_all_languages_for_one_product
{
    my ( $languagestring, $allvariables ) = @_;

    my @languagearray = ();

    my $last = $languagestring;

    $installer::globals::ismultilingual = 0;        # setting the global variable $ismultilingual !
    if ( $languagestring =~ /\,/ ) { $installer::globals::ismultilingual = 1; }

    while ( $last =~ /^\s*(.+?)\,(.+)\s*$/) # "$" for minimal matching, comma separated list
    {
        my $first = $1;
        $last = $2;
        installer::remover::remove_leading_and_ending_whitespaces(\$first);
        push(@languagearray, "$first");
    }

    installer::remover::remove_leading_and_ending_whitespaces(\$last);
    push(@languagearray, "$last");

    if ( $installer::globals::iswindowsbuild )
    {
        my $furthercheck = 1;

        # For some languages (that are not supported by Windows, english needs to be added to the installation set

        # FIXME The script i18npool/source/isolang/langid.pl greps this very
        # source file to read this array.  Be careful!
        my @noMSLocaleLangs = (
            "br",
            "bs",
            "dz",
            "gu",
            "km",
            "nr",
            "ns",
            "nso",
            "rw",
            "ss",
            "st",
            "tg",
            "ts",
            "tn",
            "ve",
            "xh",
            "zu",
            "ne",
            "bn",
            "bn-BD",
            "bn-IN",
            "lo",
            "cy",
            "ku",
            "as-IN",
            "te-IN",
            "ml-IN",
            "mr-IN",
            "ur-IN",
            "ta-IN",
            "or-IN",
            "ti-ER",
            "eo",
            "ka",
            "ga",
            "uk",
            "gd",
            "my",
            "mai",
            "brx",
            "dgo",
            "kok",
            "mni",
            "ca-XV",
            "sat",
            "ug",
            "om",
            "si",
            "or",
            "oc",
            "ml",
            "as",
            "ast",
            "ht",
            "jbo",
            "fur",
            "ny",
            "so",
            "kab",
            "tk",
            "ky-CN",
        );

        if ( all_elements_of_array1_in_array2(\@languagearray, \@noMSLocaleLangs) )
        {
            my $officestartlanguage = $languagearray[0];
            unshift(@languagearray, "en-US");   # am Anfang einfügen!
            $installer::globals::ismultilingual = 1;
            $installer::globals::added_english  = 1;
            $installer::globals::set_office_start_language  = 1;
            # setting the variable PRODUCTLANGUAGE, needed for Linguistic-ForceDefaultLanguage.xcu
            $allvariables->{'PRODUCTLANGUAGE'} = $officestartlanguage;
            $furthercheck = 0;
        }

        # In bilingual installation sets, in which english is the first language,
        # the Office start language shall be the second language.

        if ( $furthercheck )
        {
            if (( $#languagearray == 1 ) && ( $languagearray[0] eq "en-US" ))
            {
                my $officestartlanguage = $languagearray[1];
                $installer::globals::set_office_start_language  = 1;
                # setting the variable PRODUCTLANGUAGE, needed for Linguistic-ForceDefaultLanguage.xcu
                $allvariables->{'PRODUCTLANGUAGE'} = $officestartlanguage;
            }
        }
    }

    return \@languagearray;
}

##########################################################
# Converting the language array into a string for output
##########################################################

sub get_language_string
{
    my ($languagesref) = @_;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$languagesref}; $i++ )
    {
        $newstring = $newstring . ${$languagesref}[$i] . "_";
    }

    # remove ending underline

    $newstring =~ s/\_\s*$//;

    return \$newstring;
}

##########################################################
# Analyzing the languages in the languages array and
# returning the most important language
##########################################################

sub get_default_language
{
    my ($languagesref) = @_;

    return ${$languagesref}[0];     # ToDo, only returning the first language
}

1;
