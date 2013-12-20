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
    printf ("#ifdef __arm\n");
    # Note: pc is the address of instruction being executed plus 8
    printf ("    mov ip, pc\n");
    printf ("#else\n");
    printf ("    adr x15, .+8\n");
    printf ("#endif\n");
    printf ("    b _privateSnippetExecutor\n");
    printf ("    .long %#08x\n", $funIndex);
    printf ("    .long %d\n", $vtableOffset);
}

sub gen_x86 ($$$)
{
    my ($funIndex, $vtableOffset, $executor) = @_;
    printf ("codeSnippet_%08x_%d_%s:\n", $funIndex, $vtableOffset, $executor);
    printf ("    movl \$%#08x, %%eax\n", $funIndex);
    printf ("    movl \$%d, %%edx\n", $vtableOffset);
    printf ("    jmp _privateSnippetExecutor%s\n", $executor);
}

printf (".text\n");

printf ("#if defined(__arm) || defined(__arm64)\n");

printf ("\n");
printf ("// Each codeSnippetX function stores into ip (arm64: x15) an address and branches to _privateSnippetExecutor\n");
printf ("// The address is that following the branch instruction, containing two 32-bit ints:\n");
printf ("// - the function index, as such or with the 0x80000000 bit set\n");
printf ("//   (to indicate that a hidden parameter (arm64: x8) is used for returning large values)\n");
printf ("// - the vtable offset\n");
printf ("\n");

printf ("    .align 4\n");
printf ("\n");

foreach my $funIndex (0 .. $nFunIndexes-1)
{
   foreach my $vtableOffset (0 .. $nVtableOffsets-1)
   {
       gen_arm ($funIndex, $vtableOffset);
       gen_arm ($funIndex|0x80000000, $vtableOffset);
   }
}

printf ("#else\n");
printf ("    .align 1, 0x90\n");

foreach my $funIndex (0 .. $nFunIndexes-1)
{
    foreach my $vtableOffset (0 .. $nVtableOffsets-1)
    {
        foreach my $executor ('General', 'Void', 'Hyper', 'Float', 'Double', 'Class')
        {
            gen_x86 ($funIndex, $vtableOffset, $executor);
            gen_x86 ($funIndex|0x80000000, $vtableOffset, $executor);
        }
    }
  }

printf ("#endif\n");

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
        printf ("#if defined(__arm) || defined(__arm64)\n");
        printf ("    .long codeSnippet_%08x_%d - _codeSnippets\n", $funIndex, $vtableOffset);
        printf ("    .long codeSnippet_%08x_%d - _codeSnippets\n", $funIndex|0x80000000, $vtableOffset);
        printf ("#else\n");
        foreach my $executor ('General', 'Void', 'Hyper', 'Float', 'Double', 'Class')
        {
            printf ("    .long codeSnippet_%08x_%d_%s - _codeSnippets\n", $funIndex, $vtableOffset, $executor);
            printf ("    .long codeSnippet_%08x_%d_%s - _codeSnippets\n", $funIndex|0x80000000, $vtableOffset, $executor);
        }
        printf ("#endif\n");
    }
}

# vim:set shiftwidth=4 softtabstop=4 expandtab:
