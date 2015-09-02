#!/usr/bin/env perl
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# The following files must be available in a ./input subdir:

# <http://oss.software.ibm.com/cvs/icu/~checkout~/charset/data/xml/
# gb-18030-2000.xml?rev=1.4&content-type=text/plain>:
#  "modified version="3" date="2001-02-21""

$id = "Gb180302000";

sub printUtf32
{
    my $utf32 = $_[0];
    return sprintf("U+%04X", $utf32);
}

sub printGb
{
    if (defined($_[2]))
    {
        return sprintf("%02X%02X%02X%02X", $_[0], $_[1], $_[2], $_[3]);
    }
    elsif (defined($_[1]))
    {
        return sprintf("%02X%02X", $_[0], $_[1]);
    }
    else
    {
        return sprintf("%02X", $_[0]);
    }
}

$gb_map_2_count = 0;
$gb_map_4_count = 0;
$gb_map_4_ranges = 0;
$gb_map_4_max = 0;
$uni_map_count = 0;

$range_count = 0;

if (1)
{
    $filename = "gb-18030-2000.xml";
    open IN, ("input/" . $filename) or die "Cannot read " . $filename;
    while (<IN>)
    {
        if (/^[ \t]*<a +u=\"([0-9A-F]+)\" +b=\"([0-7][0-9A-F])\"\/>$/)
        {
            $utf32 = oct("0x" . $1);
            $gb1 = oct("0x" . $2);
            ($utf32 == $gb1)
                or die "Bad " . printUtf32($utf32) . " to " . printGb($gb1);
        }
        elsif (/^[ \t]*<a +u=\"([0-9A-F]+)\" +b=\"([89A-F][0-9A-F]) ([4-789A-F][0-9A-F])\"\/>$/)
        {
            $utf32 = oct("0x" . $1);
            $gb1 = oct("0x" . $2);
            $gb2 = oct("0x" . $3);
            $gb_code = ($gb1 - 0x81) * 190
                           + ($gb2 <= 0x7E ? $gb2 - 0x40 : $gb2 - 0x80 + 63);
            !defined($gb_map_2[$gb_code])
                or die "Redefined " . printGb($gb1, $gb2);
            $gb_map_2[$gb_code] = $utf32;
            ++$gb_map_2_count;

            !defined($uni_map[$utf32]) or die "Double Unicode mapping";
            $uni_map[$utf32] = $gb1 << 8 | $gb2;
            ++$uni_map_count;
        }
        elsif (/^[ \t]*<a +u=\"([0-9A-F]+)\" +b=\"([89A-F][0-9A-F]) (3[0-9]) ([89A-F][0-9A-F]) (3[0-9])\"\/>$/)
        {
            $utf32 = oct("0x" . $1);
            $gb1 = oct("0x" . $2);
            $gb2 = oct("0x" . $3);
            $gb3 = oct("0x" . $4);
            $gb4 = oct("0x" . $5);
            $gb_code = ($gb1 - 0x81) * 12600
                           + ($gb2 - 0x30) * 1260
                           + ($gb3 - 0x81) * 10
                           + ($gb4 - 0x30);
            !defined($gb_map_4[$gb_code])
                or die "Redefined " . printGb($gb1, $gb2, $gb3, $gb4);
            $gb_map_4[$gb_code] = $utf32;
            ++$gb_map_4_count;
            $gb_map_4_max = $gb_code if ($gb_code > $gb_map_4_max);

            !defined($uni_map[$utf32]) or die "Double Unicode mapping";
            $uni_map[$utf32] = $gb1 << 24 | $gb2 << 16 | $gb3 << 8 | $gb4;
            ++$uni_map_count;
        }
        elsif (/<a /)
        {
            die "Bad format";
        }
        elsif (/^[ \t]*<range +uFirst=\"([0-9A-F]+)\" +uLast=\"([0-9A-F]+)\" +bFirst=\"([89A-F][0-9A-F]) (3[0-9]) ([89A-F][0-9A-F]) (3[0-9])\" +bLast=\"([89A-F][0-9A-F]) (3[0-9]) ([89A-F][0-9A-F]) (3[0-9])\" +bMin=\"81 30 81 30\" +bMax=\"FE 39 FE 39\"\/>$/)
        {
            $utf32_first = oct("0x" . $1);
            $utf32_last = oct("0x" . $2);
            $gb1_first = oct("0x" . $3);
            $gb2_first = oct("0x" . $4);
            $gb3_first = oct("0x" . $5);
            $gb4_first = oct("0x" . $6);
            $gb1_last = oct("0x" . $7);
            $gb2_last = oct("0x" . $8);
            $gb3_last = oct("0x" . $9);
            $gb4_last = oct("0x" . $10);
            $linear_first
                = ($gb1_first - 0x81) * 12600
                    + ($gb2_first - 0x30) * 1260
                        + ($gb3_first - 0x81) * 10
                            + ($gb4_first - 0x30);
            $linear_last
                = ($gb1_last - 0x81) * 12600
                    + ($gb2_last - 0x30) * 1260
                        + ($gb3_last - 0x81) * 10
                            + ($gb4_last - 0x30);
            ($utf32_last - $utf32_first == $linear_last - $linear_first)
                or die "Bad range";
            if ($linear_first != 189000 || $linear_last != 1237575)
            {
                $range_uni_first[$range_count] = $utf32_first;
                $range_uni_last[$range_count]
                    = ($utf32_last == 0xD7FF ? 0xDFFF : $utf32_last);
                $range_linear_first[$range_count] = $linear_first;
                $range_linear_last[$range_count] = $linear_last;
                ++$range_count;
                $gb_map_4_ranges += $linear_last - $linear_first + 1;
                $gb_map_4_max = $linear_last
                    if ($linear_last > $gb_map_4_max);
            }
        }
        elsif (/<range /)
        {
            die "Bad format";
        }
    }
    close IN;
}

print "gb_map_2_count = ", $gb_map_2_count,
      ", gb_map_4_count = ", $gb_map_4_count,
      ", gb_map_4_ranges = ", $gb_map_4_ranges,
      ", gb_map_4_max = ", $gb_map_4_max,
      ", uni_map_count = ", $uni_map_count, "\n";
($gb_map_2_count == 23940) or die "Bad gb_map_2_count != 23940";
($gb_map_4_max == $gb_map_4_count + $gb_map_4_ranges - 1)
    or die "Bad gb_map_4_max != gb_map_4_count + gb_map_4_ranges";
($uni_map_count + $gb_map_4_ranges == 0x10000 - (0xE000 - 0xD800) - 0x80)
    or die "Bad uni_map_count";

$range_index = 0;
$gb_nonrangedataindex[$range_index] = $gb_map_2_count;
for ($gb_code = 0; $gb_code < $gb_map_4_max; ++$gb_code)
{
    if (defined($gb_map_4[$gb_code]))
    {
        $gb_map_2[$gb_map_2_count++] = $gb_map_4[$gb_code];
    }
    else
    {
        ($gb_code == $range_linear_first[$range_index]) or die "Bad input";
        $gb_code = $range_linear_last[$range_index];
        ++$range_index;
        $gb_nonrangedataindex[$range_index] = $gb_map_2_count;
    }
}
($range_index == $range_count) or die "Bad input";

$filename = lc($id) . ".tab";
open OUT, ("> " . $filename) or die "Cannot write " . $filename;

{
    $filename = lc($id). ".pl";
    open IN, $filename or die "Cannot read ". $filename;
    $first = 1;
    while (<IN>)
    {
        if (/^\#!.*$/)
        {
        }
        elsif (/^\#(\*.*)$/)
        {
            if ($first == 1)
            {
                print OUT "/", $1, "\n";
                $first = 0;
            }
            else
            {
                print OUT " ", substr($1, 0, length($1) - 1), "/\n";
            }
        }
        elsif (/^\# (.*)$/)
        {
            print OUT " *", $1, "\n";
        }
        elsif (/^\#(.*)$/)
        {
            print OUT " *", $1, "\n";
        }
        else
        {
            goto done;
        }
    }
  done:
}

print OUT "\n",
          "#include \"convertgb18030.h\"\n",
          "\n",
          "#include \"sal/types.h\"\n",
          "\n";

print OUT "static sal_Unicode const aImpl", $id, "ToUnicodeData[] = {\n  ";
for ($gb_code = 0; $gb_code < $gb_map_2_count; ++$gb_code)
{
    printf OUT "0x%04X,", $gb_map_2[$gb_code];
    if ($gb_code % 8 == 7 && $gb_code != $gb_map_2_count - 1)
    {
        print OUT "\n  ";
    }
}
print OUT "\n};\n\n";

print OUT "static ImplGb180302000ToUnicodeRange const\n    aImpl",
          $id,
          "ToUnicodeRanges[] = {\n";
for ($range_index = 0; $range_index < $range_count; ++$range_index)
{
    printf OUT "  { %d, %d, %d, 0x%04X },\n",
               $gb_nonrangedataindex[$range_index],
               $range_linear_first[$range_index],
               $range_linear_last[$range_index] + 1,
               $range_uni_first[$range_index];
}
print OUT "  { -1, 0, 0, 0 }\n};\n\n";

print OUT "static sal_uInt32 const aImplUnicodeTo", $id, "Data[] = {\n  ";
$index = 0;
$range_index = 0;
$uni_nonrangedataindex[$range_index] = $index;
for ($utf32 = 0x80; $utf32 <= 0xFFFF; ++$utf32)
{
    if (defined($uni_map[$utf32]))
    {
        if ($index > 0 && ($index - 1) % 6 == 5)
        {
            print OUT "\n  ";
        }
        $bytes = $uni_map[$utf32];
        printf OUT ($bytes <= 0xFFFF ? "    0x%04X," : "0x%08X,"), $bytes;
        ++$index;
    }
    else
    {
        ($utf32 == $range_uni_first[$range_index]) or die "Bad input";
        $utf32 = $range_uni_last[$range_index];
        ++$range_index;
        $uni_nonrangedataindex[$range_index] = $index;
    }
}
($range_index == $range_count) or die "Bad input";
print OUT "\n};\n\n";

print OUT "static ImplUnicodeToGb180302000Range const\n    aImplUnicodeTo",
          $id,
          "Ranges[] = {\n";
for ($range_index = 0; $range_index < $range_count; ++$range_index)
{
    printf OUT "  { %d, 0x%04X, 0x%04X, %d },\n",
               $uni_nonrangedataindex[$range_index],
               $range_uni_first[$range_index],
               $range_uni_last[$range_index],
               $range_linear_first[$range_index];
}
print OUT "};\n";

close OUT;
