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



use lib ("$ENV{SOLARENV}/bin/modules");
use macosxotoolhelper;

sub action($$$)
{
    my %action =
        ('app/UREBIN/URELIB' => '@executable_path',
         'app/OOO/URELIB' => '@executable_path/',
         'app/OOO/OOO' => '@executable_path',
         'app/SDK/URELIB' => '@executable_path',
         'app/BRAND/URELIB' => '@executable_path',
         'app/BRAND/OOO' => '@executable_path',
         'app/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
         'app/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
         'app/NONE/NONE' => '@__VIA_LIBRARY_PATH__',
         'shl/URELIB/URELIB' => '@loader_path',
         'shl/OOO/URELIB' => '@loader_path',
         'shl/OOO/OOO' => '@loader_path',
         'shl/LOADER/LOADER' => '@loader_path',
         'shl/OXT/URELIB' => '@executable_path',
         'shl/BOXT/URELIB' => '@executable_path',
         'shl/BOXT/OOO' => '@loader_path',
         'shl/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
         'shl/NONE/NONE' => '@__VIA_LIBRARY_PATH__');

#        ('app/UREBIN/URELIB' => '@executable_path/../lib',
#         'app/OOO/URELIB' => '@executable_path/../ure-link/lib',
#         'app/OOO/OOO' => '@executable_path',
#         'app/SDK/URELIB' => '@executable_path/../../ure-link/lib',
#         'app/BRAND/URELIB' => '@executable_path/../basis-link/ure-link/lib',
#         'app/BRAND/OOO' => '@executable_path/../basis-link/program',
#         'app/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
#         'app/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
#         'app/NONE/NONE' => '@__VIA_LIBRARY_PATH__',
#         'shl/URELIB/URELIB' => '@loader_path',
#         'shl/OOO/URELIB' => '@loader_path/../ure-link/lib',
#         'shl/OOO/OOO' => '@loader_path',
#         'shl/LOADER/LOADER' => '@loader_path',
#         'shl/OXT/URELIB' => '@executable_path/urelibs',
#         'shl/BOXT/URELIB' => '@executable_path/urelibs',
#         'shl/BOXT/OOO' => '@loader_path/../../../basis-link/program',
#         'shl/NONE/URELIB' => '@__VIA_LIBRARY_PATH__',
#         'shl/NONE/OOO' => '@__VIA_LIBRARY_PATH__',
#         'shl/NONE/NONE' => '@__VIA_LIBRARY_PATH__');

    my ($type, $loc1, $loc2) = @_;
    my $act = $action{"$type/$loc1/$loc2"};
    die "illegal combination $type/$loc1/$loc2" unless defined $act;
    return $act;
}

@ARGV == 3 || @ARGV >= 2 && $ARGV[0] eq "extshl" or die
  'Usage: app|shl|extshl UREBIN|URELIB|OOO|SDK|BRAND|OXT|BOXT|NONE|LOADER <filepath>*';
$type = shift @ARGV;
$loc = shift @ARGV;
if ($type eq "SharedLibrary")
{
    $type = "shl";
}
if ($type eq "Executable")
{
    $type = "app"
}
if ($type eq "Library")
{
    $type = "shl"
}
if ($type eq "extshl")
{
    $type = "shl";
    my $change = "";
    my %inames;
    foreach $file (@ARGV)
    {
        my $iname = otoolD($file);
        (defined $iname ? $iname : $file . "\n") =~ m'^(.*?([^/]+))\n$' or
            die "unexpected otool -D output";
        $change .= " -change $1 " . action($type, $loc, $loc) . "/$2";
        $inames{$file} = $2;
    }
    if( $loc eq "LOADER" )
    {
        foreach $file (@ARGV)
        {
            my $call = "${::CC_PATH}install_name_tool$change -id \@loader_path/$inames{$file} $file";
            system($call) == 0 or die "cannot $call";
        }
    }
    else
    {
        foreach $file (@ARGV)
        {
            my $call = "${::CC_PATH}install_name_tool$change -id \@_______$loc/$inames{$file} $file";
            system($call) == 0 or die "cannot $call";
        }
    }
}
foreach $file (@ARGV)
{
    my $call = "${::CC_PATH}otool -L $file";
    open(IN, "-|", $call) or die "cannot $call";
    my $change = "";
    while (<IN>)
    {
        $change .= " -change $1 " . action($type, $loc, $2) . "$3"
            if m'^\s*(@_{7}([^/]+)(/.+)) \(compatibility version \d+\.\d+\.\d+, current version \d+\.\d+\.\d+\)\n$';
    }
    close(IN);
    if ($change ne "")
    {
        $call = "${::CC_PATH}install_name_tool$change $file";
        system($call) == 0 or die "cannot $call";
    }
}
