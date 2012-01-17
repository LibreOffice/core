:
eval 'exec perl -S $0 ${1+"$@"}'
    if 0;
#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************


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


