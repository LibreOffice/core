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

package installer;

use base 'Exporter';

use Cwd;
use Data::Dumper;
use File::Copy;
use installer::archivefiles;
use installer::control;
use installer::converter;
use installer::copyproject;
use installer::download;
use installer::environment;
use installer::epmfile;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::helppack;
use installer::languagepack;
use installer::languages;
use installer::logger;
use installer::packagelist;
use installer::parameter;
use installer::pathanalyzer;
use installer::profiles;
use installer::scppatchsoname;
use installer::scpzipfiles;
use installer::scriptitems;
use installer::setupscript;
use installer::simplepackage;
use installer::strip qw(strip_libraries);
use installer::substfilenamefiles;
use installer::systemactions;
use installer::windows::assembly;
use installer::windows::binary;
use installer::windows::component;
use installer::windows::createfolder;
use installer::windows::directory;
use installer::windows::feature;
use installer::windows::featurecomponent;
use installer::windows::file;
use installer::windows::font;
use installer::windows::icon;
use installer::windows::idtglobal;
use installer::windows::inifile;
use installer::windows::media;
use installer::windows::mergemodule;
use installer::windows::msiglobal;
use installer::windows::msp;
use installer::windows::property;
use installer::windows::removefile;
use installer::windows::registry;
use installer::windows::shortcut;
use installer::windows::strip;
use installer::windows::update;
use installer::windows::upgrade;
use installer::worker;
use installer::ziplist;

our @EXPORT_OK = qw(main);

sub main {
    installer::logger::starttime();

    my $exit_code = 0;

    eval {
        run();
    };
    if ($@) {
        warn "$@\n";
        $exit_code = -1;
    }

    installer::logger::stoptime();

    return $exit_code;
}

sub run {
    installer::logger::print_message( "... checking environment variables ...\n" );
    my $environmentvariableshashref = installer::control::check_system_environment();

    installer::environment::set_global_environment_variables($environmentvariableshashref);

    #################################
    # Check and output of parameter
    #################################

    installer::parameter::saveparameter();
    installer::parameter::getparameter();

    installer::parameter::control_fundamental_parameter();
    installer::parameter::setglobalvariables();
    installer::parameter::control_required_parameter();

    if (!($installer::globals::languages_defined_in_productlist)) { installer::languages::analyze_languagelist(); }
    installer::parameter::outputparameter();

    $installer::globals::build = uc($installer::globals::build);    # using "SRC680" instead of "src680"

    ######################################
    # Creating the log directory
    ######################################

    my $loggingdir = installer::systemactions::create_directories("logging", "");
    $loggingdir = $loggingdir . $installer::globals::separator;
    $installer::globals::exitlog = $loggingdir;

    my $installdir = "";
    my $currentdir = cwd();
    my $shipinstalldir = "";
    my $current_install_number = "";

    ######################################
    # Checking the system requirements
    ######################################

    installer::logger::print_message( "... checking required files ...\n" );
    installer::control::check_system_path();

    my $pathvariableshashref = installer::environment::create_pathvariables($environmentvariableshashref);

    ###############################################
    # Checking saved setting for Windows patches
    ###############################################

    if (( $installer::globals::iswindowsbuild ) &&  ( $installer::globals::prepare_winpatch )) { installer::windows::msiglobal::read_saved_mappings(); }

    ###################################################
    # Analyzing the settings and variables in zip.lst
    ###################################################

    installer::logger::globallog("zip list file: $installer::globals::ziplistname");

    my $ziplistref = installer::files::read_file($installer::globals::ziplistname);

    installer::logger::print_message( "... analyzing $installer::globals::ziplistname ... \n" );

    my ($productblockref, $parent) = installer::ziplist::getproductblock($ziplistref, $installer::globals::product, 1);     # product block from zip.lst

    my ($settingsblockref, undef) = installer::ziplist::getproductblock($productblockref, "Settings", 0);       # settings block from zip.lst

    $settingsblockref = installer::ziplist::analyze_settings_block($settingsblockref);              # select data from settings block in zip.lst

    my $allsettingsarrayref = installer::ziplist::get_settings_from_ziplist($settingsblockref);

    my $allvariablesarrayref = installer::ziplist::get_variables_from_ziplist($settingsblockref);

    my ($globalproductblockref, undef) = installer::ziplist::getproductblock($ziplistref, $installer::globals::globalblock, 0);     # global product block from zip.lst

    while (defined $parent)
    {
        my $parentproductblockref;
        ($parentproductblockref, $parent) = installer::ziplist::getproductblock(
            $ziplistref, $parent, 1);
        my ($parentsettingsblockref, undef) = installer::ziplist::getproductblock(
            $parentproductblockref, "Settings", 0);
        $parentsettingsblockref = installer::ziplist::analyze_settings_block(
            $parentsettingsblockref);
        my $allparentsettingsarrayref =
            installer::ziplist::get_settings_from_ziplist($parentsettingsblockref);
        my $allparentvariablesarrayref =
            installer::ziplist::get_variables_from_ziplist($parentsettingsblockref);
        $allsettingsarrayref =
            installer::converter::combine_arrays_from_references_first_win(
                $allsettingsarrayref, $allparentsettingsarrayref)
            if $#{$allparentsettingsarrayref} > -1;
        $allvariablesarrayref =
            installer::converter::combine_arrays_from_references_first_win(
                $allvariablesarrayref, $allparentvariablesarrayref)
            if $#{$allparentvariablesarrayref} > -1;
    }

    if ( $#{$globalproductblockref} > -1 )
    {
        my ($globalsettingsblockref, undef) = installer::ziplist::getproductblock($globalproductblockref, "Settings", 0);       # settings block from zip.lst

        $globalsettingsblockref = installer::ziplist::analyze_settings_block($globalsettingsblockref);              # select data from settings block in zip.lst

        my $allglobalsettingsarrayref = installer::ziplist::get_settings_from_ziplist($globalsettingsblockref);

        my $allglobalvariablesarrayref = installer::ziplist::get_variables_from_ziplist($globalsettingsblockref);

        if ( $#{$allglobalsettingsarrayref} > -1 ) { $allsettingsarrayref = installer::converter::combine_arrays_from_references_first_win($allsettingsarrayref, $allglobalsettingsarrayref); }
        if ( $#{$allglobalvariablesarrayref} > -1 ) { $allvariablesarrayref = installer::converter::combine_arrays_from_references_first_win($allvariablesarrayref, $allglobalvariablesarrayref); }
    }

    $allsettingsarrayref = installer::ziplist::remove_multiples_from_ziplist($allsettingsarrayref); # the settings from the zip.lst

    $allvariablesarrayref = installer::ziplist::remove_multiples_from_ziplist($allvariablesarrayref);

    installer::ziplist::replace_variables_in_ziplist_variables($allvariablesarrayref);

    my $allvariableshashref = installer::converter::convert_array_to_hash($allvariablesarrayref);   # the variables from the zip.lst

    installer::ziplist::set_default_productversion_if_required($allvariableshashref);

    installer::ziplist::add_variables_to_allvariableshashref($allvariableshashref);

    installer::ziplist::overwrite_branding( $allvariableshashref );


    ########################################################
    # Check if this is simple packaging mechanism
    ########################################################

    installer::simplepackage::check_simple_packager_project($allvariableshashref);

    ####################################################################
    # setting global variables
    ####################################################################

    installer::control::set_addsystemintegration($allvariableshashref);

    ########################################################
    # Re-define logging dir, after all variables are set
    ########################################################

    my $oldloggingdir = $loggingdir;
    installer::systemactions::remove_complete_directory($oldloggingdir);
    $loggingdir = installer::systemactions::create_directories("logging", "");
    $loggingdir = $loggingdir . $installer::globals::separator;
    $installer::globals::exitlog = $loggingdir;

    # checking, whether this is an opensource product

    if (!($installer::globals::is_copy_only_project)) { installer::ziplist::set_manufacturer($allvariableshashref); }

    ##############################################
    # Checking version of makecab.exe
    ##############################################

    if ( $installer::globals::iswindowsbuild && (!defined($ENV{'CROSS_COMPILING'}) || $ENV{'CROSS_COMPILING'} ne 'YES' || $installer::globals::packageformat eq 'msi')) { installer::control::check_makecab_version(); }

    ##########################################################
    # Getting the include path from the settings in zip list
    ##########################################################

    my $includepathref = installer::ziplist::getinfofromziplist($allsettingsarrayref, "include");
    if ( $$includepathref eq "" )
    {
        installer::exiter::exit_program("ERROR: Definition for \"include\" not found in $installer::globals::ziplistname", "Main");
    }

    my $includepatharrayref = installer::converter::convert_stringlist_into_array($includepathref, ",");

    installer::ziplist::replace_all_variables_in_paths($includepatharrayref, $pathvariableshashref);

    installer::ziplist::replace_minor_in_paths($includepatharrayref);

    installer::ziplist::replace_packagetype_in_paths($includepatharrayref);

    installer::ziplist::resolve_relative_paths($includepatharrayref);

    installer::ziplist::remove_ending_separator($includepatharrayref);

    ##############################################
    # Collecting all files from all include
    # paths in global hashes.
    ##############################################

    installer::worker::collect_all_files_from_includepaths($includepatharrayref);

    ##############################################
    # Analyzing languages in zip.lst if required
    # Probably no longer used.
    ##############################################

    if ($installer::globals::languages_defined_in_productlist) { installer::languages::get_info_about_languages($allsettingsarrayref); }

    #####################################
    # Windows requires the encoding list
    #####################################

    if ( $installer::globals::iswindowsbuild ) { installer::control::read_encodinglist($includepatharrayref); }

    #####################################################################
    # Including additional inc files for variable settings, if defined
    #####################################################################

    if ( $allvariableshashref->{'ADD_INCLUDE_FILES'} ) { installer::worker::add_variables_from_inc_to_hashref($allvariableshashref, $includepatharrayref); }

    #####################################
    # Analyzing the setup script
    #####################################

    if ($installer::globals::setupscript_defined_in_productlist) { installer::setupscript::set_setupscript_name($allsettingsarrayref, $includepatharrayref); }

    installer::logger::globallog("setup script file: $installer::globals::setupscriptname");

    installer::logger::print_message( "... analyzing script: $installer::globals::setupscriptname ... \n" );

    my $setupscriptref = installer::files::read_file($installer::globals::setupscriptname); # Reading the setup script file

    # Resolving variables defined in the zip list file into setup script
    # All the variables are defined in $allvariablesarrayref

    installer::scpzipfiles::replace_all_ziplistvariables_in_file($setupscriptref, $allvariableshashref);

    # Resolving %variables defined in the installation object

    my $allscriptvariablesref = installer::setupscript::get_all_scriptvariables_from_installation_object($setupscriptref);

    installer::setupscript::add_lowercase_productname_setupscriptvariable($allscriptvariablesref);

    installer::setupscript::resolve_lowercase_productname_setupscriptvariable($allscriptvariablesref);

    $setupscriptref = installer::setupscript::replace_all_setupscriptvariables_in_script($setupscriptref, $allscriptvariablesref);

    # Adding all variables defined in the installation object into the hash of all variables.
    # This is needed if variables are defined in the installation object, but not in the zip list file.
    # If there is a definition in the zip list file and in the installation object, the installation object is more important

    installer::setupscript::add_installationobject_to_variables($allvariableshashref, $allscriptvariablesref);

    # Adding also all variables, that must be included into the $allvariableshashref.
    installer::setupscript::add_forced_properties($allvariableshashref);

    # Replacing preset properties, not using the default mechanisms (for example for UNIXPRODUCTNAME)
    installer::setupscript::replace_preset_properties($allvariableshashref);

    installer::scpzipfiles::replace_all_ziplistvariables_in_file($setupscriptref, $allvariableshashref);


    installer::logger::log_hashref($allvariableshashref);

    installer::logger::print_message( "... analyzing directories ... \n" );

    # Collect all directories in the script to get the destination dirs

    my $dirsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Directory");

    if ( $installer::globals::languagepack ) { installer::scriptitems::use_langpack_hostname($dirsinproductarrayref); }
    if ( $installer::globals::patch ) { installer::scriptitems::use_patch_hostname($dirsinproductarrayref); }

    if ( $allvariableshashref->{'SHIFT_BASIS_INTO_BRAND_LAYER'} ) { $dirsinproductarrayref = installer::scriptitems::shift_basis_directory_parents($dirsinproductarrayref); }
    if ( $allvariableshashref->{'OFFICEDIRECTORYNAME'} ) { installer::scriptitems::set_officedirectory_name($dirsinproductarrayref, $allvariableshashref->{'OFFICEDIRECTORYNAME'}); }


    installer::scriptitems::resolve_all_directory_names($dirsinproductarrayref);

    installer::logger::print_message( "... analyzing files ... \n" );

    my $filesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "File");

    if (( ! $installer::globals::iswindowsbuild ) &&
        ( ! $installer::globals::isrpmbuild ) &&
        ( ! $installer::globals::isdebbuild ) &&
        ( ! $installer::globals::issolarispkgbuild ) &&
        ( $installer::globals::packageformat ne "installed" ) &&
        ( $installer::globals::packageformat ne "dmg" ) &&
        ( $installer::globals::packageformat ne "archive" ))
        { installer::control::check_oxtfiles($filesinproductarrayref); }

    if (! $installer::globals::languagepack)
    {
        $filesinproductarrayref = installer::scriptitems::remove_Languagepacklibraries_from_Installset($filesinproductarrayref);
    }

    if (! $installer::globals::helppack)
    {
        $filesinproductarrayref = installer::scriptitems::remove_Helppacklibraries_from_Installset($filesinproductarrayref);
    }

    if (! $installer::globals::patch)
    {
        $filesinproductarrayref = installer::scriptitems::remove_patchonlyfiles_from_Installset($filesinproductarrayref);
    }

    installer::logger::print_message( "... analyzing scpactions ... \n" );

    my $scpactionsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "ScpAction");

    if ( $installer::globals::languagepack ) { installer::scriptitems::use_langpack_copy_scpaction($scpactionsinproductarrayref); }
    if ( $installer::globals::helppack ) { installer::scriptitems::use_langpack_copy_scpaction($scpactionsinproductarrayref); }
    if ( $allvariableshashref->{'PRODUCTNAME'} eq "LibO-dev" ) { installer::scriptitems::use_devversion_copy_scpaction($scpactionsinproductarrayref); }
    if ( $installer::globals::patch ) { installer::scriptitems::use_patch_copy_scpaction($scpactionsinproductarrayref); }

    installer::scriptitems::change_keys_of_scpactions($scpactionsinproductarrayref);

    installer::logger::print_message( "... analyzing shortcuts ... \n" );

    my $linksinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Shortcut");

    installer::logger::print_message( "... analyzing unix links ... \n" );

    my $unixlinksinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Unixlink");

    installer::logger::print_message( "... analyzing profile ... \n" );

    my $profilesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Profile");

    installer::logger::print_message( "... analyzing profileitems ... \n" );

    my $profileitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "ProfileItem");

    my $folderinproductarrayref;
    my $folderitemsinproductarrayref;
    my $registryitemsinproductarrayref;
    my $windowscustomactionsarrayref;
    my $mergemodulesarrayref;

    if ( $installer::globals::iswindowsbuild )  # Windows specific items: Folder, FolderItem, RegistryItem, WindowsCustomAction
    {
        installer::logger::print_message( "... analyzing folders ... \n" );

        $folderinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Folder");

        installer::logger::print_message( "... analyzing folderitems ... \n" );

        $folderitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "FolderItem");

        installer::setupscript::add_predefined_folder($folderitemsinproductarrayref, $folderinproductarrayref);

        installer::setupscript::prepare_non_advertised_files($folderitemsinproductarrayref, $filesinproductarrayref);

        installer::logger::print_message( "... analyzing registryitems ... \n" );

        $registryitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "RegistryItem");

        $registryitemsinproductarrayref = installer::scriptitems::remove_uninstall_regitems_from_script($registryitemsinproductarrayref);

        installer::logger::print_message( "... analyzing Windows custom actions ... \n" );

        $windowscustomactionsarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "WindowsCustomAction");

        installer::logger::print_message( "... analyzing Windows merge modules ... \n" );

        $mergemodulesarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "MergeModule");
    }

    my $modulesinproductarrayref;

    if (!($installer::globals::is_copy_only_project))
    {
        installer::logger::print_message( "... analyzing modules ... \n" );

        $modulesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Module");

        installer::scriptitems::resolve_assigned_modules($modulesinproductarrayref);

        $modulesinproductarrayref = installer::scriptitems::remove_template_modules($modulesinproductarrayref);

        installer::scriptitems::set_children_flag($modulesinproductarrayref);

        installer::scriptitems::collect_all_languagemodules($modulesinproductarrayref);

        # Assigning the modules to the items

        installer::scriptitems::assigning_modules_to_items($modulesinproductarrayref, $filesinproductarrayref, "Files");

        installer::scriptitems::assigning_modules_to_items($modulesinproductarrayref, $unixlinksinproductarrayref, "Unixlinks");

        installer::scriptitems::assigning_modules_to_items($modulesinproductarrayref, $dirsinproductarrayref, "Dirs");
    }

    #################################################
    # Part 1b: The language dependent part
    # (still platform independent)
    #################################################

    # Now starts the language dependent part, if more than one product is defined on the command line
    # Example -l en-US,de#es,fr,it defines two multilingual products

    ###############################################################################
    # Beginning of language dependent part
    # The for iterates over all products, separated by an # in the language list
    ###############################################################################

    for ( my $n = 0; $n <= $#installer::globals::languageproducts; $n++ )
    {
        my $languagesarrayref = installer::languages::get_all_languages_for_one_product($installer::globals::languageproducts[$n], $allvariableshashref);

        my $languagestringref = installer::languages::get_language_string($languagesarrayref);
        installer::logger::print_message( "------------------------------------\n" );
        installer::logger::print_message( "... languages $$languagestringref ... \n" );

        if ( $installer::globals::patch )
        {
            $installer::globals::addlicensefile = 0;    # no license files for patches
            $installer::globals::makedownload = 0;
        }

        if ( $installer::globals::languagepack )
        {
            $installer::globals::addsystemintegration = 0;
            $installer::globals::addlicensefile = 0;
            $installer::globals::makedownload = 1;
        }

        if ( $installer::globals::helppack )
        {
            $installer::globals::addsystemintegration = 0;
            $installer::globals::addlicensefile = 0;
            $installer::globals::makedownload = 1;
        }

        ############################################################
        # Beginning of language specific logging mechanism
        # Until now only global logging into default: logfile.txt
        ############################################################

        @installer::globals::logfileinfo = ();  # new logfile array and new logfile name
        installer::logger::copy_globalinfo_into_logfile();
        $installer::globals::globalinfo_copied = 1;

        my $logminor = "";
        $logminor = $installer::globals::minor;

        my $loglanguagestring = $$languagestringref;
        my $loglanguagestring_orig = $loglanguagestring;
        if (length($loglanguagestring) > $installer::globals::max_lang_length)
        {
            my $number_of_languages = installer::systemactions::get_number_of_langs($loglanguagestring);
            chomp(my $shorter = `echo $loglanguagestring | md5sum | sed -e "s/ .*//g"`);
            my $id = substr($shorter, 0, 8); # taking only the first 8 digits
            $loglanguagestring = "lang_" . $number_of_languages . "_id_" . $id;
        }

        $installer::globals::logfilename = "log_" . $installer::globals::build;
        if ( $logminor ne "" ) { $installer::globals::logfilename .= "_" . $logminor; }
        $installer::globals::logfilename .= "_" . $loglanguagestring;
        $installer::globals::logfilename .= ".log";
        $loggingdir = $loggingdir . $loglanguagestring . $installer::globals::separator;
        installer::systemactions::create_directory($loggingdir);

        if ($loglanguagestring ne $loglanguagestring_orig) {
            (my $dir = $loggingdir) =~ s!/$!!;
            open(my $F1, "> $dir.dir");
            open(my $F2, "> " . $loggingdir . $installer::globals::logfilename . '.file');
            my @s = map { "$_\n" } split('_', $loglanguagestring_orig);
            print $F1 @s;
            print $F2 @s;
        }

        $installer::globals::exitlog = $loggingdir;

        ###################################################################
        # Reading an existing msi database, to prepare update and patch
        ###################################################################

        my $refdatabase = "";
        my $uniquefilename = "";
        my $revuniquefilename = "";
        my $revshortfilename = "";
        my $allupdatesequences = "";
        my $allupdatecomponents = "";
        my $allupdatefileorder = "";
        my $allupdatecomponentorder = "";
        my $shortdirname = "";
        my $componentid = "";
        my $componentidkeypath = "";
        my $alloldproperties = "";
        my $allupdatelastsequences = "";
        my $allupdatediskids = "";

        if ( $installer::globals::iswindowsbuild || $installer::globals::packageformat eq 'msi' )
        {
            if ( $allvariableshashref->{'UPDATE_DATABASE'} )
            {
                installer::logger::print_message( "... analyzing update database ...\n" );
                $refdatabase = installer::windows::update::readdatabase($allvariableshashref, $languagestringref, $includepatharrayref);

                if ( $installer::globals::updatedatabase )
                {
                    ($uniquefilename, $revuniquefilename, $revshortfilename, $allupdatesequences, $allupdatecomponents, $allupdatefileorder, $allupdatecomponentorder, $shortdirname, $componentid, $componentidkeypath, $alloldproperties, $allupdatelastsequences, $allupdatediskids) = installer::windows::update::create_database_hashes($refdatabase);
                    if ( $mergemodulesarrayref > -1 ) { installer::windows::update::readmergedatabase($mergemodulesarrayref, $languagestringref, $includepatharrayref); }
                }
            }
        }

        ##############################################
        # Setting global code variables for Windows
        ##############################################

        if (!($installer::globals::is_copy_only_project))
        {
            if ((( $installer::globals::iswindowsbuild ) && ( $installer::globals::packageformat ne "archive" ) && ( $installer::globals::packageformat ne "installed" ) ) || $installer::globals::packageformat eq 'msi' )
            {
                installer::windows::msiglobal::set_global_code_variables($languagesarrayref, $languagestringref, $allvariableshashref, $alloldproperties);
            }
        }

        ################################################
        # Resolving include paths (language dependent)
        ################################################

        $includepatharrayref_lang = installer::ziplist::replace_languages_in_paths($includepatharrayref, $languagesarrayref);

        if ( $installer::globals::refresh_includepaths ) { installer::worker::collect_all_files_from_includepaths($includepatharrayref_lang); }

        installer::ziplist::list_all_files_from_include_path($includepatharrayref_lang);

        ##############################################
        # Analyzing spellchecker languages
        ##############################################

        if ( $allvariableshashref->{'SPELLCHECKERFILE'} ) { installer::worker::set_spellcheckerlanguages($languagesarrayref, $allvariableshashref); }

        #####################################
        # Language dependent directory part
        #####################################

        my $dirsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($dirsinproductarrayref, $languagesarrayref);

        # A new directory array is needed ($dirsinproductlanguageresolvedarrayref instead of $dirsinproductarrayref)
        # because $dirsinproductarrayref is needed in get_Destination_Directory_For_Item_From_Directorylist

        installer::scriptitems::changing_name_of_language_dependent_keys($dirsinproductlanguageresolvedarrayref);

        installer::scriptitems::checking_directories_with_corrupt_hostname($dirsinproductlanguageresolvedarrayref, $languagesarrayref);

        installer::scriptitems::set_global_directory_hostnames($dirsinproductlanguageresolvedarrayref, $allvariableshashref);

        #####################################
        # files part, language dependent
        #####################################

        installer::logger::print_message( "... analyzing files ...\n" );

        my $filesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($filesinproductarrayref, $languagesarrayref);

        if ( ! $installer::globals::set_office_start_language )
        {
            $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_office_start_language_files($filesinproductlanguageresolvedarrayref);
        }

        installer::scriptitems::changing_name_of_language_dependent_keys($filesinproductlanguageresolvedarrayref);

        if ( $installer::globals::iswin and $^O =~ /MSWin/i ) { installer::converter::convert_slash_to_backslash($filesinproductlanguageresolvedarrayref); }

        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_non_existent_languages_in_productlists($filesinproductlanguageresolvedarrayref, $languagestringref, "Name", "file");

        installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($filesinproductlanguageresolvedarrayref, $dirsinproductarrayref);

        installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist($filesinproductlanguageresolvedarrayref, $includepatharrayref_lang, $dirsinproductlanguageresolvedarrayref, "Files");

        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_Without_Sourcedirectory($filesinproductlanguageresolvedarrayref);

        if ($installer::globals::languagepack)
        {
            $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_For_Languagepacks($filesinproductlanguageresolvedarrayref);
        }

        if ( ! $allvariableshashref->{'NO_README_IN_ROOTDIR'} )
        {
            $filesinproductlanguageresolvedarrayref = installer::scriptitems::add_License_Files_into_Installdir($filesinproductlanguageresolvedarrayref, $dirsinproductlanguageresolvedarrayref, $languagesarrayref);
        }

        installer::scriptitems::make_filename_language_specific($filesinproductlanguageresolvedarrayref);

        ######################################################################################
        # Unzipping files with flag ARCHIVE and putting all included files into the file list
        ######################################################################################

        installer::logger::print_message( "... analyzing files with flag ARCHIVE ...\n" );

        my @additional_paths_from_zipfiles = ();

        $filesinproductlanguageresolvedarrayref = installer::archivefiles::resolving_archive_flag($filesinproductlanguageresolvedarrayref, \@additional_paths_from_zipfiles, $languagestringref, $loggingdir);

        # packed files sometimes contain a "$" in their name: HighlightText$1.class. For epm the "$" has to be quoted by "$$"

        if (!( $installer::globals::iswindowsbuild || $installer::globals::simple ) )
        {
            installer::scriptitems::quoting_illegal_filenames($filesinproductlanguageresolvedarrayref);
        }

        #####################################
        # Files with flag SUBST_FILENAME
        #####################################

        installer::logger::print_message( "... analyzing files with flag SUBST_FILENAME ...\n" );

        installer::substfilenamefiles::resolving_subst_filename_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, $languagestringref);

        #####################################
        # Files with flag SCPZIP_REPLACE
        #####################################

        installer::logger::print_message( "... analyzing files with flag SCPZIP_REPLACE ...\n" );

        # Editing files with flag SCPZIP_REPLACE.

        installer::scpzipfiles::resolving_scpzip_replace_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);

        #####################################
        # Files with flag PATCH_SO_NAME
        #####################################

        installer::logger::print_message( "... analyzing files with flag PATCH_SO_NAME ...\n" );

        # Editing files with flag PATCH_SO_NAME.

        installer::scppatchsoname::resolving_patchsoname_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);

        #####################################
        # Files with flag HIDDEN
        #####################################

        installer::logger::print_message( "... analyzing files with flag HIDDEN ...\n" );

        installer::worker::resolving_hidden_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);

        ############################################
        # Collecting directories for epm list file
        ############################################

        installer::logger::print_message( "... analyzing all directories for this product ...\n" );

        # There are two ways for a directory to be included into the epm directory list:
        # 1. Looking for all destination paths in the files array
        # 2. Looking for directories with CREATE flag in the directory array
        # Advantage: Many paths are hidden in zip files, they are not defined in the setup script.
        # It will be possible, that in the setup script only those directoies have to be defined,
        # that have a CREATE flag. All other directories are created, if they contain at least one file.

        my ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);

        ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($dirsinproductlanguageresolvedarrayref, $alldirectoryhash);

        #########################################################
        # language dependent scpactions part
        #########################################################

        my $scpactionsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($scpactionsinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($scpactionsinproductlanguageresolvedarrayref);

        installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist($scpactionsinproductlanguageresolvedarrayref, $includepatharrayref_lang, $dirsinproductlanguageresolvedarrayref, "ScpActions");

        # Editing scpactions with flag SCPZIP_REPLACE and PATCH_SO_NAME.

        installer::scpzipfiles::resolving_scpzip_replace_flag($scpactionsinproductlanguageresolvedarrayref, $allvariableshashref, "ScpAction", $languagestringref);

        installer::scppatchsoname::resolving_patchsoname_flag($scpactionsinproductlanguageresolvedarrayref, $allvariableshashref, "ScpAction", $languagestringref);

        #########################################################
        # language dependent links part
        #########################################################

        installer::logger::print_message( "... analyzing links ...\n" );

        my $linksinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($linksinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($linksinproductlanguageresolvedarrayref);

        installer::scriptitems::get_destination_file_path_for_links($linksinproductlanguageresolvedarrayref, $filesinproductlanguageresolvedarrayref);

        installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($linksinproductlanguageresolvedarrayref, $dirsinproductarrayref);

        # Now taking all links that have no FileID but a ShortcutID, linking to another link

        installer::scriptitems::get_destination_link_path_for_links($linksinproductlanguageresolvedarrayref);

        $linksinproductlanguageresolvedarrayref = installer::scriptitems::remove_workstation_only_items($linksinproductlanguageresolvedarrayref);

        installer::scriptitems::resolve_links_with_flag_relative($linksinproductlanguageresolvedarrayref);

        #########################################################
        # language dependent unix links part
        #########################################################

        installer::logger::print_message( "... analyzing unix links ...\n" );

        my $unixlinksinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($unixlinksinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($unixlinksinproductlanguageresolvedarrayref);

        installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($unixlinksinproductlanguageresolvedarrayref, $dirsinproductarrayref);

        #########################################################
        # language dependent part for profiles and profileitems
        #########################################################

        my $profilesinproductlanguageresolvedarrayref;
        my $profileitemsinproductlanguageresolvedarrayref;

        if ((!($installer::globals::is_copy_only_project)) && (!($installer::globals::product =~ /ada/i )) && (!($installer::globals::languagepack)) && (!($installer::globals::helppack)))
        {
            installer::logger::print_message( "... creating profiles ...\n" );

            $profilesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($profilesinproductarrayref, $languagesarrayref);

            $profileitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($profileitemsinproductarrayref, $languagesarrayref);

            installer::scriptitems::changing_name_of_language_dependent_keys($profilesinproductlanguageresolvedarrayref);

            installer::scriptitems::changing_name_of_language_dependent_keys($profileitemsinproductlanguageresolvedarrayref);

            installer::scriptitems::replace_setup_variables($profileitemsinproductlanguageresolvedarrayref, $languagestringref, $allvariableshashref);

            if ( $installer::globals::patch_user_dir )
            {
                installer::scriptitems::replace_userdir_variable($profileitemsinproductlanguageresolvedarrayref);
            }

            installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($profilesinproductlanguageresolvedarrayref, $dirsinproductarrayref);

            # Now the Profiles can be created

            installer::profiles::create_profiles($profilesinproductlanguageresolvedarrayref, $profileitemsinproductlanguageresolvedarrayref, $filesinproductlanguageresolvedarrayref, $languagestringref, $allvariableshashref);
        }

        my $registryitemsinproductlanguageresolvedarrayref; # cannot be defined in the following "if ( $installer::globals::iswindowsbuild )"
        my $folderinproductlanguageresolvedarrayref;        # cannot be defined in the following "if ( $installer::globals::iswindowsbuild )"
        my $folderitemsinproductlanguageresolvedarrayref;   # cannot be defined in the following "if ( $installer::globals::iswindowsbuild )"

        if ( $installer::globals::iswindowsbuild )  # Windows specific items: Folder, FolderItem, RegistryItem
        {
            #########################################################
            # language dependent part for folder
            #########################################################

            installer::logger::print_message( "... analyzing folder ...\n" );

            $folderinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($folderinproductarrayref, $languagesarrayref);

            installer::scriptitems::changing_name_of_language_dependent_keys($folderinproductlanguageresolvedarrayref);

            #########################################################
            # language dependent part for folderitems
            #########################################################

            installer::logger::print_message( "... analyzing folderitems ...\n" );

            $folderitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($folderitemsinproductarrayref, $languagesarrayref);

            installer::scriptitems::changing_name_of_language_dependent_keys($folderitemsinproductlanguageresolvedarrayref);

            #########################################################
            # language dependent part for registryitems
            #########################################################

            installer::logger::print_message( "... analyzing registryitems ...\n" );

            $registryitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($registryitemsinproductarrayref, $languagesarrayref);

            installer::scriptitems::changing_name_of_language_dependent_keys($registryitemsinproductlanguageresolvedarrayref);
        }

        #########################################################
        # language dependent part for modules
        #########################################################

        my $modulesinproductlanguageresolvedarrayref;

        if (!($installer::globals::is_copy_only_project))
        {
            installer::logger::print_message( "... analyzing modules ...\n" );

            $modulesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($modulesinproductarrayref, $languagesarrayref);

            $modulesinproductlanguageresolvedarrayref = installer::scriptitems::remove_not_required_language_modules($modulesinproductlanguageresolvedarrayref, $languagesarrayref);

            if ( $installer::globals::analyze_spellcheckerlanguage )
            {
                $modulesinproductlanguageresolvedarrayref = installer::scriptitems::remove_not_required_spellcheckerlanguage_modules($modulesinproductlanguageresolvedarrayref);

                $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_not_required_spellcheckerlanguage_files($filesinproductlanguageresolvedarrayref);
            }

            installer::scriptitems::changing_name_of_language_dependent_keys($modulesinproductlanguageresolvedarrayref);

            installer::scriptitems::select_required_language_strings($modulesinproductlanguageresolvedarrayref);    # using english strings

        }

        # Copy-only projects can now start to copy all items File and ScpAction
        if ( $installer::globals::is_copy_only_project ) { installer::copyproject::copy_project($filesinproductlanguageresolvedarrayref, $scpactionsinproductlanguageresolvedarrayref, $loggingdir, $languagestringref, $shipinstalldir, $allsettingsarrayref); }

        # Language pack projects can now start to select the required information
        if ( $installer::globals::languagepack )
        {
            $filesinproductlanguageresolvedarrayref = installer::languagepack::select_language_items($filesinproductlanguageresolvedarrayref, $languagesarrayref, "File");
            $scpactionsinproductlanguageresolvedarrayref = installer::languagepack::select_language_items($scpactionsinproductlanguageresolvedarrayref, $languagesarrayref, "ScpAction");
            $linksinproductlanguageresolvedarrayref = installer::languagepack::select_language_items($linksinproductlanguageresolvedarrayref, $languagesarrayref, "Shortcut");
            $unixlinksinproductlanguageresolvedarrayref = installer::languagepack::select_language_items($unixlinksinproductlanguageresolvedarrayref, $languagesarrayref, "Unixlink");
            @{$folderitemsinproductlanguageresolvedarrayref} = (); # no folderitems in languagepacks

            # Collecting the directories again, to include only the language specific directories
            ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);
            ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($dirsinproductlanguageresolvedarrayref, $alldirectoryhash);
            @$directoriesforepmarrayref = sort { $a->{"HostName"} cmp $b->{"HostName"} } @$directoriesforepmarrayref;

            if ( $installer::globals::iswindowsbuild )
            {
                $registryitemsinproductlanguageresolvedarrayref = installer::worker::select_langpack_items($registryitemsinproductlanguageresolvedarrayref, "RegistryItem");
            }

        }

        # Help pack projects can now start to select the required information
        if ( $installer::globals::helppack )
        {
            $filesinproductlanguageresolvedarrayref = installer::helppack::select_help_items($filesinproductlanguageresolvedarrayref, $languagesarrayref, "File");
            $scpactionsinproductlanguageresolvedarrayref = installer::helppack::select_help_items($scpactionsinproductlanguageresolvedarrayref, $languagesarrayref, "ScpAction");
            $linksinproductlanguageresolvedarrayref = installer::helppack::select_help_items($linksinproductlanguageresolvedarrayref, $languagesarrayref, "Shortcut");
            $unixlinksinproductlanguageresolvedarrayref = installer::helppack::select_help_items($unixlinksinproductlanguageresolvedarrayref, $languagesarrayref, "Unixlink");
            @{$folderitemsinproductlanguageresolvedarrayref} = (); # no folderitems in helppacks

            # Collecting the directories again, to include only the language specific directories
            ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);
            ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($dirsinproductlanguageresolvedarrayref, $alldirectoryhash);
            @$directoriesforepmarrayref = sort { $a->{"HostName"} cmp $b->{"HostName"} } @$directoriesforepmarrayref;

            if ( $installer::globals::iswindowsbuild )
            {
                $registryitemsinproductlanguageresolvedarrayref = installer::worker::select_helppack_items($registryitemsinproductlanguageresolvedarrayref, "RegistryItem");
            }

        }

        # Collecting all files without flag PATCH (for maintenance reasons)
        if ( $installer::globals::patch ) { installer::worker::collect_all_files_without_patch_flag($filesinproductlanguageresolvedarrayref); }

        # Patch projects can now start to select the required information
        if (( $installer::globals::patch ) && (( $installer::globals::issolarispkgbuild ) || ( $installer::globals::iswindowsbuild )))
        {
            $filesinproductlanguageresolvedarrayref = installer::worker::select_patch_items($filesinproductlanguageresolvedarrayref, "File");
            $scpactionsinproductlanguageresolvedarrayref = installer::worker::select_patch_items($scpactionsinproductlanguageresolvedarrayref, "ScpAction");
            $linksinproductlanguageresolvedarrayref = installer::worker::select_patch_items($linksinproductlanguageresolvedarrayref, "Shortcut");
            $unixlinksinproductlanguageresolvedarrayref = installer::worker::select_patch_items($unixlinksinproductlanguageresolvedarrayref, "Unixlink");
            $folderitemsinproductlanguageresolvedarrayref = installer::worker::select_patch_items($folderitemsinproductlanguageresolvedarrayref, "FolderItem");
            # @{$folderitemsinproductlanguageresolvedarrayref} = (); # no folderitems in languagepacks

            if ( $installer::globals::iswindowsbuild )
            {
                $registryitemsinproductlanguageresolvedarrayref = installer::worker::select_patch_items_without_name($registryitemsinproductlanguageresolvedarrayref, "RegistryItem");

                installer::worker::prepare_windows_patchfiles($filesinproductlanguageresolvedarrayref, $languagestringref, $allvariableshashref);

                # For Windows patches, the directories can now be collected again
                ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);

                @$directoriesforepmarrayref = sort { $a->{"HostName"} cmp $b->{"HostName"} } @$directoriesforepmarrayref;
            }
        }

        #########################################################
        # Collecting all scp actions
        #########################################################

        installer::worker::collect_scpactions($scpactionsinproductlanguageresolvedarrayref);

        ###########################################################
        # Simple package projects can now start to create the
        # installation structure by creating Directories, Files
        # Links and ScpActions. This is the last platform
        # independent part.
        ###########################################################

        if ( $installer::globals::is_simple_packager_project )
        {
            installer::simplepackage::create_simple_package($filesinproductlanguageresolvedarrayref, $directoriesforepmarrayref, $scpactionsinproductlanguageresolvedarrayref, $linksinproductlanguageresolvedarrayref, $unixlinksinproductlanguageresolvedarrayref, $loggingdir, $languagestringref, $shipinstalldir, $allsettingsarrayref, $allvariableshashref, $includepatharrayref);
            next; # ! leaving the current loop, because no further packaging required.
        }

        ###########################################################
        # Analyzing the package structure
        ###########################################################

        installer::logger::print_message( "... analyzing package list ...\n" );

        my $packages = installer::packagelist::collectpackages($modulesinproductlanguageresolvedarrayref, $languagesarrayref);
        installer::packagelist::check_packagelist($packages);

        $packages = installer::packagelist::analyze_list($packages, $modulesinproductlanguageresolvedarrayref);
        installer::packagelist::remove_multiple_modules_packages($packages);

        # printing packages content:
        installer::packagelist::log_packages_content($packages);
        installer::packagelist::create_module_destination_hash($packages, $allvariableshashref);

        #################################################
        # Part 2: The platform dependent part
        #################################################

        #################################################
        # Part 2a: All non-Windows platforms
        #################################################

        #########################################################
        # ... creating epm list file ...
        # Only for non-Windows platforms
        #########################################################

        if (!( $installer::globals::iswindowsbuild ))
        {
            ####################################################
            # Writing log file before packages are packed
            ####################################################

            installer::logger::print_message( "... creating log file " . $loggingdir . $installer::globals::logfilename . "\n" );
            installer::files::save_file($loggingdir . $installer::globals::logfilename, \@installer::globals::logfileinfo);

            ####################################################
            # Creating directories
            ####################################################

            if ( $allvariableshashref->{'OOODOWNLOADNAME'} ) { installer::download::set_download_filename($languagestringref, $allvariableshashref); }
            else { installer::download::resolve_variables_in_downloadname($allvariableshashref, "", $languagestringref); }

            $installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);

            my $listfiledir = installer::systemactions::create_directories("listfile", $languagestringref);
            my $installlogdir = installer::systemactions::create_directory_next_to_directory($installdir, "log");

            ####################################################
            # Reading for Solaris all package descriptions
            # from file defined in property PACKAGEMAP
            ####################################################

            if (  $installer::globals::issolarisbuild ) { installer::epmfile::read_packagemap($allvariableshashref, $includepatharrayref, $languagesarrayref); }

            ###########################################
            # Checking epm state
            ###########################################

            my $epmexecutable = "";
            if ( $installer::globals::call_epm )
            {
                $epmexecutable = installer::epmfile::find_epm_on_system($includepatharrayref);
                installer::epmfile::set_patch_state($epmexecutable);    # setting $installer::globals::is_special_epm
            }

            # shuffle array to reduce parallel packaging process in pool
            installer::worker::shuffle_array($packages)
                unless $installer::globals::simple;

            # iterating over all packages
            for my $onepackage ( @{$packages} )
            {
                # checking, if this is a language pack or a project pack.
                # Creating language packs only, if $installer::globals::languagepack is set. Parameter: -languagepack

                if ( $installer::globals::languagepack ) { installer::languagepack::replace_languagestring_variable($onepackage, $languagestringref); }

                # checking, if this is a help pack
                # Creating help packs only, if $installer::globals::helppack is set. Parameter: -helppack

                if ( $installer::globals::helppack ) { installer::helppack::replace_languagestring_variable($onepackage, $languagestringref); }

                my $onepackagename = $onepackage->{'module'};           # name of the top module (required)

                my $shellscriptsfilename = "";
                if ( $onepackage->{'script'} ) { $shellscriptsfilename = $onepackage->{'script'}; }
                # no scripts for Solaris patches!
                if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild )) { $shellscriptsfilename = ""; }

                ###########################
                # package name
                ###########################

                my $packagename = "";

                if ( $installer::globals::issolarisbuild )   # only for Solaris
                {
                    if ( $onepackage->{'solarispackagename'} ) { $packagename = $onepackage->{'solarispackagename'}; }
                }
                else # not Solaris
                {
                    if ( $onepackage->{'packagename'} ) { $packagename = $onepackage->{'packagename'}; }
                }

                if (!($packagename eq ""))
                {
                    installer::packagelist::resolve_packagevariables(\$packagename, $allvariableshashref, 0);
                }

                # Debian allows no underline in package name
                if ( $installer::globals::debian ) { $packagename =~ s/_/-/g; }

                ####################################################
                # Header for this package into log file
                ####################################################

                installer::logger::include_header_into_logfile("Creating package: $packagename");

                ###########################################
                # Root path, can be defined as parameter
                ###########################################

                my $packagerootpath = "";

                if ($installer::globals::rootpath eq "")
                {
                    $packagerootpath = $onepackage->{'destpath'};
                    installer::packagelist::resolve_packagevariables(\$packagerootpath, $allvariableshashref, 1);
                    # we put branding and common stuff into the same prefix on unixes => $packagerootpath must be the whole prefix, including the product name
                    if ($installer::globals::isunix) { $packagerootpath .= "/$allvariableshashref->{'UNIXBASISROOTNAME'}"; }
                    if ( $^O =~ /darwin/i ) { $packagerootpath =~ s/\/opt\//\/Applications\//; }
                }
                else
                {
                    $packagerootpath = $installer::globals::rootpath;
                }

                #############################################
                # copying the collectors for each package
                #############################################

                my $filesinpackage = installer::converter::copy_collector($filesinproductlanguageresolvedarrayref);
                my $linksinpackage = installer::converter::copy_collector($linksinproductlanguageresolvedarrayref);
                my $unixlinksinpackage = installer::converter::copy_collector($unixlinksinproductlanguageresolvedarrayref);
                my $dirsinpackage = installer::converter::copy_collector($directoriesforepmarrayref);

                ###########################################
                # setting the root path for the packages
                ###########################################

                installer::scriptitems::add_rootpath_to_directories($dirsinpackage, $packagerootpath);
                installer::scriptitems::add_rootpath_to_files($filesinpackage, $packagerootpath);
                installer::scriptitems::add_rootpath_to_links($linksinpackage, $packagerootpath);
                installer::scriptitems::add_rootpath_to_files($unixlinksinpackage, $packagerootpath);

                #################################
                # collecting items for package
                #################################

                $filesinpackage = installer::packagelist::find_files_for_package($filesinpackage, $onepackage);
                $unixlinksinpackage = installer::packagelist::find_files_for_package($unixlinksinpackage, $onepackage);
                $linksinpackage = installer::packagelist::find_links_for_package($linksinpackage, $filesinpackage);
                $dirsinpackage = installer::packagelist::find_dirs_for_package($dirsinpackage, $onepackage);

                ###############################################
                # nothing to do, if $filesinpackage is empty
                ###############################################

                if ( ! ( $#{$filesinpackage} > -1 ))
                {
                    $infoline = "\n\nNo file in package: $packagename \-\> Skipping\n\n";
                    push(@installer::globals::logfileinfo, $infoline);
                    next;   # next package, end of loop !
                }

                #################################################################
                # nothing to do for Linux patches, if no file has flag PATCH
                #################################################################

                # Linux Patch: The complete RPM has to be built, if one file in the RPM has the flag PATCH (also for DEBs)
                if (( $installer::globals::patch ) && (( $installer::globals::isrpmbuild ) || ( $installer::globals::isdebbuild )))
                {
                    my $patchfiles = installer::worker::collect_all_items_with_special_flag($filesinpackage ,"PATCH");
                    if ( ! ( $#{$patchfiles} > -1 ))
                    {
                        $infoline = "\n\nLinux Patch: No patch file in package: $packagename \-\> Skipping\n\n";
                        push(@installer::globals::logfileinfo, $infoline);
                        next;
                    }
                }

                ###########################################
                # Stripping libraries
                ###########################################

                # Building for non Windows platforms in cws requires, that all files are stripped before packaging:
                # 1. copy all files that need to be stripped locally
                # 2. strip all these files

                if ( $installer::globals::strip )
                {
                    strip_libraries($filesinpackage, $languagestringref);
                }

                if ( $installer::globals::simple ) {
                    installer::worker::install_simple($onepackagename, $$languagestringref, $dirsinpackage, $filesinpackage, $linksinpackage, $unixlinksinpackage);
                }
                else {
                    # epm list file format:
                    # type mode owner group destination source options
                    # Example for a file: f 755 root sys /usr/bin/foo foo
                    # Example for a directory: d 755 root sys /var/spool/foo -
                    # Example for a link: l 000 root sys /usr/bin/linkname filename
                    # The source field specifies the file to link to

                    my $epmfilename = "epm_" . $onepackagename . ".lst";

                    installer::logger::print_message( "... creating epm list file $epmfilename ... \n" );

                    my $completeepmfilename = $listfiledir . $installer::globals::separator . $epmfilename;

                    my @epmfile = ();

                    my $epmheaderref = installer::epmfile::create_epm_header($allvariableshashref, $filesinproductlanguageresolvedarrayref, $languagesarrayref, $onepackage);
                    installer::epmfile::adding_header_to_epm_file(\@epmfile, $epmheaderref);

                    if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild ))
                    {
                        $filesinpackage = installer::worker::analyze_patch_files($filesinpackage);

                        if ( ! ( $#{$filesinpackage} > -1 ))
                        {
                            $infoline = "\nNo file in package: $packagename \-\> Skipping\n";
                            push(@installer::globals::logfileinfo, $infoline);
                            next;   # next package, end of loop !
                        }
                    }

                    # adding directories, files and links into epm file

                    installer::epmfile::put_directories_into_epmfile($dirsinpackage, \@epmfile, $allvariableshashref, $packagerootpath);
                    installer::epmfile::put_files_into_epmfile($filesinpackage, \@epmfile );
                    installer::epmfile::put_links_into_epmfile($linksinpackage, \@epmfile );
                    installer::epmfile::put_unixlinks_into_epmfile($unixlinksinpackage, \@epmfile );

                    if ((!( $shellscriptsfilename eq "" )) && (!($installer::globals::iswindowsbuild))) { installer::epmfile::adding_shellscripts_to_epm_file(\@epmfile, $shellscriptsfilename, $packagerootpath, $allvariableshashref, $filesinpackage); }

                    installer::files::save_file($completeepmfilename ,\@epmfile);

                    # ... splitting the rootpath into a relocatable part and a static part, if possible

                    my $staticpath = "";
                    my $relocatablepath = "";
                    # relocatable path can be defined in package list
                    if ( $onepackage->{'relocatablepath'} ) { $relocatablepath = $onepackage->{'relocatablepath'}; }
                    # setting fix part and variable part of destination path
                    installer::epmfile::analyze_rootpath($packagerootpath, \$staticpath, \$relocatablepath, $allvariableshashref);

                    # ... replacing the variable PRODUCTDIRECTORYNAME in the shellscriptfile by $staticpath

                    installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "PRODUCTDIRECTORYNAME", $staticpath);
                    installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "SOLSUREPACKAGEPREFIX", $allvariableshashref->{'SOLSUREPACKAGEPREFIX'});
                    installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "UREPACKAGEPREFIX", $allvariableshashref->{'UREPACKAGEPREFIX'});
                    installer::files::save_file($completeepmfilename ,\@epmfile);

                    {
                        # changing into the "install" directory to create installation sets

                        $currentdir = cwd();    # $currentdir is global in this file

                        chdir($installdir);     # changing into install directory ($installdir is global in this file)

                        ###########################################
                        # Starting epm
                        ###########################################

                        # With a patched epm, it is now possible to set the relocatable directory, change
                        # the directory in which the packages are created, setting "requires" and "provides"
                        # (Linux) or creating the "depend" file (Solaris) and finally to begin
                        # the packaging process with standard tooling and standard parameter
                        # Linux: Adding into the spec file: Prefix: /opt
                        # Solaris: Adding into the pkginfo file: BASEDIR=/opt
                        # Attention: Changing of the path can influence the shell scripts

                        if (( $installer::globals::is_special_epm ) && ( ($installer::globals::isrpmbuild) || ($installer::globals::issolarispkgbuild) ))   # special handling only for Linux RPMs and Solaris Packages
                        {
                            if ( $installer::globals::call_epm )    # only do something, if epm is really executed
                            {
                                # ... now epm can be started, to create the installation sets

                                installer::logger::print_message( "... starting patched epm ... \n" );

                                installer::epmfile::call_epm($epmexecutable, $completeepmfilename, $packagename, $includepatharrayref);

                                my $newepmdir = installer::epmfile::prepare_packages($loggingdir, $packagename, $staticpath, $relocatablepath, $onepackage, $allvariableshashref, $filesinpackage, $languagestringref); # adding the line for Prefix / Basedir, include rpmdir

                                installer::epmfile::create_packages_without_epm($newepmdir, $packagename, $includepatharrayref, $allvariableshashref, $languagestringref);  # start to package

                                # finally removing all temporary files

                                installer::epmfile::remove_temporary_epm_files($newepmdir, $loggingdir, $packagename);

                                # Installation:
                                # Install: pkgadd -a myAdminfile -d ./SUNWso8m34.pkg
                                # Install: rpm -i --prefix=/opt/special --nodeps so8m35.rpm

                                installer::epmfile::create_new_directory_structure($newepmdir);
                                $installer::globals::postprocess_specialepm = 1;

                                # solaris patch not needed anymore
                            }
                        }

                        else    # this is the standard epm (not relocatable) or ( nonlinux and nonsolaris )
                        {
                            installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "\$\$PRODUCTINSTALLLOCATION", $relocatablepath);
                            installer::files::save_file($completeepmfilename ,\@epmfile);   # Warning for pool, content of epm file is changed.

                            if ( $installer::globals::call_epm )
                            {
                                # ... now epm can be started, to create the installation sets

                                installer::logger::print_message( "... starting unpatched epm ... \n" );

                                if ( $installer::globals::call_epm ) { installer::epmfile::call_epm($epmexecutable, $completeepmfilename, $packagename, $includepatharrayref); }

                                if (($installer::globals::isrpmbuild) || ($installer::globals::issolarispkgbuild) || ($installer::globals::debian))
                                {
                                    $installer::globals::postprocess_standardepm = 1;
                                }
                            }
                        }

                        chdir($currentdir); # changing back into start directory

                    }

                } # end of "if ( ! $installer::globals::simple )

            }   # end of "for ( @{$packages} )"

            ##############################################################
            # Post epm functionality, after the last package is packed
            ##############################################################

            if ( $installer::globals::postprocess_specialepm )
            {
                installer::logger::include_header_into_logfile("Post EPM processes (Patched EPM):");

                chdir($installdir);

                # Copying the cde, kde and gnome packages into the installation set
                if ( $installer::globals::addsystemintegration ) { installer::epmfile::put_systemintegration_into_installset($installer::globals::epmoutpath, $includepatharrayref, $allvariableshashref, $modulesinproductarrayref); }

                # Adding license and readme into installation set
                if ($installer::globals::addlicensefile) { installer::worker::put_scpactions_into_installset("."); }

                # Adding license file into setup
                if ( $allvariableshashref->{'PUT_LICENSE_INTO_SETUP'} ) { installer::worker::put_license_into_setup(".", $includepatharrayref); }

                # Creating installation set for Unix language packs, that are not part of multi lingual installation sets
                if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::languagepack::build_installer_for_languagepack($installer::globals::epmoutpath, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

                # Creating installation set for Unix help packs, that are not part of multi lingual installation sets
                if ( ( $installer::globals::helppack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::helppack::build_installer_for_helppack($installer::globals::epmoutpath, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

                # Finalizing patch installation sets
                if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild )) { installer::epmfile::finalize_patch($installer::globals::epmoutpath, $allvariableshashref); }
                if (( $installer::globals::patch ) && ( $installer::globals::isrpmbuild )) { installer::epmfile::finalize_linux_patch($installer::globals::epmoutpath, $allvariableshashref, $includepatharrayref); }

                chdir($currentdir); # changing back into start directory
            }

            if ( $installer::globals::postprocess_standardepm )
            {
                installer::logger::include_header_into_logfile("Post EPM processes (Standard EPM):");

                chdir($installdir);

                # determine the destination directory
                my $newepmdir = installer::epmfile::determine_installdir_ooo();

                # Copying the cde, kde and gnome packages into the installation set
                if ( $installer::globals::addsystemintegration ) { installer::epmfile::put_systemintegration_into_installset($newepmdir, $includepatharrayref, $allvariableshashref, $modulesinproductarrayref); }

                # Adding license and readme into installation set
                if ($installer::globals::addlicensefile) { installer::worker::put_scpactions_into_installset("."); }

                # Adding license file into setup
                if ( $allvariableshashref->{'PUT_LICENSE_INTO_SETUP'} ) { installer::worker::put_license_into_setup(".", $includepatharrayref); }

                # Creating installation set for Unix language packs, that are not part of multi lingual installation sets
                if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::languagepack::build_installer_for_languagepack($newepmdir, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

                # Creating installation set for Unix help packs, that are not part of multi lingual installation sets
                if ( ( $installer::globals::helppack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::helppack::build_installer_for_helpepack($newepmdir, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

                chdir($currentdir); # changing back into start directory
            }

            if (( $installer::globals::issolarispkgbuild ) && ( $allvariableshashref->{'COLLECT_PKGMAP'} )) { installer::worker::collectpackagemaps($installdir, $languagestringref, $allvariableshashref); }

            #######################################################
            # Analyzing the log file
            #######################################################

            my $is_success = 0;
            my $finalinstalldir = "";

            installer::worker::clean_output_tree(); # removing directories created in the output tree
            ($is_success, $finalinstalldir) = installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
            my $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "downloadname");

            #######################################################
            # Creating download installation set
            #######################################################

            if ( $installer::globals::makedownload )
            {
                my $create_download = 0;
                if ( $$downloadname ne "" ) { $create_download = 1; }
                if ( $installer::globals::iswindowsbuild ) { $create_download = 0; }
                if (( $is_success ) && ( $create_download ) && ( $ENV{'ENABLE_DOWNLOADSETS'} ))
                {
                    my $downloaddir = installer::download::create_download_sets($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref);
                    installer::worker::analyze_and_save_logfile($loggingdir, $downloaddir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
                }
            }
        }   # end of "if (!( $installer::globals::iswindowsbuild ))"

        #################################################
        # Part 2b: The Windows platform
        #################################################

        #####################################################################
        # ... creating idt files ...
        # Only for Windows builds ($installer::globals::compiler is wntmsci)
        #####################################################################

        if ( $installer::globals::iswindowsbuild )
        {
            ###########################################
            # Stripping libraries
            ###########################################

            # Building for gcc build in cws requires, that all files are stripped before packaging:
            # 1. copy all files that need to be stripped locally
            # 2. strip all these files

            if ( $installer::globals::compiler =~ /wntgcci/ )
            {
                installer::windows::strip::strip_binaries($filesinproductlanguageresolvedarrayref, $languagestringref);
            }

            $installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);

             my $idtdirbase = installer::systemactions::create_directories("idt_files", $languagestringref);
             $installer::globals::infodirectory = installer::systemactions::create_directories("info_files", $languagestringref);
            my $installlogdir = installer::systemactions::create_directory_next_to_directory($installdir, "log");

            #################################################################################
            # Preparing cabinet files from package definitions
            #################################################################################

            installer::packagelist::prepare_cabinet_files($packages, $allvariableshashref);
            # printing packages content:
            installer::packagelist::log_cabinet_assignments();

            #################################################################################
            # Begin of functions that are used for the creation of idt files (Windows only)
            #################################################################################

            installer::logger::print_message( "... creating idt files ...\n" );

            installer::logger::include_header_into_logfile("Creating idt files:");

            my $newidtdir = $idtdirbase . $installer::globals::separator . "00";    # new files into language independent directory "00"
            installer::systemactions::create_directory($newidtdir);

            my @allfilecomponents = ();
            my @allregistrycomponents = ();

            # Collecting all files with flag "BINARYTABLE"
            my $binarytablefiles = installer::worker::collect_all_items_with_special_flag($filesinproductlanguageresolvedarrayref ,"BINARYTABLE");

            # Removing all files with flag "BINARYTABLE_ONLY"
            @installer::globals::binarytableonlyfiles = ();
            $filesinproductlanguageresolvedarrayref = installer::worker::remove_all_items_with_special_flag($filesinproductlanguageresolvedarrayref ,"BINARYTABLE_ONLY");

            # Collecting all profileitems with flag "INIFILETABLE" for table "IniFile"
            my $inifiletableentries = installer::worker::collect_all_items_with_special_flag($profileitemsinproductlanguageresolvedarrayref ,"INIFILETABLE");

            # Creating the important dynamic idt files
            installer::windows::msiglobal::set_msiproductversion($allvariableshashref);
            installer::windows::msiglobal::put_msiproductversion_into_bootstrapfile($filesinproductlanguageresolvedarrayref);

            # Add cabinet assignments to files
            installer::windows::file::assign_cab_to_files($filesinproductlanguageresolvedarrayref);
            installer::windows::file::assign_sequencenumbers_to_files($filesinproductlanguageresolvedarrayref);

            # Collection all available directory trees
            installer::windows::directory::collectdirectorytrees($directoriesforepmarrayref);

            # Attention: The table "Director.idt" contains language specific strings -> parameter: $languagesarrayref !
            installer::windows::directory::create_directory_table($directoriesforepmarrayref, $languagesarrayref, $newidtdir, $allvariableshashref, $shortdirname, $loggingdir);

            $filesinproductlanguageresolvedarrayref = installer::windows::file::create_files_table($filesinproductlanguageresolvedarrayref, $directoriesforepmarrayref, \@allfilecomponents, $newidtdir, $allvariableshashref, $uniquefilename, $allupdatesequences, $allupdatecomponents, $allupdatefileorder);
            if ( $installer::globals::updatedatabase ) { installer::windows::file::check_file_sequences($allupdatefileorder, $allupdatecomponentorder); }

            # Attention: The table "Registry.idt" contains language specific strings -> parameter: $languagesarrayref !
            installer::windows::registry::create_registry_table($registryitemsinproductlanguageresolvedarrayref, \@allregistrycomponents, $newidtdir, $languagesarrayref, $allvariableshashref);

            installer::windows::component::create_component_table($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $directoriesforepmarrayref, \@allfilecomponents, \@allregistrycomponents, $newidtdir, $componentid, $componentidkeypath, $allvariableshashref);

            # Attention: The table "Feature.idt" contains language specific strings -> parameter: $languagesarrayref !
            installer::windows::feature::add_uniquekey($modulesinproductlanguageresolvedarrayref);
            $modulesinproductlanguageresolvedarrayref = installer::windows::feature::sort_feature($modulesinproductlanguageresolvedarrayref);
            installer::windows::feature::create_feature_table($modulesinproductlanguageresolvedarrayref, $newidtdir, $languagesarrayref, $allvariableshashref);

            installer::windows::featurecomponent::create_featurecomponent_table($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $newidtdir);

            installer::windows::media::create_media_table($filesinproductlanguageresolvedarrayref, $newidtdir, $allvariableshashref, $allupdatelastsequences, $allupdatediskids);

            installer::windows::font::create_font_table($filesinproductlanguageresolvedarrayref, $newidtdir);

            # Attention: The table "Shortcut.idt" contains language specific strings -> parameter: $languagesarrayref !
            # Attention: Shortcuts (Folderitems) have icon files, that have to be copied into the Icon directory (last parameter)
            my @iconfilecollector = ();

            installer::windows::shortcut::create_shortcut_table($filesinproductlanguageresolvedarrayref, $linksinproductlanguageresolvedarrayref, $folderinproductlanguageresolvedarrayref, $folderitemsinproductlanguageresolvedarrayref, $directoriesforepmarrayref, $newidtdir, $languagesarrayref, $includepatharrayref, \@iconfilecollector);

            installer::windows::inifile::create_inifile_table($inifiletableentries, $filesinproductlanguageresolvedarrayref, $newidtdir);

            installer::windows::icon::create_icon_table(\@iconfilecollector, $newidtdir);    # creating the icon table with all iconfiles used as shortcuts (FolderItems)

            installer::windows::createfolder::create_createfolder_table($directoriesforepmarrayref, $filesinproductlanguageresolvedarrayref, $newidtdir, $allvariableshashref);

            installer::windows::upgrade::create_upgrade_table($newidtdir, $allvariableshashref);

            if (( ! $installer::globals::languagepack ) && ( ! $installer::globals::helppack )) # the following tables not for language packs or help packs
            {
                installer::windows::removefile::create_removefile_table($folderitemsinproductlanguageresolvedarrayref, $newidtdir);

                # Adding Assemblies into the tables MsiAssembly and MsiAssemblyName dynamically
                installer::windows::assembly::create_msiassembly_table($filesinproductlanguageresolvedarrayref, $newidtdir);
                installer::windows::assembly::create_msiassemblyname_table($filesinproductlanguageresolvedarrayref, $newidtdir);
                installer::windows::assembly::add_assembly_condition_into_component_table($filesinproductlanguageresolvedarrayref, $newidtdir);
            }

            $infoline = "\n";
            push(@installer::globals::logfileinfo, $infoline);

            # Localizing the language dependent idt files
            # For every language there will be a localized msi database
            # For multilingual installation sets, the differences of this
            # databases have to be stored in transforms.

            for ( my $m = 0; $m <= $#{$languagesarrayref}; $m++ )
            {
                my $onelanguage = ${$languagesarrayref}[$m];

                my $is_rtl = 0;
                my @rtllanguages = ("ar", "fa", "he", "ug", "ky-CN");
                if ( grep {$_ eq $onelanguage} @rtllanguages ) { $is_rtl = 1; }

                my $languageidtdir = $idtdirbase . $installer::globals::separator . $onelanguage;
                if ( -d $languageidtdir ) { installer::systemactions::remove_complete_directory($languageidtdir, 1); }
                installer::systemactions::create_directory($languageidtdir);

                # Copy the template idt files and the new created idt files into this language directory

                installer::logger::print_message( "... copying idt files ...\n" );

                installer::logger::include_header_into_logfile("Copying idt files to $languageidtdir:");

                installer::windows::idtglobal::prepare_language_idt_directory($languageidtdir, $newidtdir, $onelanguage, $filesinproductlanguageresolvedarrayref, \@iconfilecollector, $binarytablefiles, $allvariableshashref);

                # Now all files are copied into a language specific directory
                # The template idt files can be translated

                installer::logger::print_message( "... localizing idt files (language: $onelanguage) ...\n" );

                installer::logger::include_header_into_logfile("Localizing idt files (Language: $onelanguage):");

                my @translationfiles = ();          # all idt files, that need a translation
                push(@translationfiles, "ActionTe.idt");
                push(@translationfiles, "Control.idt");
                push(@translationfiles, "CustomAc.idt");
                push(@translationfiles, "Error.idt");
                push(@translationfiles, "LaunchCo.idt");
                push(@translationfiles, "RadioBut.idt");
                push(@translationfiles, "Property.idt");
                push(@translationfiles, "UIText.idt");

                my $oneidtfilename;
                my $oneidtfile;

                foreach $oneidtfilename (@translationfiles)
                {
                    my $languagefilename = installer::windows::idtglobal::get_languagefilename($oneidtfilename, $installer::globals::idtlanguagepath);
                    my $languagefile = installer::files::read_file($languagefilename);

                    $oneidtfilename = $languageidtdir . $installer::globals::separator . $oneidtfilename;
                    $oneidtfile = installer::files::read_file($oneidtfilename);

                    # Now the substitution can start
                    installer::windows::idtglobal::translate_idtfile($oneidtfile, $languagefile, $onelanguage);

                    installer::files::save_file($oneidtfilename, $oneidtfile);

                    $infoline = "Translated idt file: $oneidtfilename into language $onelanguage\n";
                    push(@installer::globals::logfileinfo, $infoline);
                    $infoline = "Used languagefile: $languagefilename\n";
                    push(@installer::globals::logfileinfo, $infoline);
                }

                # setting the encoding in every table (replacing WINDOWSENCODINGTEMPLATE)

                installer::windows::idtglobal::setencoding($languageidtdir, $onelanguage);

                # setting bidi attributes, if required

                if ( $is_rtl ) { installer::windows::idtglobal::setbidiattributes($languageidtdir, $onelanguage); }

                # setting the encoding in every table (replacing WINDOWSENCODINGTEMPLATE)
                installer::windows::idtglobal::set_multilanguageonly_condition($languageidtdir);

                # include the license text into the table Control.idt

                if ( ! $allvariableshashref->{'HIDELICENSEDIALOG'} )
                {
                    my $licensefilesource = installer::windows::idtglobal::get_rtflicensefilesource($onelanguage, $includepatharrayref_lang);
                    my $licensefile = installer::files::read_file($licensefilesource);
                    installer::scpzipfiles::replace_all_ziplistvariables_in_rtffile($licensefile, $allvariableshashref);
                    my $controltablename = $languageidtdir . $installer::globals::separator . "Control.idt";
                    my $controltable = installer::files::read_file($controltablename);
                    installer::windows::idtglobal::add_licensefile_to_database($licensefile, $controltable);
                    installer::files::save_file($controltablename, $controltable);

                    $infoline = "Added licensefile $licensefilesource into database $controltablename\n";
                    push(@installer::globals::logfileinfo, $infoline);
                }

                # include a component into environment table if required

                installer::windows::component::set_component_in_environment_table($languageidtdir, $filesinproductlanguageresolvedarrayref);

                # include the ProductCode and the UpgradeCode from codes-file into the Property.idt

                installer::windows::property::set_codes_in_property_table($languageidtdir);

                # the language specific properties can now be set in the Property.idt

                installer::windows::property::update_property_table($languageidtdir, $onelanguage, $allvariableshashref, $languagestringref);

                # replacing variables in RegLocat.idt

                installer::windows::msiglobal::update_reglocat_table($languageidtdir, $allvariableshashref);

                # replacing variables in RemoveRe.idt (RemoveRegistry.idt)

                installer::windows::msiglobal::update_removere_table($languageidtdir);

                # adding language specific properties for multilingual installation sets

                installer::windows::property::set_languages_in_property_table($languageidtdir, $languagesarrayref);

                # adding settings into CheckBox.idt
                installer::windows::property::update_checkbox_table($languageidtdir, $allvariableshashref);

                # adding the files from the binary directory into the binary table
                installer::windows::binary::update_binary_table($languageidtdir, $filesinproductlanguageresolvedarrayref, $binarytablefiles);

                # Adding Windows Installer CustomActions

                installer::windows::idtglobal::addcustomactions($languageidtdir, $windowscustomactionsarrayref, $filesinproductlanguageresolvedarrayref);

                # Then the language specific msi database can be created

                if ( $installer::globals::iswin || $installer::globals::packageformat eq 'msi' )
                {
                    my $msidatabasename = installer::windows::msiglobal::get_msidatabasename($allvariableshashref, $onelanguage);
                    my $msifilename = $languageidtdir . $installer::globals::separator . $msidatabasename;

                    installer::logger::print_message( "... creating msi database (language $onelanguage) ... \n" );

                    installer::windows::msiglobal::set_uuid_into_component_table($languageidtdir, $allvariableshashref);    # setting new GUID for the components using the tool uuidgen.exe
                    installer::windows::msiglobal::prepare_64bit_database($languageidtdir, $allvariableshashref);   # making last 64 bit changes
                    installer::windows::msiglobal::create_msi_database($languageidtdir ,$msifilename);

                    # validating the database   # ToDo

                    my $languagefile = installer::files::read_file($installer::globals::idtlanguagepath . $installer::globals::separator . "SIS.mlf");

                    installer::windows::msiglobal::write_summary_into_msi_database($msifilename, $onelanguage, $languagefile, $allvariableshashref);

                    # if there are Merge Modules, they have to be integrated now
                    $filesinproductlanguageresolvedarrayref = installer::windows::mergemodule::merge_mergemodules_into_msi_database($mergemodulesarrayref, $filesinproductlanguageresolvedarrayref, $msifilename, $languagestringref, $onelanguage, $languagefile, $allvariableshashref, $includepatharrayref, $allupdatesequences, $allupdatelastsequences, $allupdatediskids);

                    # copy msi database into installation directory

                    my $msidestfilename = $installdir . $installer::globals::separator . $msidatabasename;
                    installer::systemactions::copy_one_file($msifilename, $msidestfilename);
                }
            }

            # Creating transforms, if the installation set has more than one language
            # renaming the msi database

            my $defaultlanguage = installer::languages::get_default_language($languagesarrayref);

            if ( $installer::globals::iswin || $installer::globals::packageformat eq 'msi' )
            {
                if  ( $#{$languagesarrayref} > 0 )
                {
                    installer::windows::msiglobal::create_transforms($languagesarrayref, $defaultlanguage, $installdir, $allvariableshashref);
                }

                installer::windows::msiglobal::rename_msi_database_in_installset($defaultlanguage, $installdir, $allvariableshashref);
            }

            # Analyzing the ScpActions and copying the files into the installation set

            installer::logger::print_message( "... copying files into installation set ...\n" );

            installer::worker::put_scpactions_into_installset($installdir);

            # ... copying MergeModules into installation set

            if ( ! $installer::globals::fix_number_of_cab_files ) { installer::windows::msiglobal::copy_merge_modules_into_installset($installdir); }

            installer::logger::print_message( "... creating ddf files ...\n" );

            # Creating all needed ddf files and generating a list
            # for the package process containing all system calls

            my $ddfdir = installer::systemactions::create_directories("ddf", $languagestringref);

            my $packjobref = installer::windows::msiglobal::generate_cab_file_list($filesinproductlanguageresolvedarrayref, $installdir, $ddfdir, $allvariableshashref);

            # Update and patch reasons the pack order needs to be saved
            installer::windows::msiglobal::save_packorder();

            $infoline = "\n";
            push(@installer::globals::logfileinfo, $infoline);

            ####################################
            # Writing log file
            # before cab files are packed
            ####################################

            installer::logger::print_message( "... creating log file $installer::globals::logfilename \n" );

            installer::files::save_file($loggingdir . $installer::globals::logfilename, \@installer::globals::logfileinfo);

            #######################################################
            # Finally really create the installation packages,
            # Only for Windows and only on a windows platform.
            #######################################################

            if ( $installer::globals::iswin || $installer::globals::packageformat eq 'msi' )
            {
                installer::logger::print_message( "... packaging installation set ... \n" );
                installer::windows::msiglobal::execute_packaging($packjobref, $loggingdir, $allvariableshashref);
                if ( $installer::globals::include_cab_in_msi ) { installer::windows::msiglobal::include_cabs_into_msi($installdir); }

                ####################################
                # Writing log file
                # after cab files are packed
                ####################################

                installer::logger::print_message( "\n... creating log file $installer::globals::logfilename \n" );
                installer::files::save_file($loggingdir . $installer::globals::logfilename, \@installer::globals::logfileinfo);
            }

            #######################################################
            # Analyzing the log file
            #######################################################

            my $is_success = 0;
            my $finalinstalldir = "";
            installer::worker::clean_output_tree(); # removing directories created in the output tree
            ($is_success, $finalinstalldir) = installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);

            #######################################################
            # Creating Windows msp patches
            #######################################################

            if (( $is_success ) && ( $installer::globals::updatedatabase ) && ( $allvariableshashref->{'CREATE_MSP_INSTALLSET'} ))
            {
                # Required:
                # Temp path for administrative installations: $installer::globals::temppath
                # Path of new installation set: $finalinstalldir
                # Path of old installation set: $installer::globals::updatedatabasepath
                my $mspdir = installer::windows::msp::create_msp_patch($finalinstalldir, $includepatharrayref, $allvariableshashref, $languagestringref, $languagesarrayref, $filesinproductlanguageresolvedarrayref);
                ($is_success, $finalinstalldir) = installer::worker::analyze_and_save_logfile($loggingdir, $mspdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
                installer::worker::clean_output_tree(); # removing directories created in the output tree
            }

            #######################################################
            # Creating download installation set
            #######################################################

            my $create_download = 0;
            my $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "downloadname");
            if ( $installer::globals::languagepack ) { $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "langpackdownloadname"); }
            if ( $installer::globals::helppack ) { $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "helppackdownloadname"); }
            if ( $installer::globals::patch ) { $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "patchdownloadname"); }

            if ( $$downloadname ne "" ) { $create_download = 1; }
            if ( $installer::globals::iswindowsbuild )
            {
                $create_download = 0;
                if ( $allvariableshashref->{'OOODOWNLOADNAME'} ) { $$downloadname = installer::download::set_download_filename($languagestringref, $allvariableshashref); }
                else { $$downloadname = installer::download::resolve_variables_in_downloadname($allvariableshashref, $$downloadname, $languagestringref); }
                installer::systemactions::rename_one_file( $finalinstalldir . $installer::globals::separator . $installer::globals::shortmsidatabasename, $finalinstalldir . $installer::globals::separator . $$downloadname . ".msi" );

            }
            if (( $is_success ) && ( $create_download ) && ( $ENV{'ENABLE_DOWNLOADSETS'} ))
            {
                my $downloaddir = installer::download::create_download_sets($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref);
                installer::worker::analyze_and_save_logfile($loggingdir, $downloaddir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
            }

        }    # end of "if ( $installer::globals::iswindowsbuild )"

        # saving file_info file for later analysis
        my $speciallogfilename = $loggingdir . "fileinfo_" . $installer::globals::product . "\.log";
        open my $log_fh, '>', $speciallogfilename
            or die "Could not open $speciallogfilename for writing: $!";
        print $log_fh Dumper($filesinproductlanguageresolvedarrayref);

    }   # end of iteration for one language group
}

1;
