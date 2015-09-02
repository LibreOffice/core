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

# <http://www.unicode.org/Public/UNIDATA/Unihan.txt>:
#  "Unicode version: 3.1.1    Table version: 1.1    Date: 28 June 2001"
#  contains descriptions for:
#   U+3400..4DFF CJK Unified Ideographs Extension A
#   U+4E00..9FFF CJK Unified Ideographs
#   U+F900..FAFF CJK Compatibility Ideographs
#   U+20000..2F7FF CJK Unified Ideographs Extension B
#   U+2F800..2FFFF CJK Compatibility Ideographs Supplement

# <http://www.unicode.org/Public/MAPPINGS/OBSOLETE/EASTASIA/CNS11643.TXT>:
#  "Unicode version: 1.1    Table version: 0.0d1    Date: 21 October 1994"
#  contains mappings for CNS 11643-1986

# <http://kanji.zinbun.kyoto-u.ac.jp/~yasuoka/ftp/CJKtable/Uni2CNS.Z>:
#  "Unicode version: 1.1    Table version: 0.49    Date: 26 March 1998"
#  contains mappings for CNS 11643-1992 that are incompatible with
#   CNS11643.TXT

$id = "Cns116431992";

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

sub isValidCns116431992
{
    my $plane = $_[0];
    my $row = $_[1];
    my $column = $_[2];
    return $plane >= 1 && $plane <= 16
           && $row >= 1 && $row <= 94
           && $column >= 1 && $column <= 94;
}

sub printCns116431992
{
    my $plane = $_[0];
    my $row = $_[1];
    my $column = $_[2];
    return sprintf("%d-%02d/%02d", $plane, $row, $column);
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

$count_Unihan_txt = 0;
$count_CNS11643_TXT = 0;
$count_Uni2CNS = 0;

if (1)
{
    $filename = "Unihan.txt";
    open IN, ("input/" . $filename) or die "Cannot read " . $filename;
    while (<IN>)
    {
        if (/^U\+([0-9A-F]+)\tkCNS1992\t([0-9A-F])-([0-9A-F][0-9A-F])([0-9A-F][0-9A-F])$/)
        {
            $utf32 = oct("0x" . $1);
            $cns_plane = oct("0x" . $2);
            $cns_row = oct("0x" . $3) - 0x20;
            $cns_column = oct("0x" . $4) - 0x20;
            isValidUtf32($utf32)
                or die "Bad UTF32 char U+" . printUtf32($utf32);
            isValidCns116431992($cns_plane, $cns_row, $cns_column)
                or die "Bad CNS11643-1992 char "
                           . printCns116431992($cns_plane,
                                               $cns_row,
                                               $cns_column);
            if (!defined($cns_map[$cns_plane][$cns_row][$cns_column]))
            {
                $cns_map[$cns_plane][$cns_row][$cns_column] = $utf32;
                $cns_plane_used[$cns_plane] = 1;
                ++$count_Unihan_txt;
            }
            else
            {
                ($cns_map[$cns_plane][$cns_row][$cns_column] == $utf32)
                    or die "Mapping "
                               . printCns116431992($cns_plane,
                                                   $cns_row,
                                                   $cns_column)
                               . " to "
                               . printUtf32($cns_map[$cns_plane]
                                                    [$cns_row]
                                                    [$cns_column])
                               . ", NOT "
                               . printUtf32($utf32);
            }
        }
        elsif (/^U\+([0-9A-F]+)\tkIRG_TSource\t([0-9A-F])-([0-9A-F][0-9A-F])([0-9A-F][0-9A-F])$/)
        {
            $utf32 = oct("0x" . $1);
            $cns_plane = oct("0x" . $2);
            $cns_row = oct("0x" . $3) - 0x20;
            $cns_column = oct("0x" . $4) - 0x20;
            isValidUtf32($utf32)
                or die "Bad UTF32 char U+" . printUtf32($utf32);
            isValidCns116431992($cns_plane, $cns_row, $cns_column)
                or die "Bad CNS11643-1992 char "
                           . printCns116431992($cns_plane,
                                               $cns_row,
                                               $cns_column);
            if (!defined($cns_map[$cns_plane][$cns_row][$cns_column]))
            {
                $cns_map[$cns_plane][$cns_row][$cns_column] = $utf32;
                $cns_plane_used[$cns_plane] = 1;
                ++$count_Unihan_txt;
            }
            else
            {
                ($cns_map[$cns_plane][$cns_row][$cns_column] == $utf32)
                    or print "WARNING!  Mapping ",
                             printCns116431992($cns_plane,
                                               $cns_row,
                                               $cns_column),
                             " to ",
                             printUtf32($cns_map[$cns_plane]
                                                [$cns_row]
                                                [$cns_column]),
                             ", NOT ",
                             printUtf32($utf32),
                             "\n";
            }
        }
        elsif (/^U\+([0-9A-F]+)\tkCNS1992\t.*$/)
        {
            die "Bad format";
        }
    }
    close IN;
}

if (1)
{
    $filename = "CNS11643.TXT";
    open IN, ("input/" . $filename) or die "Cannot read " . $filename;
    while (<IN>)
    {
        if (/0x([0-9A-F])([0-9A-F][0-9A-F])([0-9A-F][0-9A-F])\t0x([0-9A-F]+)\t\#.*$/)
        {
            $utf32 = oct("0x" . $4);
            $cns_plane = oct("0x" . $1);
            $cns_row = oct("0x" . $2) - 0x20;
            $cns_column = oct("0x" . $3) - 0x20;
            isValidUtf32($utf32)
                or die "Bad UTF32 char U+" . printUtf32($utf32);
            isValidCns116431992($cns_plane, $cns_row, $cns_column)
                or die "Bad CNS11643-1992 char "
                           . printCns116431992($cns_plane,
                                               $cns_row,
                                               $cns_column);
            if ($cns_plane <= 2)
            {
                if (!defined($cns_map[$cns_plane][$cns_row][$cns_column]))
                {
                    $cns_map[$cns_plane][$cns_row][$cns_column] = $utf32;
                    $cns_plane_used[$cns_plane] = 1;
                    ++$count_CNS11643_TXT;
                }
                else
                {
                    ($cns_map[$cns_plane][$cns_row][$cns_column] == $utf32)
                        or die "Mapping "
                                   . printCns116431992($cns_plane,
                                                       $cns_row,
                                                       $cns_column)
                                   . " to "
                                   . printUtf32($cns_map[$cns_plane]
                                                        [$cns_row]
                                                        [$cns_column])
                                   . ", NOT "
                                   . printUtf32($utf32);
                }
            }
        }
    }
    close IN;
}

if (0)
{
    $filename = "Uni2CNS";
    open IN, ("input/" . $filename) or die "Cannot read " . $filename;
    while (<IN>)
    {
        if (/([0-9A-F]+)\t([0-9A-F])-([0-9A-F][0-9A-F])([0-9A-F][0-9A-F])\t.*$/)
        {
            $utf32 = oct("0x" . $1);
            $cns_plane = oct("0x" . $2);
            $cns_row = oct("0x" . $3) - 0x20;
            $cns_column = oct("0x" . $4) - 0x20;
            isValidUtf32($utf32)
                or die "Bad UTF32 char U+" . printUtf32($utf32);
            isValidCns116431992($cns_plane, $cns_row, $cns_column)
                or die "Bad CNS11643-1992 char "
                           . printCns116431992($cns_plane,
                                               $cns_row,
                                               $cns_column);
            if (!defined($cns_map[$cns_plane][$cns_row][$cns_column]))
            {
                $cns_map[$cns_plane][$cns_row][$cns_column] = $utf32;
                $cns_plane_used[$cns_plane] = 1;
                ++$count_Uni2CNS;
            }
            else
            {
#               ($cns_map[$cns_plane][$cns_row][$cns_column] == $utf32)
#                   or die "Mapping "
#                              . printCns116431992($cns_plane,
#                                                  $cns_row,
#                                                  $cns_column)
#                              . " to "
#                              . printUtf32($cns_map[$cns_plane]
#                                                   [$cns_row]
#                                                   [$cns_column])
#                              . ", NOT "
#                              . printUtf32($utf32);
            }
            if ($cns_plane == 1)
            {
                print printCns116431992($cns_plane, $cns_row, $cns_column),
                      "\n";
            }
        }
    }
    close IN;
}

for ($cns_plane = 1; $cns_plane <= 16; ++$cns_plane)
{
    if (defined($cns_plane_used[$cns_plane]))
    {
        for ($cns_row = 1; $cns_row <= 94; ++$cns_row)
        {
            for ($cns_column = 1; $cns_column <= 94; ++$cns_column)
            {
                if (defined($cns_map[$cns_plane][$cns_row][$cns_column]))
                {
                    $utf32 = $cns_map[$cns_plane][$cns_row][$cns_column];
                    $uni_plane = $utf32 >> 16;
                    $uni_page = ($utf32 >> 8) & 0xFF;
                    $uni_index = $utf32 & 0xFF;
                    if (!defined($uni_plane_used[$uni_plane])
                        || !defined($uni_page_used[$uni_plane][$uni_page])
                        || !defined($uni_map[$uni_plane]
                                            [$uni_page]
                                            [$uni_index]))
                    {
                        $uni_map[$uni_plane][$uni_page][$uni_index]
                            = ($cns_plane << 16)
                                  | ($cns_row << 8)
                                  | $cns_column;
                        $uni_plane_used[$uni_plane] = 1;
                        $uni_page_used[$uni_plane][$uni_page] = 1;
                    }
                    else
                    {
                        $cns1 = $uni_map[$uni_plane][$uni_page][$uni_index];
                        $cns1_plane = $cns1 >> 16;
                        $cns1_row = ($cns1 >> 8) & 0xFF;
                        $cns1_column = $cns1 & 0xFF;

                        # Do not map from Unicode to Fictitious Character Set
                        # Extensions (Lunde, p. 131), if possible:
                        if ($cns_plane == 3
                            && ($cns_row == 66 && $cns_column > 38
                                || $cns_row > 66))
                        {
                            print " (",
                                  printUtf32($utf32),
                                  " to fictitious ",
                                  printCns116431992($cns_plane,
                                                    $cns_row,
                                                    $cns_column),
                                  " ignored, favouring ",
                                  printCns116431992($cns1_plane,
                                                    $cns1_row,
                                                    $cns1_column),
                                  ")\n";
                        }
                        elsif ($cns1_plane == 3
                               && ($cns1_row == 66 && $cns1_column > 38
                                   || $cns1_row > 66))
                        {
                            $uni_map[$uni_plane][$uni_page][$uni_index]
                                = ($cns_plane << 16)
                                       | ($cns_row << 8)
                                       | $cns_column;
                            print " (",
                                  printUtf32($utf32),
                                  " to fictitious ",
                                  printCns116431992($cns1_plane,
                                                    $cns1_row,
                                                    $cns1_column),
                                  " ignored, favouring ",
                                  printCns116431992($cns_plane,
                                                    $cns_row,
                                                    $cns_column),
                                  ")\n";
                        }
                        else
                        {
                            print "WARNING!  Mapping ",
                                  printUtf32($utf32),
                                  " to ",
                                  printCns116431992($cns1_plane,
                                                    $cns1_row,
                                                    $cns1_column),
                                  ", NOT ",
                                  printCns116431992($cns_plane,
                                                    $cns_row,
                                                    $cns_column),
                                  "\n";
                        }
                    }
                }
            }
        }
    }
}
if (defined($uni_plane_used[0]) && defined($uni_page_used[0][0]))
{
    for ($utf32 = 0; $utf32 <= 0x7F; ++$utf32)
    {
        if (defined($uni_map[0][0][$uni_index]))
        {
            $cns = $uni_map[0][0][$utf32];
            die "Mapping "
                    . printUtf32($utf32)
                    . " to "
                    . printCns116431992($cns >> 16,
                                        ($cns >> 8) & 0xFF,
                                        $cns & 0xFF);
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
$cns_data_index = 0;
for ($cns_plane = 1; $cns_plane <= 16; ++$cns_plane)
{
    if (defined($cns_plane_used[$cns_plane]))
    {
        $cns_rows = 0;
        $cns_chars = 0;
        for ($cns_row = 1; $cns_row <= 94; ++$cns_row)
        {
            $cns_row_first = -1;
            for ($cns_column = 1; $cns_column <= 94; ++$cns_column)
            {
                if (defined($cns_map[$cns_plane][$cns_row][$cns_column]))
                {
                    if ($cns_row_first == -1)
                    {
                        $cns_row_first = $cns_column;
                    }
                    $cns_row_last = $cns_column;
                }
            }
            if ($cns_row_first != -1)
            {
                $cns_data_offsets[$cns_plane][$cns_row] = $cns_data_index;
                ++$cns_rows;
                print OUT " /* plane ", $cns_plane, ", row ", $cns_row,
                          " */\n";

                $cns_row_surrogates_first = -1;
                $cns_row_chars = 0;
                $cns_row_surrogates = 0;

                print OUT "  ", $cns_row_first, " | (", $cns_row_last,
                          " << 8), /* first, last */\n";
                ++$cns_data_index;

                print OUT "  ", printSpaces(7, 10, $cns_row_first);
                $bol = 0;
                for ($cns_column = $cns_row_first;
                     $cns_column <= $cns_row_last;
                     ++$cns_column)
                {
                    if ($bol == 1)
                    {
                        print OUT "  ";
                        $bol = 0;
                    }
                    if (defined($cns_map[$cns_plane][$cns_row][$cns_column]))
                    {
                        $utf32 = $cns_map[$cns_plane][$cns_row][$cns_column];
                        ++$cns_row_chars;
                        if ($utf32 <= 0xFFFF)
                        {
                            printf OUT "0x%04X,", $utf32;
                        }
                        else
                        {
                            ++$cns_row_surrogates;
                            printf OUT "0x%04X,",
                                       (0xD800 | (($utf32 - 0x10000) >> 10));
                            if ($cns_row_surrogates_first == -1)
                            {
                                $cns_row_surrogates_first = $cns_column;
                            }
                            $cns_row_surrogates_last = $cns_column;
                        }
                    }
                    else
                    {
                        printf OUT "0xffff,";
                    }
                    ++$cns_data_index;
                    if ($cns_column % 10 == 9)
                    {
                        print OUT "\n";
                        $bol = 1;
                    }
                }
                if ($bol == 0)
                {
                    print OUT "\n";
                }

                if ($cns_row_surrogates_first != -1)
                {
                    print OUT "  ", $cns_row_surrogates_first,
                              ", /* first low-surrogate */\n";
                    ++$cns_data_index;

                    print OUT "  ",
                              printSpaces(7, 10, $cns_row_surrogates_first);
                    $bol = 0;
                    for ($cns_column = $cns_row_surrogates_first;
                         $cns_column <= $cns_row_surrogates_last;
                         ++$cns_column)
                    {
                        if ($bol == 1)
                        {
                            print OUT "  ";
                            $bol = 0;
                        }
                        $utf32 = 0;
                        if (defined($cns_map[$cns_plane]
                                            [$cns_row]
                                            [$cns_column]))
                        {
                            $utf32
                                = $cns_map[$cns_plane][$cns_row][$cns_column];
                        }
                        if ($utf32 <= 0xFFFF)
                        {
                            printf OUT "     0,";
                        }
                        else
                        {
                            printf OUT "0x%04X,",
                                       (0xDC00
                                            | (($utf32 - 0x10000) & 0x3FF));
                        }
                        ++$cns_data_index;
                        if ($cns_column % 10 == 9)
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

                $cns_chars += $cns_row_chars;
                $cns_data_space[$cns_plane][$cns_row]
                    = ($cns_data_index
                           - $cns_data_offsets[$cns_plane][$cns_row]) * 2;
                $cns_data_used[$cns_plane][$cns_row]
                    = (1 + $cns_row_chars
                           + ($cns_row_surrogates == 0 ?
                                  0 : 1 + $cns_row_surrogates)) * 2;
            }
            else
            {
                print OUT " /* plane ", $cns_plane, ", row ", $cns_row,
                          ": --- */\n";
                $cns_data_offsets[$cns_plane][$cns_row] = -1;
            }
        }
        print "cns plane ",
              $cns_plane,
              ": rows = ",
              $cns_rows,
              ", chars = ",
              $cns_chars,
              "\n";
    }
}
print OUT "};\n\n";

print OUT "static sal_Int32 const aImpl", $id, "ToUnicodeRowOffsets[] = {\n";
for ($cns_plane = 1; $cns_plane <= 16; ++$cns_plane)
{
    if (defined ($cns_plane_used[$cns_plane]))
    {
        $cns_rowoffsets_used[$cns_plane] = 0;
        for ($cns_row = 1; $cns_row <= 94; ++$cns_row)
        {
            if ($cns_data_offsets[$cns_plane][$cns_row] == -1)
            {
                print OUT "  -1, /* plane ",
                          $cns_plane,
                          ", row ",
                          $cns_row,
                          " */\n";
            }
            else
            {
                print OUT "  ",
                          $cns_data_offsets[$cns_plane][$cns_row],
                          ", /* plane ",
                          $cns_plane,
                          ", row ",
                          $cns_row,
                          "; ",
                          printStats($cns_data_used[$cns_plane][$cns_row],
                                     $cns_data_space[$cns_plane][$cns_row]),
                          " */\n";
                $cns_rowoffsets_used[$cns_plane] += 4;
            }
        }
    }
    else
    {
        print OUT "  /* plane ", $cns_plane, ": --- */\n";
    }
}
print OUT "};\n\n";

print OUT "static sal_Int32 const aImpl",
          $id,
          "ToUnicodePlaneOffsets[] = {\n";
$cns_row_offset = 0;
for ($cns_plane = 1; $cns_plane <= 16; ++$cns_plane)
{
    if (defined ($cns_plane_used[$cns_plane]))
    {
        print OUT "  ",
                  $cns_row_offset++,
                  " * 94, /* plane ",
                  $cns_plane,
                  "; ",
                  printStats($cns_rowoffsets_used[$cns_plane], 94 * 4),
                  " */\n";
    }
    else
    {
        print OUT "  -1, /* plane ", $cns_plane, " */\n";
    }
}
print OUT "};\n\n";

print OUT "static sal_uInt8 const aImplUnicodeTo", $id, "Data[] = {\n";
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

                print OUT "  ", $uni_page_first, ", ", $uni_page_last,
                          ", /* first, last */\n";
                $uni_data_index += 2;
                $uni_data_used[$uni_plane][$uni_page] += 2;

                print OUT "  ", printSpaces(9, 8, $uni_page_first);
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
                        $cns = $uni_map[$uni_plane][$uni_page][$uni_index];
                        printf OUT "%2d,%2d,%2d,",
                                   $cns >> 16,
                                   $cns >> 8 & 0xFF,
                                   $cns & 0xFF;
                        $uni_data_used[$uni_plane][$uni_page] += 3;
                    }
                    else
                    {
                        print OUT " 0, 0, 0,";
                    }
                    $uni_data_index += 3;
                    if ($uni_index % 8 == 7)
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
                    = $uni_data_index
                          - $uni_data_offsets[$uni_plane][$uni_page];
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

print "Unihan.txt = ", $count_Unihan_txt,
      ", CNS11643.TXT = ", $count_CNS11643_TXT,
      ", Uni2CNS = ", $count_Uni2CNS,
      ", total = ",
          ($count_Unihan_txt + $count_CNS11643_TXT + $count_Uni2CNS),
      "\n";
