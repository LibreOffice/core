:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: createpdbrelocators.pl,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 22:05:37 $
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

#*************************************************************************
#
# createpdbrelocators - create for pdb relocator files
#                       PDB relocator files are used to find debug infos
#                       for analysis of creash reports
#
# usage: createpdbrelocators;
#
#*************************************************************************

use strict;

#### module lookup

use lib ("$ENV{SOLARENV}/bin/modules");
use CreatePDBRelocators;

#### script id #####

( my $script_name = $0 ) =~ s/^.*\b(\w+)\.pl$/$1/;

my $script_rev;
my $id_str = ' $Revision: 1.4 $ ';
$id_str =~ /Revision:\s+(\S+)\s+\$/
  ? ($script_rev = $1) : ($script_rev = "-");

print "$script_name -- version: $script_rev\n";

my $inpath    = $ENV{INPATH};
my $milestone = $ENV{UPDMINOR};

if ( $ARGV[0] ) {
    if ( $milestone && ( $milestone ne $ARGV[0] ) ) {
        die "Error: specified milestone $ARGV[0] does not match your environment";
    }
    $milestone = $ARGV[0];
}

if ( !$inpath || !$milestone ) {
    print STDERR "$script_name: INAPTH or UPDMINOR not set!\n";
    exit(1);
}

my $rc = CreatePDBRelocators::create_pdb_relocators($inpath, $milestone, "");

if ( !$rc ) {
    print STDERR "$script_name: creating PDB relocators failed!\n";
    exit(2);
}

exit(0);
