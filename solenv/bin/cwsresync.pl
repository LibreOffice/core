:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: cwsresync.pl,v $
#
#   $Revision: 1.34 $
#
#   last change: $Author: rt $ $Date: 2008-02-18 09:12:35 $
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
use Getopt::Long qw(:config no_ignore_case);
use IO::Handle;
use IO::File;
use Carp;

#### module lookup
my @lib_dirs;
BEGIN {
    if ( !defined($ENV{SOLARENV}) ) {
        die "No environment found (environment variable SOLARENV is undefined)";
    }
    push(@lib_dirs, "$ENV{SOLARENV}/bin/modules");
    push(@lib_dirs, "$ENV{SOLARENV}/bin/modules/PCVSLib/lib");
    push(@lib_dirs, "$ENV{COMMON_ENV_TOOLS}/modules") if defined($ENV{COMMON_ENV_TOOLS});
}
use lib (@lib_dirs);

use Cws;
eval { require Logging; import Logging; };
my $log = undef;
$log = Logging->new() if (!$@);

eval {
    require EnvHelper; import EnvHelper;
    require CopyPrj; import CopyPrj;
};
use Cvs;
use GenInfoParser;
use CwsConfig;
use CwsCvsOps;
use CvsModule; # to be removed ASAP

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.34 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### hardcoded globals #####

my @xtra_files = ( "*.mk", "*.flg", "libCrun*", "libCstd*", "libgcc*", "libstdc*" );
my $platform_resynced_flag = ".cwsresync_complete";

# ignore these files silently during resync
my %resync_silent_ignore_files = ( 'wntmsci3' => 1,
                                   'wntmsci7' => 1,
                                   'wntmsci8' => 1 );

# ignore these files silently during resync
my %resync_always_move_tags = ( 'wntmsci10' => 1 );

# modules to be obligatory copied to each cws
my %obligatory_modules = (  'solenv' => 1,
                            'default_images' => 1,
                            'custom_images' => 1,
                            'ooo_custom_images' => 1,
                            'external_images' => 1,
                            'postprocess' => 1,
                            'instset_native' => 1,
                            'instsetoo_native' => 1,
                            'smoketest_native' => 1,
                            'smoketestoo_native' => 1);

#### global #####

my $opt_debug            = 0;    # misc traces for debugging purposes
my $opt_commit           = 0;    # commit changes
my $opt_merge            = 0;    # merge from MWS into CWS
my $opt_link             = 0;    # relink modules, update solver
my $opt_remove_trees     = 0;    # remove output trees & solver (OO option)
my $opt_complete_modules = 0;    # checkout complete modules for merge ops
my @args_bak = @ARGV;            # store the @ARGS here for logging

my @problem_log = ();

my $umask = umask();
if ( !defined($umask) ) {
    $umask = 22;
}
my $opt_force_checkout = '';
my $opt_skip_update = 0;

# some overall stats
my %global_stats = ('new'       => 0,
                    'removed'   => 0,
                    'merged'    => 0,
                    'moved'     => 0,
                    'anchor'    => 0,
                    'conflict'  => 0,
                    'alert'     => 0,
                    'ignored'   => 0
                   );

#### main #####
my $parameter_list = $log->array2string(";",@args_bak) if (defined $log);

my ($dir, $milestone, @args) = parse_options();
my $cws = get_and_verify_cws();
if ( $milestone ) {
    verify_milestone_or_exit($cws, $milestone);
}
my @action_list = parse_args($cws, $dir, $milestone, @args);
walk_action_list($cws, $dir, $milestone, @action_list);
print_plog();
log_stats();
exit(0);

#### subroutines ####

sub log_stats
 {
    my $statistic_log_message;
    $statistic_log_message = "success : ";
    $statistic_log_message .= "merge mode : " if $opt_merge;
    $statistic_log_message .= "commit mode : " if $opt_commit;
    $statistic_log_message .= "link mode : " if $opt_link;
    $statistic_log_message .= "remove output trees mode : " if $opt_remove_trees;
    $statistic_log_message .= "new: $global_stats{'new'} " if $global_stats{'new'};
    $statistic_log_message .= "removed: $global_stats{'removed'} " if $global_stats{'removed'};
    $statistic_log_message .= "merged: $global_stats{'merged'} " if $global_stats{'merged'};
    $statistic_log_message .= "moved: $global_stats{'moved'} " if $global_stats{'moved'};
    $statistic_log_message .= "anchor: $global_stats{'anchor'} " if $global_stats{'anchor'};
    $statistic_log_message .= "conflicts: $global_stats{'conflict'} " if $global_stats{'conflict'};
    $statistic_log_message .= "alerts: $global_stats{'alert'} " if $global_stats{'alert'};
    $statistic_log_message .= "ignored: $global_stats{'ignored'} " if $global_stats{'ignored'};
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
    print "Master: $masterws, Child: $childws, $id\n" if $opt_debug;
    if ( !$id ) {
        print_error("Child workspace $childws for master workspace $masterws not found in EIS database.", 2);
    }
    return $cws;
}

# Parse options and do some sanity checks;
sub parse_options
{   my $dir  = 0;
    my $help = 0;
    my $success = GetOptions('h' => \$help, 'd=s' => \$dir, 'debug' => \$opt_debug,
                             'm=s' => \$opt_merge, 'c' => \$opt_commit, 'l=s' => \$opt_link,
                             'a' => \$opt_force_checkout, 'r' => \$opt_remove_trees,
                             'f' => \$opt_skip_update, 'F' => \$opt_complete_modules );
    if ( !$success || $help ) {
        usage();
        exit(1);
    }

    # some sanity checks
    if ( !($opt_merge || $opt_commit || $opt_link || $opt_remove_trees) ) {
        print_error("Please specify one of '-m', '-c', '-l'.", 0) if defined($log);
        print_error("Please specify one of '-m', '-c', '-l', '-r'.", 0) if !defined($log);
        usage();
        exit(1);
    }

    if ( ($opt_merge || $opt_commit) && !@ARGV ) {
        usage();
        exit(1);
    }

    my $milestone = $opt_merge || $opt_link;
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

    if ( $milestone eq 'HEAD' && $opt_link ) {
        print_error("'HEAD' is not a valid milestone for '-l milestone' option.", 0);
        usage();
        exit(1);
    }

    if ( $opt_complete_modules && !$opt_merge ) {
        print_error("option '-F' requires merge step (option '-m milestone' must be set).", 0);
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

    return ($dir, $milestone, @ARGV);
}

# Parse and verify args. Check that all necessary preconditions are fulfilled
# and fill the action_list
sub parse_args
{
    my $cws       = shift;
    my $dir       = shift;
    my $milestone = shift;
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

    if ( (@args > 0) && ($opt_link || $opt_remove_trees)) {
        print_error("Invalid argument.", 0);
        usage();
        exit(1);
    }
    if ( $opt_link ) {
        push(@action_list, [undef, 'relink_cws_action']);
    } elsif ($opt_remove_trees) {
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

        # If HEAD is specified as milestone, make sure that every argument is really a file.
        if ( $milestone eq 'HEAD' ) {
            foreach ( @args ) {
                if ( ! -f $_ ) {
                    print_error("Only files are allowed as argument for resyncing against HEAD", 0);
                    print_error("'$_' is not a file: $!", 1);
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
        print_error("or in a directory containing all specified modules,", 0);
        print_error("or, if inside a module,", 0);
        print_error("make certain that all specified files/directories exist.", 2);
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

    if ( !$opt_link ) {
        # emit some stats
        print_message(" ========== Totals: ==========") if scalar(%global_stats);
        print_message("New file(s)        : $global_stats{'new'}") if $global_stats{'new'};
        print_message("Remove file(s)     : $global_stats{'removed'}") if $global_stats{'removed'};
        print_message("Merge file(s)      : $global_stats{'merged'}") if $global_stats{'merged'};
        print_message("Conflict(s)        : $global_stats{'conflict'}") if $global_stats{'conflict'};
        print_message("Move branch tag(s) : $global_stats{'moved'}") if $global_stats{'moved'};
        print_message("Move anchor(s)     : $global_stats{'anchor'}") if $global_stats{'anchor'};
        print_message("Alert(s)           : $global_stats{'alert'}") if $global_stats{'alert'};
        print_message("Ignored            : $global_stats{'ignored'}") if $global_stats{'ignored'};
    }
    return;
}

# Merge a whole module in scratch space.
sub resync_module_action
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift;
    my $module              = shift;

    print_message("");
    print_message(" ========== Merging '$module' ==========");
    print_message("");

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();
    my $milestone_tag = get_milestone_tag($cws, $qualified_milestone);
    my $cvs_handle = get_cvs_handle($cws, 'config', $module);
    if ( !defined($cvs_handle) ) {
        print_warning("Skipping '$module' ...");
        print_warning("This might happen if '$module' is obsolete.");
        return;
    }
    print_message("Retrieving changes for '$module' ...");
    my $changes_ref = $cvs_handle->get_changed_files($module, $cws_anchor_tag, $milestone_tag);

    my @files_to_checkout;
    foreach (@{$changes_ref}) {
        push(@files_to_checkout, $_->[0]);
    }

    my $nfiles_to_checkout = @files_to_checkout;
    if ( $opt_complete_modules ) {
        print_message("Check out module '$module' ...");
        $cvs_handle->checkout($dir, $module, undef, $cws_branch_tag);
    }
    elsif ( $nfiles_to_checkout > 0 ) {
        print_message("Check out $nfiles_to_checkout file(s) from module '$module' ...");
        $cvs_handle->checkout($dir, $module, \@files_to_checkout, $cws_branch_tag);
    }
    else {
        print_message("nothing to do for module '$module'");
        # still create a directory even if nothing is to be done, for the benefit of the
        # commit operation
        if ( !mkdir("$dir/$module") ) {
            print_error("Can't mkdir() '$dir/$module': $!", 6);
        }
        if ( !open(README, ">$dir/$module/readme") ) {
            print_error("can't open file '$dir/$module/readme': $!", 7);
        }
        print README "Nothing to be resynced here\n";
        close(README);
        return;
    }

    my $save_dir = cwd();
    # chdir into module
    if ( !chdir("$dir/$module") ) {
        print_error("Can't chdir() to '$dir/$module': $!", 6);
    }

    my $stats_ref = merge_files($cws_anchor_tag, $milestone_tag, $changes_ref, $cvs_handle, $module);

    # chdir back
    chdir($save_dir);

    $global_stats{'new'} += $stats_ref->{'new'};
    $global_stats{'removed'} += $stats_ref->{'removed'};
    $global_stats{'merged'} += $stats_ref->{'merged'};
    $global_stats{'moved'} += $stats_ref->{'moved'};
    $global_stats{'anchor'} += $stats_ref->{'anchor'};
    $global_stats{'conflict'} += $stats_ref->{'conflict'};
    $global_stats{'alert'} += $stats_ref->{'alert'};
    $global_stats{'ignored'} += $stats_ref->{'ignored'};
    print_message(" ---------- '$module' stats:-----------") if scalar(%{$stats_ref});
    print_message("New file(s)    : $stats_ref->{'new'}") if $stats_ref->{'new'};
    print_message("Remove file(s) : $stats_ref->{'removed'}") if $stats_ref->{'removed'};
    print_message("Merge file(s)  : $stats_ref->{'merged'}") if $stats_ref->{'merged'};
    print_message("Conflict(s)    : $stats_ref->{'conflict'}") if $stats_ref->{'conflict'};
    print_message("Move tag(s)    : $stats_ref->{'moved'}") if $stats_ref->{'moved'};
    print_message("Move anchor(s) : $stats_ref->{'anchor'}") if $stats_ref->{'anchor'};
    print_message("Alert(s)       : $stats_ref->{'alert'}") if $stats_ref->{'alert'};
    print_message("Ignored file(s): $stats_ref->{'ignored'}") if $stats_ref->{'ignored'};
    return;
}

# resync_dir_action: not possible
sub resync_dir_action
{
    my $cws                 = shift;
    my $dir                 = shift;
    my $qualified_milestone = shift;
    my $cvs_dir             = shift;

    print_error("Resyncing directories is not possible.", 0);
    print_error("Please resync either complete modules in a scratch", 0);
    print_error("directory or resync a list of files,", 99);

    return;
}

sub resync_file_action
{
    my $cws                 = shift;
    my $dir                 = shift;  # unused, remove me
    my $qualified_milestone = shift;
    my $file                = shift;

    if ( exists $resync_silent_ignore_files{basename($file)} ) {
        print_error("Can't resync special file '$file'!", 33);
    }

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();
    my $milestone_tag;
    if ( $qualified_milestone eq 'HEAD' ) {
        $milestone_tag = $master_branch_tag ? $master_branch_tag : 'HEAD';
    }
    else {
        $milestone_tag = get_milestone_tag($cws, $qualified_milestone);
    }

    my $directory = dirname($file);
    my $cvs_handle = get_cvs_handle($cws, 'directory', $directory);
    if ( !defined($cvs_handle) ) {
        print_message("resync_file_action(): can't get cvs_handle");
    }

    my $rep = $cvs_handle->get_relative_path($directory);

    print_message("Retrieving changes for '$file' ...");
    my $changes_ref = $cvs_handle->get_changed_files("$rep/$file", $cws_anchor_tag, $milestone_tag);

    if ( !@{$changes_ref} ) {
        print_warning("'$file' already on resync level '$milestone_tag'\n");
        exit(0);
    }

    # changes_ref contains the path to the file relative to the module (including module prefix)
    # replace with local path
    $changes_ref->[0]->[0] = $file;

    my $stats_ref = merge_files($cws_anchor_tag, $milestone_tag, $changes_ref, $cvs_handle, '');

    $global_stats{'new'} += $stats_ref->{'new'};
    $global_stats{'removed'} += $stats_ref->{'removed'};
    $global_stats{'merged'} += $stats_ref->{'merged'};
    $global_stats{'moved'} += $stats_ref->{'moved'};
    $global_stats{'anchor'} += $stats_ref->{'anchor'};
    print_message("New file(s): $stats_ref->{'new'}") if $stats_ref->{'new'};
    print_message("Removed file(s): $stats_ref->{'removed'}") if $stats_ref->{'removed'};
    print_message("Commit file(s): $stats_ref->{'merged'}") if $stats_ref->{'merged'};
    print_message("Move branch tag(s): $stats_ref->{'moved'}") if $stats_ref->{'moved'};
    print_message("Move anchor tags(s): $stats_ref->{'anchor'}") if $stats_ref->{'anchor'};
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
        print_warning("Can't chdir() to '$cvs_dir'.");
        print_warning("Skipping '$cvs_dir'.");
        return;
    }

    local @main::changed_files;
    find(\&wanted, '.');

    if ( @main::changed_files ) {
        my $stats_ref = commit_files($cws, \@main::changed_files, $cvs_dir);

        $global_stats{'merged'} += $stats_ref->{'merged'};
        $global_stats{'moved'} += $stats_ref->{'moved'};
        $global_stats{'anchor'} += $stats_ref->{'anchor'};
        print_message(" ========== '$cvs_dir' stats: ==========") if scalar(%{$stats_ref});
        print_message("Commit file(s)     : $stats_ref->{'merged'}") if $stats_ref->{'merged'};
        print_message("Move branch tag(s) : $stats_ref->{'moved'}") if $stats_ref->{'moved'};
        print_message("Move anchor tags(s): $stats_ref->{'anchor'}") if $stats_ref->{'anchor'};
    }

    chdir($save_dir);
    return;
}


sub commit_file_action
{
    my $cws                 = shift;
    my $dir                 = shift; # ununsed, remove me
    my $qualified_milestone = shift; # unused and not initialized
    my $file                = shift;

    my $cvs_dir = dirname($file);

    if ( ! -e "$file.resync" ) {
        print_error("Can't find '$file.resync'. Please use the 'cwsresync -m' step on file '$file' first.", 55)
    }

    my $stats_ref = commit_files($cws, [$file], $cvs_dir);

    $global_stats{'merged'} += $stats_ref->{'merged'};
    $global_stats{'moved'} += $stats_ref->{'moved'};
    $global_stats{'anchor'} += $stats_ref->{'anchor'};
    print_message("Commit file(s)     : $stats_ref->{'merged'}") if $stats_ref->{'merged'};
    print_message("Move branch tag(s) : $stats_ref->{'moved'}") if $stats_ref->{'moved'};
    print_message("Move anchor tags(s): $stats_ref->{'anchor'}") if $stats_ref->{'anchor'};

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
        print_message("Master workspace not found at '$wslocation'!") ;
        return "";
    }
    return $wslocation;
}

sub remove_module
{
    my $module_p = shift;

    my $result = 0;

    if ( -l $module_p ) {
        print "unlink module $module_p\n" if $opt_debug;
        $result |= ! unlink $module_p;
    }
    if ( -l  $module_p.".lnk" ) {
        print "unlink module $module_p.lnk\n" if $opt_debug;
        $result |= ! unlink "$module_p.lnk";
    }
    if ( -d $module_p ) {
        print "rm -rf $module_p\n" if $opt_debug;
        $result |= system("rm -rf $module_p");

    } elsif ( -e $module_p ) {
        print "no idea what this is... $module_p\n" if $opt_debug;
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
    $ENVHASH{'force_checkout'} = 1 if ($opt_force_checkout);
    $ENVHASH{RESYNC_UPDMINOR} = $opt_link if (defined $opt_link);

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

    my @found_dirs = ();
    my $cvs_module;
    my %cvs_aliases;
    if ($opt_skip_update) {
        print "Skipping main tree update\n";
    }
    else {
        @found_dirs = readdir(SOURCE_ROOT);
        closedir(SOURCE_ROOT);
        # TODO clean this incredible mess up
        $cvs_module = get_cvs_module('solenv'); # solenv always on OOo CVS server
        %cvs_aliases = $cvs_module->get_aliases_hash();
    }

    foreach my $module (@found_dirs) {
        next if (!-d $stand_dir . "/$module/CVS");
        print "\tUpdating '$module'";
        $cvs_module->module($module);
        my $result ='';
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
        $cvs_module->handle_update_information($result);
    };
    register_cws_milestone($cws, $new_master, $milestone);
    return;
};

sub register_cws_milestone {
    my ($cws, $new_master, $milestone) = @_;
    if ( $cws->master() ne $new_master ) {
        my @push_return = $cws->set_master_and_milestone($new_master, $milestone);
        if ( ($push_return[0] ne $new_master) || ($push_return[1] ne $milestone) ) {
            print_error("Couldn't push new master and milestone to database", 1);
        };
    } else {
        my $push_return = '';
        $push_return = $cws->milestone( $milestone );
        if ( $push_return ne $milestone ) {
            print_error("Couldn't push new milestone to database", 1);
        }
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

    # hack to get the milestone :-(((((
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
        if ( $^O =~ "MSWin32" || $^O =~ "cygwin" ) {
            print_error("Sorry! not for windows",2);
        }
        $mws_accessible = 1;
    } else {
        print_message("Trying without access to master workspace.");
        $mws_accessible = 0;
    }

    #check if sourceroot points to mws location
    print "$sourceroot <-> $mws_location\n" if $opt_debug;
    my $mws_location_string = $mws_location;
    if ( $mws_location_string =~ /\/net/ ) {
        my @tmplst = split /\//, $mws_location;
        print "list @tmplst\n" if $opt_debug;
        shift @tmplst;
        shift @tmplst;
        shift @tmplst;
        print "list @tmplst\n" if $opt_debug;
        $mws_location_string = "/".join '/', @tmplst;
    }
    print "$sourceroot <-> $mws_location_string\n" if $opt_debug;
    if ( $sourceroot =~ /$mws_location_string/ ) {
        print_error ("Root dir of child workspace and master directory are too similar\n$sourceroot <-> *$mws_location_string", 1)
    }

    print_message("Updating solver.");
    my @added_modules = $cws->modules();
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
        sync_dir::set_excludelist( \@added_modules ); # omit added modules
        sync_dir::addto_excludelist(["instset_native", "instsetoo_native"]); # omit instset* modules
        my $btarget = "finalize";
        foreach my $platform ( @found_platforms )
        {
            # don't copy tree that was already successful
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

            # cleanup: remove zip files
            print "remove zip.$milestone\n";
            $result = system("rm -rf $copy_dest");
            if ( $result ) {
                print_warning ("Could not clean up zip file directory 'copy_dest'");
            }

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
                    # preserve timestamp
                    my @from_stat = stat($onefile);
                    utime($from_stat[9], $from_stat[9], $destfile);
                }
            }
        }

    } else {
        print_error("solver tree resync without mws access not yet implemented", 5);
    }
    print_message("Recreating CWS source tree with MWS milestone '$milestone'.");
    if ( $mws_accessible ) {
        # all but added modules
        my %modules_hash =();
        print "debug: added modules: @added_modules\n" if $opt_debug;
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
        print "debug: number of modules left: ".scalar(keys(%modules_hash))."\n" if $opt_debug;
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

    # master changed?
    if ( $cws_master ne $new_master ) {
        # push new master if different
        my @push_return = $cws->set_master_and_milestone($new_master, $milestone);
        if ( ($push_return[0] ne $new_master) || ($push_return[1] ne $milestone) ) {
            print_error("Couldn't push new master and milestone to database");
        } else {
            print_message("Successfully pushed new master and milestone to database");
        }

        # rename WORKSPACE directory if different
        chdir ($sourceroot) if ( cwd() eq "$sourceroot/cws_master" );
        rename "$sourceroot/$cws_master", "$sourceroot/$new_master";
        chdir ("$sourceroot/$new_master") if ( cwd() eq "$sourceroot" );
    } else {
        my $push_return = $cws->milestone( $milestone );
        if ( $push_return ne $milestone ) {
            print_error("Couldn't push new milestone to database");
        } else {
            print_message("Successfully pushed new milestone to database");
        }
    }

    # resync done. now remove all $platform_resynced_flag
    my @completelist = glob( "$sourceroot/$new_master/*/inc.$milestone/$platform_resynced_flag" );
    unlink( @completelist );
    return;
}

# Low level merge file routine:
# Merge changes on master copy into the child copy of the file.
sub merge_files
{
    my $cws_anchor_tag    = shift;
    my $milestone_tag     = shift;
    my $changes_ref       = shift;
    my $cvs_handle        = shift;
    my $module            = shift; # needed for proper logging

    my @new_files;
    my @removed_files;
    my @move_tags_files;
    my @merge_candidates;
    my %module_stats = ('new'       => 0,
                        'removed'   => 0,
                        'merged'    => 0,
                        'moved'     => 0,
                        'anchor'    => 0,
                        'conflict'  => 0,
                        'alert'     => 0,
                        'ignored'   => 0
                       );

    foreach ( @${changes_ref} ) {
        my $file = $_->[0];
        my $old_rev = $_->[1];
        my $new_rev = $_->[2];

        if ( exists $resync_silent_ignore_files{basename($file)} ) {
            # skip it without any comment at all
            next;
        }

        # sort by type of merge action
        if ( !$old_rev && !$new_rev ) {
            push(@removed_files, $file);
        }
        elsif ( !$old_rev ) {
            push(@new_files, [$file, $new_rev]);
        }
        else {
            push(@merge_candidates, [$file, $old_rev, $new_rev]);
        }
    }

    # Handle files which have recently been added on the MWS.
    # For new files we don't need to check the status of the
    # file in the CWS, just do a simple existence check.
    my $n_new_files = @new_files;
    if ( $n_new_files ) {
        print "  ... processing $n_new_files new file(s) ...\n";
        foreach ( @new_files ) {
            my $file    = $_->[0];
            my $new_rev = $_->[1];
            # Check if file has been added on two CWSs, once in this CWS
            # and once in an already integrated CWS.
            if (  -e $file ) {
                # We alert the user and skip the file.
                print "\tA\t$file: has been added independently in MWS and CWS. Skipping. Please check!\n";
                plog("A\t$module/$file: has been added independently in MWS and CWS. Skipping. Please check!");
                $module_stats{'alert'}++;
                $module_stats{'skipped'}++;
                next;
            }
            write_resync_comment($file, 'new', $milestone_tag, undef, $new_rev);
            print "\tN\t$file: added, schedule move tag\n";
            $module_stats{'new'}++;
        }
    }

    # Handle files which have recently been removed on the MWS.
    # We check the status of these files in the CWS to be able to warn
    # if a file which to be removed which has been changed in the MWS
    my $n_removed_files = @removed_files;
    if ( $n_removed_files ) {
        print "  ... processing $n_removed_files removed file(s) ...\n";
        my $stati_ref = $cvs_handle->stati(\@removed_files);
        # sanity check
        my $n_stati_ref = @{$stati_ref};
        if ( $n_stati_ref != $n_removed_files ) {
            print_error("INTERNAL ERROR: can't fetch the status for all to be removed files", 8);
        }
        my @rfiles;
        for (my $i = 0; $i < $n_removed_files; $i++) {
            my $file        = $removed_files[$i];
            my $working_rev = $stati_ref->[$i][2];
            my $branch_rev  = $stati_ref->[$i][4];
            if ( !defined($working_rev) ) {
                # File has already been removed on CWS, ignore.
                print "\tI\t$file: file already removed on CWS, ignored.\n";
                $module_stats{'ignored'}++;
                next;
            }
            if ( $working_rev =~ /$branch_rev\.\d+/ ) {
                # Example: working rev 1.5.260.1, branch rev 1.5.260
                print "\tA\t$file: has been removed on MWS but is changed on CWS. Will remove file. Please check!\n";
                plog("A\t$module/$file: has been removed on MWS but is changed on CWS. Will remove file. Please check!");
                # Note: we still procceed with removing this file. Thus this file is counted
                # as alerted and removed.
                $module_stats{'alert'}++;
            }
            push(@rfiles, $file);
        }
        my $n_rfiles = @rfiles;
        if ( $n_rfiles ) {
             $cvs_handle->remove_files(\@rfiles);
            foreach my $file ( @rfiles ) {
                write_resync_comment($file, 'removed', $milestone_tag, undef, undef, );
                print "\tR\t$file: removed, schedule commit\n";
                $module_stats{'removed'}++;
            }
        }
    }

    # Handle files in the merge list. Fetch status first to check if we get
    # away with "move tag". We also need to check for binary files and
    # files which have been removed in the CWS.
    my $n_merge_candidates = @merge_candidates;
    if ( $n_merge_candidates ) {
        print "  ... processing $n_merge_candidates merge candidate(s) ...\n";
        my @stati_lists = ();
        foreach (@merge_candidates) {
            push(@stati_lists, $_->[0]);
        }
        my $stati_ref = $cvs_handle->stati(\@stati_lists);
        # sanity check
        my $n_stati_ref = @{$stati_ref};
        if ( $n_stati_ref != $n_merge_candidates ) {
            print_error("INTERNAL ERROR: can't fetch the status for all to be merged files", 8);
        }
        my @mfiles;
        foreach (my $i = 0; $i < $n_merge_candidates; $i++) {
            my $file        = $merge_candidates[$i][0];
            my $old_rev     = $merge_candidates[$i][1];
            my $new_rev     = $merge_candidates[$i][2];
            my $status      = $stati_ref->[$i][1];
            my $working_rev = $stati_ref->[$i][2];
            my $branch_rev  = $stati_ref->[$i][4];
            my $is_binary   = (defined($stati_ref->[$i][5]) && $stati_ref->[$i][5] =~ /kb/) ? 1 : 0;

            if ( ($status eq 'Up-to-date' || $status eq 'Needs Checkout') && !defined($working_rev) ) {
                # Special case: file has been removed in CWS but there
                # were changes between old and new MWS milestones.
                # Resolution: Do nothing, skip this file. The file
                # remains being removed on this CWS.
                print "\tA\t$file: has been removed in CWS, but changes in MWS are pending. Skipping. Please check!\n";
                plog("A\t$module/$file: has been removed in CWS, but changes in MWS are pending. Skipping. Please check!");
                $module_stats{'alert'}++;
                $module_stats{'skipped'}++;
                next;
            }
            # Sanity check
            if ( !defined($branch_rev) || !defined($working_rev) ) {
                print_error("Internal Error: merge_files(): branch_rev or working_rev undefined $file\n",77);
            }
            if ( exists $resync_always_move_tags{basename($file)} ) {
                # Incredible HACK, always move both tags to the milestone for these files,
                # REMOVE_ME: as soon we get rid of defs files on trunk this should be removed
                write_resync_comment($file, 'moved',  $milestone_tag, $old_rev, $new_rev);
                print "\tT\t$file: schedule move tag.\n";
                $module_stats{'moved'}++;
                next;
            }
            if ( $branch_rev =~ /$working_rev\.\d+/ ) {
                # Example: branch rev 1.5.260, working rev 1.5
                # Joy, we get away with just a "move tag".
                write_resync_comment($file, 'moved',  $milestone_tag, $old_rev, $new_rev);
                print "\tT\t$file: schedule move tag.\n";
                $module_stats{'moved'}++;
                next;
            }
            if ( $is_binary ) {
                # We got changes pending in the MWS and we have changes in the CWS.
                # Since we can't merge binary files we give up here. Sure, we could
                # either favor the MWS or the CWS version, but there is no way to decide
                # which one is better. We alert the user and skip the file.
                print "\tA\t$file: binary file has been changed in CWS and in MWS. Skipping. Please check!\n";
                plog("A\t$file: binary file has been changed in CWS and in MWS. Skipping. Please check!");
                $module_stats{'alert'}++;
                $module_stats{'skipped'}++;
                next;
            }
            push (@mfiles, [$file, $old_rev, $new_rev]);
        }

        my $n_mfiles = @mfiles;
        if ( $n_mfiles ) {
            print "  ... do $n_mfiles actual merge(s) ...\n";

            # the merge_files() API wants a reference to a plain list
            my @to_be_merged_files;
            foreach (@mfiles) {
                push(@to_be_merged_files, $_->[0]);
            }

            my ($conflicts_ref, $already_merged_ref)
                        = $cvs_handle->merge_files(\@to_be_merged_files, $cws_anchor_tag, $milestone_tag);

            # for easier searching
            my %conflicts_hash;
            foreach (@{$conflicts_ref}) {
                $conflicts_hash{$_}++;
            }
            my %already_merged_hash;
            foreach (@{$already_merged_ref}) {
                $already_merged_hash{$_}++;
            }

            foreach (@mfiles) {
                my $file    = $_->[0];
                my $old_rev = $_->[1];
                my $new_rev = $_->[2];
                if ( exists $already_merged_hash{$file} ) {
                    print "\tT\t$file: contains differences, schedule move anchor\n";
                    $module_stats{'anchor'}++;
                    write_resync_comment($file, 'anchor', $milestone_tag, $old_rev, $new_rev);
                }
                elsif ( exists $conflicts_hash{$file} ) {
                    print "\tC\t$file: conflict, schedule commit after resolution.\n";
                    plog("C\t$module/$file: conflict, schedule commit after resolution.");
                    write_resync_comment($file, 'merged', $milestone_tag, $old_rev, $new_rev);
                    $module_stats{'conflict'}++;
                }
                else {
                    print "\tM\t$file: merged, schedule commit.\n";
                    $module_stats{'merged'}++;
                    write_resync_comment($file, 'merged', $milestone_tag, $old_rev, $new_rev);
                }
            }
        }
    }
    return \%module_stats;
}

# Low level commit_files routine:
# Commits files to the childworkspace or move tags.
# Requires a valid .resync files next to the CVS files
sub commit_files
{
    my $cws       = shift;
    my $files_ref = shift;
    my $cvs_dir   = shift;

    my %module_stats = (
                        'merged' => 0, # aka commit
                        'moved' => 0,
                        'anchor' => 0
                       );
    my @ci_files;
    my @remove_files;
    my @move_both_tags_files;
    my @move_anchor_tag_files;
    my @move_anchor_to_master_head_files;
    my @possible_conflict_files;

    my $milestone_tag;
    my $master_head;

    # collect all *.resync files
    foreach (@{$files_ref}) {
        if ( !open(CHECKIN, "<$_.resync" ) ) {
            print_error("can't open $_.resync: $!", 7);
        }
        my @resync_comment = <CHECKIN>;
        close(CHECKIN);

        my $type_line = shift(@resync_comment);

        my ($type, $old_rev, $tag, $new_rev);
        if ( $type_line =~ /^RESYNC (\w+) (\w+) ([\w\.]+) ([\w\.]+)/ ) {
            $type    = lc($1);
            $tag     = $2;
            $old_rev = $3;
            $new_rev = $4;
        }
        else {
            print_error("$_.resync has an invalid format", 8);
            return 'failure';
        }

        # shift the line with "Everything below this line will be added to the revision comment."
        shift(@resync_comment);

        # sanity checks to guard against a mix of resyncs targets
        if ( $type ne 'dead' && defined($milestone_tag) && $tag ne $milestone_tag) {
            print_error("detected more than one resync milestone tag: '$milestone_tag' and '$tag'", 9);
        }
        if ( $type eq 'dead' && defined($master_head) && $tag ne $master_head ) {
            print_error("detected more than one resync at master tags: '$master_head and '$tag'", 9);
        }

        # set target
        if ( $type ne 'dead' && !defined($milestone_tag) ) {
            $milestone_tag = $tag;
        }
        if ( $type eq 'dead' && !defined($master_head) ) {
            $master_head = $tag;
        }

        if ( $type eq 'moved' || $type eq 'new') {
            # just move both tags
            push(@move_both_tags_files, $_);
        }
        elsif ( $type eq 'anchor' ) {
            # just move the anchor tag
            push(@move_anchor_tag_files, $_);
        }
        elsif ( $type eq 'merged' ) {
            # commit and move anchor tags
            push(@ci_files, [$_, $type, $old_rev, $new_rev, \@resync_comment]);
            # these files need to be checked for conflict markers
            push(@possible_conflict_files, $_);
        }
        elsif ( $type eq 'removed') {
            # remove, commit and move anchor tags
            push(@remove_files, $_);
            push(@ci_files, [$_, $type, $old_rev, $new_rev, \@resync_comment]);
        }
        elsif ( $type eq 'dead' ) {
            # can occur here only if there has been an interruption
            # between the commit of removed files and the move
            # of the anchor tag to the master head
            push(@move_anchor_to_master_head_files, $_);
        }
    }


    check_for_conflict_markers(\@possible_conflict_files);

    my $cvs_handle = get_cvs_handle($cws, 'directory', $cvs_dir);

    my $n_remove_files = @remove_files;
    if ( $n_remove_files ) {
        print "  ... preparing removal of $n_remove_files file(s) ...\n";
        my $n = unlink(@remove_files);
        if ( $n != $n_remove_files) {
            print_error("can't unlink() files scheduled for removal.", 22);
        }
        $cvs_handle->remove_files(\@remove_files);
    }

    # commit files
    # we do single commits, because the comments are differing for each file

    my $n_ci_files = @ci_files;

    my ($master_branch_tag, $cws_branch_tag, $cws_anchor_tag) = $cws->get_tags();

    if ( !defined($master_head) ) {
        # Determine new master from milestone_tag. We can't use $master_branch_tag
        # here because after resyncing we might have a new master due to
        # a cross master resync and the CWS itself is not yet updated
        # to the new master.
        my ($master) = split(/_/, $milestone_tag);
        my $new_master_branch_tag = $cws->get_master_branch_tag($master);
        $master_head = $new_master_branch_tag ? $new_master_branch_tag : 'HEAD';
    }

    if ( $n_ci_files ) {
        print "  ... commit $n_ci_files file(s) ...\n";
    }
    foreach (@ci_files) {
        my ($file, $type, $old_rev, $new_rev, $resync_comment_ref) = @{$_};
        my @comment;
        if ( $type eq 'merged' ) {
            @comment = ("RESYNC: ($old_rev-$new_rev); FILE MERGED", @{$resync_comment_ref});
        }
        elsif ( $type eq 'removed' ) {
            @comment = ("RESYNC:; FILE REMOVED");
        }
        else {
            print_error("INTERNAL ERROR: unknown commit type", 11);
        }
        print "\t $file: commit ...\n";
        $cvs_handle->commit_files([$file], \@comment);
        if ( $type eq 'removed' ) {
            # Uh oh, file has been removed in master workspace.
            # We can't place the anchor tag on the milestone tag
            # because it's simply not there. but we know that
            # it must be the top level revision of the master branch.
            # In this case we can set the Anchor tag to the revision
            # which corresponds to the head of the master branch.

            # overwrite .resync file with a 'dead' style request, in case
            # of an interruption between commit and the move anchor tag step.
            # $old_rev and $new_rev are just dummies, the $master_head is relevant
            write_resync_comment($file, 'dead', $master_head, '0.0', '0.0');
            push(@move_anchor_to_master_head_files, $file);
        }
        else {
            # overwrite .resync file with a move anchor style request, in case
            # of an interruption between commit and move anchor tag step.
            # $old_rev and $new_rev are just dummies, the $milestone_tag is relevant
            write_resync_comment($file, 'anchor', $milestone_tag, '0.0', '0.0');
            push(@move_anchor_tag_files, $file);
        }
        $module_stats{'merged'}++;
    }


    my $n_move_both_tags_files = @move_both_tags_files;
    if ( $n_move_both_tags_files ) {
        print "  ... move branch tag for $n_move_both_tags_files file(s) ...\n";

        my $tagged_files_ref = $cvs_handle->tag_files(\@move_both_tags_files, $cws_branch_tag, 1,
                                                        $milestone_tag);
        my $n_tagged_files = @{$tagged_files_ref};
        if ( $n_move_both_tags_files != $n_tagged_files ) {
            print_warning("expected $n_move_both_tags_files tag operations, got $n_tagged_files.", 0);
            plog("A\t $cvs_dir: expected $n_move_both_tags_files tag operations, got $n_tagged_files.");
        }
        # overwrite .resync file with a move anchor style request, in case
        # of an interruption between commit and move anchor tag step.
        foreach (@move_both_tags_files) {
            write_resync_comment($_, 'anchor', $milestone_tag, '0.0', '0.0');
        }
        push(@move_anchor_tag_files, @move_both_tags_files);
        $module_stats{'moved'} += $n_move_both_tags_files;
    }

    my $n_move_anchor_to_master_head_files = @move_anchor_to_master_head_files;

    if ( $n_move_anchor_to_master_head_files ) {
        print "  ... move anchor tag of removed files to to branch head '$master_head' for $n_move_anchor_to_master_head_files file(s) ...\n";

        my $tagged_files_ref = $cvs_handle->tag_files(\@move_anchor_to_master_head_files,
                                                        $cws_anchor_tag, 0, $master_head);
        my $n_tagged_files = @{$tagged_files_ref};
        if ( $n_move_anchor_to_master_head_files != $n_tagged_files ) {
            print_warning("expected $n_move_anchor_to_master_head_files tag operations, got $n_tagged_files.", 0);
            plog("A\t $cvs_dir: expected $n_move_anchor_to_master_head_files tag operations, got $n_tagged_files.");
        }
        $module_stats{'anchor'} += $n_move_anchor_to_master_head_files;
        unlink_resync_comment_files(\@move_anchor_to_master_head_files);
    }

    my $n_move_anchor_tag_files = @move_anchor_tag_files;
    if ( $n_move_anchor_tag_files ) {
        print "  ... move anchor tag to milestone '$milestone_tag' for $n_move_anchor_tag_files file(s) ...\n";

        my $tagged_files_ref = $cvs_handle->tag_files(\@move_anchor_tag_files, $cws_anchor_tag, 0,
                                                    $milestone_tag);
        my $n_tagged_files = @{$tagged_files_ref};
        if ( $n_move_anchor_tag_files != $n_tagged_files ) {
            print_warning("expected $n_move_anchor_tag_files tag operations, got $n_tagged_files.", 0);
        }
        $module_stats{'anchor'} += $n_move_anchor_tag_files;
        unlink_resync_comment_files(\@move_anchor_tag_files);
    }
    return \%module_stats;
}

sub check_for_conflict_markers
{
    my $files_ref = shift;

    foreach my $file ( @{$files_ref} ) {
        my $conflict = 0;
        my $basename = basename($file);
        open(MERGED_FILE, "<$file") or print_error("can't open '$file' for reading: $!.", 40);
        while (<MERGED_FILE> ) {
            chomp;
            if ( /^<<<<<<< $basename$/ ) {
                $conflict++;
            }
            if ( /^>>>>>>> 1\.\d[\d\.]*$/ ) {
                $conflict++;
            }
        }
        close(MERGED_FILE);
        if ( $conflict ) {
            print_error("found conflict marker in file '$file'", 99);
        }
    }
}

sub unlink_resync_comment_files
{
    my $comment_files_ref = shift;

    foreach ( @{$comment_files_ref} ) {
        if ( !unlink("$_.resync") ) {
            print_error("can't unlink $_.cwsresync!", 30);
        }
    }

}

sub write_resync_comment
{
    my $file          = shift;
    my $type          = shift;
    my $milestone_tag = shift;
    my $old_rev       = shift;
    my $new_rev       = shift;

    if ( !open(RESYNC_COMMENT, ">$file.resync") ) {
        print_error("can't open file '$file.resync'", 7);
    }
    my $uctype = uc($type);
    $old_rev = 'none' if !defined($old_rev);
    $new_rev = 'none' if !defined($new_rev);
    print RESYNC_COMMENT "RESYNC $uctype $milestone_tag $old_rev $new_rev\n";
    if ( $type eq 'removed' && $type eq 'merged' ) {
        print RESYNC_COMMENT "Everything below this line will be added to the revision comment.\n";
    }
    close(RESYNC_COMMENT);
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

# Checks specified milestone.
sub verify_milestone_or_exit
{
    my $cws = shift;
    my $qualified_milestone = shift;

    # HEAD is not a milestone but a valid resync target for single files.
    # This is useful if you want to resync a single file against the latest
    # version on a MWS even if that file is not yet part of a valid milestone.
    return if $qualified_milestone eq 'HEAD';

    my ($master, $milestone);
    my $invalid = 0;

    $invalid++ if $qualified_milestone =~ /-/;

    if ( $qualified_milestone =~ /:/ ) {
        ($master, $milestone) = split(/:/, $qualified_milestone);
        $invalid++ unless ( $master && $milestone );
    }
    else {
        $milestone = $qualified_milestone;
    }

    if ( $invalid ) {
        print_error("Invalid milestone", 0);
        usage();
        exit(1);
    }

    $master = $cws->master() if !$master;
    if ( !$cws->is_milestone($master, $milestone) ) {
        print_error("Milestone '$milestone' is not registered with master workspace '$master'.", 2);
    }
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

# Retrieve CwsCvsOps object for passed module.
sub get_cvs_handle
{
    my $cws           = shift;
    my $from_config   = shift; # from config or from disk directory?
    my $module_or_dir = shift;

    my $server_type;

    my  $config = CwsConfig::get_config();

    if ( $from_config eq 'config' ) {
        if ( defined($log) ) {
            my ($method, $vcsid, $server, $repository) = get_cvs_root($cws, $module_or_dir);
            my @elem = split(/\./, $server);
            $server = $elem[0];
            my $local_server = $config->cvs_local_root();

            if ( $local_server =~ /$server/ ) {
                $server_type = 'local';
            }
            else {
                $server_type = 'remote';
            }
        }
        else {
            # For now just take the configured OOo sever. Later we might implement a mechanism were
            # only known OOo modules are fetched from the OOo server, the rest from a local
            # server
            $server_type = 'remote';
        }
    }
    else {
        $server_type = 'directory';
    }

    return undef if !$server_type;

    if ( $opt_debug ) {
        my $log_file = IO::File->new('>>cwsresync.debug.log');
        my $time = localtime();
        $log_file->print("===== $time =====\n");
        return CwsCvsOps->new($config, $server_type, $module_or_dir, $log_file);
    }
    else {
        return CwsCvsOps->new($config, $server_type, $module_or_dir);
    }
}

# Find out which CVS server holds the module, returns
# the elements of CVSROOT.
# TODO: simplify as soon get_cvs_module is gone
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
        print_warning("No such module '$module' for '$master' in workspace database.");
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
        $log->end_log_extended($script_name,"unknown",$message) if (defined $log);
        exit($error_code);
    }
    return;
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

sub usage
{
    my $sw_skip_checkout = !defined($log) ? " [-f] " : " ";
    print STDERR "Usage:\n";
    print STDERR "cwsresync [-h] [-d dir] [-F] -m <milest.> <all|mod.|dir|file> [mod.|dir|file ...]\n";
    print STDERR "cwsresync [-h] [-d dir] -m HEAD <file> [file ...]\n";
    print STDERR "cwsresync [-h] [-d dir] -r|-c <all|module|dir|file> [module|dir|file ...]\n";
    print STDERR "cwsresync [-h]" . $sw_skip_checkout ."-l <milestone>\n";
    print STDERR "Synchronize child workspace mod./dirs/files ";
    print STDERR "with the latest master workspace changes \n";
    print STDERR "Options:\n";
    print STDERR "\t-h\t\thelp\n";
    print STDERR "\t-d dir\t\toperate in directory dir\n";
    print STDERR "\t-F\t\tforce checkout of complete modules\n";
    print STDERR "\t-m milestone\tmerge changes from MWS into CWS\n";
    print STDERR "\t-c\t\tcommit the merged files to CWS\n";
    print STDERR "\t-l milestone\trenew solver, relink modules to new milestone\n" if defined($log);
    print STDERR "\t-l milestone\tregister new milestone with database\n" if !defined($log);
    print STDERR "\t-r\t\tremove solver and module output trees, update milestone information\n" if !defined($log);
    print STDERR "\t-f\t\tavoid updating entire tree\n" if !defined($log);
    print STDERR "Notes:\n";
    print STDERR "\tA Milestone on a different MWS can be specified as <MWS:milestone>.\n";
    print STDERR "Examples:\n";
    print STDERR "\tcwsresync -m SRX645:m1 all \n";
    print STDERR "\tcwsresync -c all \n";
    print STDERR "\tcwsresync -l SRX645:m1 \n" if defined($log);
    print STDERR "\tcwsresync -r\n" if !defined($log);
}

# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
