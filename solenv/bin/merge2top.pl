:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: merge2top.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:11:19 $
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

#
# merge2top.pl - merge branch into top level revision
#

use Cvs;
use Cwd;
use File::Basename;

#### script id #####

( $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

$id_str = ' $Revision: 1.3 $ ';
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
