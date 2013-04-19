#!/usr/bin/perl -w
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
    printf ("\tmov ip, pc\n");
    printf ("\tb _privateSnippetExecutor\n");
    printf ("\t.long %#08x\n", $funIndex);
    printf ("\t.long %d\n", $vtableOffset);
}

sub gen_x86 ($$$)
{
    my ($funIndex, $vtableOffset, $executor) = @_;
    printf ("codeSnippet_%08x_%d_%s:\n", $funIndex, $vtableOffset, $executor);
    printf ("\tmovl \$%#08x, %%eax\n", $funIndex);
    printf ("\tmovl \$%d, %%edx\n", $vtableOffset);
    printf ("\tjmp _privateSnippetExecutor%s\n", $executor);
}

printf (".text\n");

printf ("#ifdef __arm\n");

printf ("\n");
printf ("@ Each codeSnippetX function stores pc into ip and branches to _privateSnippetExecutor\n");
printf ("@ The branch instruction is followed by two longs (that ip thus points to):\n");
printf ("@ - the function index, as such and with the 0x80000000 bit set\n");
printf ("@   (to indicate a hidden parameter for returning large values)\n");
printf ("@ - the vtable offset\n");
printf ("\n");

printf ("\t.align 4\n");
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
printf ("\t.align 1, 0x90\n");

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

printf ("\t.globl _nFunIndexes\n");
printf ("_nFunIndexes:\n");
printf ("\t.long %d\n", $nFunIndexes);

printf ("\t.globl _nVtableOffsets\n");
printf ("_nVtableOffsets:\n");
printf ("\t.long %d\n", $nVtableOffsets);

printf ("\t.globl _codeSnippets\n");
printf ("_codeSnippets:\n");

foreach my $funIndex (0 .. $nFunIndexes-1)
{
    foreach my $vtableOffset (0 .. $nVtableOffsets-1)
    {
	printf ("#ifdef __arm\n");
        printf ("\t.long codeSnippet_%08x_%d - _codeSnippets\n", $funIndex, $vtableOffset);
        printf ("\t.long codeSnippet_%08x_%d - _codeSnippets\n", $funIndex|0x80000000, $vtableOffset);
	printf ("#else\n");
	foreach my $executor ('General', 'Void', 'Hyper', 'Float', 'Double', 'Class')
	{
            printf ("\t.long codeSnippet_%08x_%d_%s - _codeSnippets\n", $funIndex, $vtableOffset, $executor);
            printf ("\t.long codeSnippet_%08x_%d_%s - _codeSnippets\n", $funIndex|0x80000000, $vtableOffset, $executor);
	}
	printf ("#endif\n");
    }
}
