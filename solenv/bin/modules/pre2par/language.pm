#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: language.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: hr $ $Date: 2005-09-28 13:19:05 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
