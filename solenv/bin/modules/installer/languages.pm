#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
