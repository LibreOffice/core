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
