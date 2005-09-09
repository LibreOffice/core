:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: brand.pl,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 11:48:44 $
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

$destdir = pop @ARGV;
mkdir $destdir,0777;

$productname = "OpenOffice.org";
$productfilename = "openoffice";
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
}


while (<>) {
    unless (open INFILE,$ARGV) {
        print STDOUT "Can't open input file $ARGV: $!\n";
        exit 1;
    }

    $srcfile = substr($ARGV, rindex($ARGV, "/") + 1);

    unless (open OUTFILE,"> $destdir/$prefix$srcfile") {
        print STDOUT "Can't open output file $destdir/$prefix$srcfile: $!\n";
        exit 1;
    }

    while (<INFILE>) {
        # remove possible Windows line-ends
        chomp;

        # patch all occurances of openoffice in ICON line with
        # $prefix
        s/Icon=/Icon=$iconprefix/;

        # patch all occurances of openoffice in icon_filename
        # line with $prefix
        s/icon_filename=/icon_filename=$iconprefix/;

        # patch all occurances of openoffice in EXEC line with
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

        print OUTFILE "$_\n";
    }

    close(OUTFILE);
    close(INFILE);
}
