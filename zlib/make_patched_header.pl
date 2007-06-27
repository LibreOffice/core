:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: make_patched_header.pl,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: hr $ $Date: 2007-06-27 14:34:43 $
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
#
# make_patched_header - make patched header
#

use strict;
use File::Basename;
use File::Path;
use Carp;

my $patched_file = shift @ARGV;
$patched_file =~ s/\\/\//g;
my $module = shift @ARGV;
my $patch_dir = dirname($patched_file);
my $orig_file = $patched_file;
$orig_file =~ s/\/patched\//\//;

if (!-f $orig_file) { carp("Cannot find file $orig_file\n"); };
if (!-d $patch_dir) {
    mkpath($patch_dir, 0, 0775);
    if (!-d $patch_dir) {("mkdir: could not create directory $patch_dir\n"); };
};

open(PATCHED_FILE, ">$patched_file") or carp("Cannot open file $patched_file\n");
open(ORIG_FILE, "<$orig_file") or carp("Cannot open file $orig_file\n");
foreach (<ORIG_FILE>) {
    if (/#include\s*"(\w+\.h\w*)"/) {
        my $include = $1;
        s/#include "$include"/#include <$module\/$include>/g;
    };
    print PATCHED_FILE $_;
};
close PATCHED_FILE;
close ORIG_FILE;

exit(0);


