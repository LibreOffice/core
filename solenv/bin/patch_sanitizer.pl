:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: patch_sanitizer.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-03-05 16:38:02 $
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
        if ( @{$oldpatchfile{$file}{'data'}} ==  @{$newpatchfile{$file}{'data'}} ) {
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
