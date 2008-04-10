:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: check_setup.pl,v $
#
# $Revision: 1.3 $
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

sub checkLogDat {
    my ($setuplogfile) = shift;
    my ($errorfound) = 0;

    open SETUPLOG, "<$setuplogfile" or die "can´t open file $setuplogfile";
    while(<SETUPLOG>) {
        $line = $_;
        chomp $line;
        if ($line =~ /^ERR/ ) {
            print "Error: $line\n";
            $errorfound = 1;
        }
    }
    close SETUPLOG;
    return !$errorfound; #check ok?
}

$idStr = ' $Revision: 1.3 $ ';
$idStr =~ /Revision:\s+(\S+)\s+\$/
  ? ($prg_rev = $1) : ($prg_rev = "-");

print "checkSetup -- Version: $prg_rev\n";

if (  ($#ARGV >-1) && ($#ARGV < 1) ) {
    $ARGV[0]=~ s/\"//g;
}

if ( ! ( ($#ARGV >-1) && ($#ARGV < 1) && $ARGV[0] && (-e $ARGV[0] ) ) ) {
    print "Usage: checkSetup <setuplogfile>\n" ;
    exit(1);
}

if (checkLogDat ($ARGV[0])) {
    exit (0);
}
else {
    print STDERR "ERROR: setup failed\n";
    exit (2);
}
