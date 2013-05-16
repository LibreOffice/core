#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#


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
    print "************************************************\n";
    print "WARNING: multiple assignments of gids:\n";
    print "************************************************\n";

    my $line;
    foreach $line ( @{$multiassignmentgids} ) { print "\t$line\n"; }
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
