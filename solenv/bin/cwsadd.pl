:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: cwsadd.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2004-06-26 00:23:18 $
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
# cwsadd.pl - add modules to child workspaces
#

use strict;
use Getopt::Long;
use Cwd;
use IO::Handle;

#### module lookup

use lib ("$ENV{SOLARENV}/bin/modules");
if (defined $ENV{COMMON_ENV_TOOLS}) {
    unshift(@INC, "$ENV{COMMON_ENV_TOOLS}/modules");
};

use Cws;
use CwsConfig;

eval { require Logging; import Logging; };
# $log variable is only defined in SO environment...
my $log = undef;
$log = Logging->new() if (!$@);
eval { require CopyPrj; import CopyPrj; };
use CvsModule;
use Config::Tiny;

######### Interrupt handler #########
 $SIG{'INT'} = 'INT_handler' if defined($log);

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.2 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### global #####

my $force_checkout = '';
my $is_debug = 0;
my $opt_dir  = '';  # dir option
my $vcsid = "unkown";
my $add_output_tree = 1;
my @found_platforms = ();

my @args_bak = @ARGV;
      # store the @ARGS here for logging

# module names to be rejected
my @invalid_names =
qw(common common.pro cvs cws wntmsci unxsols unxsoli unxlngi unxlngp macosxp);


#### main #####
my $parameter_list = $log->array2string(";",@args_bak) if (defined $log);

my @modules = parse_options();
my ($dir, $cws) = get_and_verify_cws();
my @modules_to_add = check_modules($cws, @modules);
my $workspace_db;
if ( @modules_to_add ) {
    if (defined $log) {
        require EnvHelper; import EnvHelper;
        $workspace_db = EnvHelper::get_workspace_db();
        @modules_to_add = copy_modules($cws, $dir, $workspace_db, @modules_to_add);
    } else {
        @modules_to_add = update_modules($cws, $dir, @modules_to_add);
    };
    if ( @modules_to_add ) {
        my @registered_modules = ();
        my $success = 0;
        my $module;
        foreach (@modules_to_add) {
            $module = $_;
            $success = branch_module($cws, $dir, $module);
            last unless $success;
            $success = register_module($cws, $workspace_db, $module);
            last unless $success;
            push(@registered_modules, $module);
        }
        if ( @registered_modules ) {
            my $modules_str = join(", ", @registered_modules);
            my $child = $cws->child();
            print "\n";
            print_message("Summary:");
            print_message("Sucessfully added and registered module(s) '$modules_str'.");
        }
        if ( !$success ) {
            print_error("Adding and/or registering module '$module' failed!", 5);
        }
    }
}

$log->end_log_extended($script_name,$vcsid,"success") if (defined $log);
exit(0);

#### subroutines ####

#
# Subroutine updates module
#
sub update_modules {
    my ($cws, $stand_dir, @modules_to_update) = @_;
    my (@updated_modules, @rejected_modules);
    my $master_tag = $cws->get_master_tag();
    foreach my $module (@modules_to_add) {
        my $cvs_module = CvsModule->new();
        $cvs_module->module($module);
        print "\tUpdating '$module' ...\n";
        my $result = $cvs_module->update($stand_dir, $master_tag);
        my ($updated, $merged, $conflicts) =
            $cvs_module->handle_update_infomation($result);
        if ($merged || $conflicts) {
            push(@rejected_modules, $module);
            next;
        };
        push(@updated_modules, $module);
    };
    if (@rejected_modules) {
        print_warning("Found conflicts and/or locallily files in the following modules:");
        print STDERR "$_\n" foreach (@rejected_modules);
        print_warning("These modules will not be added to CWS. Clean up and try adding them again.");
    };
    return @updated_modules;
};

sub get_and_verify_cws
{
    # get current child workspace from environment
    my $childws  = $ENV{CWS_WORK_STAMP};
    my $masterws = $ENV{WORK_STAMP};

    if ( !defined($childws) || !defined($masterws) ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    # get destination directory for modules to add
    my $dir = $ENV{SRC_ROOT};

    if ( !defined($dir) )
    {
        print_error("Need to get destination from SOURCE_ROOT - but it's not defined!",1);
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
    return ($dir, $cws);
}

sub parse_options
{
    # parse options and do some sanity checks
    # returns freshly allocated Cws reference

    # linking and unlinking requires UNIX
    if ( $^O =~ "MSWin32" )
    {
        print_error("Sorry! not for windows",2);
    }
    my $help;
    my $success = GetOptions('-h' => \$help, '-a' => \$force_checkout);
    if ( $help || !$success || $#ARGV < 0 ) {
        usage();
        exit(1);
    }

    return @ARGV;
}

sub check_modules
{
    # check if modules are registered with child workspace
    # returns list of modules which can be added

    my $cws     = shift;
    my @modules = @_;

    my @registered_modules = $cws->modules();

    # create hash for easier searching
    my %registered_modules_hash = ();
    for (@registered_modules) {
        $registered_modules_hash{$_}++;
    }
    my %invalid_names_hash = ();
    for (@invalid_names) {
        $invalid_names_hash{$_}++;
    }

    my $config = CwsConfig->get_config();
    my $cvs_module = CvsModule->new();
    $cvs_module->cvs_method($config->get_cvs_server_method());
    $cvs_module->vcsid($config->get_cvs_server_id());
    $cvs_module->cvs_server($config->get_cvs_server());
    $cvs_module->cvs_repository($config->get_cvs_server_repository());
    my @new_modules = ();
    my %cvs_aliases = $cvs_module->get_aliases_hash();
    foreach (@modules) {
        if ( $_ =~ /[\s\t\|\$\(\)\[\]\{\\}]/ || exists $invalid_names_hash{lc($_)} ) {
            print_error("'$_' is an invalid module name.", 3);
        }
        if ( exists $registered_modules_hash{$_} ) {
            print_warning("Module '$_' already registered, skipping.");
            next;
        };
        if (!defined($log) && !defined $cvs_aliases{$_}) {
            print_error("There is no such module alias '$_'.", 3);
        };
        push(@new_modules, $_);
    }

    return @new_modules;
}

sub copyprj_module_output
{
    return if ($force_checkout);

    my $module_name = shift;
    my $src_dest = shift;
    print "copyprj $module_name\n";

    # hash, that should contain all the
    # data needed by CopyPrj module
    my %ENVHASH = ();
    my %projects_to_copy = ();
    $ENVHASH{'projects_hash'} = \%projects_to_copy;
    $ENVHASH{'no_otree'} = 0;
    $ENVHASH{'no_path'} = 1;
    $ENVHASH{'only_otree'} = 1;
    $ENVHASH{'only_update'} = 1;
    $ENVHASH{'last_minor'} = 0;
    $ENVHASH{'spec_src'} = 0;
    $ENVHASH{'dest'} = "$src_dest";
    $ENVHASH{'prj_to_copy'} = '';
    $ENVHASH{'i_server'} = '';
    $ENVHASH{'current_dir'} = cwd();
    $ENVHASH{'remote'} = '';

    $projects_to_copy{$module_name}++;

    CopyPrj::copy_projects(\%ENVHASH);

};

sub copyprj_module_sourcetree
{
    my $module_name = shift;
    my $src_dest = shift;
    print "copyprj $module_name\n";

    # hash, that should contain all the
    # data needed by CopyPrj module
    my %ENVHASH = ();
    my %platforms_to_copy = ();
    $ENVHASH{'platforms_hash'} = \%platforms_to_copy;
    if ( $add_output_tree && !$force_checkout ) {
        $platforms_to_copy{$_}++ foreach (@found_platforms);
    };
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

sub copy_modules
{
    # copy modules from master workspace into child workspace
    my $cws          = shift;
    my $dir          = shift;
    my $workspace_db = shift;
    my @modules      = @_;

    my $masterws = $cws->master();
    my $childws  = $cws->child();
    my $milestone  = $cws->milestone();

    # ause: Deine Spielwiese
    my $result;
    my @success_modules = ();

    my $workspace = $workspace_db->get_key($masterws);
    if ( !$workspace ) {
        print_error("Master workspace '$masterws' not found in workspace database.", 3);
    }

    my $wslocation = $workspace_db->get_value($masterws."/Drives/o:/UnixVolume");
    if ( !defined($wslocation) ) {
        print_error("Location of master workspace '$masterws' not found in workspace database.", 3);
    }

    print "$wslocation\n" if $is_debug;

    my $source_root = $ENV{SOURCE_ROOT};
    if ( !defined( $source_root )) {
        print_error("SOURCE_ROOT not defined! Please setup a valid environment for CWS \"$childws\"", 1);
    }
    my $cws_solver = "$source_root/$masterws";

    my $start_dir = getcwd();
    $result = chdir($dir);
    if ( !$result ) {
        print_error("Cannot change to $dir!", 1);
    }

    # assume that every valid platform on child "solver" has to be coppied
    $result = opendir( SOLVER, "$cws_solver");
    if ( !$result ){ print_error ("Root dir of child workspace not accessible: $!", 1) };
    my @found_dirs = readdir( SOLVER );
    closedir( SOLVER );

    # hack to get the milestone :-(((((
    if ( ! defined($milestone))
    {
        $milestone = $ENV{UPDMINOR};
    }
    if ( $#found_dirs )
    {
        foreach my $dir_candidate ( @found_dirs )
        {
            if ( -d "$cws_solver/$dir_candidate/inc.".$milestone )
            {
                push @found_platforms, $dir_candidate;
            }
        }
    }
    # preparing pseudo environment for copyprj
    $ENV{SRC_ROOT}="$wslocation/$masterws/src.$ENV{UPDMINOR}";

    print "$ENV{SRC_ROOT}\n" if ( $is_debug );
    print "working dir: ".getcwd()."\n" if ( $is_debug );
    foreach my $one_module (@modules) {
        # do some snity checks for this module
        if ( -e "$one_module.lnk" && -e "$one_module" )
        {
            print_error("Duplicate representation of module $one_module ($one_module.lnk $one_module)", 0);
            print_error("Please clean up!", 0);
            print_error("Will NOT add module $one_module to child workspace!", 0);
            # fail for this module
            next;
        }
        if ( -e "$one_module.backup.lnk" || -e "$one_module.backup" )
        {
            print_error("Backup of module $one_module already exists.", 0);
            print_error("Please clean up!", 0);
            print_error("Will NOT add module $one_module to child workspace!", 0);
            # fail for this module
            next;

        }

        $result = 0;
        $result = rename($one_module, "$one_module.backup.lnk") if ( -l $one_module );
        if ( ! -l $one_module && -e $one_module ) {
            $result ||= rename($one_module, "$one_module.backup");
            # if it is no link, assume incompatible build
            # -> don't copy output tree
            $add_output_tree = 0;
        }
        $result ||= rename("$one_module.lnk", "$one_module.backup.lnk") if ( -e "$one_module.lnk" );
        $result = 0 if ( -e $one_module || -e "$one_module.lnk" );
        if ( ! $result )
        {
            print_error("Couldn't backup existing module $one_module before copying", 0);
            print_error("Will NOT add module $one_module to child workspace!", 0);
            # fail for this module
            next;
        }

        # now copy sources
        $result = copyprj_module_sourcetree( $one_module, "." );
        if ( $result )
        {
            if ( -d $one_module )
            {
                $result = rename("$one_module", "$one_module.failed");
                $result = system("rm -rf $one_module.failed");
            }
            print_error("Couldn't copy module $one_module, restoring previous.", 0);
            if ( -e "$one_module.backup" )
            {
                $result = rename("$one_module.backup", $one_module);
            }
            else
            {
                $result = rename("$one_module.backup.lnk", "$one_module.lnk");
            }
            print_error("Restoring link for $one_module failed! Cleanup is in your hand now", 1) if ( ! $result );
            # fail for this module
            next;
        }

        # remove backuped link
        unlink("$one_module.backup.lnk") if -l "$one_module.backup.lnk";

        # or backuped directory...
        if ( -d "$one_module.backup" )
        {
            if ( $^O =~ "MSWin32" )
            {
                print_error("Sorry! not for windows, nobody should ever get here!",2);
            }
            $result = system("rm -rf $one_module.backup");
        }

        # insert module in list of successfull copied modules
        push(@success_modules, $one_module);
    }

    chdir($start_dir);

#    return my @empty = (); # ause - disable all further steps
    return @success_modules;
}

sub branch_module
{
    # tag modules with cws branch cws root tag
    my $cws          = shift;
    my $dir          = shift;
    my $module       = shift;

    my ($cws_master_tag, $cws_branch_tag, $cws_root_tag) = $cws->get_tags();

    # Sanity check
    print "operating on $dir/$_\n" if $is_debug;
    if ( -S "$dir/$module" || ! -d "$dir/$module" ) {
        print_error("Can't find physical copy of module '$module'", 4);
    }

    STDOUT->autoflush(1);
    print_message("Tagging module '$module'.");
    my $cvs_module = CvsModule->new();
    $cvs_module->module($module);
    $cvs_module->verbose(1);

    print_message("Tag with branch tag '$cws_branch_tag'.");
    my ($branched_files, $branch_errors) = $cvs_module->tag($dir, $cws_branch_tag, '-b');
    if ( $branched_files < 1 ) {
        print_error("Tagging module '$module' failed.", 0);
        return 0;
    }
    if ( $branch_errors > 0 ) {
        print_error(cleanup_tags_msg($module), 0);
        return 0;
    }
    print_message("Tagged $branched_files files in module '$module' with branch tag.");

    print_message("Tag with tag '$cws_root_tag'.");
    my ($tagged_files, $anchor_errors) = $cvs_module->tag($dir, $cws_root_tag);
    if ( $tagged_files < 1 || $anchor_errors > 0
            || $branched_files != $tagged_files )
    {
        print_error(cleanup_tags_msg($module), 0);
        return 0;
    }
    print_message("Tagged $tagged_files files in module '$module'.");

    print_message("Updating module '$module' to branch '$cws_branch_tag'.");
    my @dirs = $cvs_module->update($dir, $cws_branch_tag);
    if ( $#dirs < 0 ) {
        print_error("Updating module '$module' to branch '$cws_branch_tag' failed.\n", 0);
        return 0;
    }
    STDOUT->autoflush(0);
    return 1;
}

sub cleanup_tags_msg
{
    my $module = shift;
    my ($cws_master_tag, $cws_branch_tag, $cws_root_tag) = $cws->get_tags();

    my $msg = "Tagging module '$module' failed partly. Can't continue.\n";
    $msg .= "Please remember to manually remove the tags '$cws_branch_tag' and '$cws_root_tag'\n";
    $msg .= "from module '$module' before retrying the operation!";

    return $msg;
}

# Register module with EIS.
sub register_module
{
    my $cws          = shift;
    my $workspace_db = shift;
    my $module       = shift;

    my $public = 1;

    # find out if module has public flag
    my $master = $cws->master();
    if (defined $log) {
        my $key = "$master/Drives/o:/Projects/$module/SCS";
        my $scs = $workspace_db->get_value($key);

        if ( !defined($scs) ) {
            print_error("Can't find module '$module' in workspace db", 0);
            return 0;
        }

        # FIXME - this really shouldn't be hard coded
        if ( $scs !~ /tunnel/ ) {
            $public = 0;
        }
    }

    my $success = $cws->add_module($module, $public);
    if ( !$success ) {
        print_error("Can't register module '$module' with EIS!", 0);
        return 0;
    }
    print_message("Succesfully registered module '$module'.");
    return 1;
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
        $log->end_log_extended($script_name,$vcsid,$message) if (defined $log);
        exit($error_code);
    }
    return;
}

sub usage
{
    print STDERR "Usage: cwsadd [-h] [-a] <module> ... \n";
    print STDERR "Add one or more modules to child workspace.\n";
    print STDERR "Options:\n";
    print STDERR "    -a    use cvs checkout instead of copying\n";
    print STDERR "    -h    print this help\n";

}
