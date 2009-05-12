:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: make_patched_header.pl,v $
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


