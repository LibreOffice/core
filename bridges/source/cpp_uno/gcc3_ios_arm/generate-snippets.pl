#!/usr/bin/perl -w

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
#       Tor Lillqvist <tml@iki.fi>
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

my $nFunIndexes = 5;
my $nVtableOffsets = 1;

sub gen_arm ($$)
{
    my ($funIndex, $vtableOffset) = @_;
    printf ("codeSnippet%08x%d:\n", $funIndex, $vtableOffset);
    printf ("\tmov ip, pc\n");
    printf ("\tb _privateSnippetExecutor\n");
    printf ("\t.long %#08x\n", $funIndex);
    printf ("\t.long %d\n", $vtableOffset);
}

sub gen_x86 ($$$)
{
    my ($funIndex, $vtableOffset, $executor) = @_;
    printf ("codeSnippet%08x%d%s:\n", $funIndex, $vtableOffset, $executor);
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
	printf ("\t.long codeSnippet%08x%d - _codeSnippets\n", $funIndex, $vtableOffset);
	printf ("\t.long codeSnippet%08x%d - _codeSnippets\n", $funIndex|0x80000000, $vtableOffset);
	printf ("#else\n");
	foreach my $executor ('General', 'Void', 'Hyper', 'Float', 'Double', 'Class')
	{
	    printf ("\t.long codeSnippet%08x%d%s - _codeSnippets\n", $funIndex, $vtableOffset, $executor);
	    printf ("\t.long codeSnippet%08x%d%s - _codeSnippets\n", $funIndex|0x80000000, $vtableOffset, $executor);
	}
	printf ("#endif\n");
    }
}
