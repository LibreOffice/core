#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: parameter.pm,v $
#
# $Revision: 1.6 $
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


package par2script::parameter;

use Cwd;
use par2script::files;
use par2script::globals;
use par2script::systemactions;

############################################
# Parameter Operations
############################################

###############################################################################
# Usage:
# perl par2script.pl -i ..\wntmsci8.pro\par,o:\SRX645\wntmsci8.pro\par.m24
#                   @@C:\DOCUMEN~1\is\LOCALS~1\Temp\mk6pd
#                   -o ..\wntmsci8.pro\bin\osl\setup_osl.inf
###############################################################################

sub usage
{
    print <<Ende;

--------------------------------------------------------------
$par2script::globals::prog
The following parameter are needed:
-i: include pathes, comma separated list
-o: setup script file name
-v: writing logfile.txt (optional)
\@\@list: list of all par files

Example:
 perl par2script.pl -i ..\\wntmsci8\\par\,o\:\\SRX645\\wntmsci8\\par.m24
         \@\@C\:\\DOCUMEN\~1\\is\\LOCALS\~1\\Temp\\mk6pd
         -o ..\\wntmsci8.pro\\bin\\osl\\setup_osl.inf \[-v\]

--------------------------------------------------------------
Ende
    exit(-1);
}

#####################################
# Reading parameter
#####################################

sub getparameter
{
    while ( $#ARGV >= 0 )
    {
        my $param = shift(@ARGV);

        if ($param eq "-o") { $par2script::globals::scriptname = shift(@ARGV); }
        elsif ($param eq "-v") { $par2script::globals::logging = 1; }
        elsif ($param =~ /\@\@/) { $par2script::globals::parfilelistorig = $param; }
        elsif ($param eq "-i") { $par2script::globals::includepathlist = shift(@ARGV); }
        elsif (($param =~ /\//) || ($param =~ /\\/))    # another include parameter!
        {
            $par2script::globals::includepathlist = $par2script::globals::includepathlist . "," . $param;
        }
        else
        {
            print("\n*************************************\n");
            print("Sorry, unknown parameter: $param");
            print("\n*************************************\n");
            usage();
            exit(-1);
        }
    }
}

############################################
# Controlling  the fundamental parameter
# (required for every process)
############################################

sub control_parameter
{
    if ($par2script::globals::includepathlist eq "")
    {
        print "\n************************************************\n";
        print "Error: Include pathes not set not set (-i)!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    if ($par2script::globals::scriptname eq "")
    {
        print "\n************************************************\n";
        print "Error: Name of the setup script not set (-o)!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    if ($par2script::globals::parfilelistorig eq "")
    {
        print "\n************************************************\n";
        print "Error: List of par files not set!";
        print "\n************************************************\n";
        usage();
        exit(-1);
    }

    # The par file list has to exist

    $par2script::globals::parfilelist = $par2script::globals::parfilelistorig;
    $par2script::globals::parfilelist =~ s/\@\@//;
    par2script::files::check_file($par2script::globals::parfilelist);
}

#####################################
# Writing parameter to shell
#####################################

sub outputparameter
{
    my $outputline = "\n$par2script::globals::prog -i $par2script::globals::includepathlist $par2script::globals::parfilelistorig -o $par2script::globals::scriptname";

    if ($par2script::globals::logging) { $outputline .= " -v"; }

    $outputline .= "\n";

    print $outputline;
}

1;
