:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: cwsclone.pl,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2007-12-12 13:17:30 $
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
# cwsclone.pl - checkout buildable child workspaces (incl. master)
#

use strict;
use Getopt::Long;
use Cwd;
use File::Path;

#### module lookup
my @lib_dirs;
BEGIN {
    if ( !defined($ENV{COMMON_ENV_TOOLS}) and !defined($ENV{SOLARENV}) ) {
        die "You need to set least one of the environment variables SOLARENV or COMMON_ENV_TOOLS.\n";
    }
    push(@lib_dirs, "$ENV{SOLARENV}/bin/modules") if defined($ENV{SOLARENV});;
    push(@lib_dirs, "$ENV{COMMON_ENV_TOOLS}/modules") if defined($ENV{COMMON_ENV_TOOLS});
}
use lib (@lib_dirs);

use Cws;
use CvsModule;
use CwsConfig;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.6 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print STDERR "$script_name -- version: $script_rev\n";

#### global #####

my $is_debug = 1;       # enable debug
my $opt_master = '';    # option: master workspace
my $opt_child  = '';    # option: child workspace
my $opt_prune  = '';    # option: delete non-repository files
my $opt_dir   = '';     # optional directory argument
my %up_module;          # optional list of modules to update
my $up_modules = 0;     # number of modules in %up_module



#### main #####

parse_options();

my $masterws = $opt_master ? uc($opt_master) : $ENV{WORK_STAMP};
my $childws  = $opt_child  ? $opt_child  : $ENV{CWS_WORK_STAMP};

if ( !defined($masterws) ) {
    print_error("Can't determine master workspace environment.\n"
                . "Please initialize environment with setsolar ...", 1);
}

if ( !defined($childws) ) {
    print_error("Can't determine child workspace environment.\n"
                . "Please initialize environment with setsolar ...", 1);
}

my $stand_dir = $opt_dir  ? $opt_dir  : $ENV{SRC_ROOT};
$stand_dir = $stand_dir  ? $stand_dir  : cwd();
if( ! -d $stand_dir ) {
    print_error("Target directory doesn't exist ...", 1);
}

my $cws = Cws->new();
$cws->child($childws);
$cws->master($masterws);

my %is_cwsmodule = ();
if ( is_valid_cws($cws) ) {
    my @cwsmodules = $cws->modules();
#    print_message("Modules:");
    foreach (@cwsmodules) {
        $is_cwsmodule{$_} = 1;
#        print "$_\n";
    }
}
print_message("Checking out / updating to '$stand_dir':");

my $master_tag =  $cws->get_master_tag();
print_message("Mastertag:".$master_tag);
my $child_tag = "cws_".lc($masterws)."_".$childws;
print_message("Childtag:".$child_tag."\n");
my $config = CwsConfig::get_config;
my $cvs_module = CvsModule->new();
$cvs_module->cvs_method($config->get_cvs_server_method());
$cvs_module->vcsid($config->get_cvs_server_id());
$cvs_module->cvs_server($config->get_cvs_server());
$cvs_module->cvs_repository($config->get_cvs_server_repository());
my %cvs_aliases = $cvs_module->get_aliases_hash();

foreach my $module (split( /\s+/, $cvs_aliases{'OpenOffice2'})) {
    my $use_tag;
    my $result1;
    my $result2;
    next if ($module eq '-a');

    if( $is_cwsmodule{$module} ) {
        print "Child: ";
        $use_tag = $child_tag;
    } else {
        $use_tag = $master_tag;
    }
    if ( $up_modules == 0 or $up_module{$module} ) {
        if( $up_modules ) { $up_module{$module} = "updated"; }
        $cvs_module->module($module);
        if (!-d "$stand_dir/$module/CVS") {
            print("\tChecking out '$module' with tag '$use_tag' ...");
            $result1 = $cvs_module->checkout($stand_dir, $use_tag, '');
            output_update_information($result1);
        } else {
            print "\tUpdating '$module' to tag '$use_tag' ...";
            ($result1, $result2) = $cvs_module->update($stand_dir, $use_tag, '-dPRC', 1 );
            output_update_information($result1, $result2);
            if($opt_prune) { prune_files($result2, $module ); }
        }
    }
}
print "\n";
if( $up_modules ) {
    while ( my ($key, $value) = each %up_module) {
        if($value ne "updated") {
            print_message("Invalid module name: '$key'.");
        }
    }
}

exit(0);

#### subroutines ####

sub parse_options
{
    # parse options and do some sanity checks
    my $help = 0;
    my $success = GetOptions('h' => \$help, 'm=s' => \$opt_master, 'c=s'=> \$opt_child, 'd=s' => \$opt_dir, 'p' => \$opt_prune);
    if ( $help || !$success ) {
        usage();
        exit(1);
    }
    foreach ( @ARGV ) {
        $up_module{$_} = 1;
    }
    $up_modules = keys %up_module;
    if( $up_modules > 0 ) {
        print_message("Updating $up_modules modules.");
    } else {
        print_message("Updating complete workspace modules.");
    }
}

sub is_valid_cws
{
    my $cws = shift;

    my $masterws = $cws->master();
    my $childws  = $cws->child();
    # check if we got a valid child workspace
    my $id = $cws->eis_id();
    if ( !$id ) {
        print_error("Child workspace '$childws' for master workspace '$masterws' not found in EIS database.", 2);
    }
    print_message("Master workspace '$masterws', child workspace '$childws':");
    return 1;
}

sub output_update_information {
    my $updated_files_ref = shift;
    my $nonrepolist = shift;
    my ($updated, $merged, $conflicts, $non_repo);
    foreach ( @$updated_files_ref ) {
        if ( $_->[1] eq 'P' || $_->[1] eq 'U' ) {
#           print "\t$_->[1]\t$_->[0]\n";
            $updated++;
        }
        elsif ( $_->[1] eq 'M' ) {
            print "\t$_->[1]\t$_->[0]\n";
            $merged++;
        }
        elsif ( $_->[1] eq 'C' ) {
            print "\t$_->[1]\t$_->[0]\n";
            $conflicts++;
        }
        else {
            # can't happen
            die("ERROR: internal error in update_module()");
        }
    }
    foreach ( @$nonrepolist ) {
        $non_repo++;
    }
    if( !($updated or $merged or $conflicts or $non_repo) ) {
        print(" nothing to do.");
    }
    print("\n");
    print("\t   Updated: $updated\n") if $updated;
    print("\t   Merged: $merged\n") if $merged;
    print("\t   Conflict(s): $conflicts\n") if $conflicts;
    print("\t   Non-repository files/directories(s): $non_repo\n") if $non_repo;
    return ($updated, $merged, $conflicts);
};

sub prune_files {
    my $deletethis = shift;
    my $module = shift;

    foreach ( @$deletethis ) {
        print("\t   deleting ".$stand_dir."/".$module."/".$_." ...\n");
        rmtree($stand_dir."/".$module."/".$_ , 0, 1);
    }
};

sub print_message
{
    my $message     = shift;

    print STDERR "$script_name: ";
    print STDERR "$message\n";
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
    print STDERR "Usage: cwsclone [-h] [-m master] [-c cws_name] [-d dir] [-p] [<module list>]\n";
    print STDERR "Checks out / updates all (or only the modules from <modules list>)\n";
    print STDERR "of a child workspace including the needed modules from the master\n";
    print STDERR "workspace.\n";
    print STDERR "\nOptions:\n";
    print STDERR "\t-m master\toverride MWS specified in environment\n";
    print STDERR "\t-c child\toverride CWS specified in environment\n";
    print STDERR "\t-d dir\tcreate workspace in directory dir. This overrides \$SRC_ROOT.\n";
    print STDERR "\t-p prune files/directories that are not checked in the repository\n";
    print STDERR "\t   (or are hidden by cvs ignore directives).\n";
    print STDERR "\nEnvironment variables used:\n";
    print STDERR "\t\$COMMON_ENV_TOOLS, \$SOLAR_ENV\tAt least one of these has to be set\n";
    print STDERR "\tbut \$SOLAR_ENV takes precedence if available.\n";
    print STDERR "\nExamples:\n";
    print STDERR "\tcwsclone.pl -m SRC680 -c ooo20040815\n";
}
