:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: translate.pl,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 11:49:49 $
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

my ($prefix, $ext, $key);
$productname = "OpenOffice.org";
$workdir = ".";

while ($_ = $ARGV[0], /^-/) {
    shift;
    last if /^--$/;
    if (/^-p/) {
        $productname = $ARGV[0];
        shift;
    }
    if (/^-d/) {
        $workdir = $ARGV[0];
        shift;
    }
    if (/^--key/) {
        $key = $ARGV[0];
        shift;
    }
    if (/^--prefix/) {
        $prefix = $ARGV[0];
        shift;
    }
    if (/^--ext/) {
        $ext = $ARGV[0];
        shift;
    }
}

# open input file
unless (open(SOURCE, $ARGV[0])) {
    print STDERR "Can't open $ARGV[0] file: $!\n";
    return;
}


# For every section in the specified ulf file there should exist
# a template file in $workdir ..
while (<SOURCE>) {
    $line = $_;

    if ( "[" eq substr($line, 0, 1) ) {
        # Pass the tail of the template to the output file
        while (<TEMPLATE>) {
            print OUTFILE;
        }

        close(TEMPLATE);

        if (close(OUTFILE)) {
            system "mv -f $outfile.tmp $outfile\n";
        }

        $_ = substr($line, 1, index($line,"]")-1);
        $outfile = "$workdir/$prefix$_.$ext";

        # open the template file - ignore sections for which no
        # templates exist
        unless(open(TEMPLATE, $outfile)) {
            print STDERR "Warning: No template found for item $_: $outfile: $!\n";
            next;
        }

        # open output file
        unless (open(OUTFILE, "> $outfile.tmp")) {
            print STDERR "Can't create output file $outfile.tmp: $!\n";
            exit -1;
        }

        # Pass the head of the template to the output file
KEY:    while (<TEMPLATE>) {
            print OUTFILE;
            last KEY if (/$key/);
        }

    } else {
        # split locale = "value" into 2 strings
        ($locale, $value) = split(' = ', $line);

        if ( $locale ne $line ) {
            # replace en-US with en
            $locale=~s/en-US/en/;

            # use just anything inside the ""
            $value = substr($value, index($value, "\"") + 1, rindex($value, "\"") - 1);

            # replace resource placeholder
            $value=~s/%PRODUCTNAME/$productname/g;

            $locale=~s/-/_/;
            if ($ext eq "desktop") {
                print OUTFILE "$key\[$locale\]=$value\n";
            } else {
                print OUTFILE  "\t\[$locale\]$key=$value\n";
            }
        }
    }
}

while (<TEMPLATE>) {
    print OUTFILE;
}

if (close(OUTFILE)) {
    system "mv -f $outfile.tmp $outfile\n";
}

close(TEMPLATE);
