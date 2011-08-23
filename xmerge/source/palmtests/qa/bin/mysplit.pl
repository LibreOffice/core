#!/usr/local/bin/perl
#########################################################################

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

open(TESTFILE, $ARGV[0]);

while (<TESTFILE>)
{
    if ($_[1] eq "-q")
    {
    chomp $_;
    @args = split('\|', $_);
    @filestuff = split('\.', @args[0]);
    $filename = @filestuff[0] . ".infile";
    open (TESTCASE, ">$filename") || die "Error opening $filename";
    print TESTCASE "# @args[1]\n# @args[2]\n";
    print TESTCASE "TEST|@args[1]|QUICKWORD|@filestuff[0]\n" ;
    close TESTCASE;
    }
    if ($_[1] eq "-m")
    {
    chomp $_;
    @args = split('\|', $_);
    @filestuff = split('\.', @args[0]);
    $filename = @filestuff[0] . ".infile";
    open (TESTCASE, ">$filename") || die "Error opening $filename";
    print TESTCASE "# @args[1]\n# @args[2]\n";
    print TESTCASE "TEST|@args[1]|MINICALC|@filestuff[0]\n" ;
    close TESTCASE;
    }
    
}
