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

package pre2par::language;

use strict;

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

############################################
# collecting all replace variables
# in a language file
############################################

sub get_all_replace_variables
{
    my ($langfile) = @_;

    my %allvars = ();

    for ( my $i = 0; $i <= $#{$langfile}; $i++ )
    {
        if ( ${$langfile}[$i] =~ /^\s*\[\s*(.*?)\s*\]\s*$/ )
        {
        my $variable = $1;
#       print "lang block '$variable'\n";
        my @lang_block = ();
        my $counter;

        # Store the complete block in all languages for a specified variable
            for ( $counter = $i + 1; $counter <= $#{$langfile}; $counter++ ) {
        my $line = ${$langfile}[$counter];
        last if ($line =~ /^s*\[/); # next decl.
        push @lang_block, $line;
        }
#       print "$variable = '@lang_block'\n";
        $allvars{$variable} = \@lang_block;
        $i = $counter - 1;
        }
    }

    return \%allvars;
}

############################################
# localizing the par file with the
# corresponding language file
############################################

sub localize
{
    my ($parfile, $langfile) = @_;

    my $replace_hash = get_all_replace_variables($langfile);

    # parse lines of the form Name (st) = STR_NAME_MODULE_HELPPACK_OC;
    # for variable substitution
    my $langlinere = qr/^\s*\w+\s*\(([\w-]+)\)\s*\=\s*([\w-]+)\s*;/;
    for ( my $i = 0; $i <= $#{$parfile}; $i++ )
    {
        my $oneline = ${$parfile}[$i];

    if ( $oneline =~ $langlinere) {
        my $language = $1;   # can be "01" or "en" or "en-US" or ...
        my $variable = $2;

#       print "line '$oneline' split to '$language' '$variable'\n";

        if (defined $replace_hash->{$variable}) {
                    my $languageblock = $replace_hash->{$variable};
                    my $newstring = get_language_string_from_language_block($replace_hash->{$variable}, $language);
                    if ( $newstring eq "" ) { $newstring = "\"" . $variable . "\""; }

                    $oneline =~ s/$variable/$newstring/g;

                    ${$parfile}[$i] = $oneline;
                }
        }
    }
}

1;
