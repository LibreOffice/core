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

# The install names of our dynamic libraries contain a special segment token
# that denotes where the dynamic library is located in the installation set.
# The segment token consists of "@", optionally followed by ".", followed by 50
# "_", followed by a location token (one of "URELIB", "OOO", "OXT", or "NONE").
#
# Typically, the segment token is the first segment of a relative install name.
# But the segment token may also appear within an absolute install name.  That
# is useful when tunnelling the segment token into the external build process
# via a --prefix configure switch, for example.
#
# When another dynamic library or an executable links against such a dynamic
# library, the path recorded in the former to locate the latter is rewritten
# according to the below %action table.  The result path consists of the prefix
# from the action table followed by the suffix of the dynamic library's install
# name.  If the special segment token does not contain the optional "." after
# the "@", the suffix consists of all segments after the special token segment.
# If the special token segment does contain the optional ".", then the suffix
# consists of just the last segment of the original install name.
#
# That latter case is useful for libraries from external modules, where the
# external build process locates them in some sub-directory.

sub action($$$)
{
    # The @__VIA_LIBRARY_PATH__ thing has no magic meaning anywhere
    # (here in LO or to the dynamic linker), it is effectively a
    # comment telling that this library is supposed to have been found
    # by the dynamic linker already in DYLD_LIBRARY_PATH.

    my %action =
        ('app/UREBIN/URELIB' => '@executable_path/../lib',
         'app/OOO/URELIB' => '@executable_path/../ure-link/lib',
         'app/OOO/OOO' => '@executable_path',
         'app/SDKBIN/URELIB' => '@executable_path/../../ure-link/lib',
         'app/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
         'app/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
         'app/NONE/NONE' => '@__VIA_LIBRARY_PATH__',
         'shl/URELIB/URELIB' => '@loader_path',
         'shl/OOO/URELIB' => '@loader_path/../ure-link/lib',
         'shl/OOO/OOO' => '@loader_path',
         'shl/OXT/URELIB' => '@executable_path/urelibs',
         'shl/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/NONE' => '@__VIA_LIBRARY_PATH__');
    my ($type, $loc1, $loc2) = @_;
    my $act = $action{"$type/$loc1/$loc2"};
    die "illegal combination $type/$loc1/$loc2" unless defined $act;
    return $act;
}

@ARGV >= 2 or die 'Usage: app|shl UREBIN|URELIB|OOO|SDKBIN|OXT|NONE <filepath>*';
$type = shift @ARGV;
$loc = shift @ARGV;
foreach $file (@ARGV)
{
    my $call = "otool -L $file";
    open(IN, "-|", $call) or die "cannot $call";
    my $change = "";
    while (<IN>)
    {
        if (m'^\s*(((/.*)?/)?@_{50}([^/]+)(/.+)) \(compatibility version \d+\.\d+\.\d+, current version \d+\.\d+\.\d+\)\n$')
        {
            $change .= " -change $1 " . action($type, $loc, $4) . $5;
        }
        elsif (m'^\s*(((/.*)?/)?@\._{50}([^/]+)(/.+)?(/[^/]+)) \(compatibility version \d+\.\d+\.\d+, current version \d+\.\d+\.\d+\)\n$')
        {
            $change .= " -change $1 " . action($type, $loc, $4) . $6;
        }
    }
    close(IN);
    if ($change ne "")
    {
        $call = "$ENV{'XCRUN'} install_name_tool$change $file";
        system($call) == 0 or die "cannot $call";
    }
}
