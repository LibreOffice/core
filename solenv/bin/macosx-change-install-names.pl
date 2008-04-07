#*************************************************************************
#
#   $RCSfile: macosx-change-install-names.pl,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2008-04-07 12:21:13 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2008 by Sun Microsystems, Inc.
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

use lib ("$ENV{SOLARENV}/bin/modules");
use macosxotoolhelper;

sub action($$$)
{
    my %action =
        ('app/UREBIN/URELIB' => '@executable_path/../lib',
         'app/OOO/URELIB' => '@executable_path/../ure-link/lib',
         'app/OOO/OOO' => '@executable_path',
         'app/BRAND/URELIB' => '@executable_path/../basis-link/ure-link/lib',
         'app/BRAND/OOO' => '@executable_path/../basis-link/program',
         'shl/URELIB/URELIB' => '@loader_path',
         'shl/OOO/URELIB' => '@loader_path/../ure-link/lib',
         'shl/OOO/OOO' => '@loader_path',
         'shl/OXT/URELIB' => '@executable_path/urelibs');
    my ($type, $loc1, $loc2) = @_;
    my $act = $action{"$type/$loc1/$loc2"};
    die "illegal combination $type/$loc/$2" unless defined $act;
    return $act;
}

@ARGV == 3 || @ARGV >= 2 && $ARGV[0] eq "extshl" or
    die "Usage: app|shl|extshl UREBIN|URELIB|OOO|BRAND|OXT <filepath>*";
$type = shift @ARGV;
$loc = shift @ARGV;
if ($type eq "extshl")
{
    $type = "shl";
    my $change = "";
    foreach $file (@ARGV)
    {
        otoolD($file) =~ m'^(.*?([^/]+))\n$' or
            die "unexpected otool -D output";
        $change .= " -change $1 " . action($type, $loc, $loc) . "/$2";
        $iname{$file} = $2;
    }
    foreach $file (@ARGV)
    {
        my $call = "install_name_tool$change -id \@__________________________________________________$loc/$iname{$file} $file";
        system($call) == 0 or die "cannot $call";
    }
}
foreach $file (@ARGV)
{
    my $call = "otool -L $file";
    open(IN, "-|", $call) or die "cannot $call";
    my $change = "";
    while (<IN>)
    {
        $change .= " -change $1 " . action($type, $loc, $2) . "$3"
            if m'^\s*(@_{50}([^/]+)(/.+)) \(compatibility version \d+\.\d+\.\d+, current version \d+\.\d+\.\d+\)\n$';
    }
    close(IN);
    if ($change ne "")
    {
        $call = "install_name_tool$change $file";
        system($call) == 0 or die "cannot $call";
    }
}
