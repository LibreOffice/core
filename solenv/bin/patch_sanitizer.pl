:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: patch_sanitizer.pl,v $
#
# $Revision: 1.6 $
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

use utf8;
use warnings;
use strict;

# command line arguments
my $oldpatchfile = shift;
my $newpatchfile = shift;
my $sortedfile = shift;

show_help() unless defined $oldpatchfile and defined $newpatchfile and defined $sortedfile;

my %oldpatchfile = parse_patch($oldpatchfile);
my %newpatchfile = parse_patch($newpatchfile);

open SORTEDPATCH, "> $sortedfile";

foreach my $file (sort (keys %newpatchfile)) {
    print SORTEDPATCH $file."\t";
    if (defined($oldpatchfile{$file})) {
        if ( (join '', @{$oldpatchfile{$file}{'data'}}) eq (join '', @{$newpatchfile{$file}{'data'}}) ) {
            # patch data for the file hasn't been modified, use the header from
            # the old patch, to reduce noise (keep the old timestamps)
            print SORTEDPATCH $oldpatchfile{$file}{'origtimestamp'}."\n";
            print SORTEDPATCH $oldpatchfile{$file}{'patchedfilename'}."\t";
            print SORTEDPATCH $oldpatchfile{$file}{'patchedtimestamp'}."\n";
            print SORTEDPATCH @{$oldpatchfile{$file}{'data'}};
            next;
        }
    }
    # either file wasn't patched before, or the patchset changed, so use the new
    # values for it..
    print SORTEDPATCH $newpatchfile{$file}{'origtimestamp'}."\n";
    print SORTEDPATCH $newpatchfile{$file}{'patchedfilename'}."\t";
    print SORTEDPATCH $newpatchfile{$file}{'patchedtimestamp'}."\n";
    print SORTEDPATCH @{$newpatchfile{$file}{'data'}};
}
close SORTEDPATCH;

###############
# Helper subs
###############
sub show_help {
    print "Usage: $0 oldpatch newpatch outputfilename\n";
    print "oldpatch and newpatch can be the very same file\n";
    print "will output a sanitized form of newpatch to outputfilename\n";
    print "if outputfilename is '-', the patch will be printed to stdout\n";
    print "sanitized means: It will avoid all unnecessary changes\n";
    exit 1;
}
sub parse_patch {
    my $patchfile = shift;
    my $patchtype;
    my $pfirst;
    my $psecond;

    my %hunks = ();
    my $origfilename;
    open PATCHFILE, "< $patchfile" or die "Cannot open file $patchfile $!";
    my @patchfile = <PATCHFILE>;
    close PATCHFILE;
    return %hunks if ( $#patchfile == -1 );
    if ( $patchfile[0] =~ /^---/ ) {
        $patchtype = "unified";
        $pfirst = '^--- [^\*]*$';
        $psecond = '^\+\+\+ [^\*]*$';
    } elsif ( $patchfile[0] =~ /^\*\*\*/ ) {
        $patchtype = "content";
        $pfirst = '^\*\*\* [^\*]*$';
        $psecond = '^--- .*\t.*$';
    } else {
        die "unknown patch format\n";
    }

    foreach (@patchfile) {
        if ( /$pfirst/ ) {
            my $timestamp;
            # extract the filename, to be able to compare the old
            # with the new file...
            ($origfilename, $timestamp) = split(/\t/, $_, 2);
            chomp $timestamp;
            # ideally convert the timestamp to iso-format...
            $hunks{$origfilename}{'origtimestamp'} = $timestamp;
            next;
        } elsif ( $_ =~ /$psecond/ ) {
            my ($filename, $timestamp) = split(/\t/, $_, 2);
            chomp $timestamp;
            # ideally convert the timestamp to iso-format...
            $hunks{$origfilename}{'patchedfilename'} = $filename;
            $hunks{$origfilename}{'patchedtimestamp'} = $timestamp;
            next;
        }
        push (@{$hunks{$origfilename}{'data'}}, $_);

    }
    return %hunks;
}
