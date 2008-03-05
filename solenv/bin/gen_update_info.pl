:
eval 'exec perl -wS $0 ${1+"$@"}'
        if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: gen_update_info.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:24:24 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************


#*********************************************************************
#
# main
#

my($product, $buildid, $id, $os, $arch, $lstfile, $languages, $productname, $productversion);

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
    if( /\bPRODUCTVERSION / ) {
        chomp;
        s/.*PRODUCTVERSION //;
        $productversion = $_;
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
   s/version></version>$productversion</;
   s/name></name>$productname</;
   print;
}

close(SOURCE);
