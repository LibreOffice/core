#!/usr/local/bin/perl
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: mysplit.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 10:27:44 $
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
