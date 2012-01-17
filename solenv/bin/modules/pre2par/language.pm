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
