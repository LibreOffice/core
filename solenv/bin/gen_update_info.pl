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
