#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: exiter.pm,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:25:52 $
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
    my ($errorgids) = @_;

    print "************************************************\n";
    print "ERROR: multiple definition of gids:\n";
    print "************************************************\n";

    for ( my $i = 0; $i <= $#par2script::globals::multidefinitiongids; $i++ )
    {
        print "$par2script::globals::multidefinitiongids[$i]\n";
    }

    exit(-1);
}

1;
