:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: cwsaddtask.pl,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:06:07 $
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
# cwsaddtask.pl - add taskids (issues) to child workspaces
#

use strict;
use Getopt::Long;

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
use CwsConfig;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.5 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

#### global #####

my $is_debug = 0;

# CWS states for which adding tasks are blocked.
my @states_blocked_for_adding = (
                                  "integrated",
                                  "nominated",
                                  "approved by QA"
                                );

#### main #####

my ($cws, $vcsid) = get_and_verify_cws();
my @taskids = parse_options();
register_taskids($cws, $vcsid, @taskids);

exit(0);

#### subroutines ####

# Get current child workspace from environment.
sub get_and_verify_cws
{
    my $childws  = $ENV{CWS_WORK_STAMP};
    my $masterws = $ENV{WORK_STAMP};

    my $config = CwsConfig->get_config();
    my $vcsid  = $config->vcsid();
    if ( !defined($vcsid) ) {
        print_error("VCSID environment variable not set", 1);
    }

    if ( !defined($childws) || !defined($masterws) ) {
        print_error("Can't determine child workspace environment.\n"
                    . "Please initialize environment with setsolar ...", 1);
    }

    my $cws = Cws->new();
    $cws->child($childws);
    $cws->master($masterws);

    # Check if we got a valid child workspace.
    my $id = $cws->eis_id();
    print "Master: $masterws, Child: $childws, $id\n" if $is_debug;
    if ( !$id ) {
        print_error("Child workspace $childws for master workspace $masterws not found in EIS database.", 2);
    }

    return ($cws, $vcsid);
}

# Parse options and do some sanity checks;
sub parse_options
{
    my $help;
    my $success = GetOptions('h' => \$help);
    if ( !$success || $help || $#ARGV < 0 ) {
        usage();
        exit(1);
    }

    return @ARGV;
}

# Register taskids with EIS database;
# checks taksids for sanity, will notify user
# if taskid is already registered.
sub register_taskids
{

    my $cws     = shift;
    my $vcsid   = shift;
    my @taskids = @_;

    my $child = $cws->child();

    my $status = $cws->get_approval();

    if ( !defined($status) ) {
        print_error("Can't determine status of child workspace `$child`.", 4);
    }

    if ( grep($status eq $_, @states_blocked_for_adding) ) {
        print_error("Can't add tasks to child workspace '$child' with state '$status'.", 5);
    }

    my @registered_taskids = $cws->taskids();

    # Create hash for easier searching.
    my %registered_taskids_hash = ();
    for (@registered_taskids) {
        $registered_taskids_hash{$_}++;
    }

    my @new_taskids = ();
    foreach (@taskids) {
        if ( $_ !~ /^([ib]?\d+)$/ ) {
            print_error("'$_' is an invalid task ID.", 3);
        }
        if ( exists $registered_taskids_hash{$1} ) {
            print_warning("Task ID '$_' already registered, skipping.");
            next;
        }
        push(@new_taskids, $_);
    }

    my $added_taskids_ref = $cws->add_taskids($vcsid, @new_taskids);
    if ( !$added_taskids_ref )  {
        my $taskids_str = join(" ", @new_taskids);
        print_error("Couldn't register taskID(s) '$taskids_str' with child workspace '$child'.", 4);
    }
    my @added_taskids = @{$added_taskids_ref};
    if ( @added_taskids ) {
        my $taskids_str = join(" ", @added_taskids);
        print_message("Registered taskID(s) '$taskids_str' with child workspace '$child'.");
    }
    return;

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
        exit($error_code);
    }
    return;
}

sub usage
{
    print STDERR "Usage: cwsaddtask [-h] <taskID> ... \n";
    print STDERR "Add one or more taskIDs to child workspace.\n";
    print STDERR "Options:\n";
    print STDERR "    -h    print this help\n";
}
