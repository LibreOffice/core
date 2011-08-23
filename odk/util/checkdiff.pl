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

#
# checkit - a perl script to check the output of 'dircmp'
#

$return = 0;
$possible_error = 0;
$possible_error_descript = "";
while( <STDIN> )
{
    if( /^diff/ )
    {
        $possible_error = 1;
        $possible_error_descript = $_;
    }
    elsif( /^Binary/ )
    {
        print STDERR "ERROR : $_";
        $return++;
    }
    elsif( /^[0-9]/ && $possible_error == 1 )
    {
        print STDERR "ERROR : diff ".$possible_error_descript;
        $return++;
        $possible_error = 0;
    }
    else
    {
        $possible_error = 0;
    }
}
if( $return != 0 )
{
    unlink "$ARGV[0]";
}
exit $return;
