:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************




#*********************************************************************
#
# main
#

$destdir = pop @ARGV;
mkdir $destdir,0777;

$productname = "Apache OpenOffice";
$productfilename = "apacheopenoffice";
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
