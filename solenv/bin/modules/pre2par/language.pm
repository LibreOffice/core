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

use strict;
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
