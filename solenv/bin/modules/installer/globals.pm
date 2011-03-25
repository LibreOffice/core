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
    $prog="make_installer.pl";

    @noMSLocaleLangs = (
        "br",
        "bs",
        "dz",
        "gu",
        "km",
        "nr",
        "ns",
        "nso",
        "rw",
        "ss",
        "st",
        "tg",
        "ts",
        "tn",
        "ve",
        "xh",
        "zu",
        "ne",
        "bn",
        "bn-BD",
        "bn-IN",
        "lo",
        "cy",
        "ku",
        "as-IN",
        "te-IN",
        "ml-IN",
        "mr-IN",
        "ur-IN",
        "ta-IN",
        "or-IN",
        "ti-ER",
        "eo",
        "ka",
        "ga",
        "uk",
        "gd",
        "my",
        "mai",
        "brx",
        "dgo",
        "kok",
        "mni",
        "ca-XV",
        "sat",
        "ug",
        "om",
        "si",
        "or",
        "oc",
        "ml",
        "as",
        "ast",
        "ht",
        "jbo",
        "fur",
        "ny",
        "so",
        "kab"
    );
    @items_at_modules = ("Files", "Dirs", "Unixlinks");
    @asianlanguages = ("ja", "ko", "zh-CN", "zh-TW");
    @bidilanguages = ("ar", "he");

    $ziplistname = "";
    $pathfilename = "";
    $setupscriptname = "";
    $headerfilename = "";
    $shellscriptsfilename = "";
    $product = "";
    $languagelist = "";
    $added_english = 0;
    $set_office_start_language = 0;
    $solarjavaset = 0;

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
    $islinux = 0;
    $issolaris = 0;
    $ismacosx = 0;
    $iswindowsbuild = 0;
    $islinuxbuild = 0;
    $islinuxrpmbuild = 0;
    $islinuxdebbuild = 0;
    $islinuxintelrpmbuild = 0;
    $islinuxppcrpmbuild = 0;
    $islinuxinteldebbuild = 0;
    $islinuxppcdebbuild = 0;
    $islinuxx86_64rpmbuild = 0;
    $islinuxx86_64debbuild = 0;
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
    $packjobref = "";
    $buildid = "Not set";
    $guidcounter = 1000; # for uniqueness of guids
    $fontsfolder = "FontsFolder";
    $fontsfoldername = "Fonts";
    $fontsdirparent = "";
    $fontsdirname = "";
    $fontsdirhostname = "truetype";
    $officefolder = "OfficeFolder";
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
    $jdstemppathdefined = 0;
    $packageversion = 1;
    $packagerevision = 1;
    $rpm = "";
    $rpmcommand = "";
    $rpmquerycommand = "";
    $rpminfologged = 0;
    $debian = "";
    $installertypedir = "";
    $controlledmakecabversion = "5";
    $saved_packages_path = "";
    $max_lang_length = 50;
    $globalblock = "Globals";
    $rootmodulegid = "";
    %alllangmodules = ();
    $englishlicenseset = 0;
    $englishlicense = "";
    $englishsolarislicensename = "LICENSE_en-US";
    $solarisdontcompress = 0;
    $patharray = "";

    $is_special_epm = 0;
    $epm_in_path = 0;
    $epm_path = "";
    $epmoutpath = "";
    $upx_in_path = 0;
    $upxfile = "";
    $simple = 0;
    $simpledefaultuserdir = "\$ORIGIN/..";
    $call_epm = 1;
    $packageformat = "";
    $packagename = "";
    $packagelist = "";
    $addpackagelist = "";
    $is_unix_multi = 0;
    $unixmultipath = "";
    $unixmultipath_orig = "";
    $alllanguagesinproductarrayref = "";
    $shiptestdirectory = "";
    $makelinuxlinkrpm = 0;
    $linuxlinkrpmprocess = 0;
    $add_required_package = "";
    $linuxrespin = 0;
    @linuxpatchfiles = ();
    $linuxlibrarybaselevel = "1";
    $linuxlibrarypatchlevel = "1.1";
    @linuxlinks = ();
    @linkrpms = ();
    $archiveformat = "";
    $minorupgradekey = "";
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

    $strip = 1;

    $globallogging = 0;
    $globalloggingform21 = 1;
    $logfilename = "logfile.log";   # the default logfile name for global errors
    @logfileinfo = ();
    @errorlogfileinfo = ();
    @globallogfileinfo = ();
    $ignore_error_in_logfile = 0;
    $exitlog = "";
    $globalinfo_copied = 0;
    $quiet = 0;
    $nodownload = 0;
    $writetotemp = 0;
    $useminor = 0;
    $followme_from_directory = 0;
    $internal_cabinet_signing = 0;

    $debug = 0;
    $debugfilename = "debug.txt";
    $checksumfilename = "checksum.txt";
    @functioncalls = ();

    $ismultilingual = 0;
    @multilanguagemodules = ();
    $languagemodulesbase = "gid_Module_Root_";
    %alluniquefilenames = ();
    %alllcuniquefilenames = ();
    %uniquefilenamesequence = ();
    %dependfilenames = ();
    $isopensourceproduct = 1;
    $manufacturer = "";
    $longmanufacturer = "";
    $sundirname = "Oracle";
    $codefilename = "codes.txt";
    $componentfilename = "components.txt";
    $productcode = "";
    $upgradecode = "";
    $msiproductversion = "";
    $msimajorproductversion = "";
    $created_new_component_guid = 0;
    @allddffiles = ();
    $infodirectory = "";
    @currentcontent = ();
    @installsetcontent = ();
    %xpdpackageinfo = ();
    $signfiles_checked = 0;
    $dosign = 0;
    $pwfile = "";
    $pwfile = "";
    $pfxfile = "";

    %mergemodules = ();
    %merge_media_line = ();
    %merge_allfeature_hash = ();
    %merge_alldirectory_hash = ();
    %copy_msm_files = ();
    $mergefeaturecollected = 0;
    $mergedirectoriescollected = 0;
    $lastsequence_before_merge = 0;
    $lastcabfilename = "";

    $createdxpddefaultlang = 0;
    $xpddir = "";
    $productxpdfile = "setup.xpd";
    $xpd_files_prepared = 0;
    $defaultlanguage = "";
    # @emptyxpdparents = ();
    @createdxpdfiles = ();
    @allxpdfiles = ();
    $isxpdplatform = 0;
    $javalanguagepath = "";
    $javasettozero = 0;
    $addlicensefile = 1;
    $addsystemintegration = 0;
    $addjavainstaller = 0;
    $added_directories = 0;
    $makedownload = 1;
    $makejds = 1;
    $jdsexcludefilename = "";
    $jds_language_controlled = 0;
    $correct_jds_language = 0;
    @installsetfiles = ();
    @binarytableonlyfiles = ();
    @allscpactions = ();
    $languagepackaddon = "LanguagePack";
    $patchaddon = "Patch";
    $ooodownloadfilename = "";
    $downloadfilename = "";
    $downloadfileextension = "";
    $followmeinfofilename = "";
    $oooversionstring = "";
    $shellnewfilesadded = 0;
    %multilingual_only_modules = ();
    %application_modules = ();
    $defaultinstallorder = 1000;
    $defaultsystemintinstallorder = 1200;

    $is_copy_only_project = 0;
    $is_simple_packager_project = 0;
    $patch_user_dir = 0;
    $addchildprojects = 0;
    $languagepack = 0;
    $tab = 0;
    $patch = 0;
    $patchincludepath = "";
    $refresh_includepathes = 0;
    $include_pathes_read = 0;
    $patchfilelistname = "patchfilelist.txt";
    @patchfilecollector = ();
    $nopatchfilecollector = "";
    @userregistrycollector = ();
    $addeduserregitrykeys = 0;
    $poolpathset = 0;
    $poolpath = 0;
    $poollockfilename = "";
    $poolcheckfilename = "";
    %poolshiftedpackages = ();
    %poolpackages = ();
    %createpackages = ();
    $processhaspoolcheckfile = 0;
    $processhaspoollockfile = 0;
    $newpcfcontentcalculated = 0;
    $sessionid = 0;
    $sessionidset = 0;
    $savelockfilecontent = "";
    $savelockfilename = "";
    $getuidpath = "";
    $getuidpathset = 0;
    $newpcfcontent = "";
    %pcfdifflist = ();
    @pcfdiffcomment = ();
    @epmdifflist = ();
    $desktoplinkexists = 0;
    $sundirexists = 0;
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
    %allregistryidentifier = ();

    $installlocationdirectory = "";
    $installlocationdirectoryset = 0;
    $vendordirectory = "";
    $vendordirectoryset = 0;
    $officeinstalldirectory = "";
    $officeinstalldirectoryset = 0;
    $basisinstalldirectory = "";
    $basisinstalldirectoryset = 0;
    $ureinstalldirectory = "";
    $ureinstalldirectoryset = 0;
    $rootbrandpackage = "";
    $rootbrandpackageset = 0;
    $officedirhostname = "";
    $basisdirhostname = "";
    $uredirhostname = "";
    $sundirhostname = "";
    $officedirgid = "";
    $basisdirgid = "";
    $uredirgid = "";
    $sundirgid = "";

    %sign_extensions = ("dll" => "1", "exe" => "1", "cab" => "1");
    %treestyles = ();
    %installlocations = ("INSTALLLOCATION" => "1");
    %treelayername = ();
    %hostnametreestyles = ();
    %treeconditions = ();
    %usedtreeconditions = ();
    %moduledestination = ();

    $one_cab_file = 0;
    $fix_number_of_cab_files = 1;
    $cab_file_per_component = 0;
    $cabfilecompressionlevel = 2;
    $number_of_cabfiles = 1;    # only for $fix_number_of_cab_files = 1
    $include_cab_in_msi = 0;
    $use_packages_for_cabs = 0;
    $msidatabasename = "";
    $prepare_winpatch = 0;
    $previous_idt_dir = "";
    $updatepack = 0;
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

    $javafilename = "";
    $javafilename2 = "";
    $javafilename3 = "";
    $javafile = "";
    $urefile = "";

    $postprocess_specialepm = 0;
    $postprocess_standardepm = 0;
    $mergemodules_analyzed = 0;

    $starttime = "";

    @solarispatchscripts = ("checkinstall", "copyright", "patch_checkinstall", "patch_postinstall", "postinstall", "preinstall", "i.none");
    @solarispatchscriptsforextensions = ("checkinstall", "copyright", "patch_checkinstall", "patch_postinstall_extensions", "postinstall_extensions", "preinstall", "i.none");
    @solarispatchfiles = (".diPatch", "patchinfo");
    @environmentvariables = ( "SOLARVERSION", "GUI", "WORK_STAMP", "OUTPATH", "LOCAL_OUT", "LOCAL_COMMON_OUT" );
    @packagelistitems = ("module", "solarispackagename", "packagename", "copyright", "vendor", "description" );
    @languagepackfeature =();
    @featurecollector =();
    $msiassemblyfiles = "";
    $nsisfilename = "Nsis";
    $macinstallfilename = "macinstall.ulf";
    $nsis204 = 0;
    $nsis231 = 0;
    $unicodensis = 0;
    $linuxlinkrpms = "";
    $extensioninstalldir = "gid_Dir_Share_Extension_Install";
    @languagenames = ();
    @requiredpackages = ();
    %componentcondition = ();
    %componentid = ();
    %comparecomponentname = ();
    %languageproperties = ();
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
    @jdsremovedirs = ();
    @emptypackages = ();
    %fontpackageexists = ();

    $exithandler = undef;

    $plat = $^O;

    if ( $plat =~ /cygwin/i )
    {
        $zippath = "zip";                   # Has to be in the path: /usr/bin/zip
        $checksumfile = "so_checksum";
        $unopkgfile = "unopkg.exe";
        $separator = "/";
        $pathseparator = "\:";
        $libextension = "\.dll";
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
        $checksumfile = "so_checksum";
        $unopkgfile = "unopkg";
        $separator = "/";
        $pathseparator = "\:";
        if ( $plat =~ /darwin/i )
        {
            $libextension = "\.dylib";
        }
        else
        {
            $libextension = "\.so";
        }
        $archiveformat = ".tar.gz";
        $isunix = 1;
        $iswin = 0;
    }
    # WRAPCMD is gone - remove this and all related
    # $installer::globals::wrapcmd entries
    $wrapcmd = "";

    if ( $plat =~ /linux/i ) { $islinux = 1; }
    if ( $plat =~ /kfreebsd/i ) { $islinux = 1; }
    if ( $plat =~ /solaris/i ) { $issolaris = 1; }
    if ( $plat =~ /darwin/i ) { $ismacosx = 1; }

    # ToDo: Needs to be expanded for additional platforms

}

1;
