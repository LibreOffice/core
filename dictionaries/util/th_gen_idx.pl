:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
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

sub by_entry {
    my ($aent, $aoff) = split('\|',$a);
    my ($bent, $boff) = split('\|',$b);
    $aent cmp $bent;
}

#FIXME: someone may want "infile" or even parameter parsing
sub get_outfile {
    my $next_is_file = 0;
    foreach ( @ARGV ) {
        if ( $next_is_file ) {
            return $_
        }
        if ( $_ eq "-o" ) {
            $next_is_file = 1;
        }
    }
    return "";
}

sub usage {
    print "usage:\n";
    print "$0 -o outfile < input\n";

    exit 99;
}

# main routine
my $ne = 0;       # number of entries in index
my @tindex=();    # the index itself
my $foffset = 0;  # file position offset into thesaurus
my $rec="";       # current string and related pieces
my $rl=0;         # misc string length
my $entry="";     # current word being processed
my $nm=0;         # number of meaning for the current word
my $meaning="";   # current meaning and synonyms
my $p;            # misc uses
my $encoding;     # encoding used by text file
my $outfile = "";

$outfile = get_outfile();
usage() if ( $outfile eq "" );

# top line of thesaurus provides encoding
$encoding=<STDIN>;
$foffset = $foffset + length($encoding);
chomp($encoding);

# read thesaurus line by line
# first line of every block is an entry and meaning count
while ($rec=<STDIN>){
    $rl = length($rec);
    chomp($rec);
    ($entry, $nm) = split('\|',$rec);
    $p = 0;
    while ($p < $nm) {
        $meaning=<STDIN>;
        $rl = $rl + length($meaning);
        chomp($meaning);
        $p++;
    }
    push(@tindex,"$entry|$foffset");
    $ne++;
    $foffset = $foffset + $rl;
}

# now we have all of the information
# so sort it and then output the encoding, count and index data
@tindex = sort by_entry @tindex;

print "$outfile\n";
open OUTFILE, ">$outfile" or die "ERROR: Can't open $outfile for writing!";
print OUTFILE "$encoding\n";
print OUTFILE "$ne\n";
foreach $one (@tindex) {
    print OUTFILE "$one\n";
}
close OUTFILE;
