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




package par2script::exiter;

use par2script::files;
use par2script::globals;

############################################
# Exiting the program with an error
# This function is used instead of "die"
############################################

sub exit_program
{
    my ($message, $function) = @_;

    my $infoline;

    $infoline = "\n***************************************************************\n";
    push(@par2script::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "$message\n";
    push(@par2script::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "in function: $function\n";
    push(@par2script::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "***************************************************************\n";
    push(@par2script::globals::logfileinfo, $infoline);

    if ($par2script::globals::logging)
    {
        par2script::files::save_file($par2script::globals::logfilename, \@par2script::globals::logfileinfo);
        print("Saved logfile: $par2script::globals::logfilename\n");
    }

    print("$infoline");

    exit(-1);
}

#####################################
# Error, because a gid is defined
# more than once
#####################################

sub multidefinitionerror
{
    my ( $multidefinitiongids ) = @_;
    print "************************************************\n";
    print "ERROR: multiple definition of gids:\n";
    print "************************************************\n";

    my $gid;
    foreach $gid ( @{$multidefinitiongids} ) { print "\t$gid\n"; }
    exit(-1);
}

#####################################
# Error, because a gid is assigned
# more than once
#####################################

sub multiassignmenterror
{
    my ( $multiassignmentgids ) = @_;
    #print "************************************************\n";
    #print "ERROR: multiple assignments of gids:\n";
    #print "************************************************\n";

    my $line;
    foreach $line ( @{$multiassignmentgids} ) { print "\t$line\n"; }
    # exit(-1);
}

#####################################
# Error, because a defined gid
# is not assigned
#####################################

sub missingassignmenterror
{
    my ( $missingassignmentgids ) = @_;
    print "********************************************************\n";
    print "ERROR: Missing assignments for the following GIDs:\n";
    print "********************************************************\n";

    my $gid;
    foreach $gid ( @{$missingassignmentgids} ) { print "\t$gid\n"; }
    exit(-1);
}

1;
