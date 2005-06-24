#*************************************************************************
#
#   $RCSfile: languages.pm,v $
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

package installer::languages;

use installer::converter;
use installer::existence;
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

    if ( $installer::globals::is_unix_multi )
    {
        if ( $installer::globals::languagelist =~ /\#/ )
        {
            installer::exiter::exit_program("ERROR: Hash not allowed in language list for Unix multlingual installation sets: $installer::globals::languagelist", "analyze_languagelist");
        }

        $first =~ s/\,/\#/g; # !!! That is the trick, to make different products for Unix multilingual installation sets

        $installer::globals::unixmultipath = $installer::globals::languagelist;
        $installer::globals::unixmultipath =~ s/\,/\_/g;    # hashes not allowed, comma to underline
        $installer::globals::alllanguagesinproductarrayref = installer::converter::convert_stringlist_into_array(\$installer::globals::unixmultipath, "_");
    }

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
        if (! installer::existence::exists_in_array(${$array1}[$i], $array2))
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

    # For some languages (that are not supported by Windows, english needs to be added to the installation set
    # Languages saved in "@installer::globals::noMSLocaleLangs"

    if ( $installer::globals::iswindowsbuild )
    {
        if ( all_elements_of_array1_in_array2(\@languagearray, \@installer::globals::noMSLocaleLangs) )
        {
            my $officestartlanguage = $languagearray[0];
            unshift(@languagearray, "en-US");   # am Anfang einfügen!
            $installer::globals::ismultilingual = 1;
            $installer::globals::added_english  = 1;
            $installer::globals::set_office_start_language  = 1;
            # setting the variable PRODUCTLANGUAGE, needed for Linguistic-ForceDefaultLanguage.xcu
            $allvariables->{'PRODUCTLANGUAGE'} = $officestartlanguage;
        }
    }

    return \@languagearray;
}

####################################################################################
# FAKE: The languages string may contain only "de", "en-US", instead of "01", ...
# But this has to be removed as soon as possible.
# In the future the languages are determined with "en-US" instead "01"
# already on the command line and in the zip list file.
####################################################################################

sub fake_languagesstring
{
    my ($stringref) = @_;

    # ATTENTION: This function has to be removed as soon as possible!

    $$stringref =~ s/01/en-US/;
    $$stringref =~ s/03/pt/;
    $$stringref =~ s/07/ru/;
    $$stringref =~ s/30/el/;
    $$stringref =~ s/31/nl/;
    $$stringref =~ s/33/fr/;
    $$stringref =~ s/34/es/;
    $$stringref =~ s/35/fi/;
    $$stringref =~ s/36/hu/;
    $$stringref =~ s/37/ca/;
    $$stringref =~ s/39/it/;
    $$stringref =~ s/42/cs/;
    $$stringref =~ s/43/sk/;
    $$stringref =~ s/44/en-GB/;
    $$stringref =~ s/45/da/;
    $$stringref =~ s/46/sv/;
    $$stringref =~ s/47/no/;
    $$stringref =~ s/48/pl/;
    $$stringref =~ s/49/de/;
    $$stringref =~ s/55/pt-BR/;
    $$stringref =~ s/66/th/;
    $$stringref =~ s/77/et/;
    $$stringref =~ s/81/ja/;
    $$stringref =~ s/82/ko/;
    $$stringref =~ s/86/zh-CN/;
    $$stringref =~ s/88/zh-TW/;
    $$stringref =~ s/90/tr/;
    $$stringref =~ s/91/hi-IN/;
    $$stringref =~ s/96/ar/;
    $$stringref =~ s/97/he/;
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

##########################################################
# Contains the installation set one of the languages
# ja, ko, zh-CH, or zh-TW ?
##########################################################

sub detect_asian_language
{
    my ($languagesref) = @_;

    my @asianlanguages = ("ja", "ko", "zh-CN", "zh-TW");

    my $containsasia = 0;

    for ( my $i = 0; $i <= $#{$languagesref}; $i++ )
    {
        my $onelang = ${$languagesref}[$i];
        $onelang =~ s/\s*$//;

        for ( my $j = 0; $j <= $#asianlanguages; $j++ )
        {
            my $asialang = $asianlanguages[$j];
            $asialang =~ s/\s*$//;

            if ( $onelang eq $asialang )
            {
                $containsasia = 1;
                last;
            }
        }

        if ( $containsasia ) { last; }
    }

    return $containsasia;
}

################################################################
# Contains the installation set one of the western languages
# ja, ko, zh-CH, or zh-TW ?
################################################################

sub detect_western_language
{
    my ($languagesref) = @_;

    my @westernlanguages = ("en-US","pt","ru","el","nl","fr","es","fi","hu","ca","sl","it","cs","sk","en-GB","da","sv","no","pl","de","pt-BR","th","et","tr","hi-IN","ar","he");

    my $containswestern = 0;

    for ( my $i = 0; $i <= $#{$languagesref}; $i++ )
    {
        my $onelang = ${$languagesref}[$i];
        $onelang =~ s/\s*$//;

        for ( my $j = 0; $j <= $#westernlanguages; $j++ )
        {
            my $westernlang = $westernlanguages[$j];
            $westernlang =~ s/\s*$//;

            if ( $onelang eq $westernlang )
            {
                $containswestern = 1;
                last;
            }
        }

        if ( $containswestern ) { last; }
    }

    return $containswestern;
}

1;
