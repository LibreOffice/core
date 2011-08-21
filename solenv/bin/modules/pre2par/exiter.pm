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

package pre2par::exiter;

use pre2par::files;
use pre2par::globals;

############################################
# Exiting the program with an error
# This function is used instead of "die"
############################################

sub exit_program
{
    my ($message, $function) = @_;

    my $infoline;

    $infoline = "\n***************************************************************\n";
    push(@pre2par::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "$message\n";
    push(@pre2par::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "in function: $function\n";
    push(@pre2par::globals::logfileinfo, $infoline);
    print("$infoline");

    $infoline = "***************************************************************\n";
    push(@pre2par::globals::logfileinfo, $infoline);

    if ($pre2par::globals::logging)
    {
        pre2par::files::save_file($pre2par::globals::logfilename ,\@pre2par::globals::logfileinfo);
        print("Saved logfile: $pre2par::globals::logfilename\n");
    }

    print("$infoline");

    exit(-1);
}

1;
