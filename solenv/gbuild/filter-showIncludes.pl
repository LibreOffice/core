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

use File::Basename;
use File::Copy;
use File::Temp qw/tempfile/;

my $outfile = $ARGV[0];
my $objfile = $ARGV[1];
my $srcfile = $ARGV[2];
if ( !defined $outfile || !defined $objfile || !defined $srcfile ) {
    die "Not enough parameters to create dependencies.";
}

my $showincludes_prefix = $ENV{'SHOWINCLUDES_PREFIX'};
if ( !defined( $showincludes_prefix ) || $showincludes_prefix eq "" ) {
    $showincludes_prefix = 'Note: including file:';
}

my ($OUT, $tmp_filename) = tempfile( 'showIncludesXXXXXX', DIR => dirname( $outfile ) ) or die "Cannot create a temp file.";

print $OUT "$objfile: \\\n $srcfile";

my %seen;
my $first_line = 1;
while ( <STDIN> ) {
    if ( /^$showincludes_prefix/ ) {
        s/^$showincludes_prefix\s*//;
        s/\r$//;

        chomp;
        s/\\/\//g;


        # skip system headers, i.e. everything not under source or build dirs
        if ( /$ENV{'SRCDIR'}|$ENV{'OUTDIR'}|$ENV{'WORKDIR'}/ )
        {

        # X: -> /cygdrive/x/
        s/^(.):/\/cygdrive\/\l\1/;

        s/ /\\ /g;

        if ( !defined $seen{$_} ) {
            $seen{$_} = 1;
            print $OUT " \\\n  $_";
        }
        }
    }
    else {
        # skip the first line, it always just duplicates what is being
        # compiled
        print unless ( $first_line );
    }
    $first_line = 0;
}

print $OUT "\n";

# fdo#40099 if header.h does not exist, it will simply be considered out of
# date and any targets that use it as a prerequisite will be updated,
# which avoid misery when the header is deliberately deleted and removed
# as an include
# see http://www.makelinux.net/make3/make3-CHP-8-SECT-3
foreach my $key ( keys %seen ) {
  print $OUT "\n$key:\n";
}

close( $OUT ) or die "Cannot close $tmp_filename.";

move( $tmp_filename, $outfile ) or die "Cannot move $tmp_filename to $outfile.";

# vim: shiftwidth=4 softtabstop=4 expandtab:
