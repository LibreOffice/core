#!/usr/bin/perl -w # -*- tab-width: 4; indent-tabs-mode: nil; cperl-indent-level: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

my $nFunIndexes = 8;
my $nVtableOffsets = 4;

sub gen_arm ($$)
{
    my ($funIndex, $vtableOffset) = @_;
    printf ("codeSnippet_%08x_%d:\n", $funIndex, $vtableOffset);
    printf ("    adr x15, .+8\n");
    printf ("    b _privateSnippetExecutor\n");
    printf ("    .long %#08x\n", $funIndex);
    printf ("    .long %d\n", $vtableOffset);
}

printf (".text\n");
printf ("\n");
printf ("    .align 4\n");
printf ("\n");

foreach my $funIndex (0 .. $nFunIndexes-1)
{
   foreach my $vtableOffset (0 .. $nVtableOffsets-1)
   {
       gen_arm ($funIndex, $vtableOffset);
   }
}

printf ("    .globl _nFunIndexes\n");
printf ("_nFunIndexes:\n");
printf ("    .long %d\n", $nFunIndexes);

printf ("    .globl _nVtableOffsets\n");
printf ("_nVtableOffsets:\n");
printf ("    .long %d\n", $nVtableOffsets);

printf ("    .globl _codeSnippets\n");
printf ("_codeSnippets:\n");

foreach my $funIndex (0 .. $nFunIndexes-1)
{
    foreach my $vtableOffset (0 .. $nVtableOffsets-1)
    {
        printf ("    .long codeSnippet_%08x_%d - _codeSnippets\n", $funIndex, $vtableOffset);
    }
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
