#!/usr/bin/env perl

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This tool is used to prepare lookup tables of Unicode character properties.
# The properties are read from the Unicode Character Database and compiled into
# multi-level arrays for efficient lookup.
#
# To regenerate the tables in VerticalOrientationData.cxx:
#
# (1) Download the current Unicode data files from
#
#     We require the latest data file for UTR50, currently revision-17:
#        http://www.unicode.org/Public/vertical/revision-17/VerticalOrientation-17.txt
#
#
# (2) Run this tool using a command line of the form
#
#         perl genVerticalOrientationData.pl      \
#                 /path/to/VerticalOrientation-17.txt
#
#     This will generate (or overwrite!) the files
#
#         VerticalOrientationData.cxx
#
#     in the current directory.

use strict;
use List::Util qw(first);

my $DATA_FILE = $ARGV[0];

my %verticalOrientationCode = (
  'U' => 0,  #   U - Upright, the same orientation as in the code charts
  'R' => 1,  #   R - Rotated 90 degrees clockwise compared to the code charts
  'Tu' => 2, #   Tu - Transformed typographically, with fallback to Upright
  'Tr' => 3  #   Tr - Transformed typographically, with fallback to Rotated
);

my @verticalOrientation;
for (my $i = 0; $i < 0x110000; ++$i) {
    $verticalOrientation[$i] = 1; # default for unlisted codepoints is 'R'
}

# read VerticalOrientation-17.txt
my @versionInfo;
open FH, "< $DATA_FILE" or die "can't open UTR50 data file VerticalOrientation-17.txt\n";
push @versionInfo, "";
while (<FH>) {
    chomp;
    push @versionInfo, $_;
    last if /Date:/;
}
while (<FH>) {
    chomp;
    s/#.*//;
    if (m/([0-9A-F]{4,6})(?:\.\.([0-9A-F]{4,6}))*\s*;\s*([^ ]+)/) {
        my $vo = $3;
        warn "unknown Vertical_Orientation code $vo"
            unless exists $verticalOrientationCode{$vo};
        $vo = $verticalOrientationCode{$vo};
        my $start = hex "0x$1";
        my $end = (defined $2) ? hex "0x$2" : $start;
        for (my $i = $start; $i <= $end; ++$i) {
            $verticalOrientation[$i] = $vo;
        }
    }
}
close FH;

my $timestamp = gmtime();

open DATA_TABLES, "> VerticalOrientationData.cxx" or die "unable to open VerticalOrientationData.cxx for output";

my $licenseBlock = q[
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Derived from the Unicode Character Database by genVerticalOrientationData.pl
 *
 * For Unicode terms of use, see http://www.unicode.org/terms_of_use.html
 */
];

my $versionInfo = join("\n", @versionInfo);

print DATA_TABLES <<__END;
$licenseBlock
/*
 * Created on $timestamp from UCD data files with version info:
 *

$versionInfo

 *
 * * * * * This file contains MACHINE-GENERATED DATA, do not edit! * * * * *
 */

__END

our $totalData = 0;

sub sprintVerticalOrientation
{
  my $usv = shift;
  return sprintf("%d,",
                 $verticalOrientation[$usv]);
}

&genTables("VerticalOrientation", "uint8_t", 9, 7, \&sprintVerticalOrientation, 16, 1, 1);

sub genTables
{
  my ($prefix, $type, $indexBits, $charBits, $func, $maxPlane, $bytesPerEntry, $charsPerEntry) = @_;

  print DATA_TABLES "#define k${prefix}MaxPlane  $maxPlane\n";
  print DATA_TABLES "#define k${prefix}IndexBits $indexBits\n";
  print DATA_TABLES "#define k${prefix}CharBits  $charBits\n";

  my $indexLen = 1 << $indexBits;
  my $charsPerPage = 1 << $charBits;
  my %charIndex = ();
  my %pageMapIndex = ();
  my @pageMap = ();
  my @char = ();

  my $planeMap = "\x00" x $maxPlane;
  foreach my $plane (0 .. $maxPlane) {
    my $pageMap = "\x00" x $indexLen * 2;
    foreach my $page (0 .. $indexLen - 1) {
        my $charValues = "";
        for (my $ch = 0; $ch < $charsPerPage; $ch += $charsPerEntry) {
            my $usv = $plane * 0x10000 + $page * $charsPerPage + $ch;
            $charValues .= &$func($usv);
        }
        chop $charValues;

        unless (exists $charIndex{$charValues}) {
            $charIndex{$charValues} = scalar keys %charIndex;
            $char[$charIndex{$charValues}] = $charValues;
        }
        substr($pageMap, $page * 2, 2) = pack('S', $charIndex{$charValues});
    }

    unless (exists $pageMapIndex{$pageMap}) {
        $pageMapIndex{$pageMap} = scalar keys %pageMapIndex;
        $pageMap[$pageMapIndex{$pageMap}] = $pageMap;
    }
    if ($plane > 0) {
        substr($planeMap, $plane - 1, 1) = pack('C', $pageMapIndex{$pageMap});
    }
  }

  if ($maxPlane) {
    print DATA_TABLES "static const uint8_t s${prefix}Planes[$maxPlane] = {";
    print DATA_TABLES join(',', map { sprintf("%d", $_) } unpack('C*', $planeMap));
    print DATA_TABLES "};\n\n";
  }

  my $chCount = scalar @char;
  my $pmBits = $chCount > 255 ? 16 : 8;
  my $pmCount = scalar @pageMap;
  if ($maxPlane == 0) {
    die "there should only be one pageMap entry!" if $pmCount > 1;
    print DATA_TABLES "static const uint${pmBits}_t s${prefix}Pages[$indexLen] = {\n";
  } else {
    print DATA_TABLES "static const uint${pmBits}_t s${prefix}Pages[$pmCount][$indexLen] = {\n";
  }
  for (my $i = 0; $i < scalar @pageMap; ++$i) {
    print DATA_TABLES $maxPlane > 0 ? "  {" : "  ";
    print DATA_TABLES join(',', map { sprintf("%d", $_) } unpack('S*', $pageMap[$i]));
    print DATA_TABLES $maxPlane > 0 ? ($i < $#pageMap ? "},\n" : "}\n") : "\n";
  }
  print DATA_TABLES "};\n\n";

  my $pageLen = $charsPerPage / $charsPerEntry;
  print DATA_TABLES "static const $type s${prefix}Values[$chCount][$pageLen] = {\n";
  for (my $i = 0; $i < scalar @char; ++$i) {
    print DATA_TABLES "  {";
    print DATA_TABLES $char[$i];
    print DATA_TABLES $i < $#char ? "},\n" : "}\n";
  }
  print DATA_TABLES "};\n";

  my $dataSize = $pmCount * $indexLen * $pmBits/8 +
                 $chCount * $pageLen * $bytesPerEntry +
                 $maxPlane;
  $totalData += $dataSize;

  print STDERR "Data for $prefix = $dataSize\n";
}
print DATA_TABLES <<__END;
/*
 * * * * * This file contains MACHINE-GENERATED DATA, do not edit! * * * * *
 */
__END

close DATA_TABLES;
