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

$branddir = $ARGV[0];
$userinstalldir = $ARGV[1];
$buildid  = $ARGV[2];

if ($ENV{GUI} ne "UNX") {
    $quickstart = $branddir . "program/quickstart.exe";
    print "kill $quickstart\n";
    unlink $quickstart or die "cannot unlink $fullquickstart_path";
}

$regpath = $userinstalldir . "user";
$regfile = $regpath . "/registrymodifications.xcu";
print "create $regfile\n";
mkpath($regpath, 0, 0777);
open (OUT, "> $regfile") or die "cannot open $regfile for writing";
print OUT << "EOF";
<oor:items xmlns:oor='http://openoffice.org/2001/registry'>
 <item oor:path='/org.openoffice.Office.Common/Help/Registration'>
  <prop oor:name='ReminderDate'>
   <value>Patch$buildid</value>
  </prop>
 </item>
 <item oor:path='/org.openoffice.Office.Common/Misc'>
  <prop oor:name='FirstRun'>
   <value>false</value>
  </prop>
 </item>
 <item oor:path='/org.openoffice.Office.Common/Security/Scripting'>
  <prop oor:name='MacroSecurityLevel'>
   <value>0</value>
  </prop>
  <prop oor:name='OfficeBasic'>
   <value>2</value>
  </prop>
 </item>
 <item oor:path='/org.openoffice.Office.OOoImprovement.Settings/Participation'>
  <prop oor:name='InvitationAccepted'>
   <value>false</value>
  </prop>
  <prop oor:name='ShowedInvitation'>
   <value>true</value>
  </prop>
 </item>
 <item oor:path='/org.openoffice.Setup/Office'>
  <prop oor:name='FirstStartWizardCompleted'>
   <value>true</value>
  </prop>
  <prop oor:name='LicenseAcceptDate'>
   <value>2100-01-01T00:00:00</value>
  </prop>
 </item>
</oor:items>
EOF
close OUT;
