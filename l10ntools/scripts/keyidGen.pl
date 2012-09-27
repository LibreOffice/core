:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
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
        if (defined $path[-3])
        {
            $key = $path[-3] . "_" . $path[-2] . "_" . $path[-1] . "#" . $gid . ".";
        }
        else
        {
            $key = $prj . "_" . $path[-2] . "_" . $path[-1] . "#" . $gid . ".";
        }
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
