#!/usr/bin/env perl
#
# filter-showIncludes.pl depfile.d objfile.o orginal.cxx
#
# Create dependency information from the output of cl.exe's showInclude.  It
# needs additional information - the output name to which to write, objfile
# that depends on the includes, and the original file name.
#
# It also consolidates the file names to a canonical form, and filters out
# duplicates.
#
# LGPL v3 / GPL v3 / MPL 1.1
#
# Original author: Jan Holesovsky <kendy@suse.cz>

my $outfile = $ARGV[0];
my $objfile = $ARGV[1];
my $srcfile = $ARGV[2];
if ( !defined $outfile || !defined $objfile || !defined $srcfile ) {
    die "Not enough parameters to create dependencies.";
}

open( OUT, "> $outfile" ) or die "Cannot open $outfile for writing.";
print OUT "$objfile: \\\n $srcfile";

my %seen;
my $first_line = 1;
while ( <STDIN> ) {
    if ( /^Note: including file:/ ) {
        s/^Note: including file:\s*//;
        s/\r$//;

        chomp;
        s/\\/\//g;

        # X: -> /cygdrive/x/
        s/^(.):/\/cygdrive\/\l\1/;

        s/ /\\ /g;

        if ( !defined $seen{$_} ) {
            $seen{$_} = 1;
            print OUT " \\\n  $_";
        }
    }
    else {
        # skip the first line, it always just duplicates what is being
        # compiled
        print unless ( $first_line );
    }
    $first_line = 0;
}

print OUT "\n";
close( OUT ) or die "Cannot close $outfile.";

# vim: shiftwidth=4 softtabstop=4 expandtab:
