#!/usr/bin/perl -w # -*- tab-width: 4; indent-tabs-mode: nil;

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
# Major Contributor(s):
# Copyright (C) 2012 Tor Lillqvist <tml@iki.fi> (initial developer)
# Copyright (C) 2012 SUSE Linux http://suse.com (initial developer's employer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable

my @FUNCS = qw(
    getAllCalendars
    getAllCurrencies
    getAllFormats0
    getBreakIteratorRules
    getCollationOptions
    getCollatorImplementation
    getContinuousNumberingLevels
    getDateAcceptancePatterns
    getFollowPageWords
    getForbiddenCharacters
    getIndexAlgorithm
    getLCInfo
    getLocaleItem
    getOutlineNumberingLevels
    getReservedWords
    getSearchOptions
    getTransliterations
    getUnicodeScripts
    getAllFormats1
    );

print 'extern "C" {

';

foreach my $lang (@ARGV) {
    foreach my $func (@FUNCS) {
        if ($func eq 'getAllFormats1') {
            if ($lang eq 'en_US') {
                printf("void %s_%s();\n", $func, $lang);
            }
        } else {
            printf("void %s_%s();\n", $func, $lang);
        }
    }
}

print '
static const struct {
    const char *pLocale;
';
foreach my $func (@FUNCS) {
    printf("    void  (*%s)();\n", $func);
}
print '} aLibTable[] = {
';

foreach my $lang (@ARGV) {
    printf("    {\n");
    printf("        \"%s\",\n", $lang);
    foreach my $func (@FUNCS) {
        if ($func eq 'getAllFormats1') {
            if ($lang eq 'en_US') {
                printf("        %s_%s,\n", $func, $lang);
            } else {
                printf("        0,\n");
            }
        } else {
            printf("        %s_%s,\n", $func, $lang);
        }
    }
    printf("    }%s\n", ($lang ne $ARGV[$#ARGV]) ? ',' : '');
}

print '};

}
';

# vim:set shiftwidth=4 softtabstop=4 expandtab:
