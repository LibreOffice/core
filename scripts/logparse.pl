#!/usr/bin/perl -w
# -*- tab-width: 4; indent-tabs-mode: nil -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

use strict;

sub clear_hash($)
{
    my $hash = shift;
    for my $key (keys %${hash}) {
	delete $hash->{$key}
    }
}

sub clear_state($$$)
{
    my $sessions = shift;
    my $lok_starting = shift;
    my $lok_running = shift;

    my @names = keys %{$sessions};
    print "Leaked sessions:\n\t" . join(',', @names) . "\n" if (@names > 0);

    @names = keys %{$lok_starting};
    print "Sessions starting:\n\t" . join(',', @names) . "\n" if (@names > 0);

    @names = keys %{$lok_running};
    print "Sessions running:\n\t" . join(',', @names) . "\n" if (@names > 0);

    clear_hash($sessions);
    clear_hash($lok_starting);
    clear_hash($lok_running);
}

my @input = <STDIN>;

my $total_blocks = 0;

# live state:
my $inodes = 0;
my $blocks = 0;
my $running = 0;
my $mem_used = 0;
my $load_avg = 0;
my $cpu_user = 0;
my $cpu_sys = 0;
my %sessions;
my %lok_starting;
my %lok_running;
my $printed_header = 0;

while (my $line = shift @input) {
    my ($pevent, $pdetail);

    $line =~ s/\r*\n*//g;

    # df -i /dev/sda1 | logger
    # df /dev/sda1 | logger
    # root: /dev/sda1      655360 77457 577903   12% /
    if ($line =~ /root: \S+\s+(\d+)\s+(\d+)/) {
	my ($total, $used) = ($1, $2);
	$total_blocks = $total if ($total_blocks < $total);
	if ($total == $total_blocks) {
	    $blocks = $used;
	} else {
	    $inodes = $used;
	}
    }

    # top -b -n1 | head -4 | logger
    # root: top - 15:38:01 up 5 days,  1:56,  1 user,  load average: 0.00, 0.05, 0.06
    # root: Tasks:  82 total,   1 running,  81 sleeping,   0 stopped,   0 zombie
    # root: %Cpu(s):  0.2 us,  0.1 sy,  0.0 ni, 99.7 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
    # root: KiB Mem:   7870968 total,  3202484 used,  4668484 free,   181104 buffers
    $load_avg = $1 if ($line =~ m/top.*load average: (\d+\.\d+)/);
    $mem_used = $1 if ($line =~ m/KiB Mem:.*\s(\d+)\s+used,/);
    if ($line =~ m/%Cpu(s):\s+(\d+\.\d+) us,\s+(\d+\.\d+) sy,/) {
	$cpu_user = $1;
	$cpu_sys = $2;
    }
    $running = $1 if ($line =~ m/Tasks:.*\s+(\d+) running,/);
    $mem_used = $1 if ($line =~ m/KiB Mem:.*\s(\d+)\s+used,/);
    $mem_used = $1 if ($line =~ m/KiB Mem:.*\s(\d+)\s+used,/);

# Session 000d is loading. 0 views loaded.
# Session 000e is unloading. 0 views will remain.
    if ($line =~ m/Session (\S+) is (\S*)loading. (\d+) views/) {
	my ($skey, $type, $count) = ($1, $2, $3);
	if ($type eq 'un') {
	    $pevent = "unload\t\"$skey\"";
	    if (defined $sessions{$skey}) {
		$pdetail = $sessions{$skey};
	    } else {
		# setup - somewhere earlier in the logs.
		$pdetail = 'pre-existing';
	    }
	    delete $sessions{$skey};
	} else {
	    $sessions{$skey} = 1;
	}
    }

    # systemd[1]: coolwsd.service: main process exited, code=killed, status=11/SEGV
    if ($line =~m/coolwsd.service: main process exited.*status=(.*)$/) {
	print "coolwsd exit: $1\n";
    }

    if ($line =~m/Initializing wsd/) {
	print "Re-started\n";
	clear_state(\%sessions, \%lok_starting, \%lok_running);
    }


    # different PIDs: ...
    # [coolbroker     ] Spawned kit [1689].
    # [coolkit        ] coolkit [1689] is ready.
    # [coolbroker     ] Child 1536 terminated.
    # [coolbroker     ] Child process [1689] exited with code: 0.
    if ($line =~ m/coolbroker.*Forked kit \[(\d+)\]./) {
	my $pid = $1;
	$lok_starting{$pid} = 1;
	$pevent = "newkit\t\"$pid\"";
	$pdetail = '';
    }
    if ($line =~ m/coolkit \[(\d+)\] is ready./) {
	my $pid = $1;
	delete $lok_starting{$pid};
	$lok_running{$pid} = 1;
	$pevent = "livekit\t\"$pid\"";
	$pdetail = '';
    }
    # [coolbroker     ] Child process [1689] exited with code: 0.
    if ($line =~ m/coolbroker.*Child process \[(\d+)\]\s+(\S+)\s+.*with /) {
	my $pid = $1;
	my $code = $2;
	$code = 'exit' if ($code eq 'exited');
	delete $lok_running{$pid};
	$pevent = $code . "kit\t\"$pid\"";
	$pdetail = '';
    }

    # coolwsd:
    if ($line =~ m/Loading new document from URI: \[([^\]]+)\].*for session \[([^\]]+)\]/) {
	my ($doc_uri, $session) = ($1, $2);
	$sessions{$session} = $doc_uri;
	$pevent = "load\t\"$session\"";
	$pdetail = $doc_uri;
    }

    if (defined $pevent)
    {
	my $session_count = keys %sessions;
	my $start_count = keys %lok_starting;
	my $running_count = keys %lok_running;
	if ($printed_header++ == 0) {
	    print "event\tkey\tinodes\tfs_blk\t";
	    print "mem/Kb\trun\t";
	    print "loadavg\tsess\t";
	    print "start\tlive\t";
	    print "file\n";
	}
	print "$pevent\t$inodes\t$blocks\t";
	print "$mem_used\t$running\t";
	print "$load_avg\t$session_count\t";
	print "$start_count\t$running_count\t";
	print "$pdetail\n";
    }
}

clear_state(\%sessions, \%lok_starting, \%lok_running);

# vim: set shiftwidth=4 softtabstop=4 expandtab:
