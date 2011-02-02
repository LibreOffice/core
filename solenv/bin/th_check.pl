:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Steven Butler <sebutler@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

use strict;

sub processFile($) {
    my ($input) = @_;

    if (!open(INPUT, $input)) {
        print "FAIL: $input (no input found)\n";
        return 1;
    }
    # top line of thesaurus provides encoding (we ignore it)
    $_=<INPUT>;
    my $line = 1;


    my $expectedEntries;
    my $actualEntries = 0;
    my $word;
    my %words = ();
    my @errors = ();
    while (<INPUT>){
        $line++;
        s/\n$//;
        s/\r$//;
        s/\s+$//;
        if (m/^([^\|]+)\|(\d+)$/) {

            my $tword = $1;
            my $texpectedEntries = $2;
            #print $tword, $texpectedEntries, "\n";
            if (defined $expectedEntries) {
                # Check if the last word's actual entries matched the expected
                if ($actualEntries != $expectedEntries) {
                    push @errors, "$words{$word}: $word defined to have $expectedEntries but seems to have $actualEntries (next word ($tword) found on line $line\n";
                }
            }
            $word = $tword;
            $expectedEntries = $texpectedEntries;
            if (defined $words{$word}) {
                push @errors, "$line: $word previously defined on $words{$word}\n";
            } else {
                $words{$word} = $line;
            }
            $actualEntries = 0;
        } elsif (m/^[\(\-\|]/) {
            $actualEntries++;
        } else {
            push @errors, "$line: Unrecognised line format: $_\n";
            if (m/^(interj|prep|conj)\|/) {
                $actualEntries++;
            }
        }

    }
    close(INPUT);


    if (scalar(@errors)) {
        print $input, ':', join($input.':', @errors);
        return 1;
    }
    else {
        return 0;
    }
}

if (scalar(@ARGV) == 0) {
    print "Usage: $0 <thesaurus .dat file>+\n";
    print "\tscans for some common issues found in mythes format thesaurus files\n";
    exit(1);
}

my $errors = 0;
foreach (@ARGV) {
    $errors += processFile($_);
}
exit($errors);
