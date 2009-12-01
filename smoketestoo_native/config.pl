:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: config.pl,v $
#
# $Revision: 1.5 $
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

use File::Path;
use File::Copy;

### globals ###

$is_debug = 0;

$gui = $ENV{GUI};
$cygwin = "cygwin";

if ($^O =~ /cygwin/) {
    $gui = $cygwin;
}

if (($gui eq "UNX") or ($gui eq $cygwin)) {
    $pathslash = "/";
}
else
{
    $pathslash = "\\";
}

#### Hardly coded path for
# \share\registry\data\org\openoffice\Office\Jobs.xcu

$jobs_XML_Path = "share" . $pathslash  . "registry" . $pathslash . "data" . $pathslash . "org" . $pathslash . "openoffice" . $pathslash . "Office" .  $pathslash . "Jobs.xcu";

# \user\registry\data\org\openoffice\

$User_Path = "user" . $pathslash  . "registry" . $pathslash . "data" . $pathslash . "org" . $pathslash . "openoffice" . $pathslash;

# \user\registry\data\org\openoffice\Office\

$User_Office_Path = $User_Path . "Office" . $pathslash;

# \user\registry\data\org\openoffice\Office\OOoImprovement

$OOoImprovement_Path = $User_Office_Path . "OOoImprovement" . $pathslash;

$common_XML = "Common.xcu";
$setup_XML = "Setup.xcu";
$oooimprovement_XML = "Settings.xcu";

# $(INPATH)\misc\Common.xcu
$inpath_Common_XCU = $ENV{INPATH} . $pathslash  . "misc" . $pathslash  .  $common_XML;

### main ###

$idStr = ' $Revision: 1.5 $ ';
$idStr =~ /Revision:\s+(\S+)\s+\$/
  ? ($cpflat2minor_rev = $1) : ($cpflat2minor_rev = "-");

if ( ($#ARGV >= 3) ) {
    $ARGV[0] =~ s/\"//g;
    $ARGV[1] =~ s/\"//g;
    $ARGV[2] =~ s/\"//g;
    $ARGV[3] =~ s/\"//g;
    chop($ARGV[0]);
    chop($ARGV[1]);
    chop($ARGV[2]);
    chop($ARGV[3]);
}

if ( ! ( ($#ARGV >= 4) && $ARGV[0] && $ARGV[1] && $ARGV[2] && $ARGV[3] && (-d $ARGV[3]) && $ARGV[4] ) ) {
    print "Usage: config <basispath> <brandpath> <userinstallpath> <datapath> <buildid>\n" ;
    exit(1);
}

$basisdir = $ARGV[0];
$branddir = $ARGV[1];
$userinstalldir = $ARGV[2];
$datapath = $ARGV[3];
$buildid  = $ARGV[4];

$fullquickstart_path = $branddir . "program" . $pathslash . "quickstart.exe";

print "patching config ... \n";
if (!-d "$userinstalldir$User_Office_Path") {
    mkpath("$userinstalldir$User_Office_Path", 0, 0777);
}
if (!-d "$userinstalldir$OOoImprovement_Path") {
    mkpath("$userinstalldir$OOoImprovement_Path", 0, 0777);
}

# copy Common.xcu
print "Patching Common.xcu\n" if $is_debug;
PatchCommonXcu($buildid);
print "cp $inpath_Common_XCU $userinstalldir$User_Office_Path$common_XML\n" if $is_debug;
copy ("$inpath_Common_XCU", "$userinstalldir$User_Office_Path$common_XML");

# copy OOoImprovement/Settings.xcu

print "cp $datapath$oooimprovement_XML $userinstalldir$OOoImprovement_Path$oooimprovement_XML\n" if $is_debug;
copy ("$datapath$oooimprovement_XML", "$userinstalldir$OOoImprovement_Path$oooimprovement_XML");

# copy Setup.xcu

print "cp $datapath$setup_XML $userinstalldir$User_Path$setup_XML\n" if $is_debug;
copy ("$datapath$setup_XML", "$userinstalldir$User_Path$setup_XML");


# quickstarter loeschen
if ($gui ne "UNX") {
    print "kill $fullquickstart_path \n";
    unlink( $fullquickstart_path ) or die "cannot unlink $fullquickstart_path";
}

#delete joblist
$fullsource_path = $basisdir . $jobs_XML_Path;
unlink ($fullsource_path);

exit(0);

############################################################################
sub PatchCommonXcu      #17.04.2009 10:37
############################################################################
 {
    my $buildid = shift;
    open(INFILE, "< $datapath$common_XML") || die "Can't open $datapath$common_XML (read)\n";
    open(OUTFILE, "> $inpath_Common_XCU") || die "Can't open $inpath_Common_XCU (write)\n";
    my $patch_next_line = 0;
    my $value = "<value>Patch" . $buildid . "</value>\n";
    while ( $line = <INFILE> ) {
        if ( $patch_next_line ) {
            print OUTFILE "      $value";
            $patch_next_line = 0;
        } else
        {
            print OUTFILE $line;
        }
        if ( $line =~ /ReminderDate/ ) {
            $patch_next_line = 1;
        }
    }
    close(INFILE);
    close(OUTFILE);
}   ##PatchCommonXcu
