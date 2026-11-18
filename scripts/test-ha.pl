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
use Time::HiRes qw( time );

my $uri = shift @ARGV || die 'pass in URI of server or file to fetch';
my $tmp = '/tmp/downloaded';
my %hitcount;
my %slowcount;
my %timecount;

# Try to detect bad WOPI hosts by bucketing latency.
sub testWOPIServer()
{
    for (my $i = 0; $i < 50; $i++)
    {
	unlink $tmp;

	my $begin_time = time();

	my $pipe;
	open ($pipe, "curl --insecure --stderr - -i -v -m 300 -o $tmp $uri  |") || die "Can't launch curl";
	my $node = 'unknown';
	my $size = 0;
	while (<$pipe>) {
	    my $line = $_;
	    $line =~ m/^< Set-Cookie:/ || next;
	    if ($line =~ m/BIGipServerIP[^=]+=([0-9\.]+);/) {
		$node = $1;
	    }
	    if ($line =~ m/Content-Length: ([0-9]+)/) {
		$size = $1;
	    }
	}
	close ($pipe);

	my $end_time = time();
	my $time_taken = $end_time - $begin_time;

	if (!defined $slowcount{$node}) {
	    $hitcount{$node} = 0;
	    $slowcount{$node} = 0;
	    $timecount{$node} = 0.0;
	}
	$hitcount{$node}++;
	$timecount{$node} += $time_taken;

	my $slow = '';
	if ($time_taken > 1.0) {
	    $slow = ' slow';
	    $slowcount{$node}++;
	}
	printf("%.2fs from $node size: $size $slow\n", $time_taken);
    }

    print "hits\t#slow\ttotal\tNode\n";
    for my $node (keys %slowcount) {
	printf ("%s\t%s\t%.2fs\t%s\n", $hitcount{$node}, $slowcount{$node}, $timecount{$node}, $node);
    }
}

sub randstr()
{
    my @hex = ('0' ..'9', 'A' .. 'F');
    return join '' => map $hex[rand @hex], 1 .. 8;
}


sub testCoolCluster()
{
    my @ids;
    my %serverId_by_src;

    my $i;
    my $size = 0;
    my $iters = 50; # number of checks
    my $tests = 10; # number of src strings

    my $uri_base = "$uri/hosting/capabilities?WOPISrc=";
    for ($i = 0; $i < $tests; $i++) {
	push(@ids,randstr());
    }

    printf("Touching $uri_base $iters times\n");
    for ($i = 0; $i < $iters; $i++)
    {
	my $pipe;
	my $src = $ids[rand(@ids)];
	my $id = 0;

	open ($pipe, "curl --insecure -m 500 -s $uri_base$src |") || die "Can't launch curl";
	while (<$pipe>) {
	    my $line = $_;
#	    printf ("$line\n");
	    if ($line =~ m/\"serverId\":\"([^"]+)\"/) {
		$id = $1;
	    }
	}
	close ($pipe);
#	printf("id: $id\n");

	$serverId_by_src{$src} = $id if (!defined $serverId_by_src{$src});
	if ($serverId_by_src{$src} ne $id)
	{
	    die("ERROR: ID mismatch for $id vs. $serverId_by_src{$src} for WOPISrc '$src'\n");
	}
    }
    print ("WOPISrc check with $iters iterations and $tests keys passed cleanly.\n");
}

testCoolCluster();

# vim: set shiftwidth=4 softtabstop=4 expandtab:
