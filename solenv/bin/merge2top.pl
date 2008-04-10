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
# $RCSfile: merge2top.pl,v $
#
# $Revision: 1.4 $
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
# merge2top.pl - merge branch into top level revision
#

use Cvs;
use Cwd;
use File::Basename;

#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

$id_str = ' $Revision: 1.4 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### globals ####

$is_debug    = 0;       # debug flag

$files       = ();      # files to merge

#### main ####

my $branchlabel = parse_args(\@ARGV);
my ($success, $failure) = process_files($branchlabel, \@files);

print "Merged $success file(s) successfully\n" if $success;
print "Failed to merge $failure file(s)\n" if $failure;
$failure ? exit(2) : exit(0);

#### subroutines #####

sub parse_args
{
    # parse command line arguments

    my $args_ref = shift;

    # check for branchlabel
    my $branchlabel_next = 0;
    foreach (@{$args_ref}) {
        /^-b$/ && ++$branchlabel_next && next;
        /^-b(\w+)$/ && ($branchlabel=$1) && next;
        if ( $branchlabel_next ) {
            $branchlabel = $_;
            $branchlabel_next = 0;
        }
        else {
            push(@files, $_);
        }
    }

    print STDERR "branchlabel: $branchlabel\n" if $is_debug;
    print STDERR "file(s) to merge: " . join(',', @files) . "\n" if $is_debug;

    if ( !$branchlabel || $#files < 0 ) {
        usage();
        exit(1);
    }
    return $branchlabel;
}

sub process_files
{
    # iterate over file list and merge

    my $branchlabel = shift;
    my $files_ref   = shift;
    my $success;
    my $failure;
    my $cwd = getcwd();
    foreach my $file (@{$files_ref}) {
        if ( -r $file ) {
            chdir(dirname($file));
            merge($branchlabel, basename($file)) ? $success++ : $failure++;
            chdir($cwd);
        }
        else {
            print STDERR "$script_name: can't read $file: $!\n";
        }
    }
    return ($success, $failure);
}

sub merge
{
    # Merge $branchlabel branch to top for $file.
    # Expects cwd to be tuned to $file.

    my $branchlabel = shift;
    my $file        = shift;

    my $archive = Cvs->new();
    $archive->name($file);

    # Check if branchlabel exists.
    if ( !$archive->is_tag($branchlabel) ) {
        print STDERR "$script_name: '$file': no such branchlabel '$branchlabel'!\n";
        return $0;
    }
    # Is $branchlabel really a branchlabel?
    my $branch_rev = $archive->get_branch_rev($branchlabel);
    if ( !$branch_rev ) {
        print STDERR "$script_name: '$file': '$branchlabel' is not a branchlabel!\n";
        return $0;
    }

    # Ok we got the branch root.
    # Now collect all the logging information of the revisions on the branch
    my @merge_comment = ();
    push(@merge_comment, "$script_name: merge branch $branchlabel ($branch_rev)\n");
    my $revs_ref = $archive->get_sorted_revs();
    foreach my $rev (@{$revs_ref}) {
        if ( $rev =~ /^$branch_rev\.\d+/ ) {
            my $log_ref = $archive->get_data_by_rev()->{$rev};
            my $comment = "$$log_ref{'DATE'} $$log_ref{'AUTHOR'} $rev: $$log_ref{'COMMENT'}";
            push(@merge_comment, $comment);
        }
    }

    # update to top level revision
    if ( $archive->update('-A -kk') ne 'success' ) {
        print STDERR "$script_name: failed to update '$file', cancel merge!\n";
        return 0;
    }
    # write out merge comment
    my $mergecommentfile = $file . '.mergecomment';
    if ( !open(COMMENTFILE, ">$mergecommentfile") ) {
        print STDERR "$script_name: can't open $mergecommentfile $!\n";
        return 0;
    }
    print COMMENTFILE @merge_comment;
    close(COMMENTFILE);

    # merge
    my $rc = $archive->update("-j $branchlabel -kk");
    if ( $rc eq 'conflict' ) {
        print STDERR "$script_name: '$file': merge conflicts!\n";
        print STDERR "Resolve conflict manually and commit again with:\n";
        print STDERR "cvs commit -F $mergecommentfile $file\n";
        return 0;
    }
    elsif ( $rc ne 'success' ) {
        print STDERR "$script_name: '$file': merge failed for unknown reasons!\n";
        print STDERR "Resolve problem manually\n";
        unlink($mergecommentfile);
        return 0;
    }

    if ( $archive->commit("-F $mergecommentfile") ne 'success' ) {
        print STDERR "$script_name: '$file': PANIC: commit failed for unknown reasons\n";
        return 0;
    }

    # ok, everything went well, remove comment file
    unlink($mergecommentfile);
    # do top level update with -A switch to remove -kk sticky flag
    my $rc = $archive->update('-A');
    if ( $rc ne 'success' ) {
        print STDERR "$script_name: '$file': PANIC: cleanup update failed for unknown reasons\n";
    }
    return 1;
}

sub usage
{
    print STDERR "usage:\n";
    print STDERR "       $script_name -b <branch_label> <files> ...\n";
}
