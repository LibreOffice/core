:
eval 'exec perl -wS $0 ${1+"$@"}'
   if 0;
#*************************************************************************
#
#   $RCSfile: gccinstlib.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2003-04-28 16:41:16 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): Michael Meeks
#
#
#
#*************************************************************************

%SearchDirs = GetGccSearchDirs ();

$LibPaths = $SearchDirs{'libraries'} || die 'Foo';

@Paths = split(':', $LibPaths);

$Dest = pop(@ARGV) || die "No destination to copy to";

if ($Dest =~ /--help/ || @ARGV < 1) {
    printf ("Syntax:\n  gcc-instlib <library-in-libpath ...>
        <destination-dir>\n");
    exit (0);
}
foreach $File (@ARGV) {
   my $found = 0;
   foreach $Path (@Paths) {
        if ( -e $Path.$File) {
           push (@CopySrc, $Path.$File);
           $found = 1;
           last;
        }
   }
   if (!$found) {
           die "Couldn't find file $File in path\n";
   }
}

foreach $Src (@CopySrc) {
    printf "copy $Src to $Dest\n";
    system ("/bin/cp $Src $Dest") && die "copy failed: $!";
}

exit (0);

sub GetGccSearchDirs {
    my %Dirs = ();
    my $cc;

    $cc = $ENV{'CC'} || die "No CC environment set";

    open (GCCOut, "$cc -print-search-dirs|") || die "Failed to exec $cc -print-search-dirs: $!";

    while (<GCCOut>) {
        if (/^([a-zA-Z]+): [=]{0,1}(.*)/) {
            $Dirs{$1} = $2;
        }
    }

    close (GCCOut);

    return %Dirs;
}
