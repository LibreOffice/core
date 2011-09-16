:
eval 'exec perl -S $0 ${1+"$@"}'
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
#
# add keyids to sdf file
#

my $infile = $ARGV[0];
if ( !$infile ) {die "Usage: $0 <infile> [<outfile>]\n";}

my $outfile = $ARGV[1];
if ( ! defined $outfile )
{
    $outfile = $infile;
    $outfile =~ s/\.sdf$//i;
    $outfile .= "_KeyID.sdf";
}

print "writing to $outfile\n";
open INFILE,"<$infile" || die "could not open $infile $! $^E\n";
open OUTFILE,">$outfile" || die "could not open $outfile $! $^E\n";

while ( <INFILE> )
{
    chomp;
    if ( /^([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)\t([^\t]*)$/ )
    {
        my $prj            = defined $1 ? $1 : '';
        my $file           = defined $2 ? $2 : '';
        my $dummy          = defined $3 ? $3 : '';
        my $type           = defined $4 ? $4 : '';
        my $gid            = defined $5 ? $5 : '';
        my $lid            = defined $6 ? $6 : '';
        my $helpid         = defined $7 ? $7 : '';
        my $platform       = defined $8 ? $8 : '';
        my $width          = defined $9 ? $9 : '';
        my $lang           = defined $10 ? $10 : '';
        my $text           = defined $11 ? $11 : '';
        my $helptext       = defined $12 ? $12 : '';
        my $quickhelptext  = defined $13 ? $13 : '';
        my $title          = defined $14 ? $14 : '';
        my $timestamp      = defined $15 ? $15 : '';
        @path = split ( "\\\\" , $file );
        $key = $path[-3] . "_" . $path[-2] . "_" . $path[-1] . "#" . $gid . ".";
        if (length($lid)) {$key .= $lid . ".";}
        if (length($type)) {$key .= $type . ".";}
        # replace non-word characters to _ just as in po files in source keys
        $key =~ s|[^\w#\./]|_|g;
        # ISO 639 private use code
	$lang = "qtz";
	if ( $text ne "") {$text = makekeyidstr(keyidgen($key . "text"),$text);}
	if ( $quickhelptext ne "") {$quickhelptext = makekeyidstr(keyidgen($key . "quickhelptext"),$quickhelptext);}
	if ( $title ne "") {$title = makekeyidstr(keyidgen($key . "title"),$title);}
        print OUTFILE "$prj\t$file\t$dummy\t$type\t$gid\t$lid\t$helpid\t$platform\t$width\t$lang\t$text\t$helptext\t$quickhelptext\t$title\t$timestamp\n";
    }
}

close INFILE;
close OUTFILE;

sub keyidgen
{
    my $key = shift;
    my $crc = crc24($key);
    my $symbols="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz#_";
    my $keyid = "";
    while ( length($keyid) < 4 )
    {
        $keyid .= substr($symbols, $crc & 63, 1);
        $crc = $crc >> 6;
    }
    return $keyid;
}

sub crc24
{
    my $CRC24_INIT = 0x00b704ce;
    my $CRC24_POLY = 0x00864cfb;
    my $key = shift;
    my $key_length = length($key);
    my $crc = $CRC24_INIT;
    my $position = 0;

    while ( $position < $key_length )
    {
        $crc ^= (unpack("C", substr($key, $position, 1)) << 16);
        my $i;
        for ($i = 0; $i < 8; $i++)
        {
            $crc <<= 1;
            $crc ^= $CRC24_POLY if ($crc & 0x01000000)
        }
        $position++;
    }
    return $crc & 0x00ffffff;
}


sub makekeyidstr
{
    my $keyid = shift;
    my $str = shift;

    if ( $str ne "" )
    {
        # special handling for strings starting with font descriptions like {&Tahoma8} (win system integration)
        if ( $str =~ s/^(\{[\&\\][^\}]+\})// )
        {
            return "$1$keyid‖$str";
        }
        else
        {
            return "$keyid‖$str";
        }
    }
    else
    {
        return "";
    }
}
