:
    eval 'exec perl -S $0 ${1+"$@"}'
        if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: hicontrast-to-theme.pl,v $
#
# $Revision: 1.4 $
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
# Create ordinary theme from HiContrast images.
#

use File::Copy;
use File::Find;
use File::Path;
use File::Spec;

( $src, $dst ) = @ARGV;

if ( $src eq "" || $dst eq "" ) {
    print STDERR "Usage: hicontrast-to-theme.pl src dest\n\n";
    print STDERR "Create ordinary theme from HiContrast images.\n";
    exit 1;
}

$dst = File::Spec->rel2abs( $dst );

@hc_table = (
    [ ".*_h.png",         "_h.png",    ".png" ],
    [ ".*_sch.png",       "_sch.png",  ".png" ],
    [ ".*_hc.png",        "_hc.png",   ".png" ],
    [ "lch_.*.png",       "lch_",      "lc_" ],
    [ "sch_.*.png",       "sch_",      "sc_" ],
    [ "lch[0-9].*.png",   "lch",       "lc" ],
    [ "sch[0-9].*.png",   "sch",       "sc" ],
    [ "loh[0-9].*.png",   "loh",       "lo" ],
    [ "lxh[0-9].*.png",   "lxh",       "lx" ],
    [ "sxh[0-9].*.png",   "sxh",       "sx" ],
    [ "avh[0-9].*.png",   "avh",       "av" ],
    [ "avlh[0-9].*.png",  "avlh",      "avl" ],
    [ "idh[0-9].*.png",   "idh",       "id" ],
    [ "imh[0-9].*.png",   "imh",       "im" ],
    [ "mih[0-9].*.png",   "mih",       "mi" ],
    [ "tbh[0-9].*.png",   "tbh",       "tb" ],
    [ "nah[0-9].*.png",   "nah",       "na" ],
    [ "nch[0-9].*.png",   "nch",       "nc" ],
    [ "nvh[0-9].*.png",   "nvh",       "nv" ],
    [ "ouh[0-9].*.png",   "ouh",       "ou" ],
    [ "ddh[0-9].*.png",   "ddh",       "dd" ],
    [ "sfh[0-9].*.png",   "sfh",       "sf" ],
    [ "srh[0-9].*.png",   "srh",       "sr" ],
    [ "wrh[0-9].*.png",   "wrh",       "wr" ],
    [ "alh[0-9].*.png",   "alh",       "al" ],
    [ "ath[0-9].*.png",   "ath",       "at" ],
    [ "bih[0-9].*.png",   "bih",       "bi" ],
    [ "coh[0-9].*.png",   "coh",       "co" ],
    [ "foh[0-9].*.png",   "foh",       "fo" ],
    [ "fuh[0-9].*.png",   "fuh",       "fu" ],
    [ "oph[0-9].*.png",   "oph",       "op" ],
    [ "unh[0-9].*.png",   "unh",       "un" ],
    [ "edh[0-9].*.png",   "edh",       "ed" ],
    [ "cdh[0-9].*.png",   "cdh",       "cd" ],
    [ "frh[0-9].*.png",   "frh",       "fr" ],
    [ "fwh[0-9].*.png",   "fwh",       "fw" ],
    [ "nuh[0-9].*.png",   "nuh",       "nu" ],
    [ "prh[0-9].*.png",   "prh",       "pr" ],
    [ "shh[0-9].*.png",   "shh",       "sh" ],
    [ "trh[0-9].*.png",   "trh",       "tr" ],
    [ "reh[0-9].*.png",   "reh",       "re" ],
    [ "joh[0-9].*.png",   "joh",       "jo" ],
    [ "fph[0-9].*.png",   "fph",       "fp" ],
    [ "dah[0-9].*.png",   "dah",       "da" ]
);

my (@from_stat, @to_stat);

sub copy_normalized {
    $file = $_;
    for $hc ( @hc_table ) {
        ( $what, $from, $to ) = @$hc;
        if ( $file =~ /$what/&&!($file=~/\.svn/) ) {
            my $dir = File::Spec->catdir( $dst, $File::Find::dir );

            if ( ! -d $dir ) {
                mkpath( $dir );
            }

            ( my $copy = $file ) =~ s/$from/$to/;
            $copy = File::Spec->catfile( $dir, $copy );

            @from_stat = stat($file);
            @to_stat = stat($copy);
            if ( $from_stat[9] > $to_stat[9] ) {
                copy( $file, $copy ) || die $!;
                utime( $from_stat[9], $from_stat[9], $copy );
            }

            last;
        }
    }
}

chdir( $src );
find( \&copy_normalized, '.' );
