:
eval 'exec perl -wS $0 ${1+"$@"}'
        if 0;

#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: gen_update_info.pl,v $
#
# $Revision: 1.5 $
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


#*********************************************************************
#
# main
#

my($product, $buildid, $id, $os, $arch, $lstfile, $languages, $productname, $productversion, $productedition);

while ($_ = $ARGV[0], /^-/) {
    shift;
    last if /^--$/;
    if (/^--product/) {
        $product= $ARGV[0];
        shift;
    }
    if (/^--buildid/) {
        $buildid = $ARGV[0];
        shift;
    }
    if (/^--os/) {
        $os = $ARGV[0];
        shift;
    }
    if (/^--arch/) {
        $arch = $ARGV[0];
        shift;
    }
    if (/^--lstfile/) {
        $lstfile = $ARGV[0];
        shift;
    }
    if (/^--languages/) {
        $languages = $ARGV[0];
        shift;
    }
}

$sourcefile = $ARGV[0];

if( $^O =~ /cygwin/i ) {
    # We might get paths with backslashes, fix that.
    $lstfile =~ s/\\/\//g;
    $sourcefile =~ s/\\/\//g;
}

# read openoffice.lst
# reading Globals section
unless(open(LSTFILE, "sed -n \"/^Globals\$/,/^}\$/ p\" $lstfile |")) {
    print STDERR "Can't open $lstfile file: $!\n";
    return;
}

while (<LSTFILE>) {
    if( /\bPRODUCTNAME / ) {
        chomp;
        s/.*PRODUCTNAME //;
        $productname = $_;
    }
    if( /\bPACKAGEVERSION / ) {
        chomp;
        s/.*PACKAGEVERSION //;
        $productversion = $_;
    }
    if( /\bPRODUCTEDITION / ) {
        chomp;
        s/.*PRODUCTEDITION //;
        $productedition = $_;
    }
}

close(LSTFILE);

### may be hierarchical ...
if(open(LSTFILE, "sed -n \"/^$product:/,/^}\$/ p\" $lstfile |")) {
    while (<LSTFILE>) {
        if ( /^$product\s?:\s?(\w+)$/ ) {
            $product = $1;
        }
        if( /\bPRODUCTEDITION / ) {
            chomp;
            s/.*PRODUCTEDITION //;
            $productedition = $_;
        }
    }
}
close(LSTFILE);

# Reading product specific settings

unless(open(LSTFILE, "sed -n \"/^$product\$/,/^}\$/ p\" $lstfile |")) {
    print STDERR "Can't open $lstfile file: $!\n";
    return;
}

while (<LSTFILE>) {
    if( /\bPRODUCTNAME / ) {
        chomp;
        s/.*PRODUCTNAME //;
        $productname = $_;
    }
    if( /\bPACKAGEVERSION / ) {
        chomp;
        s/.*PACKAGEVERSION //;
        $productversion = $_;
    }
    if( /\bPRODUCTEDITION / ) {
        chomp;
        s/.*PRODUCTEDITION //;
        $productedition = $_;
    }
}

close(LSTFILE);

# simulate the behavior of make_installer.pl when writing versionrc
unless( "$os" eq "Windows" ) {
  $languages =~ s/_.*//;
}

$id = $productversion;
$id =~ s/\..*//;
$id = $productname . "_" . $id . "_" . $languages;

# open input file
unless (open(SOURCE, $sourcefile)) {
    print STDERR "Can't open $sourcefile file: $!\n";
    return;
}

while (<SOURCE>) {
   s/:id></:id>$id</;
   s/buildid></buildid>$buildid</;
   s/os></os>$os</;
   s/arch></arch>$arch</;
   if ( $productedition ) {
       s/edition></edition>$productedition</;
   } else {
       next if ( /edition></ );
   }
   s/version></version>$productversion</;
   s/name></name>$productname</;
   print;
}

close(SOURCE);
