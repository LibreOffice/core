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

# <http://www.info.gov.hk/digital21/eng/hkscs/download/big5-iso.txt>

# <http://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/OTHER/BIG5.TXT>:
#  "Unicode version: 1.1    Table version: 0.0d3    Date: 11 February 1994"
#  Only used to track Unicode characters that are mapped from both Big5 and
#  HKSCS.

# <http://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP950.TXT>:
#  "Unicode version: 2.0    Table version: 2.01    Date: 1/7/2000"
#  Only used to track Unicode characters that are mapped from both CP950 and
#  HKSCS.

$surrogates = 0; # set to 1 to allow mappings to Unicode beyond Plane 0

$id = "Big5Hkscs2001";

sub isValidUtf32
{
    my $utf32 = $_[0];
    return $utf32 >= 0 && $utf32 <= 0x10FFFF
           && !($utf32 >= 0xD800 && $utf32 <= 0xDFFF)
           && !($utf32 >= 0xFDD0 && $utf32 <= 0xFDEF)
           && ($utf32 & 0xFFFF) < 0xFFFE;
}

sub printUtf32
{
    my $utf32 = $_[0];
    return sprintf("U+%04X", $utf32);
}

sub isValidBig5
{
    my $big5 = $_[0];
    my $big5_row = $big5 >> 8;
    my $big5_column = $big5 & 0xFF;
    return $big5_row >= 0x81 && $big5_row <= 0xFE
           && ($big5_column >= 0x40 && $big5_column <= 0x7E
               || $big5_column >= 0xA1 && $big5_column <= 0xFE);
}

sub printBig5
{
    my $big5 = $_[0];
    return sprintf("%04X", $big5);
}

sub printStats
{
    my $used = $_[0];
    my $space = $_[1];
    return sprintf("%d/%d bytes (%.1f%%)",
                   $used,
                   $space,
                   $used * 100 / $space);
}

sub printSpaces
{
    my $column_width = $_[0];
    my $columns_per_line = $_[1];
    my $end = $_[2];
    $output = "";
    for ($i = int($end / $columns_per_line) * $columns_per_line;
         $i < $end;
         ++$i)
    {
        for ($j = 0; $j < $column_width; ++$j)
        {
            $output = $output . " ";
        }
    }
    return $output;
}

sub addMapping
{
    my $utf32 = $_[0];
    my $big5 = $_[1];
    my $comp = $_[2];

    $uni_plane = $utf32 >> 16;
    $uni_page = ($utf32 >> 8) & 0xFF;
    $uni_index = $utf32 & 0xFF;

    if (!defined($uni_plane_used[$uni_plane])
        || !defined($uni_page_used[$uni_plane][$uni_page])
        || !defined($uni_map[$uni_plane][$uni_page][$uni_index]))
    {
        $uni_map[$uni_plane][$uni_page][$uni_index] = $big5;
        $uni_plane_used[$uni_plane] = 1;
        $uni_page_used[$uni_plane][$uni_page] = 1;
        if ($comp != -1)
        {
            ++$compat[$comp];
        }
    }
    else
    {
        $big5_1 = $uni_map[$uni_plane][$uni_page][$uni_index];
        print "WARNING!  Mapping ", printUtf32($utf32), " to ",
              printBig5($big5_1), ", NOT ", ($comp ? "compat " : ""),
              printBig5($big5), "\n";
    }
}

# Build mappings to track Unicode characters that are mapped from both Big5/
# CP950 and HKSCS:
{
    $filename = "BIG5.TXT";
    open IN, ("input/" . $filename) or die "Cannot read " . $filename;
    while (<IN>)
    {
        if (/(0x[0-9A-F][0-9A-F][0-9A-F][0-9A-F])[ \t]+(0x[0-9A-F]+)[ \t]+\#.*$/)
        {
            $big5 = oct($1);
            $utf32 = oct($2);
            isValidBig5($big5)
                or die "Bad Big5 char " . printBig5($big5);
            isValidUtf32($utf32)
                or die "Bad UTF32 char " . printUtf32($utf32);
            if ($utf32 != 0xFFFD)
            {
                if (defined($underlying_big5[$utf32]))
                {
                    print "WARNING!  In ", $filename, ", both ",
                          printBig5($underlying_big5[$utf32]), " and ",
                          printBig5($big5), " map to ", printUtf32($utf32),
                          "\n";
                }
                else
                {
                    $underlying_big5[$utf32] = $big5;
                }
            }
        }
    }
    close IN;

    $filename = "CP950.TXT";
    open IN, ("input/" . $filename) or die "Cannot read " . $filename;
    while (<IN>)
    {
        if (/(0x[0-9A-F][0-9A-F][0-9A-F][0-9A-F])[ \t]+(0x[0-9A-F]+)[ \t]+\#.*$/)
        {
            $big5 = oct($1);
            $utf32 = oct($2);
            isValidBig5($big5)
                or die "Bad Big5 char " . printBig5($big5);
            isValidUtf32($utf32)
                or die "Bad UTF32 char " . printUtf32($utf32);
            if (defined($underlying_cp950[$utf32]))
            {
                print "WARNING!  In ", $filename, ", both ",
                      printBig5($underlying_cp950[$utf32]), " and ",
                      printBig5($big5), " map to ", printUtf32($utf32), "\n";
            }
            else
            {
                $underlying_cp950[$utf32] = $big5;
            }
        }
    }
    close IN;
}

# The following are mapped by the underlying RTL_TEXTENCODING_BIG5 to some
# nonstandard Unicode points, so they are explicitly mentioned here to map
# to the standard Unicode PUA points.  (In the other direction, the unofficial
# mappings from Unicode to RTL_TEXTENCODING_BIG5 C6A1--C7FE are harmless,
# since all Unicode characters involved are already covered by the official
# Big5-HKSCS mappings.)
$big5_map[0xC6][0xCF] = 0xF6E0; addMapping(0xF6E0, 0xC6CF, -1);
$big5_map[0xC6][0xD3] = 0xF6E4; addMapping(0xF6E4, 0xC6D3, -1);
$big5_map[0xC6][0xD5] = 0xF6E6; addMapping(0xF6E6, 0xC6D5, -1);
$big5_map[0xC6][0xD7] = 0xF6E8; addMapping(0xF6E8, 0xC6D7, -1);
$big5_map[0xC6][0xDE] = 0xF6EF; addMapping(0xF6EF, 0xC6DE, -1);
$big5_map[0xC6][0xDF] = 0xF6F0; addMapping(0xF6F0, 0xC6DF, -1);

# The following implements the mapping of Big5-HKSCS compatibility points
# (GCCS characters unified with other HKSCS characters) to Unicode.  In the
# other direction, characters from Unicode's PUA will map to these Big5-HKSCS
# compatibility points.  (See the first list in <http://www.info.gov.hk/
# digital21/eng/hkscs/download/big5cmp.txt>.)
$big5_map[0x8E][0x69] = 0x7BB8;
$big5_map[0x8E][0x6F] = 0x7C06;
$big5_map[0x8E][0x7E] = 0x7CCE;
$big5_map[0x8E][0xAB] = 0x7DD2;
$big5_map[0x8E][0xB4] = 0x7E1D;
$big5_map[0x8E][0xCD] = 0x8005;
$big5_map[0x8E][0xD0] = 0x8028;
$big5_map[0x8F][0x57] = 0x83C1;
$big5_map[0x8F][0x69] = 0x84A8;
$big5_map[0x8F][0x6E] = 0x840F;
$big5_map[0x8F][0xCB] = 0x89A6;
$big5_map[0x8F][0xCC] = 0x89A9;
$big5_map[0x8F][0xFE] = 0x8D77;
$big5_map[0x90][0x6D] = 0x90FD;
$big5_map[0x90][0x7A] = 0x92B9;
$big5_map[0x90][0xDC] = 0x975C;
$big5_map[0x90][0xF1] = 0x97FF;
$big5_map[0x91][0xBF] = 0x9F16;
$big5_map[0x92][0x44] = 0x8503;
$big5_map[0x92][0xAF] = 0x5159;
$big5_map[0x92][0xB0] = 0x515B;
$big5_map[0x92][0xB1] = 0x515D;
$big5_map[0x92][0xB2] = 0x515E;
$big5_map[0x92][0xC8] = 0x936E;
$big5_map[0x92][0xD1] = 0x7479;
$big5_map[0x94][0x47] = 0x6D67;
$big5_map[0x94][0xCA] = 0x799B;
$big5_map[0x95][0xD9] = 0x9097;
$big5_map[0x96][0x44] = 0x975D;
$big5_map[0x96][0xED] = 0x701E;
$big5_map[0x96][0xFC] = 0x5B28;
$big5_map[0x9B][0x76] = 0x7201;
$big5_map[0x9B][0x78] = 0x77D7;
$big5_map[0x9B][0x7B] = 0x7E87;
$big5_map[0x9B][0xC6] = 0x99D6;
$big5_map[0x9B][0xDE] = 0x91D4;
$big5_map[0x9B][0xEC] = 0x60DE;
$big5_map[0x9B][0xF6] = 0x6FB6;
$big5_map[0x9C][0x42] = 0x8F36;
$big5_map[0x9C][0x53] = 0x4FBB;
$big5_map[0x9C][0x62] = 0x71DF;
$big5_map[0x9C][0x68] = 0x9104;
$big5_map[0x9C][0x6B] = 0x9DF0;
$big5_map[0x9C][0x77] = 0x83CF;
$big5_map[0x9C][0xBC] = 0x5C10;
$big5_map[0x9C][0xBD] = 0x79E3;
$big5_map[0x9C][0xD0] = 0x5A67;
$big5_map[0x9D][0x57] = 0x8F0B;
$big5_map[0x9D][0x5A] = 0x7B51;
$big5_map[0x9D][0xC4] = 0x62D0;
$big5_map[0x9E][0xA9] = 0x6062;
$big5_map[0x9E][0xEF] = 0x75F9;
$big5_map[0x9E][0xFD] = 0x6C4A;
$big5_map[0x9F][0x60] = 0x9B2E;
$big5_map[0x9F][0x66] = 0x9F17;
$big5_map[0x9F][0xCB] = 0x50ED;
$big5_map[0x9F][0xD8] = 0x5F0C;
$big5_map[0xA0][0x63] = 0x880F;
$big5_map[0xA0][0x77] = 0x62CE;
$big5_map[0xA0][0xD5] = 0x7468;
$big5_map[0xA0][0xDF] = 0x7162;
$big5_map[0xA0][0xE4] = 0x7250;
$big5_map[0xFA][0x5F] = 0x5029;
$big5_map[0xFA][0x66] = 0x507D;
$big5_map[0xFA][0xBD] = 0x5305;
$big5_map[0xFA][0xC5] = 0x5344;
$big5_map[0xFA][0xD5] = 0x537F;
$big5_map[0xFB][0x48] = 0x5605;
$big5_map[0xFB][0xB8] = 0x5A77;
$big5_map[0xFB][0xF3] = 0x5E75;
$big5_map[0xFB][0xF9] = 0x5ED0;
$big5_map[0xFC][0x4F] = 0x5F58;
$big5_map[0xFC][0x6C] = 0x60A4;
$big5_map[0xFC][0xB9] = 0x6490;
$big5_map[0xFC][0xE2] = 0x6674;
$big5_map[0xFC][0xF1] = 0x675E;
$big5_map[0xFD][0xB7] = 0x6C9C;
$big5_map[0xFD][0xB8] = 0x6E1D;
$big5_map[0xFD][0xBB] = 0x6E2F;
$big5_map[0xFD][0xF1] = 0x716E;
$big5_map[0xFE][0x52] = 0x732A;
$big5_map[0xFE][0x6F] = 0x745C;
$big5_map[0xFE][0xAA] = 0x74E9;
$big5_map[0xFE][0xDD] = 0x7809;

$pua = 0;
$compat[0] = 0; # 1993
$compat[1] = 0; # 2000
$compat[2] = 0; # 2001

$filename = "big5-iso.txt";
open IN, ("input/" . $filename) or die "Cannot read " . $filename;
while (<IN>)
{
    if (/^([0-9A-F]+) +([0-9A-F]+) +([0-9A-F]+) +([0-9A-F]+)$/)
    {
        $big5 = oct("0x" . $1);
        $utf32_1993 = oct("0x" . $2);
        $utf32_2000 = oct("0x" . $3);
        $utf32_2001 = oct("0x" . $4);
        isValidBig5($big5)
            or die "Bad Big5 char " . printBig5($big5);
        isValidUtf32($utf32_1993)
            or die "Bad UTF32 char " . printUtf32($utf32_1993);
        isValidUtf32($utf32_2000)
            or die "Bad UTF32 char " . printUtf32($utf32_2000);
        isValidUtf32($utf32_2001)
            or die "Bad UTF32 char " . printUtf32($utf32_2001);

        $utf32 = $surrogates ? $utf32_2001 : $utf32_2000;

        if (defined($underlying_big5[$utf32])
            || defined($underlying_cp950[$utf32]))
        {
            if (defined($underlying_big5[$utf32])
                && defined($underlying_cp950[$utf32])
                && $underlying_big5[$utf32] == $underlying_cp950[$utf32]
                && $underlying_big5[$utf32] == $big5
                ||
                defined($underlying_big5[$utf32])
                && !defined($underlying_cp950[$utf32])
                && $underlying_big5[$utf32] == $big5
                ||
                !defined($underlying_big5[$utf32])
                && defined($underlying_cp950[$utf32])
                && $underlying_cp950[$utf32] == $big5)
            {
                # ignore

                # Depending on real underlying mapping (cf.
                # ../convertbig5hkscs.tab), it would be possible to save some
                # table space by dropping those HKSCS code points that are
                # already covered by the underlying mapping.
            }
            else
            {
                print "XXX mapping underlying";
                if (defined($underlying_big5[$utf32])
                    && defined($underlying_cp950[$utf32])
                    && $underlying_big5[$utf32] == $underlying_cp950[$utf32])
                {
                    print " Big5/CP950 ", printBig5($underlying_big5[$utf32]);
                }
                else
                {
                    if (defined($underlying_big5[$utf32]))
                    {
                        print " Big5 ", printBig5($underlying_big5[$utf32]);
                    }
                    if (defined($underlying_cp950[$utf32]))
                    {
                        print " CP950 ", printBig5($underlying_cp950[$utf32]);
                    }
                }
                print " and HKSCS ", printBig5($big5), " to ",
                      printUtf32($utf32), "\n";
            }
        }

        if ($utf32 >= 0xE000 && $utf32 <= 0xF8FF)
        {
            ++$pua;
        }

        $big5_row = $big5 >> 8;
        $big5_column = $big5 & 0xFF;
        if (defined($big5_map[$big5_row][$big5_column]))
        {
            die "Bad Big5 mapping " . printBig5($big5);
        }
        $big5_map[$big5_row][$big5_column] = $utf32;

        addMapping($utf32, $big5, -1);

        if ($utf32_2001 != $utf32)
        {
            addMapping($utf32_2001, $big5, 2);
        }
        if ($utf32_2000 != $utf32 && $utf32_2000 != $utf32_2001)
        {
            addMapping($utf32_2000, $big5, 1);
        }
        if ($utf32_1993 != $utf32 && $utf32_1993 != $utf32_2000
            && $utf32_1993 != $utf32_2001)
        {
            addMapping($utf32_1993, $big5, 0);
        }
    }
}
close IN;

print $pua, " mappings to PUA\n";
print $compat[0], " 1993 compatibility mappings\n" if ($compat[0] != 0);
print $compat[1], " 2000 compatibility mappings\n" if ($compat[1] != 0);
print $compat[2], " 2001 compatibility mappings\n" if ($compat[2] != 0);

if (defined($uni_plane_used[0]) && defined($uni_page_used[0][0]))
{
    for ($utf32 = 0; $utf32 <= 0x7F; ++$utf32)
    {
        if (defined($uni_map[0][0][$uni_index]))
        {
            $big5 = $uni_map[0][0][$utf32];
            die "Mapping " . printUtf32($utf32) . " to " . printBig5($big5);
        }
    }
}

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
          "#include \"sal/types.h\"\n",
          "\n";

print OUT "static sal_uInt16 const aImpl", $id, "ToUnicodeData[] = {\n";
$big5_data_index = 0;
$big5_rows = 0;
$big5_chars = 0;
for ($big5_row = 0; $big5_row <= 255; ++$big5_row)
{
    $big5_row_first = -1;
    for ($big5_column = 0; $big5_column <= 255; ++$big5_column)
    {
        if (defined($big5_map[$big5_row][$big5_column]))
        {
            if ($big5_row_first == -1)
            {
                $big5_row_first = $big5_column;
            }
            $big5_row_last = $big5_column;
        }
    }
    if ($big5_row_first != -1)
    {
        $big5_data_offsets[$big5_row] = $big5_data_index;
        ++$big5_rows;
        print OUT " /* row ", $big5_row, " */\n";

        $big5_row_surrogates_first = -1;
        $big5_row_chars = 0;
        $big5_row_surrogates = 0;

        print OUT "  ", $big5_row_first, " | (", $big5_row_last,
                  " << 8), /* first, last */\n";
        ++$big5_data_index;

        print OUT "  ", printSpaces(7, 10, $big5_row_first);
        $bol = 0;
        for ($big5_column = $big5_row_first;
             $big5_column <= $big5_row_last;
             ++$big5_column)
        {
            if ($bol == 1)
            {
                print OUT "  ";
                $bol = 0;
            }
            if (defined($big5_map[$big5_row][$big5_column]))
            {
                $utf32 = $big5_map[$big5_row][$big5_column];
                ++$big5_row_chars;
                if ($utf32 <= 0xFFFF)
                {
                    printf OUT "0x%04X,", $utf32;
                }
                else
                {
                    ++$big5_row_surrogates;
                    printf OUT "0x%04X,",
                               (0xD800 | (($utf32 - 0x10000) >> 10));
                    if ($big5_row_surrogates_first == -1)
                    {
                        $big5_row_surrogates_first = $big5_column;
                    }
                    $big5_row_surrogates_last = $big5_column;
                }
            }
            else
            {
                printf OUT "0xffff,";
            }
            ++$big5_data_index;
            if ($big5_column % 10 == 9)
            {
                print OUT "\n";
                $bol = 1;
            }
        }
        if ($bol == 0)
        {
            print OUT "\n";
        }

        if ($big5_row_surrogates_first != -1)
        {
            print OUT "  ", $big5_row_surrogates_first,
                      ", /* first low-surrogate */\n";
            ++$big5_data_index;

            print OUT "  ", printSpaces(7, 10, $big5_row_surrogates_first);
            $bol = 0;
            for ($big5_column = $big5_row_surrogates_first;
                 $big5_column <= $big5_row_surrogates_last;
                 ++$big5_column)
            {
                if ($bol == 1)
                {
                    print OUT "  ";
                    $bol = 0;
                }
                $utf32 = 0;
                if (defined($big5_map[$big5_row][$big5_column]))
                {
                    $utf32 = $big5_map[$big5_row][$big5_column];
                }
                if ($utf32 <= 0xFFFF)
                {
                    printf OUT "     0,";
                }
                else
                {
                    printf OUT "0x%04X,",
                               (0xDC00 | (($utf32 - 0x10000) & 0x3FF));
                }
                ++$big5_data_index;
                if ($big5_column % 10 == 9)
                {
                    print OUT "\n";
                    $bol = 1;
                }
            }
            if ($bol == 0)
            {
                print OUT "\n";
            }
        }

        $big5_chars += $big5_row_chars;
        $big5_data_space[$big5_row]
            = ($big5_data_index - $big5_data_offsets[$big5_row]) * 2;
        $big5_data_used[$big5_row]
            = (1 + $big5_row_chars + ($big5_row_surrogates == 0 ?
                                          0 : 1 + $big5_row_surrogates))
                  * 2;
    }
    else
    {
        print OUT " /* row ", $big5_row, ": --- */\n";
        $big5_data_offsets[$big5_row] = -1;
    }
}
print OUT "};\n\n";
print "big5 rows = ", $big5_rows, ", chars = ", $big5_chars, "\n";

print OUT "static sal_Int32 const aImpl", $id, "ToUnicodeRowOffsets[] = {\n";
$big5_rowoffsets_used = 0;
for ($big5_row = 0; $big5_row <= 255; ++$big5_row)
{
    if ($big5_data_offsets[$big5_row] == -1)
    {
        print OUT "  -1, /* row ", $big5_row, " */\n";
    }
    else
    {
        print OUT "  ",
                  $big5_data_offsets[$big5_row],
                  ", /* row ",
                  $big5_row,
                  "; ",
                  printStats($big5_data_used[$big5_row],
                             $big5_data_space[$big5_row]),
                  " */\n";
        $big5_rowoffsets_used += 4;
    }
}
print OUT "};\n\n";

print OUT "static sal_uInt16 const aImplUnicodeTo", $id, "Data[] = {\n";
$uni_data_index = 0;
for ($uni_plane = 0; $uni_plane <= 16; ++$uni_plane)
{
    if (defined($uni_plane_used[$uni_plane]))
    {
        for ($uni_page = 0; $uni_page <= 255; ++$uni_page)
        {
            if (defined($uni_page_used[$uni_plane][$uni_page]))
            {
                $uni_data_offsets[$uni_plane][$uni_page] = $uni_data_index;
                print OUT " /* plane ", $uni_plane, ", page ", $uni_page,
                          " */\n";

                $uni_page_first = -1;
                for ($uni_index = 0; $uni_index <= 255; ++$uni_index)
                {
                    if (defined($uni_map[$uni_plane][$uni_page][$uni_index]))
                    {
                        if ($uni_page_first == -1)
                        {
                            $uni_page_first = $uni_index;
                        }
                        $uni_page_last = $uni_index;
                    }
                }

                $uni_data_used[$uni_plane][$uni_page] = 0;

                print OUT "  ", $uni_page_first, " | (", $uni_page_last,
                          " << 8), /* first, last */\n";
                ++$uni_data_index;
                $uni_data_used[$uni_plane][$uni_page] += 2;

                print OUT "  ", printSpaces(7, 10, $uni_page_first);
                $bol = 0;
                for ($uni_index = $uni_page_first;
                     $uni_index <= $uni_page_last;
                     ++$uni_index)
                {
                    if ($bol == 1)
                    {
                        print OUT "  ";
                        $bol = 0;
                    }
                    if (defined($uni_map[$uni_plane][$uni_page][$uni_index]))
                    {
                        $big5 = $uni_map[$uni_plane][$uni_page][$uni_index];
                        printf OUT "0x%04X,", $big5;
                        $uni_data_used[$uni_plane][$uni_page] += 2;
                    }
                    else
                    {
                        print OUT "     0,";
                    }
                    ++$uni_data_index;
                    if ($uni_index % 10 == 9)
                    {
                        print OUT "\n";
                        $bol = 1;
                    }
                }
                if ($bol == 0)
                {
                    print OUT "\n";
                }

                $uni_data_space[$uni_plane][$uni_page]
                    = ($uni_data_index
                       - $uni_data_offsets[$uni_plane][$uni_page]) * 2;
            }
            else
            {
                $uni_data_offsets[$uni_plane][$uni_page] = -1;
                print OUT " /* plane ", $uni_plane, ", page ", $uni_page,
                          ": --- */\n";
            }
        }
    }
    else
    {
        print OUT " /* plane ", $uni_plane, ": --- */\n";
    }
}
print OUT "};\n\n";

print OUT "static sal_Int32 const aImplUnicodeTo", $id, "PageOffsets[] = {\n";
for ($uni_plane = 0; $uni_plane <= 16; ++$uni_plane)
{
    if (defined($uni_plane_used[$uni_plane]))
    {
        $uni_pageoffsets_used[$uni_plane] = 0;
        $uni_data_used_sum[$uni_plane] = 0;
        $uni_data_space_sum[$uni_plane] = 0;
        for ($uni_page = 0; $uni_page <= 255; ++$uni_page)
        {
            $offset = $uni_data_offsets[$uni_plane][$uni_page];
            if ($offset == -1)
            {
                print OUT "  -1, /* plane ",
                          $uni_plane,
                          ", page ",
                          $uni_page,
                          " */\n";
            }
            else
            {
                print OUT "  ",
                          $offset,
                          ", /* plane ",
                          $uni_plane,
                          ", page ",
                          $uni_page,
                          "; ",
                          printStats($uni_data_used[$uni_plane][$uni_page],
                                     $uni_data_space[$uni_plane][$uni_page]),
                          " */\n";
                $uni_pageoffsets_used[$uni_plane] += 4;
                $uni_data_used_sum[$uni_plane]
                    += $uni_data_used[$uni_plane][$uni_page];
                $uni_data_space_sum[$uni_plane]
                    += $uni_data_space[$uni_plane][$uni_page];
            }
        }
    }
    else
    {
        print OUT "  /* plane ", $uni_plane, ": --- */\n";
    }
}
print OUT "};\n\n";

print OUT "static sal_Int32 const aImplUnicodeTo",
          $id,
          "PlaneOffsets[] = {\n";
$uni_page_offset = 0;
$uni_planeoffsets_used = 0;
$uni_pageoffsets_used_sum = 0;
$uni_pageoffsets_space_sum = 0;
$uni_data_used_sum2 = 0;
$uni_data_space_sum2 = 0;
for ($uni_plane = 0; $uni_plane <= 16; ++$uni_plane)
{
    if (defined ($uni_plane_used[$uni_plane]))
    {
        print OUT "  ",
                  $uni_page_offset++,
                  " * 256, /* plane ",
                  $uni_plane,
                  "; ",
                  printStats($uni_pageoffsets_used[$uni_plane], 256 * 4),
                  ", ",
                  printStats($uni_data_used_sum[$uni_plane],
                             $uni_data_space_sum[$uni_plane]),
                  " */\n";
        $uni_planeoffsets_used += 4;
        $uni_pageoffsets_used_sum += $uni_pageoffsets_used[$uni_plane];
        $uni_pageoffsets_space_sum += 256 * 4;
        $uni_data_used_sum2 += $uni_data_used_sum[$uni_plane];
        $uni_data_space_sum2 += $uni_data_space_sum[$uni_plane];
    }
    else
    {
        print OUT "  -1, /* plane ", $uni_plane, " */\n";
    }
}
print OUT " /* ",
          printStats($uni_planeoffsets_used, 17 * 4),
          ", ",
          printStats($uni_pageoffsets_used_sum, $uni_pageoffsets_space_sum),
          ", ",
          printStats($uni_data_used_sum2, $uni_data_space_sum2),
          " */\n};\n";

close OUT;
