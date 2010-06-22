#!/usr/bin/perl -w
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

#*************************************************************************
#
# cws.pl   - wrap common childworkspace operations
#
use strict;
use Getopt::Long;
use File::Basename;
use File::Path;
use File::Copy;
use Cwd;
use Benchmark;

#### module lookup
my @lib_dirs;
BEGIN {
    if ( !defined($ENV{SOLARENV}) ) {
        die "No environment found (environment variable SOLARENV is undefined)";
    }
    push(@lib_dirs, "$ENV{SOLARENV}/bin/modules");
}
use lib (@lib_dirs);

use Cws;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

#### globals ####

# TODO: replace dummy vales with actual SVN->hg and source_config migration milestones
my $dev300_migration_milestone = 'm64';
my $dev300_source_config_milestone = 'm65';
my $ooo320_migration_milestone = 'm13';
my $ooo320_source_config_milestone = 'm999';

# valid command with possible abbreviations
my @valid_commands = (
                        'help', 'h', '?',
                        'create',
                        'fetch',  'f',
                        'rebase', 'rb',
                        'query', 'q',
                        'task', 't',
                        'integrate',
                        'cdiff', 'cd',
                        'eisclone',
                        'setcurrent'
                     );

# list the valid options to each command
my %valid_options_hash = (
                            'help'       => ['help'],
                            'create'     => ['help', 'milestone', 'migration', 'hg'],
                            'fetch'      => ['help', 'switch', 'milestone', 'childworkspace','platforms','quiet',
                                            'onlysolver'],
                            'rebase'     => ['help', 'milestone','commit'],
                            'query'      => ['help', 'milestone','masterworkspace','childworkspace'],
                            'task'       => ['help'],
                            'integrate'  => ['help', 'childworkspace'],
                            'cdiff'      => ['help', 'childworkspace', 'masterworkspace', 'files', 'modules'],
                            'setcurrent' => ['help', 'milestone'],
                            'eisclone'   => ['help']
                         );

my %valid_commands_hash;
for (@valid_commands) {
    $valid_commands_hash{$_}++;
}

#  set by --debug switch
my $debug = 0;
#  set by --profile switch
my $profile = 0;


#### main ####

my ($command, $args_ref, $options_ref) = parse_command_line();
dispatch_command($command, $args_ref, $options_ref);
exit(0);

#### subroutines ####

# Parses the command line. does prelimiary argument and option verification
sub parse_command_line
{
    if (@ARGV == 0) {
        usage();
        exit(1);
    }

    my %options_hash;
    Getopt::Long::Configure ("no_auto_abbrev", "no_ignorecase");
    my $success = GetOptions(\%options_hash, 'milestone|m=s',
                                             'masterworkspace|master|M=s',
                                             'hg',
                                             'migration',
                                             'childworkspace|child|c=s',
                                             'debug',
                                             'profile',
                                             'commit|C',
                                             'switch|s',
                                             'platforms|p=s',
                                             'onlysolver|o',
                                             'quiet|q',
                                             'files',
                                             'modules',
                                             'help|h'
                            );

    my $command = shift @ARGV;

    if (!exists $valid_commands_hash{$command}) {
        print_error("Unkown command: '$command'\n");
        usage();
        exit(1);
    }

    if ($command eq 'h' || $command eq '?') {
        $command = 'help';
    }
    elsif ($command eq 'f') {
        $command = 'fetch';
    }
    elsif ($command eq 'rb') {
        $command = 'rebase';
    }
    elsif ($command eq 'q') {
        $command = 'query';
    }
    elsif ($command eq 't') {
        $command = 'task';
    }
    elsif ($command eq 'cd') {
        $command = 'cdiff';
    }

    # An unkown option might be accompanied with a valid command.
    # Show the command specific help
    if ( !$success ) {
        do_help([$command])
    }

    verify_options($command, \%options_hash);
    return ($command, \@ARGV, \%options_hash);
}

# Verify options against the valid options list.
sub verify_options
{
    my $command     = shift;
    my $options_ref = shift;

    my $valid_command_options_ref = $valid_options_hash{$command};

    my %valid_command_options_hash;
    foreach (@{$valid_command_options_ref}) {
        $valid_command_options_hash{$_}++;
    }

    # check all specified options against the valid options for the sub command
    foreach (keys %{$options_ref}) {
        if ( /debug/ ) {
            $debug = 1;
            next;
        }
        if ( /profile/ ) {
            $profile = 1;
            next;
        }
        if (!exists $valid_command_options_hash{$_}) {
            print_error("can't use option '--$_' with subcommand '$command'.", 1);
        }
    }

    # TODO here should be specific checks for the arguments
    # if the check is globally valid
}

# Dispatches to the do_xxx() routines depending on command.
sub dispatch_command
{
    my $command     = shift;
    my $args_ref    = shift;
    my $options_ref = shift;

    no strict 'refs';
    &{"do_".$command}($args_ref, $options_ref);
}

# Returns the global cws object.
BEGIN {
my $the_cws;

    sub get_this_cws {
        if (!defined($the_cws)) {
            $the_cws = Cws->new();
            return $the_cws;
        }
        else {
            return $the_cws;
        }
    }
}

# Returns a list of the master workspaces.
sub get_master_workspaces
{
    my $cws = get_this_cws();
    my @masters = $cws->get_masters();

    return wantarray ? @masters : \@masters;
}

# Checks if master argument is a valid MWS name.
BEGIN {
    my %master_hash;

    sub is_master
    {
        my $master_name = shift;

        if (!%master_hash) {
            my @masters = get_master_workspaces();
            foreach (@masters) {
                $master_hash{$_}++;
            }
        }
        return exists $master_hash{$master_name} ? 1 : 0;
    }
}

# Fetches milestone URL for given server and milestone.
sub get_milestone_url
{
    my $server    = shift;
    my $master    = shift;
    my $milestone = shift;

    my $milestone_url = "$server/tags/${master}_${milestone}";
    return $milestone_url;
}

# Fetches CWS URL for given server and CWSname.
sub get_cws_url
{
    my $server = shift;
    my $cws    = shift;

    my $cws_url = "$server/cws/$cws";
    return $cws_url;
}

sub get_master_url
{
    my $server   = shift;
    my $master   = shift;
    my $revision = shift;

    my $url = "${server}/";

    # TODO: update EIS function for subversion
    my $cws = get_this_cws();
    my $trunk = $cws->get_cvs_head();
    if ( $master eq $trunk ) {
        $url .= 'trunk';
    }
    else {
        my $master_label = uc($master);
        $url .= "branches/$master_label";
    }

    # attach revision if needed
    if ( $revision != 0 ) {
        $url .= "\@$revision";
    }
    return $url;
}

# Returns the URL shortened by the server part
sub get_short_url
{
    my $server = shift;
    my $url    = shift;

    my $offset = length("$server/");
    $url = substr($url, $offset);

    return $url;
}


# Fetches the current CWS from environment, returns a Cws object
sub get_cws_from_environment
{
    my $child  = $ENV{CWS_WORK_STAMP};
    my $master = $ENV{WORK_STAMP};

    if ( !$child ) {
        print_error("Environment variable CWS_WORK_STAMP is not set. Please set it to your CWS name.", 2);
    }

    if ( !$master ) {
        print_error("Environment variable WORK_STAMP is not set. Please set it to the MWS name.", 2);
    }

    my $cws = get_this_cws();
    $cws->child($child);
    $cws->master($master);

    # Check if we got a valid child workspace.
    my $id = $cws->eis_id();
    if ( $debug ) {
        print STDERR "CWS-DEBUG: ... master: $master, child: $child, $id\n";
    }
    if ( !$id ) {
        print_error("Child workspace $child for master workspace $master not found in EIS database.", 2);
    }
    return ($cws);
}

# Fetches the CWS by name, returns a Cws object
sub get_cws_by_name
{
    my $child  = shift;

    my $cws = get_this_cws();
    $cws->child($child);

    # Check if we got a valid child workspace.
    my $id = $cws->eis_id();
    if ( $debug ) {
        print STDERR "CWS-DEBUG: child: $child, $id\n";
    }
    if ( !$id ) {
        print_error("Child workspace $child not found in EIS database.", 2);
    }

    # Update masterws part of Cws object.
    my $masterws = $cws->get_mws();
    if ( $cws->master() ne $masterws ) {
        # can this still happen?
        if ( $debug ) {
            print STDERR "CWS-DEBUG: get_cws_by_name(): fixup of masterws in cws object detected\n";
        }
        $cws->master($masterws);
    }
    return ($cws);
}

# Register child workspace with eis.
sub register_child_workspace
{
    my $cws          = shift;
    my $scm          = shift;
    my $is_promotion = shift;

    my $milestone = $cws->milestone();
    my $child     = $cws->child();
    my $master    = $cws->master();

    # TODO: introduce a EIS_USER in the configuration, which should be used here
    my $config = CwsConfig->new();
    my $vcsid  = $config->vcsid();
    # TODO: there is no real need for socustom anymore, should go ASAP
    my $socustom = $config->sointernal();

    if ( !$vcsid ) {
        if ( $socustom ) {
            print_error("Can't determine owner for CWS '$child'. Please set VCSID environment variable.", 11);
        }
        else {
            print_error("Can't determine owner for CWS '$child'. Please set CVS_ID entry in \$HOME/.cwsrc.", 11);
        }
    }

    if ( $is_promotion ) {
        my $rc = $cws->set_scm($scm);
        if ( !$rc ) {
            print_error("Failed to set the SCM property '$scm' on child workspace '$child'.\nContact EIS administrator!\n", 12);
        }

        $rc = $cws->promote($vcsid, "");

        if ( !$rc ) {
            print_error("Failed to promote child workspace '$child' to status 'new'.\n", 12);
        }
        else {
            print "\n***** Successfully ***** promoted child workspace '$child' to status 'new'.\n";
            print "Milestone: '$milestone'.\n";
        }
    }
    else {

        my $eis_id = $cws->register($vcsid, "");

        if ( !defined($eis_id) ) {
            print_error("Failed to register child workspace '$child' for master '$master'.", 12);
        }
        else {
            my $rc = $cws->set_scm($scm);
            if ( !$rc ) {
                print_error("Failed to set the SCM property '$scm' on child workspace '$child'.\nContact EIS administrator!\n", 12);
            }
            print "\n***** Successfully ***** registered child workspace '$child'\n";
            print "for master workspace '$master' (milestone '$milestone').\n";
            print "Child workspace Id: $eis_id.\n";
        }
    }
    return 0;
}

sub print_time_elapsed
{
    my $t_start = shift;
    my $t_stop  = shift;

    my $time_diff = timediff($t_stop, $t_start);
    print_message("... finished in " . timestr($time_diff));
}

sub hgrc_append_push_path_and_hooks
{
    my $target     = shift;
    my $cws_source = shift;

    $cws_source =~ s/http:\/\//ssh:\/\/hg@/;
    if ( $debug ) {
        print STDERR "CWS-DEBUG: hgrc_append_push_path_and_hooks(): default-push path: '$cws_source'\n";
    }
    if ( !open(HGRC, ">>$target/.hg/hgrc") ) {
        print_error("Can't append to hgrc file of repository '$target'.\n", 88);
    }
    print HGRC "default-push = " . "$cws_source\n";
    print HGRC "[extensions]\n";
    print HGRC "hgext.win32text=\n";
    print HGRC "[hooks]\n";
    print HGRC "# Reject commits which would introduce windows-style CR/LF files\n";
    print HGRC "pretxncommit.crlf = python:hgext.win32text.forbidcrlf\n";
    close(HGRC);
}

sub hg_clone_cws_or_milestone
{
    my $rep_type             = shift;
    my $cws                  = shift;
    my $target               = shift;
    my $clone_milestone_only = shift;

    my ($hg_local_source, $hg_lan_source, $hg_remote_source);
    my $config = CwsConfig->new();
    if ( $rep_type eq 'ooo') {
        $hg_local_source = $config->get_ooo_hg_local_source();
        $hg_lan_source = $config->get_ooo_hg_lan_source();
        $hg_remote_source = $config->get_ooo_hg_remote_source();
    }
    else {
        $hg_local_source = $config->get_so_hg_local_source();
        $hg_lan_source = $config->get_so_hg_lan_source();
        $hg_remote_source = $config->get_so_hg_remote_source();
    }

    my $masterws = $cws->master();
    my $master_local_source = "$hg_local_source/" . $masterws;
    my $master_lan_source = "$hg_lan_source/" . $masterws;

    my $milestone_tag;
    if ( $clone_milestone_only ) {
        $milestone_tag = uc($masterws) . '_' . $clone_milestone_only;
    }
    else {
        my @tags = $cws->get_tags();
        $milestone_tag = $tags[3];
    }

    if ( $debug ) {
        print STDERR "CWS-DEBUG: master_local_source: '$master_local_source'\n";
        print STDERR "CWS-DEBUG: master_lan_source: '$master_lan_source'\n";
        if ( !-d $master_local_source ) {
            print STDERR "CWS-DEBUG: not a directory '$master_local_source'\n";
        }
    }

    my $pull_from_remote = 0;
    my $cws_remote_source;
    if ( !$clone_milestone_only ) {
        $cws_remote_source = "$hg_remote_source/cws/" . $cws->child();

        # The outgoing repository might not yet be available. Which is not
        # an error. Since pulling from the cws outgoing URL results in an ugly
        # and hardly understandable error message, we check for availibility
        # first. TODO: incorporate configured proxy instead of env_proxy. Use
        # a dedicated request and content-type to find out if the repo is there
        # instead of parsing the content of the page
        print_message("... check availibility of 'outgoing' repository '$cws_remote_source'.");
        require LWP::Simple;
        my $content = LWP::Simple::get($cws_remote_source);
        my $pattern = "<title>cws/". $cws->child();
        if ( $content =~ /$pattern/ ) {
            $pull_from_remote = 1;
        }
        else {
            print_message("... 'outgoing' repository '$cws_remote_source' is not accessible/available yet.");
        }
    }

    # clone repository (without working tree if we still need to pull from remote)
    my $clone_with_update = !$pull_from_remote;
    hg_clone_repository($master_local_source, $master_lan_source, $target, $milestone_tag, $clone_with_update);

    # now pull from the remote cws outgoing repository if its already available
    if ( $pull_from_remote ) {
        hg_remote_pull_repository($cws_remote_source, $target);
    }

    # if we fetched a CWS adorn the result with push-path and hooks
    if ( $cws_remote_source ) {
        hgrc_append_push_path_and_hooks($target, $cws_remote_source);
    }

    # update the result if necessary
    if ( !$clone_with_update ) {
        hg_update_repository($target);
    }

}

sub hg_clone_repository
{
    my $local_source    = shift;
    my $lan_source    = shift;
    my $dest          = shift;
    my $milestone_tag = shift;
    my $update        = shift;

    my $t1 = Benchmark->new();
    my $source;
    my $clone_option = $update ? '' : '-U ';
    if ( -d $local_source && can_use_hardlinks($local_source, $dest) ) {
        $source = $local_source;
        if ( !hg_milestone_is_latest_in_repository($local_source, $milestone_tag) ) {
                $clone_option .= "-r $milestone_tag";
        }
        print_message("... clone LOCAL repository '$local_source' to '$dest'");
    }
    else {
        $source = $lan_source;
        $clone_option .= "-r $milestone_tag";
        print_message("... clone LAN repository '$lan_source' to '$dest'");
    }
    hg_clone($source, $dest, $clone_option);

    my $t2 = Benchmark->new();
    print_time_elapsed($t1, $t2) if $profile;
}

sub hg_remote_pull_repository
{
    my $remote_source = shift;
    my $dest          = shift;

    my $t1 = Benchmark->new();
    print_message("... pull from REMOTE repository '$remote_source' to '$dest'");
    hg_pull($dest, $remote_source);
    my $t2 = Benchmark->new();
    print_time_elapsed($t1, $t2) if $profile;
}

sub hg_update_repository
{
    my $dest          = shift;

    my $t1 = Benchmark->new();
    print_message("... update repository '$dest'");
    hg_update($dest);
    my $t2 = Benchmark->new();
    print_time_elapsed($t1, $t2) if $profile;
}

sub hg_milestone_is_latest_in_repository
{
    my $repository = shift;
    my $milestone_tag = shift;

    # Our milestone is the lastest thing in the repository
    # if the parent of the repository tip is adorned
    # with the milestone tag.
    my $tags_of_parent_of_tip = hg_parent($repository, 'tip', "--template='{tags}\\n'");
    if ( $tags_of_parent_of_tip =~ /\b$milestone_tag\b/ ) {
        return 1;
    }
    return 0;
}

# Check if clone source and destination are on the same filesystem,
# in that case hg clone can employ hard links.
sub can_use_hardlinks
{
    my $source = shift;
    my $dest = shift;

    if ( $^O eq 'cygwin' ) {
        # no hard links on windows
        return 0;
    }
    # st_dev is the first field return by stat()
    my @stat_source = stat($source);
    my @stat_dest = stat(dirname($dest));

    if ( $debug ) {
        my $source_result = defined($stat_source[0]) ? $stat_source[0] : 'stat failed';
        my $dest_result = defined($stat_dest[0]) ? $stat_dest[0] : 'stat failed';
        print STDERR "CWS-DEBUG: can_use_hardlinks(): source device: '$stat_source[0]', destination device: '$stat_dest[0]'\n";
    }
    if ( defined($stat_source[0]) && defined($stat_dest[0]) && $stat_source[0] == $stat_dest[0] ) {
        return 1;
    }
    return 0;
}

sub query_cws
{
    my $query_mode = shift;
    my $options_ref = shift;
    # get master and child workspace
    my $masterws  = exists $options_ref->{'masterworkspace'} ? uc($options_ref->{'masterworkspace'}) : $ENV{WORK_STAMP};
    my $childws   = exists $options_ref->{'childworkspace'} ? $options_ref->{'childworkspace'} : $ENV{CWS_WORK_STAMP};
    my $milestone = exists $options_ref->{'milestone'} ? $options_ref->{'milestone'} : 'latest';

    if ( !defined($masterws) && $query_mode ne 'masters') {
        print_error("Can't determine master workspace environment.\n", 30);
    }

    if ( ($query_mode eq 'integratedinto' || $query_mode eq 'incompatible' || $query_mode eq 'taskids' || $query_mode eq 'status' || $query_mode eq 'current' || $query_mode eq 'owner' || $query_mode eq 'qarep' || $query_mode eq 'issubversion' || $query_mode eq 'ispublic' || $query_mode eq 'build') && !defined($childws) ) {
        print_error("Can't determine child workspace environment.\n", 30);
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
    return;
}

sub query_integratedinto
{
    my $cws = shift;

    if ( is_valid_cws($cws) ) {
        my $milestone = $cws->get_milestone_integrated();
        print_message("Integrated into:");
        print defined($milestone) ? "$milestone\n" : "unkown\n";
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

sub query_scm
{
    my $cws = shift;
    my $masterws = $cws->master();
    my $childws  = $cws->child();

    if ( is_valid_cws($cws) ) {
        my $scm = $cws->get_scm();
        if ( !defined($scm) ) {
            print_error("Internal error: can't retrieve scm info.", 3);
        } else {
                print_message("Child workspace uses '$scm'.");
        }
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
        print_message("Latest milestone available for rebase:");
        print "$masterws $latest\n";
    }
    else {
        print_error("Can't determine latest milestone of '$masterws' available for rebase.", 3);
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
    print STDERR "Master workspace '$masterws', child workspace '$childws'\n";
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

sub verify_milestone
{
    my $cws = shift;
    my $qualified_milestone = shift;

    my $invalid = 0;
    my ($master, $milestone);
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
        print_error("Milestone '$milestone' is not registered with master workspace '$master'.", 21);
    }
    return ($master, $milestone);
}

sub relink_workspace {
    my $linkdir = shift;
    my $restore = shift;

    # The list of obligatorily added modules, build will not work
    # if these are not present.
    my %added_modules_hash;
    if (defined $ENV{ADDED_MODULES}) {
        for ( split(/\s/, $ENV{ADDED_MODULES}) ) {
            $added_modules_hash{$_}++;
        }
    }

    # clean out pre-existing linkdir
    my $bd = dirname($linkdir);
    if ( !opendir(DIR, $bd) ) {
        print_error("Can't open directory '$bd': $!.", 44);
    }
    my @old_link_dirs = grep { /^src.m\d+/ } readdir(DIR);
    close(DIR);

    if ( @old_link_dirs > 1 ) {
        print_error("Found more than one old link directories:", 0);
        foreach (@old_link_dirs) {
            print STDERR "@old_link_dirs\n";
        }
        if ( $restore ) {
            print_error("Please remove all old link directories but the last one", 67);
        }
    }

    # Originally the extension .lnk indicated a linked module. This turned out to be
    # not an overly smart choice. Cygwin has some heuristics which regards .lnk
    # files as Windows shortcuts, breaking the build. Use .link instead.
    # When in restoring mode still consider .lnk as link to modules (for old CWSs)
    my $old_link_dir = "$bd/" . $old_link_dirs[0];
    if ( $restore ) {
        if ( !opendir(DIR, $old_link_dir) ) {
            print_error("Can't open directory '$old_link_dir': $!.", 44);
        }
        my @links = grep { !(/\.lnk/ || /\.link/)   } readdir(DIR);
        close(DIR);
        # everything which is not a link to a directory can't be an "added" module
        foreach (@links) {
            next if /^\./;
            my $link = "$old_link_dir/$_";
            if ( -s $link && -d $link ) {
                $added_modules_hash{$_} = 1;
            }
        }
    }
    print_message("... removing '$old_link_dir'");
    rmtree([$old_link_dir], 0);

    print_message("... (re)create '$linkdir'");
    if ( !mkdir("$linkdir") ) {
        print_error("Can't create directory '$linkdir': $!.", 44);
    }
    if ( !opendir(DIR, "$bd/ooo") ) {
        print_error("Can't open directory '$bd/sun': $!.", 44);
    }
    my @ooo_top_level_dirs = grep { !/^\./ } readdir(DIR);
    close(DIR);
    if ( !opendir(DIR, "$bd/sun") ) {
        print_error("Can't open directory '$bd/sun': $!.", 44);
    }
    my @so_top_level_dirs = grep { !/^\./ } readdir(DIR);
    close(DIR);
    my $savedir = getcwd();
    if ( !chdir($linkdir) ) {
        print_error("Can't chdir() to directory '$linkdir': $!.", 44);
    }
    my $suffix = '.link';
    foreach(@ooo_top_level_dirs) {
        if ( $_ eq 'REBASE.LOG' || $_ eq 'REBASE.CONFIG_DONT_DELETE'  ) {
            next;
        }
        my $target = $_;
        if ( -d "../ooo/$_" && !exists $added_modules_hash{$_} ) {
            $target .= $suffix;
        }
        if ( !symlink("../ooo/$_", $target) ) {
            print_error("Can't symlink directory '../ooo/$_ -> $target': $!.", 44);
        }
    }
    foreach(@so_top_level_dirs) {
        if ( $_ eq 'REBASE.LOG' || $_ eq 'REBASE.CONFIG_DONT_DELETE'  ) {
            next;
        }
        my $target = $_;
        if ( -d "../sun/$_" && !exists $added_modules_hash{$_} ) {
            $target .= $suffix;
        }
        if ( !symlink("../sun/$_", $target) ) {
            print_error("Can't symlink directory '../sun/$_ -> $target': $!.", 44);
        }
    }
    if ( !chdir($savedir) ) {
        print_error("Can't chdir() to directory '$linkdir': $!.", 44);
    }
}

sub fetch_external_tarballs
{
    my $source_root_dir = shift;
    my $external_tarballs_source = shift;

    my $ooo_external_file = "$source_root_dir/ooo/ooo.lst";
    my $sun_external_file = "$source_root_dir/sun/sun.lst";
    my $sun_path          = "$source_root_dir/sun";

    my @external_sources_list;
    push(@external_sources_list, read_external_file($ooo_external_file));
    if ( -d $sun_path ) {
        if ( -e $sun_external_file ) {
            push(@external_sources_list, read_external_file($sun_external_file));
        }
        else {
            print_error("Can't find external file list '$sun_external_file'.", 8);
        }
    }

    my $ext_sources_dir = "$source_root_dir/ext_sources";
    print_message("Copy external tarballs to '$ext_sources_dir'");
    if ( ! -d $ext_sources_dir) {
        if ( !mkdir($ext_sources_dir) ) {
            print_error("Can't create directory '$ext_sources_dir': $!.", 44);
        }
    }
    foreach (@external_sources_list) {
        if ( ! copy("$external_tarballs_source/$_", $ext_sources_dir) ) {
            print_error("Can't copy file '$external_tarballs_source' -> '$ext_sources_dir': $!", 0);
        }
    }
    return;
}

sub read_external_file
{
    my $external_file = shift;

    my @external_sources;
    open(EXT, "<$external_file") or print_error("Can't open file '$external_file' for reading: $!", 98);
    while(<EXT>) {
        if ( !/^http:/ ) {
            chomp;
            push(@external_sources, $_);
        }
    }
    close(EXT);
    return @external_sources;
}

sub update_solver
{
    my $platform      = shift;
    my $source        = shift;
    my $solver        = shift;
    my $milestone     = shift;
    my $source_config = shift;

    my @zip_sub_dirs = ('bin', 'doc', 'idl', 'inc', 'lib', 'par', 'pck', 'pdb', 'pus', 'rdb', 'res', 'xml', 'sdf');

    use Archive::Zip qw( :ERROR_CODES :CONSTANTS );

    my $platform_solver = "$solver/$platform";

    if ( -d $platform_solver ) {
        print_message("... removing old solver for platform '$platform'");
        if ( !rmtree([$platform_solver]) ) {
            print_error("Can't remove directory '$platform_solver': $!.", 44);
        }
    }

    if ( !mkdir("$platform_solver") ) {
        print_error("Can't create directory '$platform_solver': $!.", 44);
    }

    my $platform_source = "$source/$platform/zip.$milestone";
    if ( !opendir(DIR, "$platform_source") ) {
        print_error("Can't open directory '$platform_source': $!.", 44);
    }
    my @zips = grep { /\.zip$/ } readdir(DIR);
    close(DIR);

    my $nzips = @zips;
    print_message("... unzipping $nzips zip archives for platform '$platform'");


    foreach(@zips) {
        my $zip = Archive::Zip->new();
        unless ( $zip->read( "$platform_source/$_" ) == AZ_OK ) {
            print_error("Can't read zip file '$platform_source/$_': $!.", 44);
        }
        # TODO: check for erorrs
        foreach (@zip_sub_dirs) {
            my $extract_destination = $source_config ? "$platform_solver/$_" : "$platform_solver/$_.$milestone";
            unless ( $zip->extractTree($_, $extract_destination) == AZ_OK ) {
                print_error("Can't extract stream from zip file '$platform_source/$_': $!.", 44);
            }
        }
     }
}

sub write_rebase_configuration
{
    my $workspace = shift;
    my $cwsname   = shift;
    my $master    = shift;
    my $milestone = shift;

    my $rebase_config = "$workspace/REBASE.CONFIG_DONT_DELETE";

    open(REBASE, ">$rebase_config") or print_error("Can't open file '$rebase_config' for writing: $!", 98);
    print REBASE "CWS-TOOLING: do not delete this file, it's needed for 'cws rebase -C'\n";
    print REBASE "CWS: $cwsname\n";
    print REBASE "New MWS: $master\n";
    print REBASE "New milestone: $milestone\n";
    close(REBASE);
}

sub read_rebase_configuration
{
    my $workspace = shift;

    my $rebase_config = "$workspace/REBASE.CONFIG_DONT_DELETE";

    my $master;
    my $milestone;

    open(REBASE, "<$rebase_config") or print_error("Can't open file '$rebase_config' for reading: $!", 98);
    while(<REBASE>) {
        if ( /New MWS: (\w+)/ ) {
            $master = $1;
        }
        if ( /New milestone: (\w+)/ ) {
            $milestone = $1;
        }
    }
    close(REBASE);

    if ( !defined($master) || !defined($milestone) ) {
        print_error("File '$rebase_config' seems to be garbled. Can't continue.", 98)
    }

    return ($master, $milestone);
}

sub diff_print_files
{
    my $files_ref = shift;
    my $diff_options = shift;

    my @files = sort(@{$files_ref});

    if ( $diff_options eq 'files') {
        foreach(@files) {
            print "$_\n";
        }
    }
    else {
        my @modules;
        foreach(@files) {
            my ($module) = split(/\//, $_);
            push(@modules, $module);
        }
        # remove adjacent uniques
        my $prev = 'nosuchmodule';
        my @unique_modules = grep($_ ne $prev && (($prev) = $_), @modules);
        foreach(@unique_modules) {
            print "$_\n";
        }
    }
}

# TODO: special provisions for SVN->HG migrations, remove this
# some time after migration
sub get_scm_for_milestone
{
    my $masterws = shift;
    my $milestone = shift;

    my $milestone_sequence_number = extract_milestone_sequence_number($milestone);
    my $dev300_migration_sequence_number = extract_milestone_sequence_number($dev300_migration_milestone);
    my $ooo320_migration_sequence_number = extract_milestone_sequence_number($ooo320_migration_milestone);

    my $scm = 'SVN';

    if ( $masterws eq 'DEV300' ) {
        if ( $milestone_sequence_number >= $dev300_migration_sequence_number ) {
            $scm = 'HG';
        }
    }
    elsif ( $masterws eq 'OOO320' ) {
        if ( $milestone_sequence_number >= $ooo320_migration_sequence_number ) {
            $scm = 'HG';
        }
    }
    else {
        $scm = 'SVN';
    }
    return $scm;
}

# TODO: special provisions for "source_config" migration, remove this
# some time after migration
sub get_source_config_for_milestone
{
    my $masterws = shift;
    my $milestone = shift;

    my $milestone_sequence_number = extract_milestone_sequence_number($milestone);
    my $dev300_migration_sequence_number = extract_milestone_sequence_number($dev300_source_config_milestone);
    my $ooo320_migration_sequence_number = extract_milestone_sequence_number($ooo320_source_config_milestone);

    my $source_config = 0;

    if ( $masterws eq 'DEV300' ) {
        if ( $milestone_sequence_number >= $dev300_migration_sequence_number ) {
            $source_config = 1;
        }
    }
    elsif ( $masterws eq 'OOO320' ) {
        if ( $milestone_sequence_number >= $ooo320_migration_sequence_number ) {
            $source_config = '1';
        }
    }
    else {
        $source_config = 0;
    }
    return $source_config;
}

sub extract_milestone_sequence_number
{
    my $milestone = shift;

    my $milestone_sequence_number;
    if ( $milestone =~ /m(\d+)/ ) {
        $milestone_sequence_number = $1;
    }
    else {
        print_error("can't extract milestone sequence number from milestone '$milestone'", 99);
    }
    return $milestone_sequence_number;
}

# Executes the help command.
sub do_help
{
    my $args_ref    = shift;
    my $options_ref = shift;

    if (@{$args_ref} == 0) {
        print STDERR "usage: cws <subcommand> [options] [args]\n";
        print STDERR "Type 'cws help <subcommand>' for help on a specific subcommand.\n";
        print STDERR "\n";
        print STDERR "Available subcommands:\n";
        print STDERR "\thelp (h,?)\n";
        print STDERR "\tcreate\n";
        print STDERR "\tfetch (f)\n";
        print STDERR "\trebase (rb) (SVN only)\n";
        print STDERR "\tquery (q)\n";
        print STDERR "\ttask (t)\n";
        print STDERR "\tcdiff (cd) (SVN only)\n";
        print STDERR "\tsetcurrent\n";
        print STDERR "\tintegrate *** release engineers only ***\n";
        print STDERR "\teisclone *** release engineers only ***\n";
    }

    my $arg = $args_ref->[0];

    if (!defined($arg) || $arg eq 'help') {
        print STDERR "help (h, ?): Describe the usage of this script or its subcommands\n";
        print STDERR "usage: help [subcommand]\n";
    }
    elsif ($arg eq 'create') {
        print STDERR "create: Create a new child workspace\n";
        print STDERR "usage: create [--hg] [-m milestone] <master workspace> <child workspace>\n";
        print STDERR "\t-m milestone:          Milestone to base the child workspace on. If ommitted the\n";
        print STDERR "\t                       last published milestone will be used.\n";
        print STDERR "\t--milestone milestone: Same as -m milestone.\n";
        print STDERR "\t--hg:                  Create Mercurial (hg) based CWS.\n";
        print STDERR "\t--migration:           Used only for the migration of an exitisting CWS from CVS to SVN.\n";
        print STDERR "\t                       Disables existence check in EIS, creates CWS branch in SVN, sets SVN flag.\n";
    }
    elsif ($arg eq 'task') {
        print STDERR "task: Add a task to a child workspace\n";
        print STDERR "usage: task <task id> [task id ...]\n";
    }
    elsif ($arg eq 'query') {
        print STDERR "query: Query child workspace for miscellaneous information\n";
        print STDERR "usage: query [-M master] [-c child] <current|integratedinto|incompatible|owner|qarep|status|taskids>\n";
        print STDERR "       query [-M master] [-c child] <release|due|due_qa|help|ui|ispublic|scm|build>\n";
        print STDERR "       query [-M master] <latest|milestones|ispublicmaster>\n";
        print STDERR "       query  <masters>\n";
        print STDERR "       query [-M master] [-m milestone] <integrated|buildid>\n";
        print STDERR "       query [-M master] <planned|new|approved|nominated|ready>\n";
        print STDERR "\t-M master:\t\toverride MWS specified in environment\n";
        print STDERR "\t-c child:\t\toverride CWS specified in environment\n";
        print STDERR "\t-m milestone:\t\toverride latest milestone with specified one\n";
        print STDERR "\t--master master:\tSame as -M master\t\n";
        print STDERR "\t--child child:\t\tSame -c child\n";
        print STDERR "\t--milestone milestone:\tSame as -m milestone\n";
        print STDERR "Modes:\n";
        print STDERR "\tcurrent\t\tquery current milestone of CWS\n";
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
        print STDERR "\tintegratedinto\tquery milestone which CWS was integrated into\n";
        print STDERR "\tplanned\t\tquery for planned CWSs\n";
        print STDERR "\tnew\t\tquery for new CWSs\n";
        print STDERR "\tapproved\tquery CWSs approved by QA\n";
        print STDERR "\tnominated\tquery nominated CWSs\n";
        print STDERR "\tready\t\tquery CWSs ready for QA\n";
        print STDERR "\tispublic\tquery public flag of CWS\n";
        print STDERR "\tscm\t\tquery Source Control Management (SCM) system used for CWS\n";
        print STDERR "\tmasters\t\tquery available MWS\n";
        print STDERR "\tmilestones\tquery which milestones are know on the given MWS\n";
        print STDERR "\tispublicmaster\tquery public flag of MWS\n";

     }
    elsif ($arg eq 'fetch') {
        print STDERR "fetch: fetch a milestone or CWS\n";
        print STDERR "usage: fetch [-q] [-s] [-p platforms] [-o] <-m milestone> <workspace>\n";
        print STDERR "usage: fetch [-q] [-s] [-p platforms] [-o] <-c cws> <workspace>\n";
        print STDERR "usage: fetch [-q] [-s] <-m milestone> <workspace>\n";
        print STDERR "usage: fetch [-q] [-s] <-c cws> <workspace>\n";
        print STDERR "\t-m milestone:           Checkout milestone <milestone> to workspace <workspace>\n";
        print STDERR "\t                        Use 'latest' for the for lastest published milestone on the current master\n";
        print STDERR "\t                        For cross master checkouts use the form <MWS>:<milestone>\n";
        print STDERR "\t--milestone milestone:  Same as -m milestone\n";
        print STDERR "\t-c childworkspace:      Checkout CWS <childworkspace> to workspace <workspace>\n";
        print STDERR "\t--child childworkspace: Same as -c childworkspace\n";
        print STDERR "\t-s:                     Try to switch an existing workspace <workspace> to milestone or CWS\n";
        print STDERR "\t--switch:               Same as -s\n";
        print STDERR "\t-p platform:            Copy one or more prebuilt platforms 'platform'. \n";
        print STDERR "\t                        Separate multiple platforms with commas.\n";
        print STDERR "\t--platforms platform:   Same as -p\n";
        print STDERR "\t-o                      Omit checkout of sources, copy only solver. \n";
        print STDERR "\t--onlysolver:           Same as -o\n";
        print STDERR "\t-q                      Silence some of the output of the command.\n";
        print STDERR "\t--quiet:                Same as -q\n";
    }
    elsif ($arg eq 'rebase') {
        print STDERR "rebase: Rebase a child workspace to a new milestone (SVN only)\n";
        print STDERR "usage: rebase <-m milestone> <workspace>\n";
        print STDERR "usage: rebase <-C> <workspace>\n";
        print STDERR "\t-m milestone:          Merge changes on MWS into CWS up to and including milestone <milestone>\n";
        print STDERR "\t                       Use 'latest' for the for lastest published milestone on the current master\n";
        print STDERR "\t                       For cross master rebases use the form <MWS>:<milestone>\n";
        print STDERR "\t--milestone milestone: Same as -m milestone\n";
        print STDERR "\t-C:                    Commit changes made by merge step and update current milestone in database\n";
        print STDERR "\t--commit:              Same as -C\n"
    }
    elsif ($arg eq 'integrate') {
        print STDERR "integrate: Integrate a child workspace into a master workspace\n";
        print STDERR "usage: integrate <-c childworkspace>\n";
        print STDERR "usage: integrate <-C>\n";
        print STDERR "\t-c childworkspace:      Merge changes on CWS <childworkspace> into MWS\n";
        print STDERR "\t--child childworkspace: Same as -c childworkspace\n";
        print STDERR "\t-C:                     Commit changes made by merge step and update CWS status in database\n";
        print STDERR "\t--commit:               Same as -C\n"
    }
    elsif ($arg eq 'cdiff') {
        print STDERR "cdiff: Show changes on CWS relative to current milestone (SVN only)\n";
        print STDERR "usage: cdiff [-M master] [-c child] [--files] [--modules]\n";
        print STDERR "\t-M master:\t\toverride MWS specified in environment\n";
        print STDERR "\t-c child:\t\toverride CWS specified in environment\n";
        print STDERR "\t--master master:\tSame as -M master\t\n";
        print STDERR "\t--child child:\t\tSame -c child\n";
        print STDERR "\t--files:                Print only file names\n";
        print STDERR "\t--modules:              Print only top level directories aka modules\n"
    }
    elsif ($arg eq 'setcurrent') {
        print STDERR "setcurrent: Set the current milestone for the CWS (only hg based CWSs)\n";
        print STDERR "usage: setcurrent [-m milestone]\n";
        print STDERR "\t-m milestone:           Set milestone to <milestone> to workspace <workspace>\n";
        print STDERR "\t                        Use 'latest' for the for lastest published milestone on the current master\n";
        print STDERR "\t                        For cross master change use the form <MWS>:<milestone>\n";
        print STDERR "\t--milestone milestone:  Same as -m milestone\n";
    }
    else {
        print STDERR "'$arg': unknown subcommand\n";
        exit(1);
    }
    exit(0);
}

# Executes the create command.
sub do_create
{
    my $args_ref    = shift;
    my $options_ref = shift;

    if ( exists $options_ref->{'help'} || @{$args_ref} != 2) {
        do_help(['create']);
    }

    my $is_migration = 0;
    if ( exists $options_ref->{'migration'} ) {
        $is_migration = 1;
    }

    my $is_hg = 0;
    if ( exists $options_ref->{'hg'} ) {
        $is_hg = 1;
    }

    my $master   = uc $args_ref->[0];
    my $cws_name = $args_ref->[1];

    if (!is_master($master)) {
        print_error("'$master' is not a valid master workspace.", 7);
    }

    # check if cws name fits the convention
    if ( $cws_name !~ /^\w[\w\.\#]*$/ ) {
        print_error("Invalid child workspace name '$cws_name'.\nCws names should consist of alphanumeric characters, preferable all lowercase and starting with a letter.\nThe characters . and # are allowed if they are not the first character.", 7);
    }

    my $cws = get_this_cws();
    $cws->master($master);
    $cws->child($cws_name);

    # check if child workspace already exists
    my $eis_id = $cws->eis_id();
    if ( !defined($eis_id) ) {
        print_error("Connection with EIS database failed.", 8);
    }

    my $is_promotion = 0;
    if ( $eis_id > 0 ) {
        if ( $cws->get_approval() eq 'planned' ) {
            print "Promote child workspace '$cws_name' from 'planned' to 'new'.\n";
            $is_promotion++;
        }
        else {
            if ( $is_migration ) {
                print_message("Create CWS branch in Subversion for migrating CWS '$cws_name' from CVS.");
            }
            else {
                print_error("Child workspace '$cws_name' already exists.", 7);
            }
        }
    }
    else {
        # check if child workspace name is still available
        if ( !$cws->is_cws_name_available()) {
            print_error("Child workspace name '$cws_name' is already in use.", 7);
        }
    }

    my $milestone;
    # verify milestone or query latest milestone
    if ( exists $options_ref->{'milestone'} ) {
        $milestone=$options_ref->{'milestone'};
        # check if milestone exists
        if ( !$cws->is_milestone($master, $milestone) ) {
            print_error("Milestone '$milestone' is not registered with master workspace '$master'.", 8);
        }
    }
    else {
        $milestone=$cws->get_current_milestone($cws->master());
    }

    # set milestone
    $cws->milestone($milestone);

    # handle mercurial(hg) based CWSs
    if ( $is_hg ) {
        register_child_workspace($cws, 'hg', $is_promotion);
        return;
    }

    # Refuse to create Subversion hosted cildworkspaces after
    # migration milestone
    my $milestone_scm = get_scm_for_milestone($cws->master(), $cws->milestone());
    if ( $milestone_scm eq 'HG' ) {
        print_error("This code line has been migrated to Mercurial.", 0);
        print_error("Please use the '--hg' option to create a Mercurial hosted CWS.", 8);
    }

    my $config = CwsConfig->new();
    my $ooo_svn_server = $config->get_ooo_svn_server();
    my $so_svn_server = $config->get_so_svn_server();

    if (!defined($ooo_svn_server)) {
        print_error("No OpenOffice.org SVN server defined, please check your configuration file.", 8);
    }

    my $ooo_milestone_url = get_milestone_url($ooo_svn_server, $cws->master(), $milestone);
    my $ooo_cws_url = get_cws_url($ooo_svn_server, $cws_name);

    my $so_milestone_url;
    my $so_cws_url;
    if ( defined($so_svn_server) ) {
        $so_milestone_url = get_milestone_url($so_svn_server, $cws->master(), $milestone);
        $so_cws_url = get_cws_url($so_svn_server, $cws_name);
    }

    # There is a slight chance that the cws creation was interrupted before registration before.
    # Check for potential remains in the repository
    my $ooo_path_exists = 0;
    my $so_path_exists = 0;

    print STDERR "... check cws path:\t'$ooo_cws_url'";
    if ( svn_path_exists($ooo_cws_url) ) {
        $ooo_path_exists=1;
        print STDERR "\n";
    }
    else {
        print STDERR ", OK\n";
    }

    if ( defined($so_svn_server) ) {
        print STDERR "... check cws path:\t'$so_cws_url'";
        if  ( svn_path_exists($so_cws_url) ) {
            print STDERR "\n";
            $so_path_exists = 1;
        }
        else {
            print STDERR ", OK\n";
        }
    }

    if ( $ooo_path_exists ) {
        print_error("SVN path '$ooo_cws_url' already exists.\nThis can happen if a previous CWS creation attempt failed before registering the CWS with EIS.\nIf this is the case, please delete the path with:\n\t svn delete -m'CWS-TOOLING: undo broken CWS creation' $ooo_cws_url\n", 0);
    }

    if ( $so_path_exists ) {
        print_error("SVN path '$so_cws_url' already exists.\nThis can happen if a previous CWS creation attempt failed before registering the CWS with EIS.\nIf this is the case, please delete the path with:\n\t svn delete -m'CWS-TOOLING: undo broken CWS creation' $so_cws_url\n", 0);
    }

    if ( $ooo_path_exists || $so_path_exists ) {
        exit(15);
    }

    # determine the revision from which the milestone was copied
    my $ooo_milestone_revision;
    my $so_milestone_revision;

    $ooo_milestone_revision = svn_milestone_revision($ooo_milestone_url);
    if ( !$ooo_milestone_revision ) {
        print_error("Can't retrieve revision for milestone '$milestone', url '$ooo_milestone_url.", 17 );
    }
    if ( defined($so_svn_server) ) {
        $so_milestone_revision = svn_milestone_revision($so_milestone_url);
        if ( !$so_milestone_revision ) {
            print_error("Can't retrieve revision for milestone '$milestone', url '$so_milestone_url.", 17 );
        }
    }

    my $ooo_master_url;
    my $so_master_url;

    $ooo_master_url = get_master_url($ooo_svn_server, $cws->master(), $ooo_milestone_revision);
    if ( defined($so_svn_server) ) {
        $so_master_url = get_master_url($so_svn_server, $cws->master(), $so_milestone_revision);
    }

    my $ooo_short_url = get_short_url($ooo_svn_server, $ooo_master_url);
    my $ooo_creation_comment = "CWS-TOOLING: create CWS " . $cws->child() . " from $ooo_short_url (milestone: " . $cws->master() . ":$milestone)";
    # create branches an ooo server and an optional so server
    print STDERR "... create branch:\t'$ooo_cws_url'";
    svn_copy($ooo_creation_comment, $ooo_master_url, $ooo_cws_url);
    if ( defined($so_svn_server) ) {
        my $so_short_url = get_short_url($so_svn_server, $so_master_url);
        my $so_creation_comment = "CWS-TOOLING: create CWS " . $cws->child() . " from $so_short_url (milestone: " . $cws->master() . ":$milestone)";
        print STDERR "... create branch:\t'$so_cws_url'";
        svn_copy($so_creation_comment, $so_master_url, $so_cws_url);
    }

    if ( $is_migration ) {
        # Set master and milestone
        $cws->master($master);
        $cws->milestone($milestone);
        my $rc = $cws->set_subversion_flag(1);
        if ( !$rc ) {
            print_error("Failed to set subversion flag on child workspace '$cws_name'.\nContact EIS administrator!\n", 12);
        }
    }
    else {
        register_child_workspace($cws, 'svn', $is_promotion);
    }
    return;
}

sub do_rebase
{
    my $args_ref    = shift;
    my $options_ref = shift;

    my $commit_phase = 0;
    my $milestone;

    # TODO: Switching to a new master dooes work not correctly yet

    if (exists $options_ref->{'help'} || @{$args_ref} != 1) {
        do_help(['rebase']);
    }
    if ( exists($options_ref->{'commit'}) && exists($options_ref->{'milestone'}) ) {
        print_error("Option -m (--milestone) and -C (--commit) are mutually exclusive.", 0 );
        do_help(['rebase']);
    }
    if ( !exists($options_ref->{'commit'}) && !exists($options_ref->{'milestone'}) ) {
        print_error("At least one of the options -m (--milestone) or -C (--commit) are required.", 0 );
        do_help(['rebase']);
    }

    if ( !svn_version_check() ) {
        print_error("cws rebase requires svn-1.5.4 or later (merge tracking and bug fixes). Please upgrade your svn client.", 1);
    }

    my $cws = get_cws_from_environment();

    my $old_masterws = $cws->master();
    my $new_masterws;
    my $new_milestone;

    my $workspace = $args_ref->[0];

    if ( ! -d $workspace ) {
        print_error("Can't find workspace '$workspace': $!", 99);
    }

    if ( exists($options_ref->{'commit'}) ) {
        $commit_phase=1;
        ($new_masterws, $new_milestone) = read_rebase_configuration($workspace);
    }
    elsif( exists($options_ref->{'milestone'}) ) {
        $milestone = $options_ref->{'milestone'};
        if ( $milestone eq 'latest' ) {
            my $latest = $cws->get_current_milestone($old_masterws);

            if ( !$latest ) {
                print_error("Can't determine latest milestone of '$old_masterws' available for rebase.", 22);
            }
            $new_masterws  = $old_masterws;
            $new_milestone = $latest;
        }
        else {
            ($new_masterws, $new_milestone) =  verify_milestone($cws, $milestone);
        }
    }
    else {
        do_help(['rebase']);
    }

    if ( $cws->get_scm() eq 'HG' ) {
        my $child = $cws->child();
        print_error("cws rebase is not supported for mercurial based childworkspaces", 0);
        print_error("re-synchronize your CWS with:", 0);
        print_error("hg pull <master>", 0);
        print_error("hg merge", 0);
        print_error("hg commit -m\"$child: merge with $new_masterws $new_milestone\"", 0);
        print_error("and update EIS with:", 0);
        print_error("cws setcurrent -m $new_milestone", 99);
    }

    my $so_setup = 0;
    my $ooo_path;
    my $so_path;
    # Determine if we got a three directory (so) setup or a plain (ooo) setup.
    # This is only needed as long the build system still relies
    # on having "modules" from different repositories in the same
    # directory besides each other.
    if ( -d "$workspace/$old_masterws/sun" ) {
        $so_setup = 1;
        $ooo_path = "$workspace/$old_masterws/ooo";
        $so_path = "$workspace/$old_masterws/sun";
    }
    else {
        $ooo_path = "$workspace";
    }

    my $config = CwsConfig->new();
    my $ooo_svn_server = $config->get_ooo_svn_server();
    my $so_svn_server = $config->get_so_svn_server();

    if (!defined($ooo_svn_server)) {
        print_error("No OpenOffice.org SVN server defined, please check your configuration file.", 8);
    }

    my $ooo_milestone_url = get_milestone_url($ooo_svn_server, $new_masterws, $new_milestone);
    my $ooo_cws_url = get_cws_url($ooo_svn_server, $cws->child());

    my $so_milestone_url;
    my $so_cws_url;
    if ( $so_setup ) {
        $so_milestone_url = get_milestone_url($so_svn_server, $new_masterws, $new_milestone);
        $so_cws_url = get_cws_url($so_svn_server, $cws->child());
    }

    my $ooo_milestone_revision;
    my $so_milestone_revision;

    $ooo_milestone_revision = svn_milestone_revision($ooo_milestone_url);
    if ( !$ooo_milestone_revision ) {
        print_error("Can't retrieve revision for milestone '$new_milestone', url '$ooo_milestone_url.", 17 );
    }
    if ( defined($so_svn_server) ) {
        $so_milestone_revision = svn_milestone_revision($so_milestone_url);
        if ( !$so_milestone_revision ) {
            print_error("Can't retrieve revision for milestone '$new_milestone', url '$so_milestone_url.", 17 );
        }
    }

    my $ooo_master_url;
    my $so_master_url;

    $ooo_master_url = get_master_url($ooo_svn_server, $new_masterws, $ooo_milestone_revision);
    if ( defined($so_svn_server) ) {
        $so_master_url = get_master_url($so_svn_server, $new_masterws, $so_milestone_revision);
    }

    if ( $commit_phase ) {
        # commit
        print_message("... committing merged changes to workspace '$workspace'.");
        my $ooo_short_url = get_short_url($ooo_svn_server, $ooo_master_url);
        my $commit_message = "CWS-TOOLING: rebase CWS " . $cws->child() . " to $ooo_short_url (milestone: " . $new_masterws . ":$new_milestone)";
        svn_commit($ooo_path, $commit_message);
        if ( $so_setup ) {
            my $so_short_url = get_short_url($so_svn_server, $so_master_url);
            $commit_message = "CWS-TOOLING: rebase CWS " . $cws->child() . " to $so_short_url (milestone: " . $new_masterws . ":$new_milestone)";
            svn_commit($so_path, $commit_message);
        }
        if ( $so_setup) {
            print_message("... rename '$workspace/$old_masterws' -> '$workspace/$new_masterws'\n");
            if ( !rename("$workspace/$old_masterws", "$workspace/$new_masterws") ) {
                print_error("Can't rename '$workspace/$old_masterws' -> '$workspace/$new_masterws': $!", 98);
            }
            print_message("... relinking workspace\n");
            relink_workspace("$workspace/$new_masterws/src.$new_milestone", 1);
            if ( !unlink("$workspace/REBASE.CONFIG_DONT_DELETE") ) {
                print_error("Can't unlink '$workspace/REBASE.CONFIG_DONT_DELETE': $!", 0);
            }

        }

        print_message("... updating EIS database");
        my $push_return = $cws->set_master_and_milestone($new_masterws, $new_milestone);
        # sanity check
        if ( $$push_return[1] ne $new_milestone) {
            print_error("Couldn't push new milestone '$new_milestone' to database", 0);
        }
    }
    else {
        # merge phase

        # check if working directory is switched to the right cws branch
        my $ooo_wc_url;
        my $so_wc_url;
        my $cwsname = $cws->child();
        print_message("... verifying if  workspace '$workspace' is switched to CWS '$cwsname'.");
        $ooo_wc_url = svn_wc_url($ooo_path);
        if ( $ooo_wc_url !~ /\/$cwsname$/ ) {
            print_error("Your working copy '$ooo_path' is not switched to the cws branch.\nPlease fix and restart rebasing.", 24);
        }
        if ( $so_setup ) {
            $so_wc_url = svn_wc_url($so_path);

            if ( $so_wc_url !~ /\/$cwsname$/ ) {
                print_error("Your working copy '$so_path' is not switched to the cws branch.\nPlease fix and restart rebasing.", 24);
            }
        }
        # check for mixed revisions, locally modified files etc
        if ( !svn_wc_is_clean($ooo_path) || ($so_setup && !svn_wc_is_clean($so_path)) ) {
            print_error("Please fix and restart rebasing.", 25);
        }

        print_message("... merging changes up to '$new_masterws:$new_milestone' to workspace '$workspace'.");
        svn_merge($ooo_milestone_url, $ooo_path);
        if ( $so_setup ) {
            svn_merge($so_milestone_url, $so_path);
        }
        # write out the rebase configuration to store new milestone and master information
        write_rebase_configuration($workspace, $cwsname, $new_masterws, $new_milestone);
    }
}

sub do_integrate
{
    my $args_ref    = shift;
    my $options_ref = shift;

    if (exists $options_ref->{'help'} || @{$args_ref} > 0) {
        do_help(['integrate']);
    }
    if ( exists($options_ref->{'commit'}) && exists($options_ref->{'childworkspace'}) ) {
        print_error("Option -c (--child) and -C (--commit) are mutually exclusive.", 0 );
        do_help(['integrate']);
    }
}

# Executes the fetch command.
sub do_fetch
{
    my $args_ref    = shift;
    my $options_ref = shift;

    my $time_fetch_start = Benchmark->new();
    if ( exists $options_ref->{'help'} || @{$args_ref} != 1) {
        do_help(['fetch']);
    }

    my $milestone_opt = $options_ref->{'milestone'};
    my $child = $options_ref->{'childworkspace'};
    my $platforms = $options_ref->{'platforms'};
    my $quiet  = $options_ref->{'quiet'}  ? 1 : 0 ;
    my $switch = $options_ref->{'switch'} ? 1 : 0 ;
    my $onlysolver = $options_ref->{'onlysolver'} ? 1 : 0 ;

    if ( !defined($milestone_opt) && !defined($child) ) {
        print_error("Specify one of these options: -m or -c", 0);
        do_help(['fetch']);
    }

    if ( defined($milestone_opt) && defined($child) ) {
        print_error("Options -m and -c are mutally exclusive", 0);
        do_help(['fetch']);
    }

    if ( defined($platforms) && $switch ) {
        print_error("Option '-p' is not usuable with Option '-s'.", 0);
        do_help(['fetch']);
    }

    if ( $onlysolver && !defined($platforms) ) {
        print_error("Option '-o' is Only usuable combination with option '-p'.", 0);
        do_help(['fetch']);
    }

    my $cws = get_this_cws();
    my $masterws = $ENV{WORK_STAMP};
    if ( !defined($masterws) ) {
        print_error("Can't determine current master workspace: check environment variable WORK_STAMP", 21);
    }
    $cws->master($masterws);
    my $milestone;
    my $scm;
    if( defined($milestone_opt) ) {
        if ( $milestone_opt eq 'latest' ) {
            $cws->master($masterws);
            my $latest = $cws->get_current_milestone($masterws);

            if ( !$latest ) {
                print_error("Can't determine latest milestone of master workspace '$masterws'.", 22);
            }
            $milestone = $cws->get_current_milestone($masterws);
        }
        else {
            ($masterws, $milestone) =  verify_milestone($cws, $milestone_opt);
        }
        $scm = get_scm_for_milestone($masterws, $milestone);
    }
    elsif ( defined($child) ) {
        $cws = get_cws_by_name($child);
        $masterws = $cws->master(); # CWS can have another master than specified in ENV
        $milestone = $cws->milestone();
        $scm = $cws->get_scm();
    }
    else {
        do_help(['fetch']);
    }

    if ( $switch && $scm eq 'HG' ) {
        print_error("Option '-s' is not supported with a hg based CWS.", 0);
        do_help(['fetch']);
    }

    if ( $debug ) {
        print STDERR "CWS-DEBUG: SCM: $scm\n";
    }

    my $config = CwsConfig->new();
    my $ooo_svn_server = $config->get_ooo_svn_server();
    my $so_svn_server = $config->get_so_svn_server();
    my $prebuild_dir = $config->get_prebuild_binaries_location();
    my $external_tarball_source = $prebuild_dir;
    # Check early for platforms so we can bail out before anything time consuming is done
    # in case of a missing platform
    my @platforms;
    if ( defined($platforms) ) {
        use Archive::Zip; # warn early if module is missing
        if ( !defined($prebuild_dir ) ) {
            print_error("PREBUILD_BINARIES not configured, can't find platform solvers", 99);
        }
        $prebuild_dir = "$prebuild_dir/$masterws";

        @platforms = split(/,/, $platforms);

        my $added_product = 0;
        my $added_nonproduct = 0;
        foreach(@platforms) {
            if ( $_ eq 'common.pro' ) {
                $added_product = 1;
                print_warning("'$_' is added automatically to the platform list, don't specify it explicit");
            }
            if ( $_ eq 'common' ) {
                $added_nonproduct = 1;
                print_warning("'$_' is added automatically to the platform list, don't specify it explicit");
            }
        }

        # add common.pro/common to platform list
        if ( $so_svn_server ) {
            my $product = 0;
            my $nonproduct = 0;
            foreach(@platforms) {
                if ( /\.pro$/ ) {
                    $product = 1;
                }
                else {
                    $nonproduct = 1;
                }
            }
            unshift(@platforms, 'common.pro') if ($product && !$added_product);
            unshift(@platforms, 'common') if ($nonproduct && !$added_nonproduct);
        }

        foreach(@platforms) {
            if ( ! -d "$prebuild_dir/$_") {
                print_error("Can't find prebuild binaries for platform '$_'.", 22);
            }
        }

    }

    my $cwsname = $cws->child();
    my $linkdir = $milestone_opt ? "src.$milestone" : "src." . $cws->milestone;

    my $workspace = $args_ref->[0];

    if ( !$onlysolver ) {
        my $url_suffix = $milestone_opt ? ("/tags/$masterws" . "_$milestone") : ('/cws/' . $cwsname);
        if ( $switch ) {
            # check if to be switched working copy exist or bail out
            if ( ! -d $workspace ) {
                print_error("Can't open workspace '$workspace': $!", 21);
            }

            my $so_setup = 0;
            my $ooo_path;
            my $so_path;
            # Determine if we got a three directory (so) setup or a plain (ooo) setup.
            # This is only needed as long the build system still relies
            # on having "modules" from different repositories in the same
            # directory besides each other.
            if ( -d "$workspace/$masterws/sun" ) {
                $so_setup = 1;
                $ooo_path = "$workspace/$masterws/ooo";
                $so_path = "$workspace/$masterws/sun";
            }
            else {
                $ooo_path = "$workspace";
            }

            # get the working copy URLs
            my $ooo_new_url = svn_wc_root($ooo_path) . $url_suffix;
            my $so_new_url;
            if ( $so_setup ) {
                $so_new_url = svn_wc_root($so_path) . $url_suffix;
            }

            print_message("... switching '$ooo_path' to URL '$ooo_new_url'");
            svn_switch($ooo_path, $ooo_new_url, $quiet);
            # switch working copies
            if ( $so_setup ) {
                print_message("... switching '$so_path' to URL '$so_new_url'");
                svn_switch($so_path, $so_new_url, $quiet);
            }

            if ( $so_setup ) {
                relink_workspace("$workspace/$masterws/$linkdir", 0);
            }
        }
        else {
            if (!defined($ooo_svn_server)) {
                print_error("No OpenOffice.org SVN server defined, please check your configuration file.", 8);
            }

            my $ooo_url = $ooo_svn_server . $url_suffix;
            if ( -e $workspace ) {
                print_error("File or directory '$workspace' already exists.", 8);
            }

            if ( !(($scm eq 'SVN') || ($scm eq 'HG')) ) {
                print_error("Unsupported SCM '$scm'.", 8);
            }

            my $clone_milestone_only = $milestone_opt ? $milestone : 0;
            if ( defined($so_svn_server) ) {
                if ( !mkdir($workspace) ) {
                    print_error("Can't create directory '$workspace': $!.", 8);
                }
                my $work_master = "$workspace/$masterws";
                if ( !mkdir($work_master) ) {
                    print_error("Can't create directory '$work_master': $!.", 8);
                }
                if ( $scm eq 'SVN' ) {
                    print_message("... checkout '$ooo_url' to '$work_master/ooo'");
                    svn_checkout($ooo_url, "$work_master/ooo", $quiet);
                    my $so_url = $so_svn_server . $url_suffix;
                    print_message("... checkout '$so_url' to '$work_master/sun'");
                    svn_checkout($so_url, "$work_master/sun", $quiet);
                }
                else{
                    hg_clone_cws_or_milestone('ooo', $cws, "$work_master/ooo", $clone_milestone_only);
                    hg_clone_cws_or_milestone('so', $cws, "$work_master/sun", $clone_milestone_only);
                }
                if ( get_source_config_for_milestone($masterws, $milestone) ) {
                    # write source_config file
                    my $source_config_file = "$work_master/source_config";
                    if ( !open(SOURCE_CONFIG, ">$source_config_file") ) {
                        print_error("Can't create source_config file '$source_config_file': $!.", 8);
                    }
                    print SOURCE_CONFIG "[repositories]\n";
                    print SOURCE_CONFIG "ooo=active\n";
                    print SOURCE_CONFIG "sun=active\n";
                    close(SOURCE_CONFIG);
                }
                else {
                    my $linkdir = "$work_master/src.$milestone";
                    if ( !mkdir($linkdir) ) {
                        print_error("Can't create directory '$linkdir': $!.", 8);
                    }
                    relink_workspace($linkdir);
                }
            }
            else {
                if ( $scm eq 'SVN' ) {
                    print_message("... checkout '$ooo_url' to '$workspace'");
                    svn_checkout($ooo_url, $workspace, $quiet);
                }
                else {
                    hg_clone_cws_or_milestone('ooo', $cws, $workspace, $clone_milestone_only);
                }
            }
        }
    }

    if ( !$onlysolver && defined($external_tarball_source) ) {
        my $source_root_dir = "$workspace/$masterws";
        $external_tarball_source .= "/$masterws/ext_sources";
        if ( -e "$source_root_dir/ooo/ooo.lst" && -d $external_tarball_source ) {
            fetch_external_tarballs($source_root_dir, $external_tarball_source);
        }
    }

    if ( defined($platforms) ) {
        if ( !-d $workspace ) {
            if ( !mkdir($workspace) ) {
                print_error("Can't create directory '$workspace': $!.", 8);
            }
        }
        my $solver = defined($so_svn_server) ? "$workspace/$masterws" : "$workspace/solver";
        if ( !-d $solver ) {
            if ( !mkdir($solver) ) {
                print_error("Can't create directory '$solver': $!.", 8);
            }
        }
        my $source_config = get_source_config_for_milestone($masterws, $milestone);
        foreach(@platforms) {
            my $time_solver_start = Benchmark->new();
            print_message("... copying platform solver '$_'.");
            update_solver($_, $prebuild_dir, $solver, $milestone, $source_config);
            my $time_solver_stop = Benchmark->new();
            print_time_elapsed($time_solver_start, $time_solver_stop) if $profile;
        }
    }
    my $time_fetch_stop = Benchmark->new();
    my $time_fetch = timediff($time_fetch_stop, $time_fetch_start);
    print_message("cws fetch: total time required " . timestr($time_fetch));
}

sub do_query
{
    my $args_ref    = shift;
    my $options_ref = shift;

    # list of available query modes
    my @query_modes = qw(integratedinto incompatible taskids status latest current owner qarep build buildid integrated approved nominated ready new planned release due due_qa help ui milestones masters scm ispublic ispublicmaster);
    my %query_modes_hash = ();
    foreach (@query_modes) {
        $query_modes_hash{$_}++;
    }

    if ( exists $options_ref->{'help'} || @{$args_ref} != 1) {
        do_help(['query']);
    }
    my $mode = lc($args_ref->[0]);

    # cwquery mode 'state' has been renamed to 'status' to be more consistent
    # with CVS etc. 'state' is still an alias for 'status'
    $mode = 'status' if $mode eq 'state';

    # cwquery mode 'vcs' has been renamed to 'scm' to be more consistent
    # with general use etc. 'vcs' is still an alias for 'scm'
    $mode = 'scm' if $mode eq 'vcs';

    # there will be more query modes over time
    if ( !exists $query_modes_hash{$mode} ) {
        do_help(['query']);
    }
    query_cws($mode, $options_ref);
}

sub do_task
{
    my $args_ref    = shift;
    my $options_ref = shift;

    if ( exists $options_ref->{'help'} ) {
        do_help(['task']);
    }

    # CWS states for which adding tasks are blocked.
    my @states_blocked_for_adding = (
                                        "integrated",
                                        "nominated",
                                        "approved by QA",
                                        "cancelled",
                                        "finished"
                                    );
    my $cws = get_cws_from_environment();

    # register taskids with EIS database;
    # checks taksids for sanity, will notify user
    # if taskid is already registered.
    my $status = $cws->get_approval();

    my $child = $cws->child();
    my $master = $cws->master();

    my @registered_taskids = $cws->taskids();

    # if called without ids to register just query for tasks
    if ( @{$args_ref} == 0 ) {
        print_message("Task ID(s):");
        foreach (@registered_taskids) {
            if ( defined($_) ) {
                print "$_\n";
            }
        }
    }

    if ( !defined($status) ) {
        print_error("Can't determine status of child workspace `$child`.", 20);
    }

    if ( grep($status eq $_, @states_blocked_for_adding) ) {
        print_error("Can't add tasks to child workspace '$child' with state '$status'.", 21);
    }

    # Create hash for easier searching.
    my %registered_taskids_hash = ();
    for (@registered_taskids) {
        $registered_taskids_hash{$_}++;
    }

    my @new_taskids = ();
    foreach (@{$args_ref}) {
        if ( $_ !~ /^([ib]?\d+)$/ ) {
            print_error("'$_' is an invalid task ID.", 22);
        }
        if ( exists $registered_taskids_hash{$1} ) {
            print_warning("Task ID '$_' already registered, skipping.");
            next;
        }
        push(@new_taskids, $_);
    }

    # TODO: introduce a EIS_USER in the configuration, which should be used here
    my $config = CwsConfig->new();
    my $vcsid  = $config->vcsid();
    my $added_taskids_ref = $cws->add_taskids($vcsid, @new_taskids);
    if ( !$added_taskids_ref )  {
        my $taskids_str = join(" ", @new_taskids);
        print_error("Couldn't register taskID(s) '$taskids_str' with child workspace '$child'.", 23);
    }
    my @added_taskids = @{$added_taskids_ref};
    if ( @added_taskids ) {
        my $taskids_str = join(" ", @added_taskids);
        print_message("Registered taskID(s) '$taskids_str' with child workspace '$child'.");
    }
    return;
}

sub do_cdiff
{
    my $args_ref    = shift;
    my $options_ref = shift;

    if ( exists $options_ref->{'help'} || @{$args_ref} != 0) {
        do_help(['cdiff']);
    }

    my $files   = exists $options_ref->{'files'}   ? 1 : 0;
    my $modules = exists $options_ref->{'modules'} ? 1 : 0;

    if ( $files && $modules ) {
        print_error("Options --files and --modules are mutally exclusive", 0);
        do_help(['cdiff']);
    }

    my $diff_option;
    if ( $files ) {
        $diff_option = 'files';
    }
    elsif ( $modules ) {
        $diff_option = 'modules';
    }
    else {
        $diff_option = 0;
    }


    my $masterws  = exists $options_ref->{'masterworkspace'} ? uc($options_ref->{'masterworkspace'}) : $ENV{WORK_STAMP};
    my $childws   = exists $options_ref->{'childworkspace'} ? $options_ref->{'childworkspace'} : $ENV{CWS_WORK_STAMP};

    if ( !defined($masterws) ) {
        print_error("Can't determine master workspace environment.\n", 30);
    }

    if ( !defined($childws) ) {
        print_error("Can't determine child workspace environment.\n", 30);
    }

    my $cws = Cws->new();
    $cws->child($childws);
    $cws->master($masterws);

    if ( !is_valid_cws($cws) ) {
        print_error("'$childws' is not a valid CWS name.\n", 30);
    }

    if ( $cws->get_scm() eq 'HG' ) {
        print_error("cws cdiff is not supported for mercurial based childworkspaces", 80);
    }
    my $milestone = $cws->milestone();

    my $config = CwsConfig->new();
    my $ooo_svn_server = $config->get_ooo_svn_server();
    my $so_svn_server = $config->get_so_svn_server();

    my $ooo_milestone_url = get_milestone_url($ooo_svn_server, $masterws, $milestone);
    my $ooo_cws_url = get_cws_url($ooo_svn_server, $childws);
    my $ooo_files;
    if ( $diff_option  ) {
        $ooo_files = svn_diff($ooo_milestone_url, $ooo_cws_url, $diff_option);
        diff_print_files($ooo_files, $diff_option);
    }
    else {
        svn_diff($ooo_milestone_url, $ooo_cws_url, 0);
    }

    my $so_files;
    if ( $so_svn_server ) {
        my $so_milestone_url = get_milestone_url($so_svn_server, $masterws, $milestone);
        my $so_cws_url = get_cws_url($so_svn_server, $childws);
        if  ( svn_path_exists($so_cws_url) ) {
            if ( $diff_option ) {
                $so_files = svn_diff($so_milestone_url, $so_cws_url, $diff_option);
                diff_print_files($so_files, $diff_option);
            }
            else {
                svn_diff($so_milestone_url, $so_cws_url, 0);
            }
        }
    }

}

sub do_setcurrent
{
    my $args_ref    = shift;
    my $options_ref = shift;

    if ( exists $options_ref->{'help'} || @{$args_ref} != 0) {
        do_help(['setcurrent']);
    }

    if ( !exists $options_ref->{'milestone'} ) {
        do_help(['setcurrent']);
    }

    my $cws = get_cws_from_environment();
    my $old_masterws = $cws->master();
    my $new_masterws;
    my $new_milestone;

    my $milestone = $options_ref->{'milestone'};
    if ( $milestone eq 'latest' ) {
        my $latest = $cws->get_current_milestone($old_masterws);

        if ( !$latest ) {
            print_error("Can't determine latest milestone of '$old_masterws'.", 22);
        }
        $new_masterws  = $old_masterws;
        $new_milestone = $latest;
    }
    else {
        ($new_masterws, $new_milestone) =  verify_milestone($cws, $milestone);
    }

    print_message("... updating EIS database");
    my $push_return = $cws->set_master_and_milestone($new_masterws, $new_milestone);
    # sanity check
    if ( $$push_return[1] ne $new_milestone) {
        print_error("Couldn't push new milestone '$new_milestone' to database", 0);
    }
}

sub do_eisclone
{
    my $args_ref    = shift;
    my $options_ref = shift;

    print_error("not yet implemented.", 2);
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
        print STDERR "Type 'cws help' for usage.\n";
}

### SVN glue ###

# TODO: is it a better idea to use the SVN bindings?
# pro:
#       - SVN make guarantees about API stability but no about the command line
#       - finer access to the SVN functionality, better error reporting
#       - prevents parsing errors due to localized SVN messages
# con:
#       - the bindings are difficult to install, mostly due to subtle install bugs
#       - we do not really use much of the SVN functionality here

sub svn_wc_is_clean
{
    my $wc_path = shift;

    my $result = execute_svnversion_command($wc_path);

    my $error = 0;

    if ( $result =~ /:/ ) {
        print_error("Working copy '$wc_path' contains mixed revisions. Please run 'svn update'!", 0);
        $error++;
    }
    if ( $result =~ /M/ ) {
        print_error("Working copy '$wc_path' contains locally modified files. Please commit or revert all modified files.", 0);
        $error++;
    }
    if ( $result =~ /S/ ) {
        print_error("Working copy '$wc_path' is partially switched. The whole working copy needs to be switched to the CWS branch.", 0);
        $error++;
    }
    if ( $result =~ /P/ ) {
        print_error("Working copy '$wc_path' is only partially checked out. CWS tools can't work on partially checked out working copies.", 0);
        $error++;
    }

    return !$error;
}

sub svn_version_check
{
    my $major_required = 1;
    my $minor_required = 5;
    my $patchlevel_required = 4;

    my $version_required = $major_required*1000000 + $minor_required*1000 + $patchlevel_required;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... svn version\n";
    }

    my @result = execute_svn_command(0, '--version --quiet', " ");
    # svn --version --quiet returns the version in major.minor.patchlevel scheme
    # for example: 1.5.4 or 1.6.0-dev (for developer codelines)
    # hopefully they don't change the versioning scheme
    my ($major, $minor, $patchlevel);
    if ( $result[0] =~ /^(\d+)\.(\d+)\.(\d+)/ ) {
        $major       = $1;
        $minor       = $2;
        $patchlevel  = $3;
    }
    else {
        print_error("Can't determine svn version. Please file an issue with the output of 'svn --version --quiet'. CWS tooling requires svn-1.5.4 or later\n", 1)
    }

    my $version =  $major*1000000 + $minor*1000 + $patchlevel;

    if ( $version < $version_required ) {
        return 0;
    }
    return 1;
}

sub svn_copy
{
    my $comment = shift;
    my $source = shift;
    my $dest = shift;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing branch: '$source' -> '$dest'\n";
    }

    my @result = execute_svn_command(0, 'copy', "-m '$comment'", $source, $dest);
    if ( $result[1] =~ /Committed revision (\d+)\./ ) {
        print STDERR ", committed revision $1\n";
    } else {
        print STDERR "failed!\n";
        print STDERR @result;
    }
}

sub svn_milestone_revision
{
    my $milestone_url = shift;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing log --stop-on-copy: '$milestone_url'\n";
    }

    my @result = execute_svn_command(0, 'log', '--stop-on-copy', $milestone_url);

    # There might be revisions committed to a tag (allowed in subversion).
    # The lowestmost revision listed in a 'log --stop-on-copy' is the one which
    # was current when the tag was created
    my $revision = 0;
    foreach ( @result ) {
        if ( /^r(\d+)\s+\|\s+/ ) {
            $revision = $1;
        }
    }

    return $revision;
}

sub svn_path_exists
{
    my $url = shift;

    my @result = svn_info($url);

    foreach ( @result ) {
        if ( /^Path: / ) {
            return 1;
        }
    }
    return 0;
}

sub svn_wc_url
{
    my $wc_path = shift;

    my @result = svn_info($wc_path);

    foreach ( @result ) {
        if ( /^URL: (.+)$/ ) {
            return $1;
        }
    }

    print_error("Can't retrieve svn info from working copy '$wc_path'\n", 23);
}

sub svn_wc_root
{
    my $wc_path = shift;

    my @result = svn_info($wc_path);

    foreach ( @result ) {
        if ( /^Repository Root: (.+)$/ ) {
            return $1;
        }
    }

    print_error("Can't retrieve svn info from working copy '$wc_path'\n", 23);
}

sub svn_info
{
    my $url = shift;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing info: '$url'\n";
    }

    my @result = execute_svn_command(0, 'info', '--depth empty', $url);
    return @result;
}

sub svn_merge
{
    my $url = shift;
    my $wc  = shift;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing merge: '$url -> $wc'\n";
    }

    my $log_file = "$wc/REBASE.LOG";
    my @result = execute_svn_command($log_file, 'merge', '--accept postpone', $url, $wc);
    return @result;
}

sub svn_switch
{
    my $wc  = shift;
    my $url = shift;
    my $quiet = shift;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing switch: '$url -> $wc'\n";
    }

    my $switch = $quiet ? 'switch --quiet' : 'switch';

    my @result = execute_svn_command('print', $switch, $url, $wc);
    return @result;
}

sub svn_checkout
{
    my $url   = shift;
    my $wc    = shift;
    my $quiet = shift;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing checkout: '$url -> $wc'\n";
    }

    my $checkout = $quiet ? 'checkout --quiet' : 'checkout';

    my @result = execute_svn_command('print', $checkout, $url, $wc);
    return @result;
}

sub svn_commit
{
    my $wc             = shift;
    my $commit_message = shift;

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing commit: '$wc'\n";
    }

    my $log_file = "$wc/REBASE.LOG";
    my @result = execute_svn_command($log_file, 'commit', "-m '$commit_message'", $wc);
    return @result;
}

sub svn_diff
{
    my $url1 = shift;
    my $url2 = shift;
    my $diff_option = shift;

    my $summarize = '';
    if ( $diff_option ) {
        $summarize = '--summarize';
    }

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... preparing diff $summarize: '$url1' vs. '$url2'\n";
    }

    if ( $summarize ) {
        my $result = execute_svn_command(0, 'diff', $summarize, $url1, $url2);
        my $nlen = length($url1);
        my @files;
        foreach( @{$result} ) {
            my ($dummy, $url) = split();
            next if length($url) <= $nlen;   # skip short URLs (like $url1)
            my $file = substr($url, $nlen+1);
            next if index($file, '/') == -1; # skip 'modified' top level dirs
            push (@files, $file);
        }
        return \@files;
    }
    else {
        execute_svn_command('print', 'diff', $url1, $url2);
    }
}

sub execute_svn_command
{
    my $log     = shift;
    my $command = shift;
    my $options = shift;
    my @args    = @_;

    my $args_str = join(" ", @args);

    # we can only parse english strings, hopefully a C locale is available everywhere
    $ENV{LC_ALL}='C';
    $command = "svn $command $options $args_str";

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... execute command line: '$command'\n";
    }

    my @result;
    my $date;
    if ( $log && $log ne 'print') {
        open(LOG, ">>$log") or print_error("can't open log file '$log'", 30);
        $date = localtime();
        print LOG "Start $command $args_str at $date\n";
    }
    open(OUTPUT, "$command 2>&1 |") or print_error("Can't execute svn command line client", 98);
    STDOUT->autoflush(1) if $log;
    while (<OUTPUT>) {
        if ( $log ) {
            print STDOUT $_;
            print LOG $_ if $log ne 'print';
        }
        else {
            push(@result, $_);
        }
    }
    STDOUT->autoflush(0) if $log;
    close(OUTPUT);
    if ( $log && $log ne 'print') {
        $date = localtime();
        print LOG "Stop $command $args_str at $date\n";
        close (LOG);
    }

    my $rc = $? >> 8;

    if ( $rc > 0) {
        print STDERR "\n";
        print STDERR @result if !$log;
        print_error("The subversion command line client failed with exit status '$rc'", 99);
    }
    return wantarray ? @result : \@result;
}

sub execute_svnversion_command
{
    my $options = shift;
    my @args    = @_;

    my $args_str = join(" ", @args);

    # we can only parse english strings, hopefully a C locale is available everywhere
    $ENV{LC_ALL}='C';
    $command = "svnversion $options $args_str";

    if ( $debug ) {
        print STDERR "\nCWS-DEBUG: ... execute command line: '$command'\n";
    }

    my $result = `$command`;
    my $rc = $? >> 8;
    if ($rc > 0) {
        print_error("The subversion command line tool 'svnversion' failed with exit status '$rc'", 99);
    }

    return $result;
}

### HG glue ###

sub hg_clone
{
    my $source  = shift;
    my $dest    = shift;
    my $options = shift;

    if ( $debug ) {
        print STDERR "CWS-DEBUG: ... hg clone: '$source -> $dest', options: '$options'\n";
    }

    # The to be cloned revision might not yet be avaliable. In this case clone
    # the available tip.
    my @result = execute_hg_command(0, 'clone', $options, $source, $dest);
    if ( defined($result[0]) && $result[0] =~ /abort: unknown revision/ ) {
        $options =~ s/-r \w+//;
        @result = execute_hg_command(1, 'clone', $options, $source, $dest);
    }
    return @result;
}

sub hg_parent
{
    my $repository  = shift;
    my $rev_id = shift;
    my $options = shift;

    if ( $debug ) {
        print STDERR "CWS-DEBUG: ... hg parent: 'repository', revision: '$rev_id', options: $options\n";
    }

    my @result = execute_hg_command(0, 'parent', "--cwd $repository", "-r $rev_id", $options);
    my $line = $result[0];
    chomp($line);
    return $line;
}

sub hg_pull
{
    my $repository  = shift;
    my $remote = shift;

    if ( $debug ) {
        print STDERR "CWS-DEBUG: ... hg pull: 'repository', remote: '$remote'\n";
    }

    my @result = execute_hg_command(0, 'pull', "--cwd $repository", $remote);
    my $line = $result[0];
    if ($line =~ /abort: /) {
        return undef;
    }
}

sub hg_update
{
    my $repository  = shift;

    if ( $debug ) {
        print STDERR "CWS-DEBUG: ... hg update: 'repository'\n";
    }

    my @result = execute_hg_command(1, 'update', "--cwd $repository");
    return @result;
}

sub hg_show
{
    if ( $debug ) {
        print STDERR "CWS-DEBUG: ... hg show\n";
    }
    my $result = execute_hg_command(0, 'show', '');
    return $result;
}

sub execute_hg_command
{
    my $terminate_on_rc = shift;
    my $command = shift;
    my $options = shift;
    my @args = @_;

    my $args_str = join(" ", @args);

    # we can only parse english strings, hopefully a C locale is available everywhere
    $ENV{LC_ALL}='C';
    $command = "hg $command $options $args_str";

    if ( $debug ) {
        print STDERR "CWS-DEBUG: ... execute command line: '$command'\n";
    }

    my @result;
    open(OUTPUT, "$command 2>&1 |") or print_error("Can't execute mercurial command line client", 98);
    while (<OUTPUT>) {
        push(@result, $_);
    }
    close(OUTPUT);

    my $rc = $? >> 8;

    if ( $rc > 0 && $terminate_on_rc) {
        print STDERR @result;
        print_error("The mercurial command line client failed with exit status '$rc'", 99);
    }
    return wantarray ? @result : \@result;
}


# vim: set ts=4 shiftwidth=4 expandtab syntax=perl:
