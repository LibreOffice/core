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

use strict;
use warnings;

############################################
# Global settings
############################################

BEGIN
{
    $installer::globals::ziplistname = "";
    $installer::globals::pathfilename = "";
    $installer::globals::setupscriptname = "";
    $installer::globals::product = "";
    $installer::globals::languagelist = "";
    $installer::globals::added_english = 0;
    $installer::globals::set_office_start_language = 0;

    $installer::globals::destdir = "";
    $installer::globals::rootpath = "";

    @installer::globals::languageproducts = ();
    $installer::globals::build = "";
    $installer::globals::os = "";
    $installer::globals::cpuname = "";
    $installer::globals::com = "";
    $installer::globals::platformid = "";
    $installer::globals::pro = 0;
    $installer::globals::dounzip = 1;
    $installer::globals::languages_defined_in_productlist = 0;
    $installer::globals::setupscript_defined_in_productlist = 0;
    $installer::globals::iswindowsbuild = 0;
    $installer::globals::islinuxbuild = 0;
    $installer::globals::isrpmbuild = 0;
    $installer::globals::isdebbuild = 0;
    $installer::globals::issolarisbuild = 0;
    $installer::globals::issolarispkgbuild = 0;
    $installer::globals::issolarissparcbuild = 0;
    $installer::globals::issolarisx86build = 0;
    $installer::globals::isfreebsdbuild = 0;
    $installer::globals::isfreebsdpkgbuild = 0;
    $installer::globals::ismacbuild = 0;
    $installer::globals::ismacdmgbuild = 0;
    $installer::globals::unpackpath = "";
    $installer::globals::workpath = ""; # installation working dir; some helper scripts are
    # placed here by gbuild
    $installer::globals::idttemplatepath = "";
    $installer::globals::idtlanguagepath = "";
    $installer::globals::buildid = "Not set";
    $installer::globals::fontsfolder = "FontsFolder";
    $installer::globals::fontsfoldername = "Fonts";
    $installer::globals::fontsdirparent = "";
    $installer::globals::fontsdirname = "";
    $installer::globals::fontsdirhostname = "truetype";
    $installer::globals::officemenufolder = "OfficeMenuFolder";
    $installer::globals::startupfolder = "StartupFolder";
    $installer::globals::startmenufolder = "StartMenuFolder";
    $installer::globals::desktopfolder = "DesktopFolder";
    $installer::globals::programfilesfolder = "ProgramFilesFolder";
    $installer::globals::commonfilesfolder = "CommonFilesFolder";
    $installer::globals::commonappdatafolder = "CommonAppDataFolder";
    $installer::globals::localappdatafolder = "LocalAppDataFolder";
    $installer::globals::templatefolder = "TemplateFolder";
    $installer::globals::templatefoldername = "Templates";
    $installer::globals::programmenufolder = "ProgramMenuFolder";
    $installer::globals::lcidlistname = $ENV{'SRCDIR'} . "/l10ntools/source/ulfconv/msi-encodinglist.txt";
    $installer::globals::msilanguage = "";  # hash reference for msi languages LCID
    $installer::globals::sofficeiconadded = 0;
    $installer::globals::temppath = "";
    $installer::globals::cyg_temppath = "";
    $installer::globals::temppathdefined = 0;
    $installer::globals::packageversion = 1;
    $installer::globals::packagerevision = 1;
    $installer::globals::rpm = "";
    $installer::globals::rpminfologged = 0;
    $installer::globals::debian = "";
    $installer::globals::installertypedir = "";
    $installer::globals::controlledmakecabversion = "5";
    $installer::globals::max_lang_length = 50;
    $installer::globals::globalblock = "Globals";
    $installer::globals::rootmodulegid = "";
    %installer::globals::alllangmodules = ();
    $installer::globals::englishlicenseset = 0;
    $installer::globals::englishlicense = "";
    $installer::globals::englishsolarislicensename = "LICENSE"; # _en-US";
    $installer::globals::solarisdontcompress = 0;
    $installer::globals::patharray = "";

    $installer::globals::is_special_epm = 0;
    $installer::globals::epm_in_path = 0;
    $installer::globals::epm_path = "";
    $installer::globals::epmoutpath = "";
    $installer::globals::simple = 0;
    $installer::globals::simpledefaultuserdir = "\$ORIGIN/..";
    $installer::globals::call_epm = 1;
    $installer::globals::packageformat = "";
    $installer::globals::packagename = "";
    $installer::globals::packagelist = "";
    $installer::globals::shiptestdirectory = "";
    $installer::globals::archiveformat = "";
    $installer::globals::updatelastsequence = 0;
    $installer::globals::updatesequencecounter = 0;
    $installer::globals::updatedatabase = 0;
    $installer::globals::updatedatabasepath = "";
    $installer::globals::pffcabfilename = "ooobasis3.0_pff.cab";
    %installer::globals::allmergemodulefilesequences = ();
    %installer::globals::newupdatefiles = ();
    %installer::globals::allusedupdatesequences = ();
    %installer::globals::mergemodulefiles = ();
    $installer::globals::mergefiles_added_into_collector = 0;
    $installer::globals::creating_windows_installer_patch = 0;

    $installer::globals::strip = 0;

    $installer::globals::packertool = "gzip";           # the default package compression tool for *NIX

    $installer::globals::logfilename = "logfile.log";   # the default logfile name for global errors
    @installer::globals::logfileinfo = ();
    @installer::globals::errorlogfileinfo = ();
    @installer::globals::globallogfileinfo = ();
    $installer::globals::ignore_error_in_logfile = 0;
    $installer::globals::exitlog = "";
    $installer::globals::quiet = 0;

    $installer::globals::ismultilingual = 0;
    %installer::globals::alluniquefilenames = ();
    %installer::globals::alllcuniquefilenames = ();
    %installer::globals::uniquefilenamesequence = ();
    %installer::globals::dependfilenames = ();
    $installer::globals::manufacturer = "";
    $installer::globals::longmanufacturer = "";
    $installer::globals::codefilename = "codes.txt";
    $installer::globals::componentfilename = "components.txt";
    $installer::globals::productcode = "";
    $installer::globals::upgradecode = "";
    $installer::globals::msiproductversion = "";
    $installer::globals::msimajorproductversion = "";
    @installer::globals::allddffiles = ();
    $installer::globals::infodirectory = "";

    %installer::globals::mergemodules = ();
    %installer::globals::merge_media_line = ();
    %installer::globals::merge_allfeature_hash = ();
    %installer::globals::merge_alldirectory_hash = ();
    %installer::globals::merge_directory_hash = ();
    %installer::globals::copy_msm_files = ();
    $installer::globals::mergefeaturecollected = 0;
    $installer::globals::mergedirectoriescollected = 0;
    $installer::globals::lastsequence_before_merge = 0;
    $installer::globals::lastcabfilename = "";

    $installer::globals::defaultlanguage = "";
    $installer::globals::addlicensefile = 1;
    $installer::globals::addsystemintegration = 0;
    $installer::globals::makedownload = 1;
    @installer::globals::binarytableonlyfiles = ();
    @installer::globals::allscpactions = ();
    $installer::globals::languagepackaddon = "LanguagePack";
    $installer::globals::helppackaddon = "HelpPack";
    $installer::globals::ooodownloadfilename = "";
    $installer::globals::downloadfilename = "";
    $installer::globals::downloadfileextension = "";
    %installer::globals::multilingual_only_modules = ();
    %installer::globals::application_modules = ();

    $installer::globals::is_copy_only_project = 0;
    $installer::globals::is_simple_packager_project = 0;
    $installer::globals::patch_user_dir = 0;
    $installer::globals::languagepack = 0;
    $installer::globals::helppack = 0;
    $installer::globals::refresh_includepaths = 0;
    $installer::globals::include_paths_read = 0;
    @installer::globals::patchfilecollector = ();
    @installer::globals::userregistrycollector = ();
    $installer::globals::addeduserregitrykeys = 0;
    $installer::globals::desktoplinkexists = 0;
    $installer::globals::analyze_spellcheckerlanguage = 0;
    %installer::globals::spellcheckerlanguagehash = ();
    %installer::globals::spellcheckerfilehash = ();
    $installer::globals::registryrootcomponent = "";
    %installer::globals::allcomponents = ();
    %installer::globals::allcomponents_in_this_database = ();
    %installer::globals::allshortcomponents = ();
    %installer::globals::allregistrycomponents_ = ();
    %installer::globals::allregistrycomponents_in_this_database_ = ();
    %installer::globals::allshortregistrycomponents = ();

    $installer::globals::installlocationdirectory = "";
    $installer::globals::installlocationdirectoryset = 0;
    $installer::globals::vendordirectory = "";
    $installer::globals::officeinstalldirectory = "";
    $installer::globals::rootbrandpackage = "";
    $installer::globals::rootbrandpackageset = 0;
    $installer::globals::officedirhostname = "";
    $installer::globals::officedirgid = "";

    %installer::globals::treestyles = ();
    %installer::globals::treelayername = ();
    %installer::globals::hostnametreestyles = ();
    %installer::globals::treeconditions = ();
    %installer::globals::usedtreeconditions = ();
    %installer::globals::moduledestination = ();

    $installer::globals::fix_number_of_cab_files = 1;
    $installer::globals::cabfilecompressionlevel = 21; # Using LZX compression, possible values are: 15 | 16 | ... | 21 (best compression)
    $installer::globals::number_of_cabfiles = 1;    # only for $fix_number_of_cab_files = 1
    $installer::globals::include_cab_in_msi = 1;
    $installer::globals::msidatabasename = "";
    $installer::globals::prepare_winpatch = 0;
    $installer::globals::previous_idt_dir = "";
    $installer::globals::msitranpath = "";
    $installer::globals::insert_file_at_end = 0;
    $installer::globals::newfilesexist = 0;
    $installer::globals::usesharepointpath = 0;
    %installer::globals::newfilescollector = ();

    $installer::globals::saveinstalldir = "";
    $installer::globals::csp_installdir = "";       # global installdir of createsimplepackage() in simplepackage.pm
    $installer::globals::csp_installlogdir = "";    # global installlogdir of createsimplepackage() in simplepackage.pm
    $installer::globals::csp_languagestring = "";   # global languagestring of createsimplepackage() in simplepackage.pm
    $installer::globals::localunpackdir = "";
    $installer::globals::localinstalldirset = 0;
    $installer::globals::localinstalldir = "";

    $installer::globals::postprocess_specialepm = 0;
    $installer::globals::postprocess_standardepm = 0;
    $installer::globals::mergemodules_analyzed = 0;

    @installer::globals::packagelistitems = ("module", "solarispackagename", "packagename", "copyright", "vendor", "description" );
    @installer::globals::featurecollector =();
    $installer::globals::msiassemblyfiles = "";
    $installer::globals::macinstallfilename = $ENV{'WORKDIR'} . "/CustomTarget/setup_native/mac/macinstall.ulf";
    $installer::globals::extensioninstalldir = "gid_Dir_Share_Extension_Install";
    @installer::globals::languagenames = ();
    %installer::globals::componentcondition = ();
    %installer::globals::componentid = ();
    %installer::globals::allcabinets = ();
    %installer::globals::allcabinetassigns = ();
    %installer::globals::cabfilecounter = ();
    %installer::globals::lastsequence = ();
    %installer::globals::allcalculated_guids = ();
    %installer::globals::calculated_component_guids = ();
    %installer::globals::all_english_languagestrings = ();
    %installer::globals::all_required_english_languagestrings = ();

    @installer::globals::removedirs = ();
    @installer::globals::removefiletable = ();

    if ( $^O =~ /cygwin/i || $^O =~ /MSWin/i )
    {
        $installer::globals::zippath = "zip";                   # Has to be in the path: /usr/bin/zip
        $installer::globals::separator = "/";
        $installer::globals::pathseparator = "\:";
        $installer::globals::isunix = 0;
        $installer::globals::iswin = 1;
        $installer::globals::archiveformat = ".zip";
        %installer::globals::savedmapping = ();
        %installer::globals::savedrevmapping = ();
        %installer::globals::savedrev83mapping = ();
        %installer::globals::saved83dirmapping = ();
    }
    else
    {
        $installer::globals::zippath = "zip";                   # Has to be in the path: /usr/bin/zip
        $installer::globals::separator = "/";
        $installer::globals::pathseparator = "\:";
        $installer::globals::archiveformat = ".tar.gz";
        $installer::globals::isunix = 1;
        $installer::globals::iswin = 0;
    }

}

1;
