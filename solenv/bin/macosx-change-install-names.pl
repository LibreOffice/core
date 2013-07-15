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
         'fb/OOO/URELIB' => '@rpath/../ure-link/lib',
         'fb/OOO/OOO' => '@rpath',
         'shl/OXT/URELIB' => '@executable_path/urelibs',
         'shl/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/NONE' => '@__VIA_LIBRARY_PATH__');
    my ($type, $loc1, $loc2) = @_;
    my $act = $action{"$type/$loc1/$loc2"};
    die "illegal combination $type/$loc1/$loc2" unless defined $act;
    return $act;
}

@ARGV >= 2 or die 'Usage: app|shl|fb UREBIN|URELIB|OOO|SDKBIN|OXT|NONE <filepath>*';
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
    close(IN) or die "got $? from $call";
    if ($change ne "")
    {
        $call = "$ENV{'XCRUN'} install_name_tool$change $file";
        system($call) == 0 or die "cannot $call";
    }
}
