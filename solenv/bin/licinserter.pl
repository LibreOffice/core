:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

#
# licinserter.pl - create license entries in extension description.xml
#

use File::Basename;

my $langswitch;

sub usage()
{
    print STDERR "\nCreate extension descriptions with license-text entries\n";
    print STDERR "matching the language activated.\n";
    print STDERR "\nUsage:\n";
    print STDERR "\t$0 [--langsplit] infile \"naming pattern\" destination\n\n";
    print STDERR "\nExample:\n\n";
    print STDERR "$0 description.xml dir/license_xxx.txt outdir/description.xml\n\n";
    print STDERR "Creates \"someoutdir/description.xml\" with the license file entries like\n\"dir/license_en.US.txt\" ";
    print STDERR "for all languages found in the WITH_LANG environment\nvariable\n\n\n";
    print STDERR "Example2:\n\n";
    print STDERR "$0 --langsplit description.xml dir/license_xxx.txt someoutdir\n\n";
    print STDERR "Creates \"someoutdir/<language>/description.xml\" with one license file entry\n\"somedir/license_<language>.txt\" ";
    print STDERR "for all languages found in the WITH_LANG\nenvironment variable.\n\nNOTE: when using --langsplit \"destination\" needs to be a directory\n";
}

if ( $ARGV[0] =~ /^-/ ) {
    $langswitch = shift @ARGV;
    if ( $langswitch ne "--langsplit" ) {
        usage();
        exit 1;
    }
    if ( ! -d $ARGV[2] ) {
        print STDERR "\nERROR - $ARGV[2] is not directory\n";
        usage();
        exit 2;
    }
}

if ( $#ARGV != 2 ) {
    print "zzz\n";
    usage();
    exit 1;
}

open INFILE,$ARGV[0] or die "oops - no such file $ARGV[0]!\n";

my @inlines = <INFILE>;
close INFILE;

chomp @inlines;

# Empty or unset WITH_LANG environment variable is set to default en-US.
# When WITH_LANG is set but does not contain en-US then that is prepended.
my $WithLang = $ENV{WITH_LANG};
if ( ! defined $WithLang || $WithLang eq "")
{
    $WithLang = "en-US";
}
elsif ($WithLang !~ /\ben-US\b/)
{
    $WithLang = "en-US " . $WithLang;
}


if ( $langswitch eq "" ) {
    my @outlines;
    foreach my $i (@inlines) {
        if ( $i =~ /license-text/ ) {
            my $ii;
            my $name;
            foreach my $code ( split(/\s+/,$WithLang) ) {
                $ii = $i;
                $name = $ARGV[1];
                $name =~ s/xxx/$code/;
                $ii =~ s/isocode/$code/g;
                $ii =~ s?licensefile?$name?g;
                push @outlines, "$ii\n";
            }
        } else {
            push @outlines, "$i\n";
        }
    }
    open OUTFILE, ">$ARGV[2]" or die "ooops - can't open $ARGV[2] for writing\n";
    print OUTFILE @outlines;
    close OUTFILE or die "ooops - can't write to $ARGV[2]\n";
} else {
    my @outlines;
    my $outname = basename($ARGV[0],());
    foreach my $code ( split(/\s+/,$ENV{WITH_LANG}) ) {
        @outlines=();
        foreach my $i (@inlines) {
            if ( $i =~ /license-text/ ) {
                my $name;
                my $ii = $i;
                $name = $ARGV[1];
                $name =~ s/xxx/$code/;
                $ii =~ s/isocode/$code/g;
                $ii =~ s?licensefile?$name?g;
                push @outlines, "$ii\n";
            } else {
                push @outlines, "$i\n";
            }
        }
        mkdir "$ARGV[2]/$code";
        open OUTFILE, ">$ARGV[2]/$code/$outname" or die "ooops - can't open $ARGV[2]/$code/$outname for writing\n";
        print OUTFILE @outlines;
        close OUTFILE or die "ooops - can't write to $ARGV[2]/$code/$outname\n";
    }
}
