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
# $RCSfile: cwsanalyze.pl,v $
#
# $Revision: 1.20 $
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
# cwsanalyze.pl   - analyze child workspace, report findings
# cwsintegrate.pl - integrate child workspace into master workspace
#
# TODO implements 'alerts' for integrate mode
use strict;
use File::Basename;
use Getopt::Long;
use Cwd;
use IO::Handle;
use Carp;

#### module lookup
my @lib_dirs;
BEGIN {
    if ( !defined($ENV{SOLARENV}) ) {
        die "No environment found (environment variable SOLARENV is undefined)";
    }
    push(@lib_dirs, "$ENV{SOLARENV}/bin/modules");
    push(@lib_dirs, "$ENV{COMMON_ENV_TOOLS}/modules") if defined($ENV{COMMON_ENV_TOOLS});
}
use lib (@lib_dirs);

use Cws;
use CvsModule;
use Cvs;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;
my $config = CwsConfig->get_config();
my $sointernal = $config->sointernal();

my $script_rev;
my $id_str = ' $Revision: 1.20 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### FIXME: hardcoded globals #####

# Don't use this tool on a MWS workspaces in the veto
# list, because the behavior may not be clearly defined
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
my $checklicense          = 1;              # grep for LGPL 2.1 license header
my $mode                  = $script_name;   # operational mode (cwsanalyze|cwsintegrate)
my $opt_fast              = 0;              # fast mode, disable conflict check
my $opt_force             = 0;              # force integration
my $opt_no_set_integrated = 0;              # don't toggle integration status
my $vcsid = "unknown";
my $opt_auto_int          = 0;              # integrate automatically if no conflicts are shown
my $opt_resume            = 0;              # resume a broken integration
my $opt_quiet             = 0;              # minimal output
my @args_bak = @ARGV;
my @problem_log = ();

#### main #####

my ($dir, @modules) = parse_options();
my $cws = get_and_verify_cws();
@modules = verify_modules($cws, @modules);
if ( $mode eq 'cwsanalyze' ) {
    analyze($cws, $dir, @modules);
}
else {
    auto_int($cws, $dir,@modules) if ($opt_auto_int); # will exit and doesn't return
    integrate($cws, $dir, @modules);
}
print_plog();
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

    # check if we got a valid child workspace
    my $id = $cws->eis_id();
    print "Master: $masterws, Child: $childws, $id\n" if $is_debug;
    if ( !$id ) {
        print_error("Child workspace $childws for master workspace $masterws not found in EIS database.", 2);
    }

    # old code lines must not get checked against current license header
    if ( lc $masterws eq "srx645" || $masterws =~ /^\w\w\w680$/ ) {
        $checklicense = 0;
    }

    return $cws;
}

# Parse options and do some sanity checks;
# returns freshly allocated Cws reference.
sub parse_options
{   my $dir = 0;
    my $help = 0;
    my $success = GetOptions('d=s' => \$dir, 'n' => \$opt_fast, 'h' => \$help,
                             'F' => \$opt_force, 'q' => \$opt_no_set_integrated, 'A' => \$opt_auto_int, 'r' => \$opt_resume, 'z' => \$opt_quiet);
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

# Check whether there are new modules registred with this CWS
sub have_new_modules
{
    my $cws     = shift;
    my ( $newmods_ref, $newmodspriv_ref );

    $newmods_ref = eval {
        $cws->new_modules();
    } || [];
    if ( $@ ) {
        # Method 'new_modules()' for package 'Cws' is not available on all workspaces.
        # In case it's not there give an error message, but do not fail
        print_error( $@ );
    }
    $newmodspriv_ref = eval {
        $cws->new_modules_priv();
    } || [];
    if ( $@ ) {
        print_error ($@ );
    }
    return ( $newmods_ref, $newmodspriv_ref );
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
    print_message("Child workspace approval status: '$approval'");

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

    my ( $new_modules_ref, $new_private_modules_ref ) = have_new_modules( $cws );
    print_message( "New public modules: " . join ' ', @$new_modules_ref ) if @$new_modules_ref;
    print_message( "New internal modules: " . join ' ', @$new_private_modules_ref ) if @$new_private_modules_ref;

    return ($ntotal_conflicts,$ntotal_alerts) if $opt_auto_int;
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
    my @failed_registrations;
    foreach (@modules) {
        my($nmerged, $nnew, $nremoved, $nconflicts, $nalerts, $eisfailure_ref) = integrate_module($cws, $dir, $_);
        $ntotal_new       += $nnew;
        $ntotal_removed   += $nremoved;
        $ntotal_merged    += $nmerged;
        $ntotal_conflicts += $nconflicts;
        $ntotal_alerts    += $nalerts;
        push @failed_registrations, @$eisfailure_ref;
    }

    # If some files could not be registered to EIS try again
    # (EIS may have been temporarily unavailable)
    my @repeated_regfailures;
    foreach my $arg_ref ( @failed_registrations ) {
        $cws->add_file( @$arg_ref ) or push @repeated_regfailures, $$arg_ref[0] .'/'. $$arg_ref[1];
    }

    print "All modules: $ntotal_new new, $ntotal_removed removed, $ntotal_merged merge(s), $ntotal_conflicts conflicts(s). $ntotal_alerts alert(s)\n";

    my ( $new_modules_ref, $new_private_modules_ref ) = have_new_modules( $cws );
    print_message( "New public modules: " . join ' ', @$new_modules_ref ) if @$new_modules_ref;
    print_message( "New internal modules: " . join ' ', @$new_private_modules_ref ) if @$new_private_modules_ref;

    if ( @repeated_regfailures ) {
        prompt_manual_registration( $child, $master, @repeated_regfailures );
    }

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
                print_error("this might be caused by connection failures or authentification problems.", 0);
                print_error("Please check your \$HOME/.cvspass for missing entries!", 50);
            }
            # save working dir for later perusal
            # chdir into module
            $save_dir = cwd();
            if ( !chdir("$dir/$module") ) {
                my $test = cwd();
                if ( $test !~ /.+$dir\/$module$/) {
                    print_error("Can't chdir() to '$dir/$module'", 6);
                }
            }
        }
      ANALYZE: foreach my $change_ref (@{$changed_files_ref}) {
            # Check against vetoed files
            foreach my $veto_pattern ( keys %commit_veto_hash ) {
                if ( $change_ref->[0] =~ /$veto_pattern/ ) {
                    next ANALYZE; # jump to next changed file
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
                if ( $checklicense && ! $opt_fast ) {
                    # check for outdated license header
                    my $repository = $cvs_module->cvs_repository();
                    if ( $repository !~ /cvs_so/ ) {
                        my $archive = $change_ref->[0];
                        if ( defined($change_ref->[2]) && !-d dirname($archive) ) {
                            # A new file appeared in a directory which has not yet
                            # been added to our local CVS tree.
                            sanitize_cvs_hierarchy($archive);
                        }
                        my $cvs_archive = get_cvs_archive($archive);
                        my $rc = update_file($cvs_archive, $cws_branch_tag, $cws_root_tag);
                        if ( check_for_lgpl21($archive) ) {
                            print "\tA", "\t",
                                  $archive, " file has old license header (LGPL v. 2.1)\n";
                            plog("A\t$module/$archive: file has LGPL v. 2.1.");
                            $nalerts++;
                        }
                    }
                }
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

                    # check for outdated license header
                    my $repository = $cvs_module->cvs_repository();
                    if ( $checklicense && $repository !~ /cvs_so/ ) {
                        if ( check_for_lgpl21($archive) ) {
                            print "\tA", "\t",
                                  $archive, " file has old license header (LGPL 2.1)\n";
                            plog("A\t$module/$archive: file has LGPL 2.1.");
                            $nalerts++;
                        }
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
    # for error handling of EIS registrations
    my @failed2register;

    if ( @{$changed_files_ref} && $opt_resume ) {
        # opt_resume: we already have integrated parts of this CWS
        $changed_files_ref = clear_finished_files( $cws, $module, $changed_files_ref );
    }

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
                # If this go wrong it's most likely due to a CVS problem
                print_error("Can't determine CWS branch root, CVS problems?", 99);
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
                    if ( !defined($last_rev) ) {
                        # no revision has ever been committed on the
                        # master branch => give $last_rev an artificial
                        # "0" revision
                        $last_rev = $master_branch_rev . '.0';
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

            # check for outdated license header
            my $repository = $cvs_module->cvs_repository();
            if ( $checklicense && $repository !~ /cvs_so/ ) {
                if ( check_for_lgpl21($archive) ) {
                    print "\tA", "\t",
                          $archive, " file has old license header (LGPL 2.1)\n";
                    plog("A\t$module/$archive: file has LGPL 2.1.");
                    $nalerts++;
                }
            }

            # register new revision with EIS
            if ( ! $cws->add_file($module, $archive, $new_revision,
                           $revision_authors_ref, $revision_taskids_ref) ) {
                push @failed2register, [$module, $archive, $new_revision,
                           $revision_authors_ref, $revision_taskids_ref];
            }

        }
        # chdir back
        chdir($save_dir);
    }

    # emit some statistics
    print "'$module': $nnew new, $nremoved removed, $nmerged merge(s), $nconflicts conflicts(s). $nalerts alert(s)\n";
    return ($nmerged, $nnew, $nremoved, $nconflicts, $nalerts, \@failed2register);
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
    # Optionally task ids may be prefixed with
    #   'i' for issuezilla tasks
    #   'b' for for bugtraq+ tasks
    #   'm' for mozilla bugzilla tasks
    #   'g' for gnome bugzilla tasks
    #   'e' for evolution bugzilla tasks
    # We may have to cope with a bit of white space, too.
    my $comment = shift;

    my @ids;
    my @candidates;
    if ( @candidates = ($comment =~ /#([ibmge\d\s,]+)#/gi) ) {
        foreach my $candidate (@candidates) {
            my @field = split(/,/, $candidate);
            foreach (@field) {
                tr/ //d;
                push(@ids, $_) if /^[ibmge]?\d+$/i ;
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
        if ( $status eq 'unknownfailure' || $status eq 'connectionfailure' ) {
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

    $cvs_module->verbose(1) if (! $opt_quiet);
    STDOUT->autoflush(1);
    print_message("Retrieving changes ...");
    my $changed_files_ref;
    eval { $changed_files_ref = $cvs_module->changed_files($cws_root_tag,$cws_branch_tag) };
    if ( $@ ) {
        if ( $@ =~ /server died silently/ ) {
            my $time_str = localtime();
            print_error("The CVS server has died silently on 'cvs rdiff' operation.", 0);
            print_error("Please inform Release Engineering!", 0);
            print_error("Time of failure: '$time_str'", 99);
        }
        else {
            croak($@); # rethrow
        }
    }
    STDOUT->autoflush(0);
    return $changed_files_ref;
}

sub check_for_lgpl21
# checks file for old LGPL 2.1 license header
# note: also detects even older dual license header LGPL2.1 / SISSL
{
    my $file = shift;
    my $old_license = 0;
    open( FILE, "< $file" ) or return 0;
    GREP: while (<FILE>) {
        if ( /^\W*GNU Lesser General Public License version 3/ ) {
            # correct
            last GREP;
        } elsif ( /^\W*GNU Lesser General Public License Version 2\.1/ ) {
            $old_license ++;
            last GREP;
        }
    }
    close( FILE );
    return $old_license;
}

# For repeated integrations (only to be done in case of crashes):
# clear files list, avoid integrating twice
sub clear_finished_files
{
    my $cws       = shift;
    my $module    = shift;
    my $files_ref = shift;
    my ($integrated_files_ref, %integrated_files, @remaining_files);

    $integrated_files_ref = $cws->files();
    foreach ( @$integrated_files_ref ) {
        my @filedata = split /,/, $_;
        next unless ( $filedata[0] eq $module );
        if ( $filedata[1] ) {
            $integrated_files{$filedata[1]} ++;
        }
    }
    foreach my $change_ref (@{$files_ref}) {
        push @remaining_files, $change_ref unless $integrated_files{$$change_ref[0]};
    }
    if ( my $n_in_EIS = keys %integrated_files ) {
        print_message("skipping $n_in_EIS file(s): already integrated\n");
    }
    return \@remaining_files;
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

    my $cvs_binary = $config->cvs_binary();

    foreach ( @elements ) {
        if ( ! -d $_ ) {
            my $rc = mkdir($_);
            print_error("can create directory '$_': $!", 9) unless $rc;
            # TODO use a Cvs method for this
            system("$cvs_binary add $_ > /dev/null 2>&1 ");
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
    if ( $sointernal ) {
        ($method, $vcsid, $server, $repository) = get_cvs_root($cws, $module);
    }
    else {
        # For now just take the configured OOo sever. Later we might implement a mechanism were
        # only known OOo modules are fetched from the OOo server, the rest from a local
        # server
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
    $repository =~ s/^\d*//;
    my ($dummy2, $server) = split(/@/, $user_at_server);

    if ( ! ($method && $server && $repository ) ) {
        print_error("Can't determine CVS server for module '$module'.", 0);
        return (undef, undef, undef, undef);
    }

    return ($method, $vcsid, $server, $repository);
}

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

sub auto_int
{
    my $ai_cws = shift;
    my $ai_dir = shift;
    my @ai_modules = @_;
    print "\n**** starting with automatic integration after successful analyses.****\n" if (! $opt_quiet);

    # ... more stuff
    my $conflicts = 1;
    my $alarms    = 1;
    ($conflicts,$alarms) = analyze($ai_cws, $ai_dir, @ai_modules);
    $alarms = &check_alarms($alarms) if ($alarms > 0);
    if ( $conflicts + $alarms == 0 ) {
        print "no conflicts or alarms are shown during analyses.\n" if (! $opt_quiet);
        print "starting integration...\n" if (! $opt_quiet);
        # remove directory
        if ( $ai_dir ne cwd() )
    {
            system("rm -rf $ai_dir");
            print_error("Can't remove directory '$ai_dir'", 6)  if (-e $ai_dir );
            mkdir ($ai_dir,0775);
            print_error("Can't mkdir() to '$ai_dir'", 6) if ( !-e $ai_dir );
    } else
    {
            system("rm -rf *");
    }
        integrate($ai_cws, $ai_dir, @ai_modules);
        my @final_log = &check_status_plog($ai_dir);    # can't commit changes
        if ( scalar(@final_log) > 0 ) {
            print "====================== Final Problem Log =====================\n";
            foreach my $problem(@final_log)
            {
                print "$problem\n"
            }
            print "==============================================================\n";
        } else
        {
            print "\n**************** All can't commit changes problems are resolved. ****************\n";
            print "****************        No further problems are known            ****************\n";
        }
    } else
    {
    &print_plog();
        print STDERR "Conflicts and/or alarms occured during analysis.\n";
        print STDERR "Can't continue...... Bye!\n";
    }
    exit;
}

sub prompt_manual_registration
{
    return unless ($sointernal);
    my $cws_name = shift;
    my $mws_name = shift;
    my @files = @_;
    print_warning( "Could not register all files to EIS. Please manually call");
    foreach my $file ( @files ) {
        print STDERR "\tperl \$COMMON_ENV_TOOLS/cwsaddfile.pl -c $cws_name -m $mws_name $file\n";
    }
}

sub check_alarms
{
    my $alarms = shift;
    foreach my $line(@problem_log)
    {
        if ($line =~ /def\sfile\sdetected/g)
        {
            print "ignoreing def file alarm....\n";
        $alarms--;
        }
    }
    return $alarms;
}

sub check_status_plog
{
    # cvs status file -> "up to date"? -> ok!

    my $ai_dir          = shift;
    my $problem;
    my @final_problem_log = ();

    print "Checking CVS status of files which can't be commited.....\n" if (! $opt_quiet);
    foreach $problem(@problem_log) {                        # @problem_log is global defined!
        my @result = split(":",$problem);
        if ( $result[1] =~ "can't commit changes!" ) {
            $result[0] =~ s/^\s+//;   # remove leading blanks
            my @result2 = split("/",$result[0]);
            my $module = $result2[0];             # get module name
            $result[0] =~ s/^$module\///g;        # get rest of the file path
            if (!chdir("$ai_dir/$module") ) {
                my $test = cwd();
                print STDERR "Check Status: CWD=$test\n";
                if ( $test !~ /.+$dir\/$module$/) {
                    print_error("Check Status: Can't chdir() to '$dir/$module'", 6);
                }
            }

            my $cvs_archive = get_cvs_archive($result[0]);
            my ($status, $working_rev, $repository_rev, $sticky_tag, $branch_rev,
                    $sticky_date, $sticky_options) = $cvs_archive->status();
            if ( $status =~ /Up-to-date/ ) {      # OK!
                next;
            } else
            {
                push(@final_problem_log,($problem . "    CVS Status:$status"));
            }
        } else
        {
            # conflicts and other problems
            push(@final_problem_log, $problem);
        }
    }
    return @final_problem_log;
}   ##check_status

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
    print STDERR "WARNING: $message\n";
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
        print STDERR "\t-A\trun cwsanalyze previously and integrate automatically when no conflicts or alerts are shown.\n";
        print STDERR "\t  \tCheck files with 'can't commit changes' message after integration automatically.\n";
        print STDERR "\t-z\tminimial output (quiet mode)\n";
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
