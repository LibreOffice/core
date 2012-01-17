:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;

#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************

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
                $_ =
~ /.*svn.services.openoffice.org(.*\/(.*))\/\w*/;
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
