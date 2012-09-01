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

package installer::globals;

############################################
# Global settings
############################################

BEGIN
{
    $ziplistname = "";
    $pathfilename = "";
    $setupscriptname = "";
    $product = "";
    $languagelist = "";
    $added_english = 0;
    $set_office_start_language = 0;

    $destdir = "";
    $rootpath = "";

    $required_dotnet_version = "2.0.0.0";
    $productextension = "";
    @languageproducts = ();
    $build = "";
    $minor = "";
    $lastminor = "";
    $compiler = "";
    $pro = 0;
    $dounzip = 1;
    $languages_defined_in_productlist = 0;
    $setupscript_defined_in_productlist = 0;
    $iswindowsbuild = 0;
    $islinuxbuild = 0;
    $isrpmbuild = 0;
    $isdebbuild = 0;
    $issolarisbuild = 0;
    $issolarispkgbuild = 0;
    $issolarissparcbuild = 0;
    $issolarisx86build = 0;
    $isfreebsdbuild = 0;
    $isfreebsdpkgbuild = 0;
    $ismacbuild = 0;
    $ismacdmgbuild = 0;
    $unpackpath = "";
    $idttemplatepath = "";
    $idtlanguagepath = "";
    $buildid = "Not set";
    $fontsfolder = "FontsFolder";
    $fontsfoldername = "Fonts";
    $fontsdirparent = "";
    $fontsdirname = "";
    $fontsdirhostname = "truetype";
    $officemenufolder = "OfficeMenuFolder";
    $startupfolder = "StartupFolder";
    $startmenufolder = "StartMenuFolder";
    $desktopfolder = "DesktopFolder";
    $programfilesfolder = "ProgramFilesFolder";
    $commonfilesfolder = "CommonFilesFolder";
    $commonappdatafolder = "CommonAppDataFolder";
    $localappdatafolder = "LocalAppDataFolder";
    $templatefolder = "TemplateFolder";
    $templatefoldername = "Templates";
    $programmenufolder = "ProgramMenuFolder";
    $systemfolder = "SystemFolder";
    $encodinglistname = "msi-encodinglist.txt";
    $msiencoding = "";  # hash reference for msi encodings
    $msilanguage = "";  # hash reference for msi languages LCID
    $sofficeiconadded = 0;
    $temppath = "";
    $globaltempdirname = "ooopackaging";
    $cyg_temppath = "";
    $temppathdefined = 0;
    $packageversion = 1;
    $packagerevision = 1;
    $rpm = "";
    $rpminfologged = 0;
    $debian = "";
    $installertypedir = "";
    $controlledmakecabversion = "5";
    $max_lang_length = 50;
    $globalblock = "Globals";
    $rootmodulegid = "";
    %alllangmodules = ();
    $englishlicenseset = 0;
    $englishlicense = "";
    $englishsolarislicensename = "LICENSE"; # _en-US";
    $solarisdontcompress = 0;
    $patharray = "";

    $is_special_epm = 0;
    $epm_in_path = 0;
    $epm_path = "";
    $epmoutpath = "";
    $simple = 0;
    $simpledefaultuserdir = "\$ORIGIN/..";
    $call_epm = 1;
    $packageformat = "";
    $packagename = "";
    $packagelist = "";
    $shiptestdirectory = "";
    $archiveformat = "";
    $updatelastsequence = 0;
    $updatesequencecounter = 0;
    $updatedatabase = 0;
    $updatedatabasepath = "";
    $pfffileexists = 0;
    $pffcabfilename = "ooobasis3.0_pff.cab";
    $mergemodulenumber = 0;
    %allmergemodulefilesequences = ();
    %newupdatefiles = ();
    %allusedupdatesequences = ();
    %mergemodulefiles = ();
    $mergefiles_added_into_collector = 0;
    $creating_windows_installer_patch = 0;

    $strip = 0;

    $logfilename = "logfile.log";   # the default logfile name for global errors
    @logfileinfo = ();
    @errorlogfileinfo = ();
    @globallogfileinfo = ();
    $ignore_error_in_logfile = 0;
    $exitlog = "";
    $globalinfo_copied = 0;
    $quiet = 0;

    $ismultilingual = 0;
    %alluniquefilenames = ();
    %alllcuniquefilenames = ();
    %uniquefilenamesequence = ();
    %dependfilenames = ();
    $manufacturer = "";
    $longmanufacturer = "";
    $codefilename = "codes.txt";
    $componentfilename = "components.txt";
    $productcode = "";
    $upgradecode = "";
    $msiproductversion = "";
    $msimajorproductversion = "";
    @allddffiles = ();
    $infodirectory = "";

    %mergemodules = ();
    %merge_media_line = ();
    %merge_allfeature_hash = ();
    %merge_alldirectory_hash = ();
    %copy_msm_files = ();
    $mergefeaturecollected = 0;
    $mergedirectoriescollected = 0;
    $lastsequence_before_merge = 0;
    $lastcabfilename = "";

    $defaultlanguage = "";
    $addlicensefile = 1;
    $addsystemintegration = 0;
    $makedownload = 1;
    @binarytableonlyfiles = ();
    @allscpactions = ();
    $languagepackaddon = "LanguagePack";
    $patchaddon = "Patch";
    $ooodownloadfilename = "";
    $downloadfilename = "";
    $downloadfileextension = "";
    $shellnewfilesadded = 0;
    %multilingual_only_modules = ();
    %application_modules = ();

    $is_copy_only_project = 0;
    $is_simple_packager_project = 0;
    $patch_user_dir = 0;
    $languagepack = 0;
    $helppack = 0;
    $patch = 0;
    $patchincludepath = "";
    $refresh_includepaths = 0;
    $include_paths_read = 0;
    @patchfilecollector = ();
    $nopatchfilecollector = "";
    @userregistrycollector = ();
    $addeduserregitrykeys = 0;
    $desktoplinkexists = 0;
    $analyze_spellcheckerlanguage = 0;
    %spellcheckerlanguagehash = ();
    %spellcheckerfilehash = ();
    $registryrootcomponent = "";
    %allcomponents = ();
    %allcomponents_in_this_database = ();
    %allshortcomponents = ();
    %alluniquedirectorynames = ();
    %allregistrycomponents_ = ();
    %allregistrycomponents_in_this_database_ = ();
    %allshortregistrycomponents = ();

    $installlocationdirectory = "";
    $installlocationdirectoryset = 0;
    $vendordirectory = "";
    $officeinstalldirectory = "";
    $rootbrandpackage = "";
    $rootbrandpackageset = 0;
    $officedirhostname = "";
    $officedirgid = "";

    %treestyles = ();
    %treelayername = ();
    %hostnametreestyles = ();
    %treeconditions = ();
    %usedtreeconditions = ();
    %moduledestination = ();

    $one_cab_file = 0;
    $fix_number_of_cab_files = 1;
    $cab_file_per_component = 0;
    $cabfilecompressionlevel = 21; # Using LZX compression, possible values are: 15 | 16 | ... | 21 (best compression)
    $number_of_cabfiles = 1;    # only for $fix_number_of_cab_files = 1
    $include_cab_in_msi = 1;
    $use_packages_for_cabs = 0;
    $msidatabasename = "";
    $prepare_winpatch = 0;
    $previous_idt_dir = "";
    $msitranpath = "";
    $insert_file_at_end = 0;
    $newfilesexist = 0;
    $usesharepointpath = 0;
    %newfilescollector = ();

    $saveinstalldir = "";
    $csp_installdir = "";       # global installdir of createsimplepackage() in simplepackage.pm
    $csp_installlogdir = "";    # global installlogdir of createsimplepackage() in simplepackage.pm
    $csp_languagestring = "";   # global languagestring of createsimplepackage() in simplepackage.pm
    $localunpackdir = "";
    $localinstalldirset = 0;
    $localinstalldir = "";

    $urefile = "";

    $postprocess_specialepm = 0;
    $postprocess_standardepm = 0;
    $mergemodules_analyzed = 0;

    @solarispatchscripts = ("checkinstall", "copyright", "patch_checkinstall", "patch_postinstall", "postinstall", "preinstall", "i.none");
    @solarispatchscriptsforextensions = ("checkinstall", "copyright", "patch_checkinstall", "patch_postinstall_extensions", "postinstall_extensions", "preinstall", "i.none");
    @solarispatchfiles = (".diPatch", "patchinfo");
    @environmentvariables = ( "SOLARVERSION", "GUI", "WORK_STAMP", "OUTPATH", "LOCAL_OUT", "LOCAL_COMMON_OUT" );
    @packagelistitems = ("module", "solarispackagename", "packagename", "copyright", "vendor", "description" );
    @languagepackfeature =();
    @helppackfeature =();
    @featurecollector =();
    $msiassemblyfiles = "";
    $macinstallfilename = "macinstall.ulf";
    $extensioninstalldir = "gid_Dir_Share_Extension_Install";
    @languagenames = ();
    @requiredpackages = ();
    %componentcondition = ();
    %componentid = ();
    %comparecomponentname = ();
    %allcabinets = ();
    %allcabinetassigns = ();
    %cabfilecounter = ();
    %lastsequence = ();
    %dontdeletecomponents = ();
    %allcalculated_guids = ();
    %calculated_component_guids = ();
    %base_independent_components = ();
    %all_english_languagestrings = ();
    %all_required_english_languagestrings = ();

    @forced_properties = ("SERVICETAG_PRODUCTNAME", "SERVICETAG_PRODUCTVERSION", "SERVICETAG_PARENTNAME", "SERVICETAG_SOURCE", "SERVICETAG_URN");

    @removedirs = ();
    @removefiletable = ();

    $plat = $^O;

    if ( $plat =~ /cygwin/i )
    {
        $zippath = "zip";                   # Has to be in the path: /usr/bin/zip
        $separator = "/";
        $pathseparator = "\:";
        $isunix = 0;
        $iswin = 1;
        $archiveformat = ".zip";
        %savedmapping = ();
        %savedrevmapping = ();
        %savedrev83mapping = ();
        %saved83dirmapping = ();
    }
    else
    {
        $zippath = "zip";                   # Has to be in the path: /usr/bin/zip
        $separator = "/";
        $pathseparator = "\:";
        $archiveformat = ".tar.gz";
        $isunix = 1;
        $iswin = 0;
    }

}

1;
