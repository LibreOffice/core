:
eval 'exec perl -wS $0 ${1+"$@"}'
   if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: gccinstlib.pl,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:08:41 $
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
