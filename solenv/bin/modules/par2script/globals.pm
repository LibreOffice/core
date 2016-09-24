#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#


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
                 "Folder", "FolderItem", "FolderItemProperty", "RegistryItem",
                 "WindowsCustomAction", "MergeModule");

    @items_assigned_at_modules = ("File", "Directory", "Unixlink");
    @items_with_directories = ("File", "Profile", "Shortcut", "Unixlink");
    @items_with_moduleid = ("Profile", "ProfileItem", "FolderItem", "RegistryItem");
    @items_without_moduleid = ("File", "FolderItemProperty", "Directory", "Shortcut", "Unixlink");

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
