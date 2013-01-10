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

my ($prefix, $ext, $key);
$productname = "Apache OpenOffice";
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
