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
