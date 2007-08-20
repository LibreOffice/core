#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: exiter.pm,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: ihi $ $Date: 2007-08-20 15:28:32 $
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
