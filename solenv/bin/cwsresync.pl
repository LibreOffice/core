:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: cwsresync.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-06-26 00:23:20 $
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
# cwsresync.pl - sync child workspace modules/files with newer
#                milestones of master workspace
#

use strict;
use Cwd;
use File::Basename;
use File::Copy;
use File::Find;
use File::Glob;
use File::Path;
use Getopt::Long;
use IO::Handle;

#### module lookup

use lib ("$ENV{SOLARENV}/bin/modules");
if (defined $ENV{COMMON_ENV_TOOLS}) {
    unshift(@INC, "$ENV{COMMON_ENV_TOOLS}/modules");
};

use Cws;
eval { require Logging; import Logging; };
my $log = undef;
$log = Logging->new() if (!$@);

eval {
    require EnvHelper; import EnvHelper;
    require CopyPrj; import CopyPrj;
};
use CvsModule;
use Cvs;
use GenInfoParser;
use CwsConfig;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.2 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### hardcoded globals #####

my @xtra_files = ( "*.mk", "*.flg", "libCrun*", "libgcc*", "libstdc*", "OOoRunner.jar" );
my $platform_resynced_flag = ".cwsresync_complete";

# modules to be obligatory copied to each cws
my %obligatory_modules = ();
$obligatory_modules{'solenv'}++;
$obligatory_modules{'res'}++;
$obligatory_modules{'so_res'}++;
$obligatory_modules{'instset'}++;
$obligatory_modules{'instsetoo'}++;
$obligatory_modules{'smoketest'}++;
$obligatory_modules{'smoketest_oo'}++;
$obligatory_modules{'test10'}++;

#### global #####

my $is_debug        = 0;    # misc traces for debugging purposes
my $opt_commit      = 0;    # commit changes
my $opt_merge       = 0;    # merge from MWS into CWS
my $opt_link        = 0;    # relink modules, update solver
my $remove_trees    = 0;    # remove output trees & solver (OO option)
my %global_stats    = ();   # some overall stats
my @args_bak = @ARGV;      # store the @ARGS here for logging

my $umask = umask();
if ( !defined($umask) ) {
    $umask = 22;
}
my $force_checkout = '';

#### main #####
my $parameter_list = $log->array2string(";",@args_bak) if (defined $log);

my ($dir, $milestone, @args) = parse_options();
my $cws = get_and_verify_cws();
my @action_list = parse_args($cws, $dir, @args);
walk_action_list($cws, $dir, $milestone, @action_list);
&log_stats();
exit(0);

#### subroutines ####

sub log_stats
 {
    my $statistic_log_message;
    $statistic_log_message = "success : ";
    $statistic_log_message .= "merge mode : " if $opt_merge;
    $statistic_log_message .= "commit mode : " if $opt_commit;
    $statistic_log_message .= "link mode : " if $opt_link;
    $statistic_log_message .= "remove output trees mode : " if $remove_trees;
    $statistic_log_message .= "new: $global_stats{'new'} " if $global_stats{'new'};
    $statistic_log_message .= "removed: $global_stats{'removed'} " if $global_stats{'removed'};
    $statistic_log_message .= "merged: $global_stats{'merged'} " if $global_stats{'merged'};
    $statistic_log_message .= "moved: $global_stats{'moved'} " if $global_stats{'moved'};
    $statistic_log_message .= "conflicts: $global_stats{'conflict'} " if $global_stats{'conflict'};
    $statistic_log_message .= "alerts: $global_stats{'alert'} " if $global_stats{'alert'};
    $statistic_log_message .= "failures: $global_stats{'failure'} " if $global_stats{'failure'};
    $log->end_log_extended($script_name,"unknown",$statistic_log_message) if (defined $log);
}   ##create_log_stats

# Get child workspace from environment.
sub get_and_verify_cws
{
    my $childws  = $ENV{CWS_WORK_STAMP};
    my $masterws = $ENV{WORK_STAMP};

    if ( !defined($childws) || !defined($masterws) ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    my $cws = Cws->new();
    $cws->child($childws);
    $cws->master($masterws);
    $log->start_log_extended($script_name,$parameter_list,$masterws,$childws) if (defined $log);

    # check if we got a valid child workspace
    my $id = $cws->eis_id();
    print "Master: $masterws, Child: $childws, $id\n" if $is_debug;
    if ( !$id ) {
        print_error("Child workspace $childws for master workspace $masterws not found in EIS database.", 2);
    }
    return $cws;
}

# Parse options and do some sanity checks;
sub parse_options
{   my $dir  = 0;
    my $help = 0;
    my $success = GetOptions('h' => \$help, 'd=s' => \$dir,
                             'm=s' => \$opt_merge, 'c' => \$opt_commit, 'l=s' => \$opt_link,
                             '-a' => \$force_checkout, '-r' => \$remove_trees);
    if ( !$success || $help ) {
        usage();
        exit(1);
    }

    # some sanity checks
    if ( !($opt_merge || $opt_commit || $opt_link || $remove_trees) ) {
        print_error("Please specify one of '-m', '-c', '-l'.", 0) if defined($log);
        print_error("Please specify one of '-m', '-c', '-l', '-r'.", 0) if !defined($log);
        usage();
        exit(1);
    }

    if ( ($opt_merge || $opt_commit) && !@ARGV ) {
        usage();
        exit(1);
    }

    my $qualified_milestone = $opt_merge || $opt_link;
    if ( !check_milestone($qualified_milestone) ) {
        print_error("Invalid milestone '$qualified_milestone'.", 0);
        usage();
        exit(1);
    }

    if ( ($opt_merge && $opt_commit) ||
         ($opt_merge && $opt_link)   ||
         ($opt_commit && $opt_link) )
    {
        print_error("The options '-m milestone', '-c', '-l milestone' are mutally exclusive.", 0);
        usage();
        exit(1);
    }

    if ( $opt_link && $dir ) {
        print_error("The options '-l milestone' and '-d' are mutally exclusive.", 0);
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

    return ($dir, $qualified_milestone, @ARGV);
}

# Parse and verify args. Check that all necessary preconditions are fullfilled
# and fill the action_list
sub parse_args
{
    my $cws  = shift;
    my $dir  = shift;
    my @args = @_;

    # For each entry in the args list we'll prepare a corresponding entry
    # in the action list. An arg may be a module, a dir in a cvs module
    # or a file in a cvs module. With each arg is an action associated,
    # this may be 'resync_module_action', 'resync_dir_action', 'resync_file_action',
    # 'commit_dir_action', 'commit_file_action' or 'relink_cws_action' depending on the
    # type of the arg and the operation mode

    # The action list has the following format
    # $action_list [$i] = [$arg, $action]
    my @action_list = ();

    # The 'merge' and 'commit' modes share quite a bit of logic. The 'link' mode
    # is completely independent

    if ( (@args > 0) && ($opt_link || $remove_trees)) {
        print_error("Invalid argument.", 0);
        usage();
        exit(1);
    }
    if ( $opt_link ) {
        push(@action_list, [undef, 'relink_cws_action']);
    } elsif ($remove_trees) {
        push(@action_list, [undef, 'remove_output_trees']);
    }
    else {
        # Sanity check
        if ( @args > 1) {
            foreach (@args) {
                if ($_ eq 'all') {
                    print_error("either specify 'all' or a list of modules/files.", 1);
                }
            }
        }


        if ( is_in_module($dir) ) {
            # check arguments for inside module operation
            check_or_exit($cws, $dir, @args);
            foreach my $arg (@args) {
                if ( -f $arg ) {
                    my $action = $opt_commit ? 'commit_file_action' : 'resync_file_action';
                    push(@action_list, [$arg, $action]);
                }
                else {
                    my $action = $opt_commit ? 'commit_dir_action' : 'resync_dir_action';
                    push(@action_list, [$arg, $action]);
                }
            }
        }
        else {
            my @modules = expand_and_verify_modules($cws, @args);
            # check arguments for modules level operation
            if ( is_scratch_dir($dir) ) {
                my $action = $opt_commit ? 'commit_dir_action' : 'resync_module_action';
                foreach (@modules) {
                    push(@action_list, [$_, $action]);
                }
            }
            else {
                check_or_exit($cws, $dir, @modules);
                my $action = $opt_commit ? 'commit_dir_action' : 'resync_dir_action';
                foreach (@modules) {
                    push(@action_list, [$_, $action]);
                }
            }
        }
    }
    return @action_list;
}

# Check if all arguments are physically present on the disk
# either as files or directories
sub check_or_exit
{
    my $cws  = shift;
    my $dir  = shift;
    my @args = @_;

    my $error;

    foreach my $arg (@args) {
        if ( ! -e "$dir/$arg" ) {
            print_error("Can't find '$dir/$arg': $!", 0);
            $error++;
        }
    }
    if ( $error ) {
        print_error("Please run '$script_name -m' either in an empty scratch directory,", 0);
        print_error("or in a diretory containing all specified modules,", 0);
        print_error("or, if inside a module,", 0);
        print_error("make certain that all specified files/directories exist.", 2);
    }

    foreach my $arg (@args) {
        my $cvs_dir = "$dir/$arg";
        $cvs_dir = dirname("$cvs_dir") if -f $cvs_dir;
        if ( !check_sticky_tag($cws, $cvs_dir) ) {
            print_error("'$cvs_dir' has not the required sticky tag.", 3);
        }
    }
    return;
}

# Check if we operate inside a module.
sub is_in_module
{
    my $dir = shift;
    return -d "$dir/CVS" ? 1 : 0;
}

# Check if directory contains modules
sub is_modules_level
{
    my $dir = shift;

    return 0 if is_in_module($dir);

    my @cvsdirs = glob("$dir/*/CVS");
    return @cvsdirs ? 1 : 0;
}

# Check if directory is a scratch directory
sub is_scratch_dir
{
    my $dir = shift;
    return (is_in_module($dir) || is_modules_level($dir)) ? 0 : 1;
}

# Check if requested modules are registered with child workspace.
sub expand_and_verify_modules
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

# Process the action list
sub walk_action_list
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift;
    my @action_list         = @_;

    foreach my $entry_ref (@action_list) {
        no strict; # disable strict refs for the next call
        &{$entry_ref->[1]}($cws, $dir, $qualified_milestone, $entry_ref->[0]);
    }

    # emit some stats
    print_message(" ========== Totals: ==========") if scalar(%global_stats);
    if ( $opt_merge) {
        print_message("New file(s): $global_stats{'new'}") if $global_stats{'new'};
        print_message("Remove file(s): $global_stats{'removed'}") if $global_stats{'removed'};
        print_message("Merge file(s): $global_stats{'merged'}") if $global_stats{'merged'};
        print_message("Conflict(s): $global_stats{'conflict'}") if $global_stats{'conflict'};
        print_message("Move tag(s): $global_stats{'moved'}") if $global_stats{'moved'};
        print_message("Alert(s): $global_stats{'alert'}") if $global_stats{'alert'};
        print_message("Failure(s): $global_stats{'failure'}") if $global_stats{'failure'};
    }
    else {
        print_message("New file(s): $global_stats{'new'}") if $global_stats{'new'};
        print_message("Removed file(s): $global_stats{'removed'}") if $global_stats{'removed'};
        print_message("Merged file(s): $global_stats{'merged'}") if $global_stats{'merged'};
        print_message("Moved tag(s): $global_stats{'moved'}") if $global_stats{'moved'};
        print_message("Failure(s): $global_stats{'failure'}") if $global_stats{'failure'};
    }
    return;
}

# Merge a whole module in scratch space. Please note that merging
# a complete module is always done via resync_dir_action if it already
# exists on disk.
sub resync_module_action
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift;
    my $module              = shift;

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();
    my $milestone_tag = get_milestone_tag($cws, $qualified_milestone);
    my $cvs_module = get_cvs_module($cws, $module);
    STDOUT->autoflush(1);
    print_message("Check out module '$module' ...");
    $cvs_module->verbose(1);
    my $co_ref = $cvs_module->checkout($dir, $cws_branch_tag, '');
    STDOUT->autoflush(0);
    if ( !@{$co_ref} ) {
        print_error("Was not able to checkout module '$module',", 0);
        print_error("this might be caused by connection failures or authentication problems. That also can be caused by cvs mirror. If you recently added this module, please wait for your mirror server to syncronize", 0);
        print_error("Please check your \$HOME/.cvspass for missing entries!", 50);
    }

    my $changes_ref = get_changed_files($cvs_module, $cws_anchor_tag, $milestone_tag);

    my $save_dir = cwd();
    # chdir into module
    if ( !chdir("$dir/$module") ) {
        print_error("Can't chdir() to '$dir/$module'", 6);
    }

    my %stats;
    foreach my $file_ref (@{$changes_ref}) {
        my $rc = merge_file($cws_anchor_tag, $milestone_tag, $file_ref);
        $stats{$rc}++;
        $global_stats{$rc}++;
    }

    # chdir back
    chdir($save_dir);
    print_message(" ========== '$module' stats: ==========") if scalar(%stats);
    print_message("New file(s): $stats{'new'}") if $stats{'new'};
    print_message("Remove file(s): $stats{'removed'}") if $stats{'removed'};
    print_message("Merge file(s): $stats{'merged'}") if $stats{'merged'};
    print_message("Conflict(s): $stats{'conflict'}") if $stats{'conflict'};
    print_message("Move tag(s): $stats{'moved'}") if $stats{'moved'};
    print_message("Alert(s): $stats{'alert'}") if $stats{'alert'};
    print_message("Failure(s): $stats{'failure'}") if $stats{'failure'};
    return;
}

# resync_dir_action: not yet implemented
sub resync_dir_action
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift;
    my $cvs_dir             = shift;

    print_error("Resyncing directories is not yet supported.", 0);
    print_error("Please resync either complete modules in a scratch", 0);
    print_error("directory or resync files,", 99);

    return;
}

sub resync_file_action
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift;
    my $file                = shift;

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();
    my $milestone_tag = get_milestone_tag($cws, $qualified_milestone);
    my $cvs_archive = get_cvs_archive($file);

    my $tags_ref = $cvs_archive->get_tags();
    my $old_rev = $tags_ref->{$cws_anchor_tag};
    my $new_rev = $tags_ref->{$milestone_tag};

    # File has been removed on master
    if ( !$new_rev ) {
        $old_rev = undef; # follow the example of get_changed_files() reg. removed files
    }

    # skip files which are up to date with milestone
    if ( $old_rev eq $new_rev ) {
        print_message("\tResyncing '$file': skip (old rev. and new rev. are identical).");
        return;
    }

    my $rc = merge_file($cws_anchor_tag, $milestone_tag, [$file, $old_rev, $new_rev], $cvs_archive);
    $global_stats{$rc}++;
    return;
}

sub commit_dir_action
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift; # unused and not initialized
    my $cvs_dir             = shift;

    print_message("========== Commit changes to '$cvs_dir': ==========");

    my $save_dir = cwd();

    # chdir into module
    if ( !chdir("$dir/$cvs_dir") ) {
        print_error("Can't chdir() to '$cvs_dir'", 6);
    }

    local @main::changed_files;
    find(\&wanted, '.');

    my %stats;
    my $rc;
    foreach (@main::changed_files) {
        $rc = commit_file($cws, $_);
        $stats{$rc}++;
        $global_stats{$rc}++;
    }
    # chdir back
    chdir($save_dir);

    print_message(" ========== '$cvs_dir' stats: ==========") if scalar(%stats);
    print_message("New file(s): $stats{'new'}") if $stats{'new'};
    print_message("Remove file(s): $stats{'removed'}") if $stats{'removed'};
    print_message("Merge file(s): $stats{'merged'}") if $stats{'merged'};
    print_message("Move tag(s): $stats{'moved'}") if $stats{'moved'};
    print_message("Failure(s): $stats{'failure'}") if $stats{'failure'};

    return;
}


sub commit_file_action
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift; # unused and not initialized
    my $file                = shift;

    my $save_dir = cwd();
    # chdir into the reference dir
    if ( !chdir("$dir") ) {
        print_error("Can't chdir() to '$dir'", 6);
    }
    my $rc = commit_file($cws, $file);
    $global_stats{$rc}++;
    # chdir back
    chdir($save_dir);
    return;
}

# TODO: put into module later... see also cwsadd.pl
sub get_mws_location
{
    use GenInfoParser;

    my $masterws = shift;
    my $result = 0;

    my $workspace_lst = EnvHelper::get_workspace_lst();
    my $workspace_db = GenInfoParser->new();
    $result = $workspace_db->load_list($workspace_lst);
    if ( !$result ) {
        print_message("Can't load workspace list '$workspace_lst'.");
        return "";
    }
    my $workspace = $workspace_db->get_key($masterws);
    print_error("Master workspace '$masterws' not found in '$workspace_lst' database.", 3) if ( !$workspace );

    my $wslocation = $workspace_db->get_value($masterws."/Drives/o:/UnixVolume");
    print_error("Location of master workspace '$masterws' not found in '$workspace_lst' database.", 3) if ( !defined($wslocation) ) ;

    if (! -d $wslocation) {
        print_message("Master workspace not found!") ;
        return "";
    }
    return $wslocation;
}

sub remove_module
{
    my $module_p = shift;

    my $result = 0;

    if ( -l $module_p ) {
        print "unlink module $module_p\n" if $is_debug;
        $result |= ! unlink $module_p;
    }
    if ( -l  $module_p.".lnk" ) {
        print "unlink module $module_p.lnk\n" if $is_debug;
        $result |= ! unlink "$module_p.lnk";
    }
    if ( -d $module_p ) {
        print "rm -rf $module_p\n" if $is_debug;
        $result |= system("rm -rf $module_p");

    } elsif ( -e $module_p ) {
        print "no idea what this is... $module_p\n" if $is_debug;
        print_error("Couldn't remove $module_p\[.lnk\]. Giving up.", 1);
    }
    return $result;
}

#
# Procedure copies module to specified path
#
sub copyprj_module
{

    my $module_name = shift;
    my $src_dest = shift;
    print "copyprj $module_name\n";

    # hash, that should contain all the
    # data needed by CopyPrj module
    my %ENVHASH = ();
    my %projects_to_copy = ();
    $ENVHASH{'projects_hash'} = \%projects_to_copy;
    $ENVHASH{'no_otree'} = 1;
    $ENVHASH{'no_path'} = 1;
    $ENVHASH{'only_otree'} = 0;
    $ENVHASH{'only_update'} = 1;
    $ENVHASH{'last_minor'} = 0;
    $ENVHASH{'spec_src'} = 0;
    $ENVHASH{'dest'} = "$src_dest";
    $ENVHASH{'prj_to_copy'} = '';
    $ENVHASH{'i_server'} = '';
    $ENVHASH{'current_dir'} = cwd();
    $ENVHASH{'remote'} = '';
    $ENVHASH{'force_checkout'} = 1 if ($force_checkout);

    $projects_to_copy{$module_name}++;

    CopyPrj::copy_projects(\%ENVHASH);

};

sub get_platforms_hash {
    my $cws = shift;
    my $minor_mk_name = $ENV{UPD} . 'minor.mk';
    my @files;
    my %platforms = ();
    find sub { push @files, $File::Find::name if -f _ && /$minor_mk_name/ }, $ENV{SOLARVER};
    foreach my $entry (@files) {
        do {
            $entry = dirname($entry);
        } while ($entry =~ /[\\\/]inc[\.\w+]?$/o);
        $platforms{basename($entry)}++;
    };
    $platforms{$ENV{INPATH}}++;
    return %platforms;
}

#
# remove all output trees
#
sub remove_output_trees {
    my $cws = shift;
    my %platforms = get_platforms_hash($cws);
    my $stand_dir = $ENV{SRC_ROOT};
    if (!opendir(SOURCE_ROOT, $stand_dir)) {
        print_error ("Root dir of child workspace not accessible: $!", 1);
    };
    my @found_dirs = readdir(SOURCE_ROOT);
    closedir SOURCE_ROOT;
    my $module_dir = '';
    my @warnings = ();
    foreach my $module (@found_dirs) {
        $module_dir = "$stand_dir/$module";
        next if (!-d $module_dir);
        print_message("Removing output trees from $module...");
        foreach my $plarform (keys %platforms) {
            my $output_tree = "$module_dir/$plarform";
            next if (!-d $output_tree);
            if (!rmtree($output_tree, 0, 1)) {
                push(@warnings, $output_tree);
            };
        }
    };
    print_message("Removing solver ...");
    foreach my $platform (keys %platforms) {
        my $output_tree = $ENV{SOLARVERSION} . "/$platform";
        next if (!-d $output_tree);
        if (!rmtree($output_tree, 0, 1)) {
            push(@warnings, $output_tree);
        };
    };
    print_warning("'$_' cannot be deleted. Please remove it manually") foreach(@warnings);
    print_message("Please run configure & bootstrap again\n") if !defined($log);
};

#
# Update the sources to the correspondent master tag
#
sub update_sources {
    my ($cws, $new_master, $milestone) = @_;
    my @added_modules = $cws->modules();
    my %added_modules_hash = ();
    my ($master_branch_tag, $cws_branch_tag, $cws_root_tag) = $cws->get_tags($new_master, $milestone);
    my $master_milestone_tag = $cws->get_master_tag($new_master, $milestone);
    my $stand_dir = $ENV{SRC_ROOT};
    $added_modules_hash{$_}++ foreach (@added_modules);
    if (!opendir(SOURCE_ROOT, $stand_dir)) {
        print_error("Root dir of child ($stand_dir) workspace not accessible: $!", 1);
    };
    my @found_dirs = readdir(SOURCE_ROOT);
    closedir SOURCE_ROOT;
    my $cvs_module = CvsModule->new();
    my %cvs_aliases = $cvs_module->get_aliases_hash();
    my $result = '';
    foreach my $module (@found_dirs) {
        next if (!-d $stand_dir . "/$module/CVS");
        print "\tUpdating '$module'";
        $cvs_module->module($module);
        if (defined $added_modules_hash{$module}) {
            print " with '$cws_branch_tag' ...\n";
            $result = $cvs_module->update($stand_dir, $cws_branch_tag, '-dP');
            delete $added_modules_hash{$module};
        } elsif (defined $cvs_aliases{$module}) {
            print " with $master_milestone_tag' ...\n";
            $result = $cvs_module->update($stand_dir, $master_milestone_tag, '-dP');
        } else {
            print_warning("... Unknown module. Skipping...");
            next;
        };
        $cvs_module->handle_update_infomation($result);
    };
    register_cws_milestone($cws, $new_master, $milestone);
    return;
};

sub register_cws_milestone {
    my ($cws, $new_master, $milestone) = @_;
    my $push_return = '';
    if ( $cws->master() ne $new_master ) {
        $push_return = $cws->master($new_master);
        if ($push_return ne $new_master) {
            print_error("Couldn't push new milestone to database", 1);
        };
    }
    $push_return = $cws->milestone( $milestone );
    if ( $push_return ne $milestone ) {
        print_error("Couldn't push new milestone to database", 1);
    }
    print_message("Current milestone of CWS updated to '$milestone'.");
    print_message("Remove the old and most likely incompatible module output trees and solver with:");
    print_message("\tcwsresync -r");


}

# Implements the link action: relink all modules and update solver
sub relink_cws_action
{
    my $cws                 = shift;
    my $dir                 = shift;  # ignore, this is never set
    my $qualified_milestone = shift;

    my $cws_master = $cws->master();

    my ($new_master, $milestone);
    if ( $qualified_milestone =~ /:/ ) {
        ($new_master, $milestone) = split(/:/, $qualified_milestone);
    }
    else {
        $new_master = $cws_master;
        $milestone  = $qualified_milestone;
    }

    # ause: Deine Spielwiese

    no warnings;
    my $result = 0;
    my $success = 0;
    my $sourceroot;
    my $mws_location;
    my $mws_accessible = 0;
    my $dest_dir;
    my @found_platforms;
    my @opt_platforms;
    my $found_resync_flags = 0;

    print_message("Doing some checks ...");

    # hack to get the mileston :-(((((
    if ( ! defined($cws->milestone()))
    {
        $cws->milestone($ENV{UPDMINOR});# = $ENV{UPDMINOR};
    }

    # milestone is different from the current milestone
    if ( "$new_master" eq "$cws_master" ) {
        print_error("Child workspace \"".$cws->child()."\" already based on milestone \"$milestone\"", 1) if $cws->milestone() eq $milestone;
    }
    return update_sources($cws, $new_master, $milestone) if (!defined $log); # HACK

    # SOURCE_ROOT set correct
    print_error("Environment variable \"SOURCE_ROOT\" not set.", 1) if ! defined($ENV{SOURCE_ROOT});
    $sourceroot = $ENV{SOURCE_ROOT};
    print_error("Environment variable \"SOURCE_ROOT\" pointing to something incorrect.", 1) if ! -d "$sourceroot/".$cws_master."/src.".$cws->milestone();

    # desired milestone doesn't exist in cws dir or mws changes
    if ( "$new_master" eq "$cws_master" ) {
        print_error("Looks like there is already a milestone \"$milestone\" in $sourceroot/".$cws_master.".", 1) if  -d "$sourceroot/".$cws_master."/src.$milestone";

        my @checklist = glob( "$sourceroot/$cws_master/*/inc.$milestone/*.mk" );
        print_message("Looks like there is already a milestone \"$milestone\" in $sourceroot/".$cws_master." output trees.") if $#checklist != -1;
    }

    # mws filesystem accessible
    $mws_location = get_mws_location($new_master);
    if ( "$mws_location" ne "" ) {
        # our OS isn't windows
        if ( $^O =~ "MSWin32" )
        {
            print_error("Sorry! not for windows",2);
        }
        $mws_accessible = 1;
    } else {
        print_message("Trying without access to master workspace.");
        $mws_accessible = 0;
    }

    #check if sourceroot points to mws location
    print "$sourceroot <-> $mws_location\n" if $is_debug;
    my $mws_location_string = $mws_location;
    if ( $mws_location_string =~ /\/net/ ) {
        my @tmplst = split /\//, $mws_location;
        print "list @tmplst\n" if $is_debug;
        shift @tmplst;
        shift @tmplst;
        shift @tmplst;
        print "list @tmplst\n" if $is_debug;
        $mws_location_string = "/".join '/', @tmplst;
    }
    print "$sourceroot <-> $mws_location_string\n" if $is_debug;
    if ( $sourceroot =~ /$mws_location_string/ ) {
        print_error ("Root dir of child workspace and master directory are too similar\n$sourceroot <-> *$mws_location_string", 1)
    }

    print_message("Updating solver.");
    if ( $mws_accessible )
    {
        require sync_dir; import sync_dir;
        #TODO: check for complete mws milestone

        print_message("Removing previous solver...");
        # find all existing output trees in cws
        $result = opendir( SOLVER, "$sourceroot/".$cws_master);
        if ( !$result ){ print_error ("Root dir of child workspace not accessible: $!", 1) };
        my @found_dirs = readdir( SOLVER );
        closedir( SOLVER );

        foreach my $dir_candidate ( @found_dirs )
        {
            #remove . and ..
            next if ( $dir_candidate eq "." || $dir_candidate eq ".." );
            # check for the remains of previous tries...
            if ( -d "$sourceroot/$cws_master/$dir_candidate/inc.$milestone") {
                # check if there is a complete tree
                if ( -f "$sourceroot/$cws_master/$dir_candidate/inc.$milestone/$platform_resynced_flag" ) {
                    $found_resync_flags++;
                    next;
                }
                # try to remove rubbish...
                $result = system("rm -rf $sourceroot/$cws_master/$dir_candidate/*.$milestone");
                if ( $result ) {
                    print_error( "Couldn't cleanup \"$sourceroot/$cws_master/$dir_candidate/*.$milestone\". Please do manually!", 1 );
                }
                # ...and fake the old minor.
                $result = mkpath( "$sourceroot/".$cws_master."/$dir_candidate/inc.".$cws->milestone(), 0, 0777-$umask);
            }
            if ( -d "$sourceroot/".$cws_master."/$dir_candidate/inc.".$cws->milestone() )
            {
                push @found_platforms, $dir_candidate;
            }
        }
        if ( !@found_platforms )
        {
            print_message("No output trees to remove");
            $success = 0;
        }

        # remove them
        $result = 0;
        foreach my $platform ( @found_platforms ) {
            next if ( -f "$sourceroot/$cws_master/$platform/inc.$milestone/$platform_resynced_flag" );
            print_message("Removing $platform");
            if ( -d "$sourceroot/$cws_master/$platform") {
                # make sure it's recognized when restarting
                my $tmp_result = mkpath( "$sourceroot/$cws_master/$platform/inc.$milestone", 0, 0777-$umask);
                $result |= system("rm -rf $sourceroot/$cws_master/$platform/*.".$cws->milestone());
            } else {
                print_error("\"$sourceroot/$cws_master/$platform\" isn't a directory,", 0);
                print_error("trying to rename...", 0);
                $result |= system("mv $sourceroot/$cws_master/$platform $sourceroot/$cws_master/$platform.renamed");
            }
        }

        print_error("Couldn\'t remove existing solver on child workspace.", 1) if $result;

        # find all existing output trees in mws or use list
        # no opt list yet - take the trees that were use before resync
        @opt_platforms = @found_platforms;
        @found_platforms = ();
        # find platforms to copy
        if ( $#opt_platforms != -1 || $found_resync_flags > 0 ) {
            @found_platforms = map( lc, @opt_platforms );
        } else {
            print_error("this is an implementaion bug!", 1);
        }
        # copy all wanted output trees
        $sync_dir::do_keepzip = 1;
        my $btarget = "instset";
        foreach my $platform ( @found_platforms )
        {
            # don't copy tree that was already successfull
            next if ( -f "$sourceroot/$cws_master/$platform/inc.$milestone/$platform_resynced_flag" );
            %sync_dir::done_hash = ();
            print "Create copy of solver for $platform ( ~ 1GB disk space needed !)\n";
            my $zipsource = "$mws_location/".$new_master."/$platform/zip.$milestone";
            my $copy_dest = "$sourceroot/".$cws_master."/$platform/zip.$milestone";
            if ( -d "$sourceroot/".$cws_master."/$platform" )
            {
            #    print_error ("$dir/$platform : Please restart on a clean directory tree!", 1);
            }
            if ( ! -d $copy_dest )
            {
                $result = mkpath($copy_dest, 0, 0777-$umask);
                if ( !$result ){ print_error ("Cannot create output tree $copy_dest : $!", 1) };
            }

            my $unzip_dest = $copy_dest;
            $unzip_dest =~ s/(.*)\/.*$/$1/;

            if ( ! -e "$unzip_dest/prepared" ) {
                $result = sync_dir::prepare_minor_unzip( $unzip_dest, ".".$milestone );
                open( PREPARED, ">$unzip_dest/prepared");
                close( PREPARED );
            }

            STDOUT->autoflush(1);
            $result = &sync_dir::recurse_unzip( $zipsource, $copy_dest, $btarget );
            STDOUT->autoflush(0);
            if ( $result )
            {
                # renaming back before exit
                $result = sync_dir::finish_minor_unzip( $unzip_dest, ".".$milestone );
                print_error ("Copying files to $copy_dest failed : $!", 1);
            }
            $result = sync_dir::finish_minor_unzip( $unzip_dest, ".".$milestone );
            unlink "$unzip_dest/prepared.$milestone" if -e "$unzip_dest/prepared.$milestone";
            open( COMPLETE, ">$sourceroot/$cws_master/$platform/inc.$milestone/$platform_resynced_flag");
            close( COMPLETE );

        }
        foreach my $oneextra ( @xtra_files )
        {
            my @globlist = glob( "$mws_location/".$new_master."/[!s]*/*.$milestone/$oneextra" );
            if ( $#globlist == -1 ) {
                print "tried $oneextra in $mws_location/".$new_master."/[!s]*/*.$milestone/$oneextra\n";
            }
            foreach my $onefile ( @globlist )
            {
                my $destfile = $onefile;
                my $m_dir = "$mws_location/".$new_master;
                my $c_dir = "$sourceroot/".$cws_master;
                $destfile =~ s#$m_dir#$c_dir#;

                if ( -d dirname( $destfile ))
                {
                    $result = copy( $onefile,  $destfile);
                    if ( !$result ){ print_error ("Copying $onefile to CWS failed: $!", 1) };
                }
            }
        }

    } else {
        print_error("solver tree resync without mws access not yet implemented", 5);
    }
    print_message("Recreating CWS source tree with MWS milestone '$milestone'.");
    if ( $mws_accessible ) {
        # all but added modules
        my @added_modules = $cws->modules();
        my %modules_hash =();
        print "debug: added modules: @added_modules\n" if $is_debug;
        $result = opendir( SOURCE, "$mws_location/".$new_master."/src.".$milestone);
        if ( !$result ){ print_error ("Source dir of master workspace not accessible: $!", 1) };
        my @mws_found_modules = readdir( SOURCE );
        closedir( SOURCE );
        foreach my $module (@mws_found_modules) { $modules_hash{$module}++; }
        delete $modules_hash{"."};
        delete $modules_hash{".."};
        foreach my $module (@added_modules) { delete $modules_hash{$module}; }

        if ( !scalar(keys(%modules_hash)) )
        {
            print_error("No valid source tree to copy", 0);
            $success = 0;
        }
        print "debug: number of modules left: ".scalar(keys(%modules_hash))."\n" if $is_debug;
        # now remove them
        print_message("Removing old modules");
        $dest_dir = "$sourceroot/".$cws_master."/src.".$cws->milestone();
        $result = 0;

        # don't touch solenv if added
        if ( defined $modules_hash{"solenv"} ) {
            # backup "solenv" for restarting...
            $result = system("mv $dest_dir/solenv $dest_dir/solenv.keep");
            print_error("Couldn't backup \"solenv\". You better check your tree...", 1) if $result;
        }

        foreach my $module ( keys %modules_hash ) {
            my $module_path = $dest_dir."/$module";
            $result |= remove_module( $module_path );
        }

        print_error("Couldn\'t cleanup source tree. Please check.", 1) if ( $result );

        # copy instset, res, solenv,  link all missing
        # preparing pseudo environment for copyprj
        $ENV{SRC_ROOT}="$mws_location/".$new_master."/src.$milestone";

        $success = 1;
        foreach my $module ( keys %modules_hash ) {
            # copy modules which are required to be accessable with their
            # orginal name without .lnk extension
            if (defined $obligatory_modules{$module}) {
               &copyprj_module($module, $dest_dir);
               next ;
            };
            if ( -d "$mws_location/".$new_master."/src.$milestone/$module" ) {
                if ( -l "$dest_dir/$module.lnk" &&
                    readlink( "$dest_dir/$module.lnk" ) eq "$mws_location/".$new_master."/src.$milestone/$module" )
                {
                    next;
                } else {
                # better...
                    $result = symlink( "$mws_location/".$new_master."/src.$milestone/$module", "$dest_dir/$module.lnk");
                }
                if ( !$result ) {
                    print_error ( "Couldn't create link from $mws_location/".$new_master."/src.$milestone/$module to $dest_dir/$module", 0);
                    $success = 0;
                }
            }
        }
        if ( defined $modules_hash{"solenv"} ) {
            my $solenv_path = $dest_dir."/solenv.keep";
            $result |= remove_module( $solenv_path );
        }

    } else {
        print_error("source tree resync without mws access not yet implemented", 5);
        # remove all but added modules
        # checkout all missing
    }

    # rename src.* directory
    rename $dest_dir, "$sourceroot/".$cws_master."/src.$milestone";

    # TODO: check if return equals $milestone
    my $push_return = $cws->milestone( $milestone );
    if ( $push_return ne $milestone ) {
        print_error("Couldn't push new milestone to database");
    } else {
        print_message("Successfully pushed new milestone to database");
    }

    # master changed?
    if ( $cws_master ne $new_master ) {
        # push new master if different
        my $push_return = $cws->master( $new_master );
        if ( $push_return ne $new_master ) {
            print_error("Couldn't push new milestone to database");
        }

        # rename WORKSPACE directory if different
        chdir ($sourceroot) if ( cwd() eq "$sourceroot/cws_master" );
        rename "$sourceroot/$cws_master", "$sourceroot/$new_master";
        chdir ("$sourceroot/$new_master") if ( cwd() eq "$sourceroot" );
    } else {
        print_message("Successfully pushed new master to database");
    }

    # resync done. now remove all $platform_resynced_flag
    my @completelist = glob( "$sourceroot/$new_master/*/inc.$milestone/$platform_resynced_flag" );
    unlink( @completelist );
    return;
}

# Low level merge file routine:
# Merge changes on master copy into the child copy of the file.
# Parameter $cvs_archive is optional and can be used to pass already available
# CVS archive objects to this routine. If not set it will create a new CVS
# archive object from the file named in $file_ref[0].
#
# Note: The parameters are somewhat redundant.
# We do need both, the tags and the revisions which corresponds to the tags as
# parameter to this method. The tags are needed for proper CVS operation
# (think of added and removed files), the corresponding revisions are needed for
# informational purposes. It would be possible to look them up on a file by file
# basis via the tags but this is quite expensive. Considers this an optimization
# hack
sub merge_file
{
    my $cws_anchor_tag = shift;
    my $milestone_tag  = shift;
    my $file_ref       = shift;
    my $cvs_archive    = shift;

    my $file = $file_ref->[0];
    my $old_rev = $file_ref->[1];
    my $new_rev = $file_ref->[2];

    my ($new_file, $removed_file);
    print "\tResyncing '$file' ";
    if ( !$old_rev && !$new_rev ) {
        print "remove file: ";
        $removed_file++;
    }
    elsif ( !$old_rev ) {
        print "($new_rev) new file: ";
        $new_file++;
    }
    else {
        print "($old_rev-$new_rev): ";
    }

    # Initialize CVS archive object if not passed to routine
    $cvs_archive = get_cvs_archive($file) if !$cvs_archive;

    my ($status, $working_rev, $repository_rev, $sticky_tag, $branch_rev,
                $sticky_date, $sticky_options);

    # A fresh CVS checkout with a branch label is always 'pruned'. Sanitize
    # CVS hierarchy so that we can write out our .resync-files in any case.
    sanitize_cvs_hierarchy($file);
    if ( $new_file ) {
        # Check if file has been added on two CWSs, once in this CWS
        # and once in an already integrated CWS.
        if (  -e $file ) {
            # We alert the user and skip the file.
            print "file has been added independently in MWS and CWS. Skipping. Please check!.\n";
            return 'alert';
        }
        else {
            write_resync_comment($file, 'new', undef, $new_rev);
            print "added, schedule move tag.\n";
            return 'new';
        }
    }
    elsif ( $removed_file ) {
        my $rc = $cvs_archive->update("-j$cws_anchor_tag -j$milestone_tag");
        if ( $rc eq 'success' ) {
            print "removed, schedule commit.\n";
            write_resync_comment($file, 'removed', undef, undef);
            return 'removed';
        }
        else {
            print "failure!\n";
            print_error("INTERNAL ERROR: can't resync file.", 0);
            return 'failure';
        }
    }
    else{
        # Get status of file to be merged
        ($status, $working_rev, $repository_rev, $sticky_tag, $branch_rev,
                $sticky_date, $sticky_options) = $cvs_archive->status();
        if ( $status eq 'Up-to-date' && ! -e $file ) {
            # Special case: file has been removed in CWS but there
            # were changes between old and new MWS milestones.
            # Resolution: Do nothing, skip this file. The file
            # remains being removed on this CWS.
            print "removed in CWS, but changes in MWS are pending. Please check!.\n";
            return 'alert';
        }
        if ( $status eq 'unkownfailure' ) {
            print_error("can't get status of '$file': $status", 0);
            return 'failure';
        }
        if ( $status eq 'Locally Modified' ) {
            print_error("Can't merge locally modified file!", 0);
            return 'failure';
        }
    }

    # Check if we can get by by just moving the branch and the anchor tag
    if ( !$removed_file && !$new_file && defined($branch_rev)
                        && $branch_rev =~ /$working_rev\.\d+/ ) {
        write_resync_comment($file, 'moved', $old_rev, $new_rev);
        print "schedule move tag.\n";
        return 'moved';
    }

    my $rc;
    my $success;
    if ( $sticky_options eq 'kb' ) {
        # We got changes pending in the MWS and we
        # have changes in the CWS. Since we can't merge
        # binary files we give up here. Sure, we could
        # either favor the MWS or the CWS version, but
        # there is no way to decide which one is better.
        # We alert the user and skip the file.
        if ( 1 ) { # TODO check for file mode with explicit version selection
            print "binary file has been changed in CWS and in MWS. Skipping. Please check!.\n";
            return 'alert';
        }
        else {
            # TODO implement explicit retrieval of version if in file mode and
            # user requested either the CWS or MWS version
            $rc = $cvs_archive->update("-j$cws_anchor_tag -j$milestone_tag");
            if ( $rc ne 'success' ) {
                print "failure!\n";
                print_error("INTERNAL ERROR: can't resync binary file.", 0);
                return 'failure';
            }
            write_resync_comment($file, 'binary', undef, $new_rev);
            print "binary, taking $new_rev.\n";
            $success = 'binary';
        }
    }
    else {
        # option -kk needed for clean merge in source files
        $rc = $cvs_archive->update("-kk -j$cws_anchor_tag -j$milestone_tag");
        if ( $rc eq 'success' ) {
            print "merged, schedule commit.\n";
            $success = 'merged'
        }
        elsif ( $rc eq 'conflict' ) {
            print "conflict, schedule commit after resolution.\n";
            $success = 'conflict';
        }
        else {
            print "failure!\n";
            print_error("INTERNAL ERROR: can't resync file.", 0);
            return 'failure';
        }
        write_resync_comment($file, 'merged', $old_rev, $new_rev);
    }
    return $success;
}

# Low level commit file routine:
# Commits file to the childworkspace or move tags
# Requires a valid .resync file next to the CVS file
sub commit_file
{
    my $cws  = shift;
    my $file = shift;

    if ( !open(CHECKIN, "<$file.resync" ) ) {
        print_error("can't open $file.resync: $!", 0);
        return 'failure';
    }
    my @resync_comment = <CHECKIN>;
    close(CHECKIN);

    my ($type, $old_rev, $new_rev);
    if ( $resync_comment[0] =~ /^RESYNC (\w+) ([\w\.]+) ([\w\.]+)$/ ) {
        $type = lc($1);
        $old_rev = $2;
        $new_rev = $3;
    }
    else {
        print_error("$file.resync has an invalid format", 0);
        return 'failure';
    }

    if ( $type eq 'moved' || $type eq 'new') {
        # just move the tags, no cvs->commit()
        print "\tCommit '$file': move tag: ";
        my $rc = move_tags($cws, $file, $new_rev);
        if ( $rc ) {
            print_error("can't unlink $file.resync: $!.", 0) unless unlink("$file.resync");
        }
        return $rc ? 'moved' : 'failure';
    }
    else {
        # cvs->commit()
        my $comment;
        if  ( $type eq 'merged' ) {
            $comment = "RESYNC: ($old_rev-$new_rev); FILE MERGED\n";
        }
        elsif  ( $type eq 'binary' ) {
            $comment = "RESYNC: ($new_rev); BINARY\n";
        }
        elsif  ( $type eq 'removed' ) {
            $comment = "RESYNC:; FILE REMOVED\n";
        }
        else {
            # can't happen
            print_error("internal_error commit_file(): unkown type: $type", 0);
        }

        # prepare commit comment
        shift(@resync_comment); shift(@resync_comment);
        unshift(@resync_comment, $comment);

        print "\tCommit '$file': ";
        my $rc = ci_file($cws, $file, $new_rev, \@resync_comment);
        if ( $rc ) {
            print_error("can't unlink $file.resync: $!.", 0) unless unlink("$file.resync");
        }
        return $rc ? $type : 'failure';
    }

    return 'failure'; # should never be reached
}

# Move CWS tags to new revision.
sub move_tags
{
    my $cws     = shift;
    my $file    = shift;
    my $new_rev = shift;

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();
    my $cvs_archive = get_cvs_archive($file);

    my $rc = $cvs_archive->update("-r$new_rev");
    if ( $rc ne 'success' ) {
        print_error("updating '$file' to new revision '$new_rev' failed.", 0);
        return 0;
    }

    $rc = $cvs_archive->tag($cws_branch_tag, '-F -b');
    if ( $rc ne 'success' ) {
        print_error("Tagging '$file': tag operation returned: '$rc'.", 0);
        print "failed!\n";
        return 0;
    }
    $rc = $cvs_archive->tag($cws_anchor_tag, '-F');
    if ( $rc ne 'success' ) {
        print "failed!\n";
        print_error("Tagging '$file': tag operation returned: '$rc'.", 0);
        return 0;
    }

    $rc = $cvs_archive->update("-r$cws_branch_tag");
    if ( $rc ne 'success' ) {
        print "failed!\n";
        print_error("updating '$file' to '$cws_branch_tag' failed.", 0);
        return 0;
    }
    print "OK.\n";
    return 1;
}

sub ci_file
{
    my $cws         = shift;
    my $file        = shift;
    my $new_rev     = shift;
    my $comment_ref = shift;

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();

    my $cvs_archive = get_cvs_archive($file);

    my ($rc, $rev);
    my $skip_commit = 0;
    if ( $new_rev eq 'none' && ! -e $file ) {
        # check if file has been locally removed
        my $status = $cvs_archive->status();
        if ( $status eq 'Up-to-date' ) {
            # Ok, file has been removed on MWS and it has been also
            # removed on the CWS. A commit will fail in this case
            # so we don't bother.
            $rev = 'nothing to remove';
            $skip_commit = 1;
        }
    }

    if ( !$skip_commit ) {
        # comments may be huge, use a tempfile instead of passing
        # them via the command line to the cvs client
        if ( !open(COMMIT, ">$file.comment") ) {
            print_error("can't open file '$file.comment'", 7);
        }
        print COMMIT @{$comment_ref};
        close(COMMIT);
        ($rc, $rev) = $cvs_archive->commit("-F $file.comment");
        if ( !($rc eq 'success' || $rc eq 'nothingcommitted') ) { # nothingcommitted valid here
            print "failed!\n";
            print_error("can't commit file '$file': $rc", 0);
            return 0;
        }
        print_error("can't unlink $file.comment: $!.", 0) unless unlink("$file.comment");
        $rev = 'nothing to commit' if $rc eq 'nothingcommitted';
    }

    if ( $new_rev eq 'none' ) {
        # Uh oh, file has been removed in master workspace.
        # There is no easy way to find out in which revision
        # exactly the file has been removed, but we know that
        # it must be the top level revision of the master branch.
        # In this case we can set the Anchor tag to the revision
        # which corresponds to the head of the master branch.
        $new_rev = $master_branch_tag ? $master_branch_tag : 'HEAD';
    }

    # tag with the anchor tag
    $rc = $cvs_archive->tag("-F -r$new_rev $cws_anchor_tag");
    if ( $rc ne 'success' ) {
        print "failed!\n";
        print_error("Tagging '$file': tag operation returned: '$rc'.", 0);
        return 0;
    }
    print "$rev: OK.\n";
    return 1;
}

sub write_resync_comment
{
    my $file    = shift;
    my $type    = shift;
    my $old_rev = shift || 'none';
    my $new_rev = shift || 'none';

    if ( !open(RESYNC_COMMENT, ">$file.resync") ) {
        print_error("can't open file '$file.resync'", 7);
    }
    my $uctype = uc($type);
    print RESYNC_COMMENT "RESYNC $uctype $old_rev $new_rev\n";
    if ( $type ne 'moved' ) {
        print RESYNC_COMMENT "Everything below this line will be added to the revision comment.\n";
    }
    close(RESYNC_COMMENT);
}

# Check if the CVS subdir is available or
# add it to the local CVS tree
sub sanitize_cvs_hierarchy
{
    my $file = shift;

    my $cvs_dir = dirname($file);
    return if $cvs_dir eq '.'; # no need to check current dir
    return if -d $cvs_dir;     # directory exists, nothing to do

    my @elements = split(/\//, $cvs_dir);

    my $save_dir = cwd();

    foreach ( @elements ) {
        if ( ! -d $_ ) {
            my $rc = mkdir($_);
            print_error("can create directory '$_': $!", 9) unless $rc;
            # TODO use a Cvs method for this
            my $config = CwsConfig::get_config();
            my $cvs_binary = $config->cvs_binary();
            system("$cvs_binary add $_ > /dev/null 2>&1 ");
        }
        if ( !chdir($_) ) {
            print_error("Can't chdir() to '$_'", 9);
        }
    }

    # chdir back
    chdir($save_dir);
}

sub wanted {
    my $file = $_;

    my ($name, $path, $suffix) = fileparse($file,'\.\w+') if -f $file;

    if ( defined($suffix) && $suffix eq '.resync' ) {
        my $cvs_file;
        if ( length($File::Find::dir) > 2 ) {
            my $dir = substr($File::Find::dir, 2);
            $cvs_file = "$dir/$name";
        }
        else {
            $cvs_file = $name;
        }
        push(@main::changed_files, $cvs_file);
    }
}

sub check_sticky_tag
{
    my $cws     = shift;
    my $cvs_dir = shift;

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();
    if ( !open(CVSTAG, "<$cvs_dir/CVS/Tag") ) {
        print_error("'$cvs_dir': can't determine sticky tag.", 0);
        return 0;
    }
    my @lines = <CVSTAG>;
    close(CVSTAG);

    if ( $lines[0] =~ /^T$cws_branch_tag/ ) {
        return 1;
    }
    else {
        print_error("'$cvs_dir': wrong sticky tag, need '$cws_branch_tag'", 0);
        return 0;
    }
}

# some simple checks on the plausibility of the specified milestone
sub check_milestone
{
    # TODO needs more checks based on EIS
    my $qualified_milestone = shift;

    return 0 if $qualified_milestone =~ /-/;
    if ( $qualified_milestone =~ /:/ ) {
        my ($master, $milestone) = split(/:/, $qualified_milestone);
        return 0 unless ( $master && $milestone );
    }
    return 1;
}

# Returns milestone tag determined from the
# full qualified milestone name.
sub get_milestone_tag
{
    my $cws                 = shift;
    my $qualified_milestone = shift;

    if ( $qualified_milestone =~ /:/ ) {
        my ($master, $milestone) = split(/:/, $qualified_milestone);
        return uc($master) . "_$milestone";
    }
    else {
        return $cws->master() . "_$milestone";
    }
}

# Returns changed files
sub get_changed_files
{
    my $cvs_module = shift;
    my $old_tag    = shift;
    my $new_tag    = shift;

    $cvs_module->verbose(1);
    STDOUT->autoflush(1);
    print_message("Retrieving changes ...");
    my $changed_files_ref = $cvs_module->changed_files($old_tag, $new_tag);
    STDOUT->autoflush(0);
    return $changed_files_ref;
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
    my $cws    = shift;
    my $module = shift;

    my $master = $cws->master();

    my $vcsid = $ENV{VCSID};
    if ( !$vcsid ) {
        print_error("Can't determine VCSID. Please use setsolar.", 5);
    }

    my $workspace_lst = EnvHelper::get_workspace_lst();
    my $workspace_db = GenInfoParser->new();
    my $success = $workspace_db->load_list($workspace_lst);
    if ( !$success ) {
        print_error("Can't load workspace list '$workspace_lst'.", 4);
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


sub print_message
{
    my $message     = shift;

    print "$script_name: ";
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
        $log->end_log_extended($script_name,"unknown",$message) if (defined $log);
        exit($error_code);
    }
    return;
}

sub usage
{
    print STDERR "Usage:\n";
    print STDERR "cwsresync [-h] [-a] [-d dir] -m <milest.> <all|mod.|dir|file> [mod.|dir|file ...]\n";
    print STDERR "cwsresync [-h] [-d dir] -r|-c <all|module|dir|file> [module|dir|file ...]\n";
    print STDERR "cwsresync [-h] -l <milestone>\n";
    print STDERR "Synchronize child workspace mod./dirs/files ";
    print STDERR "with the latest master workspace changes \n";
    print STDERR "Options:\n";
    print STDERR "\t-h\t\thelp\n";
    print STDERR "\t-d dir\t\toperate in directory dir\n";
    print STDERR "\t-m milestone\tmerge changes from MWS into CWS\n";
    print STDERR "\t-c\t\tcommit the merged files to CWS\n";
    print STDERR "\t-l milestone\trenew solver, relink modules to new milestone\n" if defined($log);
    print STDERR "\t-l milestone\tregister new milestone with database\n" if !defined($log);
    print STDERR "\t-r\t\tremove solver and module output trees, update milestone information\n" if !defined($log);
    print STDERR "\t-a\t\tuse cvs checkout instead of copying\n" if defined($log);
    print STDERR "Notes:\n";
    print STDERR "\tA Milestone on a different MWS can be specified as <MWS:milestone>.\n";
    print STDERR "Examples:\n";
    print STDERR "\tcwsresync -m SRX645:m1 all \n";
    print STDERR "\tcwsresync -c all \n";
    print STDERR "\tcwsresync -l SRX645:m1 \n" if defined($log);
    print STDERR "\tcwsresync -r\n" if !defined($log);
}
