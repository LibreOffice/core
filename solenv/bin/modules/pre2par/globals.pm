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
                 "Folder", "FolderItem", "RegistryItem", "StarRegistryItem", "WindowsCustomAction");

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
