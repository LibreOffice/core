#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: modules.pl,v $
#
# $Revision: 1.3 $
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

my $completelangiso_var = $ENV{COMPLETELANGISO_VAR};
my $lastcompletelangiso_var = "";
my $outfile = "";
my $infile = "";
my @infile = ();
my $globalcounter = 0;
my $globallinecounter = 0;

if ( !defined $completelangiso_var) {
    print STDERR "ERROR: No language defined!\n";
    exit 1;
}

startup_check();

# if ( "$completelangiso_var" eq "$lastcompletelangiso_var" ) {
#    print STDERR "No new languages. Keeping old file\n";
#    exit 0;
# }

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
    print STDERR "Wrote modules for $globalcounter languages ($globallinecounter lines)!\n";
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
        print STDERR "Reading template file: $infile\n";
        my $num = $#infile + 1;
        # print STDERR "Number of lines: $num\n";
    } else {
        die "Template file \"$infile\" not found!\n";
        exit 1;
    }

    if ( -f "$outfile" ) {
        # changed script - run always
        return if (stat($0))[9] > (stat("$outfile"))[9] ;
        # changed template file - run always
        return if (stat($infile))[9] > (stat("$outfile"))[9] ;

        open OLDFILE, "$outfile" or die "$0 - ERROR: $outfile exists but isn't readable.\n";
        while ( $line = <OLDFILE> ) {
            if ( $line =~ /^\/\/.*completelangiso:/ ) {
                $lastcompletelangiso_var = $line;
                chomp $lastcompletelangiso_var;
                $lastcompletelangiso_var =~ s/^\/\/.*completelangiso:\s*//;
                last;
            }
        }
        close OLDFILE;
    }

}

sub usage
{
    print STDERR "Generate language modules from language script particle template (*.sct file)\n";
    print STDERR "perl $0 -o outputfile -i inputfile\n";
    exit  1;
}
