:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: cwsanalyze.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-06-26 00:23:19 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

#
# cwsanalyze.pl   - analyze child workspace, report findings
# cwsintegrate.pl - integrate child workspace into master workspace
#
# TODO implements 'alerts' for integrate mode

use strict;
use File::Basename;
use Getopt::Long;
use Cwd;
use IO::Handle;

#### module lookup

use lib ("$ENV{SOLARENV}/bin/modules", "$ENV{COMMON_ENV_TOOLS}/modules");
use Cws;
use CvsModule;
use Cvs;
eval { require Logging; import Logging; };
# $log variable is only defined in SO environment...
my $log = undef;
$log = Logging->new() if (!$@);

######### Interrupt handler #########
 $SIG{'INT'} = 'INT_handler';

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.2 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### hardcoded globals #####

# Don't use this tool on a MWS workspaces in the veto
# list, because the behavior my not be clearly defined
# examples:
#   no MWS branch exists
#   MWS branch is obsolete
my @veto_list = (
                    'SRX644',
                    'FIX645'
                );

my %veto_hash;
foreach (@veto_list) {
    $veto_hash{$_}++;
}

# Commit veto pattern. Never integrate files matching this pattern
my %commit_veto_hash;
my @commit_veto_list = (
                        # The defs file mechanism is not compatible with CWS.
                        # It really needs to be changed.
                        'defs\/wntmsci\d+$'
                );
foreach (@commit_veto_list) {
    $commit_veto_hash{$_}++;
}


#### global #####

my $is_debug              = 0;              # misc traces for debugging purposes
my $mode                  = $script_name;   # operational mode (cwsanalyze|cwsintegrate)
my $opt_fast              = 0;              # fast mode, disable conflict check
my $opt_force             = 0;              # force integration
my $opt_no_set_integrated = 0;              # don't toggle integration status
my $vcsid = "unkown";
my @args_bak = @ARGV;

#### main #####
my $parameter_list = $log->array2string(";",@args_bak) if defined($log);

my ($dir, @modules) = parse_options();
my $cws = get_and_verify_cws();
@modules = verify_modules($cws, @modules);
if ( $mode eq 'cwsanalyze' ) {
    analyze($cws, $dir, @modules);
}
else {
    integrate($cws, $dir, @modules);
}
print_plog();
$log->end_log_extended($script_name,$vcsid,"success") if defined($log);
exit(0);

#### subroutines ####

# Get current child workspace from environment.
sub get_and_verify_cws
{
    my $childws  = $ENV{CWS_WORK_STAMP};
    my $masterws = $ENV{WORK_STAMP};

    if ( exists $veto_hash{$masterws} ) {
        print_error("Please don't use this tool for a CWS based on MWS '$masterws',", 0);
        print_error("before you haven't resynced it to a more recent MWS.", 1);
    }

    if ( !defined($childws) || !defined($masterws) ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    my $cws = Cws->new();
    $cws->child($childws);
    $cws->master($masterws);
    $log->start_log_extended($script_name,$parameter_list,$masterws,$childws) if defined($log);

    # check if we got a valid child workspace
    my $id = $cws->eis_id();
    print "Master: $masterws, Child: $childws, $id\n" if $is_debug;
    if ( !$id ) {
        print_error("Child workspace $childws for master workspace $masterws not found in EIS database.", 2);
    }
    return $cws;
}

# Parse options and do some sanity checks;
# returns freshly allocated Cws reference.
sub parse_options
{   my $dir = 0;
    my $help = 0;
    my $success = GetOptions('d=s' => \$dir, 'n' => \$opt_fast, 'h' => \$help,
                             'F' => \$opt_force, 'q' => \$opt_no_set_integrated);
    if ( !$success || @ARGV<1 ) {
        usage();
        exit(1);
    }

    if ( $dir && $opt_fast ) {
        usage();
        exit(1);
    }

    $dir = $dir ? $dir : cwd();

    # check directory
    if ( ! -d $dir ) {
        print_error("'$dir' is not a directory.", 1);
    }
    if ( ! -w $dir ) {
        print_error("Can't write to directory '$dir'.", 1);
    }

    unless ( $opt_fast ) {
        my @cvsdirs = glob("$dir/*/CVS");
        if ( @cvsdirs ) {
            print_error("'$dir' contains CVS modules. Please specify an empty scratch directory.", 1);
        }
    }

    return ($dir, @ARGV);
}

# Check if requested modules are registered with child workspace.
sub verify_modules
{
    my $cws     = shift;
    my @modules = @_;

    my @registered_modules = $cws->modules();
    my $child = $cws->child();

    # call with empty modules list
    return @registered_modules if $modules[0] eq 'all';

    # create hash for easier searching
    my %registered_modules_hash = ();
    for (@registered_modules) {
        $registered_modules_hash{$_}++;
    }

    foreach (@modules) {
        if ( !exists $registered_modules_hash{$_} ) {
            print_error("Module '$_' is not registered for child '$child'.", 2);
        }
    }
    return @modules;
}

# Analyze changes in child work space
sub analyze
{
    my $cws     = shift;
    my $dir     = shift;
    my @modules = @_;

    my $child = $cws->child();
    my $master = $cws->master();

    print_message("Analyzing child workspace '$child', master workspace '$master'.");
    print_message("*** Conflict check disabled ***") if $opt_fast;
    # display child workspace approval status
    my $approval = $cws->get_approval();
    if ( !$approval ) {
        print_error("Internal error: can't get approval status for '$child'.", 3);
    }
    print_message("Child workspace approval status: $approval");

    my ($ntotal_merged, $ntotal_new, $ntotal_removed, $ntotal_conflicts, $ntotal_alerts)
            = (0, 0, 0, 0);
    foreach (@modules) {
        my ($nmerged, $nnew, $nremoved, $nconflicts, $nalerts) = analyze_module($cws, $dir, $_);
        $ntotal_merged    += $nmerged;
        $ntotal_new       += $nnew;
        $ntotal_removed   += $nremoved;
        $ntotal_conflicts += $nconflicts;
        $ntotal_alerts    += $nalerts;
    }
    my $merge_str = $opt_fast ? 'change(s)' : 'clean merge(s)';
    my $stats = "All Modules: $ntotal_merged $merge_str, $ntotal_new new, ";
    $stats .=   "$ntotal_removed removed";
    $stats .= $opt_fast ? '' : ", $ntotal_conflicts conflict(s), $ntotal_alerts alert(s)";
    print_message($stats);
}

# Integrate child workspace into master.
sub integrate
{
    my $cws     = shift;
    my $dir     = shift;
    my @modules = @_;

    my $child  = $cws->child();
    my $master = $cws->master();

    print_message("Integrating child workspace '$child' into master workspace '$master'.");
    # check if child workspace has been approved for integration
    my $approval = $cws->get_approval();
    if ( !$approval ) {
        print_error("Internal error: can't get approval status for '$child'.", 3);
    }

    if ( $approval eq 'new' ) {
        print_error("Child workspace '$child' is not approved for integration.", 3);
    }
    elsif ( $approval eq 'integrated' ) {
        print_error("Child workspace '$child' is already integrated.", 3) unless $opt_force;
    }
    elsif ( $approval eq 'canceled' ) {
        print_error("Child workspace '$child' has been canceled.", 3);
    }
    elsif ( $approval eq 'nominated' ) {
        print_message("Child workspace '$child' is nominated for integration, proceeding ...");
    }
    else {
        # can't happen
        print_error("Internal error: can't get approval status for '$child'.", 3);
    }

    my ($ntotal_merged, $ntotal_new, $ntotal_removed, $ntotal_conflicts, $ntotal_alerts) =
                (0, 0, 0, 0, 0);
    foreach (@modules) {
        my($nmerged, $nnew, $nremoved, $nconflicts, $nalerts) = integrate_module($cws, $dir, $_);
        $ntotal_new       += $nnew;
        $ntotal_removed   += $nremoved;
        $ntotal_merged    += $nmerged;
        $ntotal_conflicts += $nconflicts;
        $ntotal_alerts    += $nalerts;
    }

    print "All modules: $ntotal_new new, $ntotal_removed removed, $ntotal_merged merge(s), $ntotal_conflicts conflicts(s). $ntotal_alerts alert(s)\n";

    # set CWS status
    if ( !$opt_no_set_integrated ) {
        if ( $cws->set_integrated() ) {
            print_message("\nSet child workspace status to: integrated.");
        }
        else {
            print_error("Could not set child workspace status to integrated.", 0);
        }
    }
}

# Analyze child workspace module
sub analyze_module
{
    my $cws    = shift;
    my $dir    = shift;
    my $module = shift;

    print_message("========== Analyzing module '$module' ==========");

    my $cvs_module = get_cvs_module($cws, $module);
    if ( !$cvs_module ) {
        print_error("Failure during analysis of '$module', skipping.", 0);
        plog("A\t$module: analysis failed!");
        return (0, 0, 0, 0, 1);
    }
    my $changed_files_ref = get_changed_files($cws, $cvs_module);

    if ( !defined($changed_files_ref) ) {
        print_warning("Analysis of module '$module' failed.");
        return undef;
    }

    my ($master_branch_tag, $cws_branch_tag, $cws_root_tag) = $cws->get_tags();
    my ($nnew, $nremoved, $nmerged, $nconflicts, $nalerts) = (0, 0, 0, 0, 0);
    if ( @{$changed_files_ref} ) {
        # ok we've got changed files
        # to check for conflicts we check out the module on the master branch
        # and update all changed files against the child workspace branch
        my $save_dir;
        if ( !$opt_fast ) {
            STDOUT->autoflush(1);
            print_message("Check out for conflict check ...");
            my $co_ref = $cvs_module->checkout($dir, $master_branch_tag);
            STDOUT->autoflush(0);
            if ( !@{$co_ref} ) {
                print_error("Was not able to checkout module '$module',", 0);
                print_error("this might be caused by connection failures or authentication problems.", 0);
                print_error("Please check your \$HOME/.cvspass for missing entries!", 50);
            }
            # save working dir for later perusal
            # chdir into module
            $save_dir = cwd();
            if ( !chdir("$dir/$module") ) {
                print_error("Can't chdir() to '$dir/$module'", 6);
            }
        }
        foreach my $change_ref (@{$changed_files_ref}) {
            # Check against vetoed files
            foreach my $veto_pattern ( keys %commit_veto_hash ) {
                if ( $change_ref->[0] =~ /$veto_pattern/ ) {
                    print "\tA", "\t",
                            $change_ref->[0], " def file detected, will be ignored\n";
                    plog("A\t$module/$change_ref->[0]: def file detected!");
                    $nalerts++;
                }
            }
            if ( !defined($change_ref->[1]) && !defined($change_ref->[2]) ) {
                # removed file
                print "\tR", "\t", $change_ref->[0], "\n";
                $nremoved++;
            }
            elsif ( !defined($change_ref->[1]) ) {
                # new file
                print "\tN","\t", $change_ref->[0], "\t", $change_ref->[2], "\n";
                $nnew++;
            }
            else {
                # modified file
                if ( $opt_fast ) {
                    print "\tM", "\t", $change_ref->[0], "\t",
                          $change_ref->[1], " -> ",
                          $change_ref->[2], "\n";
                    $nmerged++;
                }
                else {
                    my $archive = $change_ref->[0];
                    if ( !-e $archive ) {
                        # changes have been made on the CWS for
                        # a file which has been removed on the MWS
                        print "\tA", "\t",
                              $change_ref->[0], " file removed on MWS but changed on CWS\n";
                        plog("A\t$module/$change_ref->[0]: file removed on MWS but changed on CWS. Please check!");
                        $nalerts++;
                    }
                    my $cvs_archive = get_cvs_archive($archive);
                    my $rc = update_file($cvs_archive, $cws_branch_tag, $cws_root_tag);
                    if ( $rc eq 'merged' ) {
                        print "\tM", "\t", $change_ref->[0], "\t",
                              $change_ref->[1], " -> ",
                              $change_ref->[2], "\n";
                        $nmerged++;
                    }
                    elsif ( $rc eq 'conflict' ) {
                        print "\tC", "\t", $change_ref->[0], "\t",
                              $change_ref->[1], " -> ",
                              $change_ref->[2], "\n";
                        plog("C\t$module/$change_ref->[0]: conflict!");
                        $nconflicts++;
                    }
                    else {
                        print_error("update_file() operation failed.", 6);
                    }
                }
            }
        }
        # chdir back
        chdir($save_dir) if( !$opt_fast );
    }

    # emit some statistics
    my $merge_str = $opt_fast ? 'change(s)' : 'clean merge(s)';
    my $stats = "'$module': $nmerged $merge_str, $nnew new, ";
    $stats .=   "$nremoved removed";
    $stats .= $opt_fast ? '' : ", $nconflicts conflict(s), $nalerts alert(s)";
    print_message($stats);

    return ($nmerged, $nnew, $nremoved, $nconflicts, $nalerts);
}

# Integrate child workspace module.
sub integrate_module
{
    # TODO break integrate_module() down into several routines
    my $cws    = shift;
    my $dir    = shift;
    my $module = shift;

    print_message("========== Integrating module '$module' ==========");

    my $cvs_module = get_cvs_module($cws, $module);
    if ( !$cvs_module ) {
        print_error("Failure during integration of '$module', skipping.", 0);
        plog("A\t$module: integration failed!");
        return (0, 0, 0, 0, 1);
    }
    my $changed_files_ref = get_changed_files($cws, $cvs_module);

    if ( !defined($changed_files_ref) ) {
        print_warning("Integration of module '$module' failed.");
        return undef;
    }
    my $child = $cws->child();

    my ($master_branch_tag, $cws_branch_tag, $cws_root_tag) = $cws->get_tags();

    # statistics counters
    my ($nnew, $nremoved, $nmerged, $nconflicts, $nalerts) = (0, 0, 0, 0, 0);
    if ( @{$changed_files_ref} ) {
        # ok we've got changed files
        STDOUT->autoflush(1);
        print_message("Checking out ...");
        $cvs_module->checkout($dir, $master_branch_tag);
        STDOUT->autoflush(0);
        # save working dir for later perusal
        my $save_dir = cwd();
        # chdir into module
        if ( !chdir("$dir/$module") ) {
                print_error("Can't chdir() to '$dir/$module'", 0);
        }
      COMMIT: foreach my $change_ref (@{$changed_files_ref}) {
            my $archive = $change_ref->[0];
            # Check against vetoed files
            foreach my $veto_pattern ( keys %commit_veto_hash ) {
                if ( $archive =~ /$veto_pattern/ ) {
                    print "\tA", "\t",
                            $archive, " def file detected. skipped\n";
                    plog("C\t$module/$archive: def file detected!");
                    $nalerts++;
                    next COMMIT; # jump to next changed file
                }
            }
            if ( defined($change_ref->[2]) && !-d dirname($archive) )
            {
                if ( defined($change_ref->[1]) ) {
                }
                else {
                    # A new file appeared in a directory which has not yet
                    # been added to our local CVS tree.
                    sanitize_cvs_hierarchy($archive);
                }
            }
            my $cvs_archive = get_cvs_archive($archive);
            #  Determine the branch root of the CWS branch and the master branch
            my $branch_rev = $cvs_archive->get_branch_rev($cws_branch_tag);
            my $master_branch_rev = $cvs_archive->get_branch_rev($master_branch_tag);
            if ( !$branch_rev ) {
                # can't happen
                print_error("Internal error: can't determine CWS branch root", 0);
            }
            my @merge_comments;
            push(@merge_comments, get_revision_comments($cvs_archive, $branch_rev));
            my $change_type;
            if ( !defined($change_ref->[1]) && !defined($change_ref->[2]) ) {
                # removed file
                push(@merge_comments, "INTEGRATION: CWS $child ($branch_rev); FILE REMOVED\n");
                $change_type = 'removed';
            }
            elsif ( !defined($change_ref->[1]) ) {
                # new file
                push(@merge_comments, "INTEGRATION: CWS $child ($branch_rev); FILE ADDED\n");
                $change_type = 'new';
            }
            else {
                # modified file
                push(@merge_comments, "INTEGRATION: CWS $child ($branch_rev); FILE MERGED\n");
                $change_type = 'changed';
            }

            # write out merge comment file
            my $merge_comment_file = $archive . '.mergecomment';
            if ( !open(COMMENTFILE, ">$merge_comment_file") ) {
                print_error("$script_name: can't open $merge_comment_file: $!", 0);
            }
            print COMMENTFILE reverse @merge_comments;
            close(COMMENTFILE);

            # get the revision authors and taskids
            my $revision_authors_ref = get_revision_authors($cvs_archive, $branch_rev);
            my $revision_taskids_ref = get_revision_taskids($cvs_archive, $branch_rev);

            # do the update
            my $new_revision;
            my $rc = update_file($cvs_archive, $cws_branch_tag, $cws_root_tag);
            if ( !$rc ) {
                print_error("Update operation for file '$archive' failed for unknown reasons.", 0);
                plog("$module/$archive: update operation failed for unknown reasons, can't commit changes!");
            }
            elsif ( $rc eq 'merged' ) {
                # commit merges
                ($rc, $new_revision) = commit_file($cvs_archive, $merge_comment_file);
                unless ( $rc eq 'success' || $rc eq 'nothingcomitted' ) {
                    print_error("Can't commit changes to '$archive'", 0);
                    plog("\t$module/$archive: can't commit changes!");
                    next;
                }
                print "\tN\t$archive\n" and $nnew++ if $change_type eq 'new';
                print "\tR\t$archive\n" and $nremoved++ if $change_type eq 'removed';
                print "\tM\t$archive\n" and $nmerged++ if $change_type eq 'changed';
                unlink($merge_comment_file) if $rc eq 'success';
            }
            elsif ( $rc eq 'conflict' ) {
                print "\tC\t$archive\n";
                plog("C\t$module/$archive: conflict!");
                $nconflicts++;

                # TODO: clean up this mess
                # Find out what the new revision will be after the conflict has been
                # solved. We are going to report it to EIS even if the commit
                # has not yet been done
                my $last_rev;
                if ( !$master_branch_rev ) {
                    # master branch is head branch
                    $last_rev = $cvs_archive->get_head();
                }
                else {
                    my $revs_ref = $cvs_archive->get_sorted_revs();
                    foreach my $rev (@{$revs_ref}) {
                        if ( $rev =~ /^$master_branch_rev\.\d+/ ) {
                            $last_rev = $rev;
                        }
                    }
                }
                my @rev_field = split(/\./, $last_rev);
                $rev_field[-1]++;
                $new_revision = join('.', @rev_field);
            }
            else {
                # can't happen
                print_error("Internal error: update failed for unknown reasons", 0);
            }

            # register new revision with EIS
            $cws->add_file($module, $archive, $new_revision,
                           $revision_authors_ref, $revision_taskids_ref);

        }
        # chdir back
        chdir($save_dir);
    }

    # emit some statistics
    print "'$module': $nnew new, $nremoved removed, $nmerged merge(s), $nconflicts conflicts(s). $nalerts alert(s)\n";
    return ($nmerged, $nnew, $nremoved, $nconflicts, $nalerts);
}

# Get all revision comments on child workspace branch.
sub get_revision_comments
{
    my $cvs_archive    = shift;
    my $branch_rev     = shift;

    # Collect all the logging information of the revisions on the branch
    my @merge_comment = ();
    my $revs_ref = $cvs_archive->get_sorted_revs();
    foreach my $rev (@{$revs_ref}) {
        if ( $rev =~ /^$branch_rev\.\d+/ ) {
            my $log_ref = $cvs_archive->get_data_by_rev()->{$rev};
            my $comment = "$log_ref->{'DATE'} $log_ref->{'AUTHOR'} $rev: $log_ref->{'COMMENT'}";
            if ( $log_ref !~ /^RESYNC:/ ) {
                push(@merge_comment, $comment);
            }
        }
    }
    wantarray ? @merge_comment : \@merge_comment;
}

sub get_revision_authors
{
    my $cvs_archive    = shift;
    my $branch_rev     = shift;

    # Collect all the authors of the revisions on the branch
    my @authors;
    my %authors_hash;
    my $revs_ref = $cvs_archive->get_sorted_revs();
    foreach my $rev (@{$revs_ref}) {
        if ( $rev =~ /^$branch_rev\.\d+/ ) {
            my $log_ref = $cvs_archive->get_data_by_rev()->{$rev};
            $authors_hash{$log_ref->{'AUTHOR'}}++;
        }
    }
    push(@authors, sort keys %authors_hash);
    wantarray ? @authors : \@authors;
}

sub get_revision_taskids
{
    my $cvs_archive    = shift;
    my $branch_rev     = shift;

    # Collect all the taskids of the revisions on the branch
    my @task_ids;
    my %task_ids_hash;
    my $revs_ref = $cvs_archive->get_sorted_revs();
    foreach my $rev (@{$revs_ref}) {
        if ( $rev =~ /^$branch_rev\.\d+/ ) {
            my $log_ref = $cvs_archive->get_data_by_rev()->{$rev};
            my @ids = extract_taskids($log_ref->{'COMMENT'});
            foreach (@ids) {
                $task_ids_hash{$_}++;
            }
        }
    }
    push(@task_ids, sort keys %task_ids_hash);
    wantarray ? @task_ids : \@task_ids;
}

# Extract task ids from revision comment.
sub extract_taskids
{
    # Task ids can be of the form:
    #     #12345#,#4711# pr #12345,4711#
    # Optionally task ids may be prefixed with 'i' for issuezilla-
    # We may have to cope with a bit of white space, too.
    my $comment = shift;

    my @ids;
    my @candidates;
    if ( @candidates = ($comment =~ /#([i\d\s,]+)#/gi) ) {
        foreach my $candidate (@candidates) {
            my @field = split(/,/, $candidate);
            foreach (@field) {
                tr/ //d;
                push(@ids, $_) if /^i?\d+$/i ;
            }
        }
    }
    wantarray ? @ids : \@ids;
}


# Update file and report findings.
sub update_file
{

    my $cvs_archive    = shift;
    my $cws_branch_tag = shift;
    my $cws_root_tag   = shift;

    my $file = $cvs_archive->name();
    my $merge_option = '';
    if ( -e $file ) {
        # find out if we need to use the '-kk' flag for merging
        my ($status, $working_rev, $repository_rev, $sticky_tag, $branch_rev,
                $sticky_date, $sticky_options) = $cvs_archive->status();
        if ( $status eq 'unkownfailure' || $status eq 'connectionfailure' ) {
            print_error("can't get status of '$file': $status", 0);
            return undef;
        }
        $merge_option = ($sticky_options eq 'kb') ? '' : '-kk';
    }

    my $rc = $cvs_archive->update("$merge_option -j$cws_root_tag -j$cws_branch_tag");
    if ( $rc eq 'success' ) {
        return 'merged';
    }
    elsif ( $rc eq 'conflict' ) {
        return 'conflict';
    }
    else {
        return undef;
    }
}

sub commit_file
{
    my $cvs_archive = shift;
    my $merge_comment_file = shift;

    my ($rc, $new_revision) = $cvs_archive->commit("-F $merge_comment_file");
    return ($rc, $new_revision);
}

# Returns changed files for module.
sub get_changed_files
{
    my $cws        = shift;
    my $cvs_module = shift;

    my ($master_branch_tag, $cws_branch_tag, $cws_root_tag) = $cws->get_tags();

    $cvs_module->verbose(1);
    STDOUT->autoflush(1);
    print_message("Retrieving changes ...");
    my $changed_files_ref = $cvs_module->changed_files($cws_root_tag,$cws_branch_tag);
    STDOUT->autoflush(0);
    return $changed_files_ref;
}

# New files may be in new CVS subdirectories.
# Check if the CVS subdir is available or
# add it to the local CVS tree
sub sanitize_cvs_hierarchy
{
    my $file = shift;

    my $cvs_dir = dirname($file);
    return if $cvs_dir eq '.'; # no need to check current dir

    my @elements = split(/\//, $cvs_dir);

    my $save_dir = cwd();

    foreach ( @elements ) {
        if ( ! -d $_ ) {
            my $rc = mkdir($_);
            print_error("can create directory '$_': $!", 9) unless $rc;
            # TODO use a Cvs method for this
            print_error("Operation not (yet) supported on Windows", 9) if $^O eq 'MSWin32';
            system("cvs.clt2 add $_ > /dev/null 2>&1 ");
        }
        if ( !chdir($_) ) {
            print_error("Can't chdir() to '$_'", 9);
        }
    }

    # chdir back
    chdir($save_dir);
}

# Retrieve CvsModule object for passed module.
sub get_cvs_module
{
    my $cws    = shift;
    my $module = shift;

    my $cvs_module = CvsModule->new();
    my ($method, $vcsid, $server, $repository);
    if ( defined($log) ) {
        ($method, $vcsid, $server, $repository) = get_cvs_root($cws, $module);
    }
    else {
        # For now just take the configured OOo sever. Later we might implement a mechanism were
        # only known OOo modules are fetched from the OOo server, the rest from a local
        # server
        my $config = CwsConfig::get_config();
        ($method, $vcsid, $server, $repository) = ($config->get_cvs_server_method(),
                                                   $config->get_cvs_server_id(),
                                                   $config->get_cvs_server(),
                                                   $config->get_cvs_server_repository());
    }

    return undef if  !($method && $vcsid && $server && $repository);

    $cvs_module->module($module);
    $cvs_module->cvs_method($method);
    $cvs_module->vcsid($vcsid);
    $cvs_module->cvs_server($server);
    $cvs_module->cvs_repository($repository);

    return $cvs_module;
}

# Return Cvs object for passed file.
sub get_cvs_archive
{
    my $file = shift;

    my $cvs_archive = Cvs->new();
    $cvs_archive->name($file);

    return $cvs_archive;
}

# Find out which CVS server holds the module, returns
# the elements of CVSROOT.
sub get_cvs_root
{
    require EnvHelper; import EnvHelper;
    my $cws    = shift;
    my $module = shift;

    my $master = $cws->master();

    my $vcsid = $ENV{VCSID};
    if ( !$vcsid ) {
        print_error("Can't determine VCSID. Please use setsolar.", 5);
    }

    my $workspace_db = EnvHelper::get_workspace_db();
    if ( !$workspace_db ) {
        print_error("Can't load workspace database.", 4);
    }

    my $key = "$master/drives/o:/projects/$module/scs";
    my $cvsroot = $workspace_db->get_value($key);

    if ( !$cvsroot  ) {
        print_error("No such module '$module' for '$master' in workspace database.", 0);
        return (undef, undef, undef, undef);
    }

    my ($dummy1, $method, $user_at_server, $repository) = split(/:/, $cvsroot);
    my ($dummy2, $server) = split(/@/, $user_at_server);

    if ( ! ($method && $server && $repository ) ) {
        print_error("Can't determine CVS server for module '$module'.", 0);
        return (undef, undef, undef, undef);
    }

    return ($method, $vcsid, $server, $repository);
}

# problem logging
{
    my @problem_log = ();

    sub plog
    {
        my $message = shift;

        push(@problem_log, $message);
    }

    sub print_plog
    {
        if ( @problem_log ) {
            print_message("========== Problem Log ==========");
            foreach ( @problem_log ) {
                print "\t$_\n";
            }
            print_message("========== End Problem Log ==========");
        }
    }
}

sub print_message
{
    my $message     = shift;

    print "$message\n";
    return;
}

sub print_warning
{
    my $message     = shift;
    print STDERR "$script_name: ";
    print STDERR "WARNING $message\n";
    return;
}

sub print_error
{
    my $message     = shift;
    my $error_code  = shift;

    print STDERR "$script_name: ";
    print STDERR "ERROR: $message\n";

    if ( $error_code ) {
        print STDERR "\nFAILURE: $script_name aborted.\n";
        $log->end_log_extended($script_name,$vcsid,$message) if defined($log);
        exit($error_code);
    }
    return;
}

sub usage
{
    if ( $mode eq 'cwsintegrate' ) {
        print STDERR "Usage: cwsintegrate [-h] [-d dir] [-q] [-F] <all|module> ... \n";
        print STDERR "Integrate one or more child workspace modules into master workspace.\n";
        print STDERR "Options:\n";
        print STDERR "\t-h \tprint this help\n";
        print STDERR "\t-d dir\tscratch space\n";
        print STDERR "\t-q \tdo not set child workspace status to 'integrated'\n";
        print STDERR "\t-F\tdo integration even if status is already 'integrated'\n";
    }
    else {
        print STDERR "Usage: cwsanalyze [-h] [-d dir] [-n] <all|module> ... \n";
        print STDERR "Analyze one or more child workspace modules.\n";
        print STDERR "Options:\n";
        print STDERR "\t-h \tprint this help\n";
        print STDERR "\t-d dir\tscratch space\n";
        print STDERR "\t-n\tdisable conflict check (no need for checkout)\n";
        print STDERR "The -d and -n options are mutually exclusive.\n";
    }
}
