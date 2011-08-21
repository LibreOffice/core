#!/usr/bin/perl -w
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

sub usage
{
    print "Cleanup unwanted unix attributes in zip archives\n\n";
    print "Usage:\n";
    print "$0 archive\n\n";
    exit(1);
}

usage() if ! defined $ARGV[0];

my $filename = $ARGV[0];
use Archive::Zip qw(:ERROR_CODES :CONSTANTS);
my $zip = Archive::Zip->new();

unless ( $zip->read( $filename ) == AZ_OK ) {
    die "$0: ERROR reading $filename\n";
}
my @members = $zip ->members();

foreach my $member ( @members ) {
    my $attribs = $member->unixFileAttributes();
    if ( $member->isDirectory ) {
        $attribs = $attribs & 0b101111111111;
        $member->unixFileAttributes($attribs)
    }
}
unless ( $zip->writeToFileNamed( ${filename}."_new" ) == AZ_OK ) {
    die "$0: ERROR reading ${filename}_new\n";
}
rename($filename."_new", $filename);

