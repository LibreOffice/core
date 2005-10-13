:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: cwsquery.pl,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2005-10-13 16:44:33 $
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
my $id_str = ' $Revision: 1.8 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print STDERR "$script_name -- version: $script_rev\n";

#### global #####

my $is_debug = 1;           # enable debug
my $opt_master = '';        # option: master workspace
my $opt_child  = '';        # option: child workspace
my $opt_milestone  = '';    # option: milestone

# list of available query modes
my @query_modes = qw(modules taskids state latest current owner integrated approved nominated ready);
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

    if ( !defined($masterws) ) {
        print_error("Can't determine master workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    if ( ($query_mode eq 'modules' || $query_mode eq 'taskids' || $query_mode eq 'state'
         || $query_mode eq 'current' || $query_mode eq 'owner') && !defined($childws) ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    my $cws = Cws->new();
    $cws->child($childws);
    $cws->master($masterws);

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
            print "$_\n";
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
            print "$_\n";
        }
    }
    return;
}

sub query_state
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $state = $cws->get_approval();
        if ( !$state ) {
            print_error("Internal error: can't get approval status.", 3);
        } else {
            print_message("Approval status:");
            print "$state\n";
        }
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

    # there will be more query modes over time
    if ( !exists $query_modes_hash{lc($ARGV[0])} ) {
        usage();
        exit(1);
    }
    return lc($ARGV[0]);
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
    print STDERR "Usage: cwsquery [-h] [-m master] [-c child] <current|modules|owner|state|taskids>\n";
    print STDERR "       cwsquery [-h] [-m master] <latest>\n";
    print STDERR "       cwsquery [-h] [-m master] [-ms milestone/step] <integrated>\n";
    print STDERR "       cwsquery [-h] [-m master] <approved|nominated|ready>\n";
    print STDERR "Query child workspace for miscellaneous information.\n";
    print STDERR "Modes:\n";
    print STDERR "\tmodules\t\tquery modules added to the CWS\n";
    print STDERR "\towner\t\tquery CWS owner\n";
    print STDERR "\tstate\t\tquery approval status of CWS\n";
    print STDERR "\ttaskids\t\tquery taskids to be handled on the CWS\n";
    print STDERR "\tcurrent\t\tquery current milestone of CWS\n";
    print STDERR "\tlatest\t\tquery the latest milestone available for resync\n";
    print STDERR "\tintegrated\tquery integrated CWSs for milestone\n";
    print STDERR "\tapproved\tquery CWSs approved by QA\n";
    print STDERR "\tnominated\tquery nominated CWSs\n";
    print STDERR "\tready\t\tquery CWSs ready for QA\n";
    print STDERR "Options:\n";
    print STDERR "\t-h\t\thelp\n";
    print STDERR "\t-m master\toverride MWS specified in environment\n";
    print STDERR "\t-c child\toverride CWS specified in environment\n";
    print STDERR "\t-ms milestone\toverride latest milestone/step with specified one\n";
    print STDERR "Examples:\n";
    print STDERR "\tcwsquery modules \n";
    print STDERR "\tcwsquery -m SRX644 -c uno4 modules \n";
    print STDERR "\tcwsquery -m SRX645 -c pmselectedfixes state\n";
    print STDERR "\tcwsquery taskids\n";
    print STDERR "\tcwsquery -m SRC680 latest\n";
    print STDERR "\tcwsquery -m SRC680 -ms m130 integrated\n";

}
