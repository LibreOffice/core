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
    $iswin64build = 0;
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
    $workpath = ""; # installation working dir; some helper scripts are
    # placed here by gbuild
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
    $lcidlistname = "msi-encodinglist.txt";
    $msilanguage = "";  # hash reference for msi languages LCID
    $sofficeiconadded = 0;
    $temppath = "";
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
    $pffcabfilename = "ooobasis3.0_pff.cab";
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
    $ooodownloadfilename = "";
    $downloadfilename = "";
    $downloadfileextension = "";
    %multilingual_only_modules = ();
    %application_modules = ();

    $is_copy_only_project = 0;
    $is_simple_packager_project = 0;
    $patch_user_dir = 0;
    $languagepack = 0;
    $devsnapshotbuild = 0;
    $helppack = 0;
    $refresh_includepaths = 0;
    $include_paths_read = 0;
    @patchfilecollector = ();
    $desktoplinkexists = 0;
    $analyze_spellcheckerlanguage = 0;
    %spellcheckerlanguagehash = ();
    %spellcheckerfilehash = ();
    $registryrootcomponent = "";
    %allcomponents = ();
    %allcomponents_in_this_database = ();
    %allshortcomponents = ();
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

    $fix_number_of_cab_files = 1;
    $cabfilecompressionlevel = 21; # Using LZX compression, possible values are: 15 | 16 | ... | 21 (best compression)
    $number_of_cabfiles = 1;    # only for $fix_number_of_cab_files = 1
    $include_cab_in_msi = 1;
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

    $postprocess_specialepm = 0;
    $postprocess_standardepm = 0;
    $mergemodules_analyzed = 0;

    @packagelistitems = ("module", "solarispackagename", "packagename", "copyright", "vendor", "description" );
    @featurecollector =();
    $msiassemblyfiles = "";
    $macinstallfilename = "macinstall.ulf";
    $extensioninstalldir = "gid_Dir_Share_Extension_Install";
    @languagenames = ();
    %componentcondition = ();
    %componentid = ();
    %allcabinets = ();
    %allcabinetassigns = ();
    %cabfilecounter = ();
    %lastsequence = ();
    %dontdeletecomponents = ();
    %allcalculated_guids = ();
    %calculated_component_guids = ();
    %all_english_languagestrings = ();
    %all_required_english_languagestrings = ();

    @removedirs = ();
    @removefiletable = ();

    if ( $^O =~ /cygwin/i )
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
