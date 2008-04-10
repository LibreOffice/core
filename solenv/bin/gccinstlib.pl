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
# $RCSfile: gccinstlib.pl,v $
#
# $Revision: 1.8 $
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

$ENV{'LC_MESSAGES'} = 'C';

$Dest = pop(@ARGV) || die "No destination to copy to";

$cc = $ENV{'CC'} || die "No CC environment set";

if ($Dest =~ /--help/ || @ARGV < 1) {
    print "Syntax:\n  gcc-instlib <library-in-libpath ...> <destination-dir>\n";
    exit (0);
}
foreach $File (@ARGV) {
    my $string;

    open (GCCOut, "LANGUAGE=C LC_ALL=C $cc -print-file-name=$File|") || die "Failed to exec $cc -print-file-name=$File $!";
    $string=<GCCOut>;
    chomp ($string);
    push (@CopySrc, $string);
    close (GCCOut);
}

foreach $Src (@CopySrc) {
    printf "copy $Src to $Dest\n";
    system ("/bin/cp $Src $Dest") && die "copy failed: $!";
}

exit (0);
