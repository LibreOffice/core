:
eval 'exec perl -wS $0 ${1+"$@"}'
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
# This tool is used to re-write and substitute variables
# into Unix .desktop, .keys files etc.
#

$destdir = pop @ARGV;
mkdir $destdir,0777;

$productname = "LibreOffice";
$productfilename = "libreoffice";
$urls = 0;
$prefix = "";
$iconprefix = "";

while ($_ = $ARGV[0], /^-/) {
    shift;
    last if /^--$/;
    if (/^-p/) {
        $productname = $ARGV[0];
        shift;
    }
    if (/^-u/) {
        $productfilename = $ARGV[0];
        shift;
    }
    if (/^--prefix/) {
        $prefix = $ARGV[0];
        shift;
    }
    if (/^--iconprefix/) {
        $iconprefix = $ARGV[0];
        shift;
    }
    if (/^--category/) {
        $category = $ARGV[0];
        shift;
    }
    # Whether we can handle URLs on the command-line
    if (/^--urls/) {
    $urls = 1;
    }
}


while ($arg = shift) {
    unless (open INFILE,$arg) {
        print STDOUT "Can't open input file $arg: $!\n";
        exit 1;
    }

    $srcfile = substr($arg, rindex($arg, "/") + 1);

    unless (open OUTFILE,"> $destdir/$prefix$srcfile") {
        print STDOUT "Can't open output file $destdir/$prefix$srcfile: $!\n";
        exit 1;
    }

    while (<INFILE>) {
        # remove possible Windows line-ends
        chomp;

        # patch all icons in ICON line with $prefix
        if (! /Icon=document-new/ ) { # except for Icon=document-new which
                                      # is a stock icon, not one of ours
            s/Icon=/Icon=$iconprefix/;
        }

        # patch all occurrences of openoffice in icon_filename
        # line with $prefix
        s/icon_filename=/icon_filename=$iconprefix/;

        # patch all occurrences of openoffice in EXEC line with
        # $productfilename
        if ( /Exec/ ) {
            s/openoffice/$productfilename/;
        }

        # if $productfilename != "openoffice, add it to the list
        # of applications.
        if ( /user_level=$/ ) {
            $_ = $_ . $productfilename;
        } elsif ( /user_level/ ) {
            s/$productfilename,//;
            s/user_level=/user_level=$productfilename,/
        }

        # append special category if specified
        if ( /Categories/ ) {
            if ( length($category) > 0 ) {
                $_ = "$_$category;";
            }
        }

        # replace %PRODUCTNAME placeholders
        s/%PRODUCTNAME/$productname/g;

        if ( $urls ) {
            s/%%FILE%%/%U/g;
        } else {
            s/%%FILE%%/%F/g;
        }

        print OUTFILE "$_\n";
    }

    close(OUTFILE);
    close(INFILE);
}
