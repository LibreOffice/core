:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: check_setup.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 21:28:03 $
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

$idStr = ' $Revision: 1.2 $ ';
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
