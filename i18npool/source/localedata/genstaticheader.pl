#!/usr/bin/perl -w # -*- tab-width: 4; indent-tabs-mode: nil;
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
