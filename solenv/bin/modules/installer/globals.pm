#*************************************************************************
#
#   $RCSfile: globals.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: svesik $ $Date: 2004-04-20 12:27:25 $
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

package installer::globals;

############################################
# Global settings
############################################

BEGIN
{
    $prog="make_installer.pl";

    $ziplistname = "";
    $pathfilename = "";
    $setupscriptname = "";
    $headerfilename = "";
    $shellscriptsfilename = "";
    $product = "";
    $languagelist = "";

    $rootpath = "";

    $productextension = "";
    @languageproducts = ();
    $build = "";
    $minor = "";
    $compiler = "";
    $pro = 0;
    $dounzip = 1;
    $languages_defined_in_productlist = 0;
    $setupscript_defined_in_productlist = 0;
    $use_default_ziplist = 0;
    $services_rdb_created = 0;
    $servicesrdb_can_be_created = 0;
    $islinux = 0;
    $issolaris = 0;
    $iswindowsbuild = 0;
    $islinuxbuild = 0;
    $issolarisbuild = 0;
    $unpackpath = "";
    $idttemplatepath = "";
    $idtlanguagepath = "";
    $msifilespath = "";
    $packjobref = "";
    $buildid = "Not set";
    $guidcounter = 1000; # for uniqueness of guids
    $fontsfolder = "FontsFolder";
    $fontsfoldername = "Fonts";
    $officefolder = "OfficeFolder";
    $officemenufolder = "OfficeMenuFolder";
    $programfilesfolder = "ProgramFilesFolder";
    $programmenufolder = "ProgramMenuFolder";
    $listfileproductname = "";

    $is_special_epm = 0;
    $epm_in_path = 0;
    $epm_path = "";
    $call_epm = 1;
    $packageformat = "";
    $packagename = "";
    $packagelist = "";

    $strip = 0;
    $solarjava = 0;
    $jdklib = "";
    $jrepath = "";

    $globallogging = 0;
    $logfilename = "logfile.log";   # the default logfile name for global errors
    @logfileinfo = ();
    @globallogfileinfo = ();
    $exitlog = "";

    $debug = 0;
    $debugfilename = "debug.txt";
    @functioncalls = ();

    $ismultilingual = 0;
    $isopensourceproduct = 1;
    $manufacturer = "";
    $codefilename = "codes.txt";
    $productcode = "";
    $upgradecode = "";

    $is_copy_only_project = 0;

    @environmentvariables = ( "SOLARVERSION", "GUI", "COMMON_OUTDIR", "ENV_ROOT", "WORK_STAMP", "UPDMINOR", "OUTPATH" );
    @packagelistitems = ("module", "solarispackagename", "packagename", "copyright", "vendor", "description" );
    @regcompjars = ( "unoil.jar", "java_uno.jar", "ridl.jar", "sandbox.jar", "jurt.jar", "juh.jar", "xmerge.jar" );
    @regcompregisterlibs = ( "javavm.uno", "javaloader.uno" );

    $plat = $^O;

    if (( $plat =~ /MSWin/i ) || (( $plat =~ /cygwin/i ) && ( $ENV{'USE_SHELL'} eq "4nt" )))
    {
        $unzippath = "unzip.exe";           # Has to be in the path: r:\btw\unzip.exe
        $zippath= "zip.exe";                # Has to be in the path: r:\btw\zip.exe
        $separator = "\\";
        $pathseparator = "\;";
        $libextension = "\.dll";
        $quote = "\"";
        $isunix = 0;
        $iswin = 1;
    }
    else
    {
        $unzippath = "unzip";               # Has to be in the path: /usr/bin/unzip
        $zippath = "zip";                   # Has to be in the path: /usr/bin/zip
        $separator = "/";
        $pathseparator = "\:";
        $libextension = "\.so";
        $quote = "\'";
        $isunix = 1;
        $iswin = 0;
    }

    if ( $plat =~ /linux/i ) { $islinux = 1; }
    if ( $plat =~ /solaris/i ) { $issolaris = 1; }

    # ToDo: Needs to be expanded for additional platforms

}

1;
