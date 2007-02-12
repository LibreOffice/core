:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: cwsquery.pl,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: kz $ $Date: 2007-02-12 15:09:26 $
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
# cwsquery.pl - query misc. information from CWS
#

use strict;
use Getopt::Long;
use Cwd;

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

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.12 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print STDERR "$script_name -- version: $script_rev\n";

#### global #####

my $is_debug = 1;           # enable debug
my $opt_master = '';        # option: master workspace
my $opt_child  = '';        # option: child workspace
my $opt_milestone  = '';    # option: milestone

# list of available query modes
my @query_modes = qw(modules incompatible taskids status latest current owner qarep build buildid integrated approved nominated ready new planned release due due_qa help ui milestones masters vcs ispublic ispublicmaster);
my %query_modes_hash = ();
foreach (@query_modes) {
    $query_modes_hash{$_}++;
}

#### main #####

my $query_mode = parse_options();
query_cws($query_mode);
exit(0);

#### subroutines ####

sub query_cws
{
    my $query_mode = shift;
    # get master and child workspace
    my $masterws  = $opt_master ? uc($opt_master) : $ENV{WORK_STAMP};
    my $childws   = $opt_child  ? $opt_child  : $ENV{CWS_WORK_STAMP};
    my $milestone = $opt_milestone  ? $opt_milestone  : 'latest';

    if ( !defined($masterws) && $query_mode ne 'masters') {
        print_error("Can't determine master workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    if ( ($query_mode eq 'modules' || $query_mode eq 'incompatible' || $query_mode eq 'taskids' || $query_mode eq 'state'
         || $query_mode eq 'current' || $query_mode eq 'owner' || $query_mode eq 'qarep' || $query_mode eq 'issubversion' || $query_mode eq 'ispublic' || $query_mode eq 'build') && !defined($childws) ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    my $cws = Cws->new();
    if ( defined($childws) ) {
        $cws->child($childws);
    }
    if ( defined($masterws) ) {
        $cws->master($masterws);
    }

    no strict;
    &{"query_".$query_mode}($cws, $milestone);
    exit(0)
}

sub query_modules
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my @modules = $cws->modules();
        print_message("Modules:");
        foreach (@modules) {
            if ( defined($_) ) {
                print "$_\n";
            }
        }
    }
    return;
}

sub query_incompatible
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my @modules = $cws->incompatible_modules();
        print_message("Incompatible Modules:");
        foreach (@modules) {
            if ( defined($_) ) {
                print "$_\n";
            }
        }
    }
    return;
}

sub query_taskids
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my @taskids = $cws->taskids();
        print_message("Task ID(s):");
        foreach (@taskids) {
            if ( defined($_) ) {
                print "$_\n";
            }
        }
    }
    return;
}

sub query_status
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $status = $cws->get_approval();
        if ( !$status ) {
            print_error("Internal error: can't get approval status.", 3);
        } else {
            print_message("Approval status:");
            print "$status\n";
        }
    }
    return;
}

sub query_vcs
{
    my $cws = shift;
    my $masterws = $cws->master();
    my $childws  = $cws->child();

    if ( is_valid_cws($cws) ) {
        my $issvn = $cws->get_subversion_flag();
        if ( !defined($issvn) ) {
            print_error("Internal error: can't get isSubVersion flag.", 3);
        } else {
            if ( $issvn==1 ) {
                print_message("Child workspace uses SubVersion");
            } else {
                print_message("Child workspace uses CVS");
            }
        }
    }

    # check if we got a valid child workspace
    my $id = $cws->eis_id();
    if ( !$id ) {
        print_error("Child workspace '$childws' for master workspace '$masterws' not found in EIS database.", 2);
    }

    return;
}

sub query_ispublic
{
    my $cws = shift;
    my $masterws = $cws->master();
    my $childws  = $cws->child();

    if ( is_valid_cws($cws) ) {
        my $ispublic = $cws->get_public_flag();
        if ( !defined($ispublic) ) {
            print_error("Internal error: can't get isPublic flag.", 3);
        } else {
            if ( $ispublic==1 ) {
                print_message("Child workspace is public");
            } else {
                print_message("Child workspace is internal");
            }
        }
    }

    # check if we got a valid child workspace
    my $id = $cws->eis_id();
    if ( !$id ) {
        print_error("Child workspace '$childws' for master workspace '$masterws' not found in EIS database.", 2);
    }

    return;
}

sub query_current
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $milestone = $cws->milestone();
        if ( !$milestone ) {
            print_error("Internal error: can't get current milestone.", 3);
        } else {
            print_message("Current milestone:");
            print "$milestone\n";
        }
    }
    return;
}

sub query_owner
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $owner = $cws->get_owner();
        print_message("Owner:");
        if ( !$owner ) {
            print "not set\n" ;
        } else {
            print "$owner\n";
        }
    }
    return;
}

sub query_qarep
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $qarep = $cws->get_qarep();
        print_message("QA Representative:");
        if ( !$qarep ) {
            print "not set\n" ;
        } else {
            print "$qarep\n";
        }
    }
    return;
}


sub query_build
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $build = $cws->get_build();
        print_message("Build:");
        if ( $build ) {
            print "$build\n";
        }
    }
    return;
}

sub query_latest
{
    my $cws = shift;

    my $masterws = $cws->master();
    my $latest = $cws->get_current_milestone($masterws);


    if ( $latest ) {
        print_message("Master workspace '$masterws':");
        print_message("Latest milestone available for resync:");
        print "$masterws $latest\n";
    }
    else {
        print_error("Can't determine latest milestone of '$masterws' available for resync.", 3);
    }

    return;
}

sub query_masters
{
    my $cws = shift;

    my @mws = $cws->get_masters();
    my $list="";

    if ( @mws ) {
        foreach (@mws) {
            if ( $list ne "" ) {
                $list .= ", ";
            }
            $list .= $_;
        }
        print_message("Master workspaces available: $list");
    }
    else {
        print_error("Can't determine masterworkspaces.", 3);
    }

    return;
}

sub query_milestones
{
    my $cws = shift;
    my $masterws = $cws->master();

    my @milestones = $cws->get_milestones($masterws);
    my $list="";

    if ( @milestones ) {
        foreach (@milestones) {
            if ( $list ne "" ) {
                $list .= ", ";
            }
            $list .= $_;
        }
        print_message("Master workspace '$masterws':");
        print_message("Milestones known on Master: $list");
    }
    else {
        print_error("Can't determine milestones of '$masterws'.", 3);
    }

    return;
}

sub query_ispublicmaster
{
    my $cws = shift;
    my $masterws = $cws->master();

    my $ispublic = $cws->get_publicmaster_flag();
    my $list="";

    if ( defined($ispublic) ) {
        print_message("Master workspace '$masterws':");
        if ( !defined($ispublic) ) {
            print_error("Internal error: can't get isPublicMaster flag.", 3);
        } else {
            if ( $ispublic==1 ) {
                print_message("Master workspace is public");
            } else {
                print_message("Master workspace is internal");
            }
        }
    }
    else {
        print_error("Can't determine isPublicMaster flag of '$masterws'.", 3);
    }

    return;
}

sub query_buildid
{
    my $cws       = shift;
    my $milestone = shift;

    my $masterws = $cws->master();
    if ( $milestone eq 'latest' ) {
        $milestone = $cws->get_current_milestone($masterws);
    }

    if ( !$milestone ) {
        print_error("Can't determine latest milestone of '$masterws'.", 3);
    }

    if ( !$cws->is_milestone($masterws, $milestone) ) {
        print_error("Milestone '$milestone' is no a valid milestone of '$masterws'.", 3);
    }

    my $buildid = $cws->get_buildid($masterws, $milestone);


    if ( $buildid ) {
        print_message("Master workspace '$masterws':");
        print_message("BuildId for milestone '$milestone':");
        print("$buildid\n");
    }

    return;
}

sub query_integrated
{
    my $cws       = shift;
    my $milestone = shift;

    my $masterws = $cws->master();
    if ( $milestone eq 'latest' ) {
        $milestone = $cws->get_current_milestone($masterws);
    }

    if ( !$milestone ) {
        print_error("Can't determine latest milestone of '$masterws'.", 3);
    }

    if ( !$cws->is_milestone($masterws, $milestone) ) {
        print_error("Milestone '$milestone' is no a valid milestone of '$masterws'.", 3);
    }

    my @integrated_cws = $cws->get_integrated_cws($masterws, $milestone);


    if ( @integrated_cws ) {
        print_message("Master workspace '$masterws':");
        print_message("Integrated CWSs for milestone '$milestone':");
        foreach (@integrated_cws) {
            print "$_\n";
        }
    }

    return;
}

sub query_approved
{
    my $cws       = shift;

    my $masterws = $cws->master();

    my @approved_cws = $cws->get_cws_with_state($masterws, 'approved by QA');

    if ( @approved_cws ) {
        print_message("Master workspace '$masterws':");
        print_message("CWSs approved by QA:");
        foreach (@approved_cws) {
            print "$_\n";
        }
    }

    return;
}

sub query_nominated
{
    my $cws       = shift;

    my $masterws = $cws->master();

    my @nominated_cws = $cws->get_cws_with_state($masterws, 'nominated');

    if ( @nominated_cws ) {
        print_message("Master workspace '$masterws':");
        print_message("Nominated CWSs:");
        foreach (@nominated_cws) {
            print "$_\n";
        }
    }

    return;
}

sub query_ready
{
    my $cws       = shift;

    my $masterws = $cws->master();

    my @ready_cws = $cws->get_cws_with_state($masterws, 'ready for QA');

    if ( @ready_cws ) {
        print_message("Master workspace '$masterws':");
        print_message("CWSs ready for QA:");
        foreach (@ready_cws) {
            print "$_\n";
        }
    }

    return;
}

sub query_new
{
    my $cws       = shift;

    my $masterws = $cws->master();

    my @ready_cws = $cws->get_cws_with_state($masterws, 'new');

    if ( @ready_cws ) {
        print_message("Master workspace '$masterws':");
        print_message("CWSs with state 'new':");
        foreach (@ready_cws) {
            print "$_\n";
        }
    }

    return;
}

sub query_planned
{
    my $cws       = shift;

    my $masterws = $cws->master();

    my @ready_cws = $cws->get_cws_with_state($masterws, 'planned');

    if ( @ready_cws ) {
        print_message("Master workspace '$masterws':");
        print_message("CWSs with state 'planned':");
        foreach (@ready_cws) {
            print "$_\n";
        }
    }

    return;
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

sub query_release
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $release = $cws->get_release();
            print_message("Release target:");
        if ( !$release ) {
            print "not set\n";
        } else {
            print "$release\n";
        }
    }
    return;
}

sub query_due
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $due = $cws->get_due_date();
            print_message("Due date:");
        if ( !$due ) {
            print "not set\n";
        } else {
            print "$due\n";
        }
    }
    return;
}

sub query_due_qa
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $due_qa = $cws->get_due_date_qa();
            print_message("Due date (QA):");
        if ( !$due_qa ) {
            print "not set\n";
        } else {
            print "$due_qa\n";
        }
    }
    return;
}

sub query_help
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $help = $cws->is_helprelevant();
            print_message("Help relevant:");
        if ( !$help ) {
            print "false\n";
        } else {
            print "true\n";
        }
    }
    return;
}

sub query_ui
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $help = $cws->is_uirelevant();
            print_message("UI relevant:");
        if ( !$help ) {
            print "false\n";
        } else {
            print "true\n";
        }
    }
    return;
}

sub parse_options
{
    # parse options and do some sanity checks
    my $help = 0;
    my $success = GetOptions('h' => \$help, 'm=s' => \$opt_master, 'c=s'=> \$opt_child,
                             'ms=s' => \$opt_milestone);
    if ( $help || !$success || $#ARGV < 0 ) {
        usage();
        exit(1);
    }

    my $mode = lc($ARGV[0]);

    # cwquery mode 'state' has been renamed to 'status' to be more consistent
    # with CVS etc. 'state' is still an alias for 'status'
    $mode = 'status' if $mode eq 'state';

    # there will be more query modes over time
    if ( !exists $query_modes_hash{$mode} ) {
        usage();
        exit(1);
    }
    return $mode;
}

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
    print STDERR "Usage: cwsquery [-h] [-m master] [-c child] <current|modules|incompatible|owner|qarep|status|taskids>\n";
    print STDERR "       cwsquery [-h] [-m master] [-c child] <release|due|due_qa|help|ui|ispublic|vcs|build>\n";
    print STDERR "       cwsquery [-h] [-m master] <latest|milestones|ispublicmaster>\n";
    print STDERR "       cwsquery [-h] <masters>\n";
    print STDERR "       cwsquery [-h] [-m master] [-ms milestone/step] <integrated|buildid>\n";
    print STDERR "       cwsquery [-h] [-m master] <planned|new|approved|nominated|ready>\n";
    print STDERR "Query child workspace for miscellaneous information.\n";
    print STDERR "Modes:\n";
    print STDERR "\tcurrent\t\tquery current milestone of CWS\n";
    print STDERR "\tmodules\t\tquery modules added to the CWS\n";
    print STDERR "\tincompatible\tquery modules which should be build incompatible\n";
    print STDERR "\towner\t\tquery CWS owner\n";
    print STDERR "\tqarep\t\tquery CWS QA Representative\n";
    print STDERR "\tstatus\t\tquery approval status of CWS\n";
    print STDERR "\ttaskids\t\tquery taskids to be handled on the CWS\n";
    print STDERR "\trelease\t\tquery for target release of CWS\n";
    print STDERR "\tdue\t\tquery for due date of CWS\n";
    print STDERR "\tdue_qa\t\tquery for due date (QA) of CWS\n";
    print STDERR "\thelp\t\tquery if the CWS is help relevant\n";
    print STDERR "\tui\t\tquery if the CWS is UI relevant\n";
    print STDERR "\tbuild\t\tquery build String for CWS\n";
    print STDERR "\tlatest\t\tquery the latest milestone available for resync\n";
    print STDERR "\tbuildid\t\tquery build ID for milestone\n";
    print STDERR "\tintegrated\tquery integrated CWSs for milestone\n";
    print STDERR "\tplanned\t\tquery for planned CWSs\n";
    print STDERR "\tnew\t\tquery for new CWSs\n";
    print STDERR "\tapproved\tquery CWSs approved by QA\n";
    print STDERR "\tnominated\tquery nominated CWSs\n";
    print STDERR "\tready\t\tquery CWSs ready for QA\n";
    print STDERR "\tispublic\tquery public flag of CWS\n";
    print STDERR "\tvcs\t\tquery Version Control System used for CWS (either CVS or SubVersion)\n";
    print STDERR "\tmasters\t\tquery available MWS\n";
    print STDERR "\tmilestones\tquery which milestones are know on the given MWS\n";
    print STDERR "\tispublicmaster\tquery public flag of MWS\n";
    print STDERR "Options:\n";
    print STDERR "\t-h\t\thelp\n";
    print STDERR "\t-m master\toverride MWS specified in environment\n";
    print STDERR "\t-c child\toverride CWS specified in environment\n";
    print STDERR "\t-ms milestone\toverride latest milestone/step with specified one\n";
    print STDERR "Examples:\n";
    print STDERR "\tcwsquery modules \n";
    print STDERR "\tcwsquery -m SRX644 -c uno4 modules \n";
    print STDERR "\tcwsquery -m SRX645 -c pmselectedfixes status\n";
    print STDERR "\tcwsquery taskids\n";
    print STDERR "\tcwsquery -m SRC680 latest\n";
    print STDERR "\tcwsquery -m SRC680 -ms m130 integrated\n";

}
