:
eval 'exec perl -wS $0 ${1+"$@"}'
    if 0;
#*************************************************************************
#
#   $RCSfile: config.pl,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: kz $ $Date: 2004-08-05 10:36:42 $
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
#   Contributor(s): _______________________________________
#
#
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


$common_XML = "Common.xcu";
$setup_XML = "Setup.xcu";

### main ###

$idStr = ' $Revision: 1.1 $ ';
$idStr =~ /Revision:\s+(\S+)\s+\$/
  ? ($cpflat2minor_rev = $1) : ($cpflat2minor_rev = "-");

if ( ($#ARGV >= 2) ) {
    $ARGV[0] =~ s/\"//g;
    $ARGV[1] =~ s/\"//g;
    $ARGV[2] =~ s/\"//g;
    chop($ARGV[0]);
    chop($ARGV[1]);
    chop($ARGV[2]);
}

if ( ! ( ($#ARGV >= 2) && $ARGV[0] && $ARGV[1] && $ARGV[2] && (-d $ARGV[2]) ) ) {
    print "Usage: config <installpath> <userinstallpath> <datapath>\n" ;
    exit(1);
}

$installdir = $ARGV[0];
$userinstalldir = $ARGV[1];
$datapath = $ARGV[2];

$fullquickstart_path = $installdir . "program" . $pathslash . "quickstart.exe";

print "patching config ... \n";
if (!-d "$userinstalldir$User_Office_Path") {
    mkpath("$userinstalldir$User_Office_Path", 0, 0777);
}

# copy Common.xcu

print "cp $datapath$common_XML $userinstalldir$User_Office_Path$common_XML\n" if $is_debug;
copy ("$datapath$common_XML", "$userinstalldir$User_Office_Path$common_XML");

# copy Setup.xcu

print "cp $datapath$setup_XML $userinstalldir$User_Path$setup_XML\n" if $is_debug;
copy ("$datapath$setup_XML", "$userinstalldir$User_Path$setup_XML");


# quickstarter loeschen
if ($gui ne "UNX") {
    print "kill $fullquickstart_path \n";
    unlink( $fullquickstart_path );
}

#delete joblist
$fullsource_path = $installdir . $jobs_XML_Path;
unlink ($fullsource_path);

exit(0);

