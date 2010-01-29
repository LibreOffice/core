:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#*************************************************************************
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
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
#***********************************************************************/

use strict;
use Cwd;

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

# Prototypes
sub getMinor($);
sub getCwsWorkStamp();

my $workstamp = $ENV{'WORK_STAMP'};
my $solenv= $ENV{'SOLARENV'};
my $cwsWorkStamp = getCwsWorkStamp();
my $minor = getMinor($cwsWorkStamp);

my $oldWorkStamp = $workstamp."_".$minor;
my $svndiff="svn diff --summarize --old=svn://svn.services.openoffice.org/ooo/tags/".$oldWorkStamp." --new=svn://svn.services.openoffice.org/ooo/cws/".$cwsWorkStamp;

my @diff = `$svndiff`;

my @modules;
foreach(@diff)
{
    if (/.*svn:\/\/svn.services.openoffice.org.*/)
    {
        $_ =~ /.*$oldWorkStamp\/(\w*)/;
        my $newModule=$1;
        if (defined($newModule))
        {
            if ( ! grep(/$newModule/,@modules))
            {
                push(@modules, $newModule);
            }

        }
    }
}

foreach(@modules)
{
    print "$_\n";
}

exit(0);

sub getMinor($)
{
    my $workst = shift;
    my $min="";

    if ( ! defined($ENV{'UPDMINOR'}))
    {
        my $cws = Cws->new();
        $cws->child($workst);
        $cws->master($ENV{'WORK_STAMP'});
        my $masterws = $cws->master();
        my $childws  = $cws->child();

        # check if we got a valid child workspace
        my $id = $cws->eis_id();
        if ( !$id )
        {
            print("Child workspace '$childws' for master workspace '$masterws' not found in EIS database.\n");
            exit(1);
        }

        my @milestones = $cws->milestone();
        foreach (@milestones) {
            if ( defined($_) )
            {
                $min=$_;
            }
        }
    }
    else
    {
        $min = $ENV{'UPDMINOR'};
    }

    chomp($min);
    return $min;
}

sub getCwsWorkStamp()
{
    my $cwsWorkSt="";

    if ( ! defined($ENV{'CWS_WORK_STAMP'}))
    {
        my $currPath= cwd;

        chdir($ENV{'SOLARENV'});

        my @info = `svn info`;

        foreach(@info)
        {
            if ( /URL:.*/ )
            {
                # URL: svn+ssh://svn@svn.services.openoffice.org/ooo/cws/qadev37/solenv
                $_ =~ /.*svn.services.openoffice.org(.*\/(.*))\/\w*/;
                $cwsWorkSt=$2; #qadev37
            }
        }

    }
    else
    {
        $cwsWorkSt = $ENV{'CWS_WORK_STAMP'};
    }
    return $cwsWorkSt
}
