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



#*************************************************************************
#
# createpdbrelocators - create for pdb relocator files
#                       PDB relocator files are used to find debug infos
#                       for analysis of creash reports
#
# usage: createpdbrelocators;
#
#*************************************************************************

use strict;

#### module lookup

use lib ("$ENV{SOLARENV}/bin/modules");
use CreatePDBRelocators;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.5 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

my $inpath    = $ENV{INPATH};
my $milestone = $ENV{UPDMINOR};
my $milestoneext = $ENV{UPDMINOREXT};

if ( $ARGV[0] ) {
    if ( $milestone && ( $milestone ne $ARGV[0] ) ) {
        die "Error: specified milestone $ARGV[0] does not match your environment";
    }
    $milestone = $ARGV[0];
}

if ( !$inpath || !$milestone ) {
    print STDERR "$script_name: INAPTH or UPDMINOR not set!\n";
    exit(1);
}
my $relocators = CreatePDBRelocators->new();
my $rc = $relocators->create_pdb_relocators($inpath, $milestoneext, "");

if ( !$rc ) {
    print STDERR "$script_name: creating PDB relocators failed!\n";
    exit(2);
}

exit(0);
