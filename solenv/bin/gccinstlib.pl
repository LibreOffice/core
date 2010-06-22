:
eval 'exec perl -wS $0 ${1+"$@"}'
   if 0;
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


foreach $File (@ARGV) {
    #https://bugzilla.redhat.com/show_bug.cgi?id=149465
    printf "unprelinking $Dest/$File\n";
    #If it's already unprelinked .i.e. no .gnu.prelink_undo section, that's fine
    #If prelink is not installed, it's massively unlikely that it's prelinked
    system ("prelink -u $Dest/$File > /dev/null 2>&1");
}

exit (0);
