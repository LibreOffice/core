:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Andras Timar <atimar@suse.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#
# add keyids to pot files
#

use File::Find;

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

my $potdir = $ARGV[0];

if (!$potdir) {die "Usage_ $0 <directory of pot files>\n";}

my @potfiles = ();

File::Find::find( sub {/^.*\.pot\z/s && push @potfiles, $File::Find::name;}, $potdir );

foreach $f (@potfiles)
{
    open OLDPOT , "< $f" || die("Cannot open source pot file: $f\n");
    $fnew = $f . ".new";
    @path = split ("/",$f);
    $keypart1 = pop @path;
    $keypart1 =~ s/.pot//;
    $keypart1 .= "_";
    $keypart2 = pop @path;
    $keypart1 = $keypart2 . "_" . $keypart1;
    open NEWPOT , "> $fnew" || die("Cannot open target pot file: $f\n");
    while ( <OLDPOT> )
    {
        chomp;
        if( /^#: (.*)$/ )
        {
            print NEWPOT "#. " . keyidgen($keypart1 . $1) . "\n";
        }
        print NEWPOT $_ . "\n";
    }
    close OLDPOT;
    close NEWPOT;
    rename $fnew,$f;
}

exit 0;
