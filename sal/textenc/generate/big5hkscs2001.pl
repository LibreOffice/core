#!/usr/bin/perl
#*************************************************************************
#
#   $RCSfile: big5hkscs2001.pl,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: sb $ $Date: 2002-01-15 17:04:04 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#*************************************************************************

# The following file must be available in a ./input subdir:

# <http://www.info.gov.hk/digital21/eng/hkscs/download/big5-iso.txt>

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

$ignored = 0;

$filename = "big5-iso.txt";
open IN, ("input/" . $filename) or die "Cannot read " . $filename;
while (<IN>)
{
    if (/^([0-9A-F]+) +[0-9A-F]+ +[0-9A-F]+ +([0-9A-F]+)$/)
    {
        $big5 = oct("0x" . $1);
        $utf32 = oct("0x" . $2);
        isValidBig5($big5)
            or die "Bad Big5 char " . printBig5($big5);
        isValidUtf32($utf32)
            or die "Bad UTF32 char " . printUtf32($utf32);
        if ($utf32 >= 0xE000 && $utf32 <= 0xF8FF)
        {
            print "Mapping to PUA ", printUtf32($utf32), " ignored\n";
            ++$ignored;
        }
        elsif (defined($big5_map[$big5]))
        {
            die "Bad Big5 mapping " . printBig5($big5);
        }
        else
        {
            $big5_row = $big5 >> 8;
            $big5_column = $big5 & 0xFF;
            $big5_map[$big5_row][$big5_column] = $utf32;
        }
    }
}
close IN;

print $ignored, " mappings to PUA ignored\n";

for ($big5_row = 0; $big5_row <= 255; ++$big5_row)
{
    for ($big5_column = 0; $big5_column <= 255; ++$big5_column)
    {
        if (defined($big5_map[$big5_row][$big5_column]))
        {
            $utf32 = $big5_map[$big5_row][$big5_column];
            $uni_plane = $utf32 >> 16;
            $uni_page = ($utf32 >> 8) & 0xFF;
            $uni_index = $utf32 & 0xFF;
            if (!defined($uni_plane_used[$uni_plane])
                || !defined($uni_page_used[$uni_plane][$uni_page])
                || !defined($uni_map[$uni_plane][$uni_page][$uni_index]))
            {
                $uni_map[$uni_plane][$uni_page][$uni_index]
                    = ($big5_row << 8) | $big5_column;
                $uni_plane_used[$uni_plane] = 1;
                $uni_page_used[$uni_plane][$uni_page] = 1;
            }
            else
            {
                $big5_1 = $uni_map[$uni_plane][$uni_page][$uni_index];
                print "WARNING!  Mapping ",
                      printUtf32($utf32),
                      " to ",
                      printBig5($big5_1),
                      ", NOT ",
                      printBig5(($big5_row << 8) | $big5_column),
                      "\n";
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
          "#ifndef _SAL_TYPES_H_\n",
          "#include \"sal/types.h\"\n",
          "#endif\n",
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
