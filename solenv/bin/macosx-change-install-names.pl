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

use lib ("$ENV{SOLARENV}/bin/modules");
use macosxotoolhelper;

sub action($$$)
{
    my %action =
        ('app/UREBIN/URELIB' => '@executable_path/../lib',
         'app/OOO/URELIB' => '@executable_path/../ure-link/lib',
         'app/OOO/OOO' => '@executable_path',
         'app/SDK/URELIB' => '@executable_path/../../ure-link/lib',
         'app/BRAND/URELIB' => '@executable_path/../basis-link/ure-link/lib',
         'app/BRAND/OOO' => '@executable_path/../basis-link/program',
         'app/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
         'app/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
         'app/NONE/NONE' => '@__VIA_LIBRARY_PATH__',
         'shl/URELIB/URELIB' => '@loader_path',
         'shl/OOO/URELIB' => '@loader_path/../ure-link/lib',
         'shl/OOO/OOO' => '@loader_path',
         'shl/OXT/URELIB' => '@executable_path/urelibs',
         'shl/BOXT/URELIB' => '@executable_path/urelibs',
         'shl/BOXT/OOO' => '@loader_path/../../../basis-link/program',
         'shl/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/NONE' => '@__VIA_LIBRARY_PATH__');
    my ($type, $loc1, $loc2) = @_;
    my $act = $action{"$type/$loc1/$loc2"};
    die "illegal combination $type/$loc1/$loc2" unless defined $act;
    return $act;
}

@ARGV == 3 || @ARGV >= 2 && $ARGV[0] eq "extshl" or die
  'Usage: app|shl|extshl UREBIN|URELIB|OOO|SDK|BRAND|OXT|BOXT|NONE <filepath>*';
$type = shift @ARGV;
$loc = shift @ARGV;
if ($type eq "SharedLibrary")
{
    $type = "shl";
}
if ($type eq "Executable")
{
    $type = "app"
}
if ($type eq "Library")
{
    $type = "shl"
}
if ($type eq "extshl")
{
    $type = "shl";
    my $change = "";
    my %inames;
    foreach $file (@ARGV)
    {
        my $iname = otoolD($file);
        (defined $iname ? $iname : $file . "\n") =~ m'^(.*?([^/]+))\n$' or
            die "unexpected otool -D output";
        $change .= " -change $1 " . action($type, $loc, $loc) . "/$2";
        $inames{$file} = $2;
    }
    foreach $file (@ARGV)
    {
        my $call = "install_name_tool$change -id \@__________________________________________________$loc/$inames{$file} $file";
        system($call) == 0 or die "cannot $call";
    }
}
foreach $file (@ARGV)
{
    my $call = "otool -L $file";
    open(IN, "-|", $call) or die "cannot $call";
    my $change = "";
    while (<IN>)
    {
        $change .= " -change $1 " . action($type, $loc, $2) . "$3"
            if m'^\s*(@_{50}([^/]+)(/.+)) \(compatibility version \d+\.\d+\.\d+, current version \d+\.\d+\.\d+\)\n$';
    }
    close(IN);
    if ($change ne "")
    {
        $call = "install_name_tool$change $file";
        system($call) == 0 or die "cannot $call";
    }
}
