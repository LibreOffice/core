#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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


package pre2par::globals;

############################################
# Global settings
############################################

BEGIN
{
    $prog="pre2par";

    $prefilename = "";
    $parfilename = "";
    $langfilename = "";

    @allitems = ("Installation", "ScpAction", "HelpText", "Directory", "DataCarrier", "StarRegistry", "File",
                 "Shortcut", "Custom", "Unixlink", "Procedure", "Module", "Profile", "ProfileItem",
                 "Folder", "FolderItem", "RegistryItem", "StarRegistryItem", "WindowsCustomAction",
                 "MergeModule");

    $logging     = 0;
    $logfilename = "logfile.log";   # the default logfile name for global errors
    @logfileinfo = ();

    $plat = $^O;

    $separator = "/";
    $pathseparator = "\:";
    $isunix = 1;
    $iswin = 0;

    $islinux = 0;
    $issolaris = 0;

    if ( $plat =~ /linux/i ) { $islinux = 1; }
    if ( $plat =~ /kfreebsd/i ) { $islinux = 1; }
    if ( $plat =~ /solaris/i ) { $issolaris = 1; }

}

1;
