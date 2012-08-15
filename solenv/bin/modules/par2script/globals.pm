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


package par2script::globals;

############################################
# Global settings
############################################

BEGIN
{
    $prog="par2script";

    $includepathlist = "";
    $scriptname = "";
    $parfilelistorig = "";
    $parfilelist = "";

    @allitems = ("Installation", "ScpAction", "Directory", "File",
                 "Shortcut", "Unixlink", "Module", "Profile", "ProfileItem",
                 "Folder", "FolderItem", "RegistryItem", "WindowsCustomAction",
                 "MergeModule");

    @items_assigned_at_modules = ("File", "Directory", "Unixlink");
    @items_with_directories = ("File", "Profile", "Shortcut", "Unixlink");
    @items_with_moduleid = ("Profile", "ProfileItem", "FolderItem", "RegistryItem");
    @items_without_moduleid = ("File", "Directory", "Shortcut", "Unixlink");

    %searchkeys = ("File" => "Files", "Directory" => "Dirs", "Unixlink" => "Unixlinks");

    $logging = 0;
    $logfilename = "logfile.log";   # the default logfile name for global errors
    @logfileinfo = ();

    $verbose = 1;

    $multidefinitionerror = 0;
    $multiassignmenterror = 0;

    %definitions;
    %assignedgids;

    $plat = $^O;

    $separator = "/";
    $pathseparator = "\:";
    $isunix = 1;
    $iswin = 0;
}

1;
