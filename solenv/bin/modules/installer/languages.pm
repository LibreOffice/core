#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



package installer::languages;

use installer::converter;
use installer::existence;
use installer::exiter;
use installer::globals;
use installer::remover;
use installer::ziplist;

=head2 analyze_languagelist()

    Convert $installer::globals::languagelist into $installer::globals::languageproduct.

    That is now just a replacement of '_' with ','.

    $installer::globals::languageproduct (specified by the -l option
    on the command line) can contain multiple languages separated by
    '_' to specify multilingual builds.

    Separation by '#' to build multiple languages (single or
    multilingual) in one make_installer.pl run is not supported
    anymore.  Call make_installer.pl with all languages separately instead:
    make_installer.pl -l L1#L2
    ->
    make_installer.pl -l L1
    make_installer.pl -l L2

=cut
sub analyze_languagelist()
{
    my $languageproduct = $installer::globals::languagelist;

    $languageproduct =~ s/\_/\,/g;  # substituting "_" by ",", in case of dmake definition 01_49

    if ($languageproduct =~ /\#/)
    {
        installer::exiter::exit_program(
            "building more than one language (or language set) is not supported anymore\n"
            ."please replace one call of 'make_installer.pl -l language1#language2'\n"
            ."with two calls 'make_installer.pl -l language1' and 'make_installer.pl -l language2'",
            "installer::language::analyze_languagelist");
    }

    $installer::globals::languageproduct = $languageproduct;
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

    if ( $installer::globals::iswindowsbuild )
    {
        my $furthercheck = 1;

        # For some languages (that are not supported by Windows, english needs to be added to the installation set
        # Languages saved in "@installer::globals::noMSLocaleLangs"

        if ( all_elements_of_array1_in_array2(\@languagearray, \@installer::globals::noMSLocaleLangs) )
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

#############################################################
# Contains the installation set one of the asian languages?
#############################################################

sub detect_asian_language
{
    my ($languagesref) = @_;

    my $containsasia = 0;

    for ( my $i = 0; $i <= $#{$languagesref}; $i++ )
    {
        my $onelang = ${$languagesref}[$i];
        $onelang =~ s/\s*$//;

        for ( my $j = 0; $j <= $#installer::globals::asianlanguages; $j++ )
        {
            my $asialang = $installer::globals::asianlanguages[$j];
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

#############################################################
# Contains the installation set only asian languages?
#############################################################

sub contains_only_asian_languages
{
    my ($languagesref) = @_;

    my $onlyasian = 1;

    for ( my $i = 0; $i <= $#{$languagesref}; $i++ )
    {
        my $onelang = ${$languagesref}[$i];
        $onelang =~ s/\s*$//;

        if (! installer::existence::exists_in_array($onelang, \@installer::globals::asianlanguages))
        {
            $onlyasian = 0;
            last;
        }
    }

    return $onlyasian;
}

################################################################
# Contains the installation set one of the western languages
################################################################

sub detect_western_language
{
    my ($languagesref) = @_;

    my $containswestern = 1;

    if ( contains_only_asian_languages($languagesref) ) { $containswestern = 0; }

    return $containswestern;
}

################################################################
# Determining the language used by the Java installer
################################################################

sub get_java_language
{
    my ( $language ) = @_;

    # my $javalanguage = "";

    # if ( $language eq "en-US" ) { $javalanguage = "en_US"; }
    # elsif ( $language eq "ar" ) { $javalanguage = "ar_AR"; }
    # elsif ( $language eq "bg" ) { $javalanguage = "bg_BG"; }
    # elsif ( $language eq "ca" ) { $javalanguage = "ca_CA"; }
    # elsif ( $language eq "cs" ) { $javalanguage = "cs_CS"; }
    # elsif ( $language eq "da" ) { $javalanguage = "da_DA"; }
    # elsif ( $language eq "de" ) { $javalanguage = "de"; }
    # elsif ( $language eq "de" ) { $javalanguage = "de_DE"; }
    # elsif ( $language eq "et" ) { $javalanguage = "et_ET"; }
    # elsif ( $language eq "el" ) { $javalanguage = "el_EL"; }
    # elsif ( $language eq "fi" ) { $javalanguage = "fi_FI"; }
    # elsif ( $language eq "fr" ) { $javalanguage = "fr_FR"; }
    # elsif ( $language eq "hu" ) { $javalanguage = "hu_HU"; }
    # elsif ( $language eq "he" ) { $javalanguage = "he_HE"; }
    # elsif ( $language eq "it" ) { $javalanguage = "it_IT"; }
    # elsif ( $language eq "nl" ) { $javalanguage = "nl_NL"; }
    # elsif ( $language eq "es" ) { $javalanguage = "es_ES"; }
    # elsif ( $language eq "sv" ) { $javalanguage = "sv_SV"; }
    # elsif ( $language eq "sk" ) { $javalanguage = "sk_SK"; }
    # elsif ( $language eq "pl" ) { $javalanguage = "pl_PL"; }
    # elsif ( $language eq "pt-BR" ) { $javalanguage = "pt_BR"; }
    # elsif ( $language eq "ru" ) { $javalanguage = "ru_RU"; }
    # elsif ( $language eq "tr" ) { $javalanguage = "tr_TR"; }
    # elsif ( $language eq "ja" ) { $javalanguage = "ja"; }
    # elsif ( $language eq "ja" ) { $javalanguage = "ja_JP"; }
    # elsif ( $language eq "ko" ) { $javalanguage = "ko_KR"; }
    # elsif ( $language eq "th" ) { $javalanguage = "th_TH"; }
    # elsif ( $language eq "zh-CN" ) { $javalanguage = "zh_CN"; }
    # elsif ( $language eq "zh-TW" ) { $javalanguage = "zh_TW"; }

    # languages not defined yet
    # if ( $javalanguage eq "" )
    # {
    #   $javalanguage = $language;
    #   $javalanguage =~ s/\-/\_/;
    # }

    $javalanguage = $language;
    $javalanguage =~ s/\-/\_/;

    return $javalanguage;
}

1;
