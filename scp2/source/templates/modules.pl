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

my $completelangiso_var = $ENV{COMPLETELANGISO_VAR};
my $lastcompletelangiso_var = "";
my $outfile = "";
my $infile = "";
my @infile = ();
my $globalcounter = 0;
my $globallinecounter = 0;
my $verbose = 0;

if ( !defined $completelangiso_var) {
    print STDERR "ERROR: No language defined!\n";
    exit 1;
}

startup_check();

my @completelangiso = split " +", $completelangiso_var;

open OUTFILE, ">$outfile" or die "$0 ERROR: cannot open $outfile for writing!\n";
print OUTFILE "// generated file, do not edit\n\n";
print OUTFILE "// languages used for last time generation\n";
print OUTFILE "// completelangiso: $completelangiso_var\n\n";
write_ALL_MODULES();
close OUTFILE;
check_counter();

sub check_counter
{
    print STDERR "Wrote modules for $globalcounter languages ($globallinecounter lines)!\n" if $verbose;
    if ( $globalcounter == 0 )
    {
        print STDERR "ERROR: No languages found!\n";
        exit 1;
    }

    if ( $globallinecounter == 0 )
    {
        print STDERR "ERROR: No lines written!\n";
        exit 1;
    }
}


sub write_ALL_MODULES
{
    my $counter = 0;
    my $linecounter = 0;
    my $linecount = $#infile + 1;
    # print STDERR "Lines in inputfile: $linecount!\n";

    foreach $lang (@completelangiso) {
        $language = $lang;
        $language_ = $lang;
        $language_ =~ s/-/_/;
        $languagebig_ = uc($lang);
        $languagebig_ =~ s/-/_/;
        $counter++;
        my $sortkey = 100 * $counter;

        for ( $i = 0; $i <= $#infile; $i++) {
            my $line = $infile[$i];
            if (( $line =~  /^\s*\*/ ) || ( $line =~  /^\s*\/\*/ )) { next; }
            $line =~ s/\<LANGUAGE\>/$language/g;
            $line =~ s/\<LANGUAGE_\>/$language_/g;
            $line =~ s/\<LANGUAGEBIG_\>/$languagebig_/g;
            $line =~ s/\<SORTKEY\>/$sortkey/g;
            print OUTFILE $line;
            $linecounter++;
        }
        print OUTFILE "\n";
    }
    print OUTFILE "\n";

    $globalcounter = $counter;
    $globallinecounter = $linecounter;
}

sub startup_check
{
    my $i;

    if ( $#ARGV >= 0 )
    {
        if ( $ARGV[0] eq "-verbose" )
        {
            $verbose = 1;
            shift @ARGV;
        }
        elsif ( $ARGV[0] eq "-quiet" )
        {
            # no special quiet flag/mode
            shift @ARGV;
        }
    }

    for ( $i=0; $i <= $#ARGV; $i++) {
        if ( "$ARGV[$i]" eq "-o" ) {
            if ( defined $ARGV[ $i + 1] ) {
                $outfile = $ARGV[ $i + 1];
                $i++;
            }
        } elsif ( "$ARGV[$i]" eq "-i" ) {
            if ( defined $ARGV[ $i + 1] ) {
                $infile = $ARGV[ $i + 1];
                $i++;
            }
        } else {
            usage();
        }
    }

    usage() if $i < 3;
    usage() if "$outfile" eq "";
    usage() if "$infile" eq "";

    if ( -f "$infile" ) {
        open INFILE, "$infile" or die "$0 - ERROR: $infile exists but isn't readable.\n";
        @infile = <INFILE>;
        close( INFILE );
        print STDERR "Reading template file: $infile\n" if $verbose;
        my $num = $#infile + 1;
        # print STDERR "Number of lines: $num\n";
    } else {
        die "Template file \"$infile\" not found!\n";
        exit 1;
    }
}

sub usage
{
    print STDERR "Generate language modules from language script particle template (*.sct file)\n";
    print STDERR "perl $0 [-verbose] -o outputfile -i inputfile\n";
    exit  1;
}
