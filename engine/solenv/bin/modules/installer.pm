#
# This file is part of the Collabora Office project.
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

package installer;

use strict;
use warnings;

use base 'Exporter';

use Cwd;
use Data::Dumper;
use File::Copy;
use List::Util qw(shuffle);
use installer::control;
use installer::converter;
use installer::copyproject;
use installer::download;
use installer::environment;
use installer::epmfile;
use installer::files;
use installer::filelists;
use installer::globals;
use installer::helppack;
use installer::languagepack;
use installer::languages;
use installer::logger;
use installer::packagelist;
use installer::parameter;
use installer::pathanalyzer;
use installer::profiles;
use installer::scpzipfiles;
use installer::scriptitems;
use installer::setupscript;
use installer::simplepackage;
use installer::splitdbg qw(splitdbg_libraries);
use installer::strip qw(strip_libraries);
use installer::systemactions;
use installer::worker;
use installer::ziplist qw(read_ziplist);

our @EXPORT_OK = qw(main);

sub main {
    installer::logger::starttime();

    my $exit_code = 0;

    eval {
        run();
    };
    if ($@) {
        my $message = "ERROR: $@";

        warn "ERROR: Failure in installer.pm\n";
        warn "$message\n";
        $exit_code = -1;

        cleanup_on_error($message);
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

    my $empty = "";
    my $loggingdir = installer::systemactions::create_directories("logging", \$empty);
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

    ###################################################
    # Analyzing the settings and variables in zip.lst
    ###################################################

    my ($allsettingsarrayref, $allvariableshashref) = read_ziplist($installer::globals::ziplistname);

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
    # FIXME: It would be better to use installer::systemactions::remove_complete_directory
    #        Though, we would need to remove only the lang-specific subdirectory for langpacks and helppacks
    rmdir $oldloggingdir;
    $loggingdir = installer::systemactions::create_directories("logging", \$empty);
    $loggingdir = $loggingdir . $installer::globals::separator;
    $installer::globals::exitlog = $loggingdir;

    # checking, whether this is an opensource product

    if (!($installer::globals::is_copy_only_project)) { installer::ziplist::set_manufacturer($allvariableshashref); }

    ##########################################################
    # Getting the include path from the settings in zip list
    ##########################################################

    my $includepathref = installer::ziplist::getinfofromziplist($allsettingsarrayref, "include");
    if ( $$includepathref eq "" )
    {
        die 'Definition for "include" not found in ' . $installer::globals::ziplistname;
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
    # Windows requires the LCID list
    #####################################


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

    # Replacing preset properties, not using the default mechanisms (for example for UNIXPRODUCTNAME)
    installer::setupscript::replace_preset_properties($allvariableshashref);

    installer::scpzipfiles::replace_all_ziplistvariables_in_file($setupscriptref, $allvariableshashref);


    installer::logger::log_hashref($allvariableshashref);

    installer::logger::print_message( "... analyzing directories ... \n" );

    # Collect all directories in the script to get the destination dirs

    my $dirsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Directory");

    if ( $allvariableshashref->{'SHIFT_BASIS_INTO_BRAND_LAYER'} ) { $dirsinproductarrayref = installer::scriptitems::shift_basis_directory_parents($dirsinproductarrayref); }
    if ( $allvariableshashref->{'OFFICEDIRECTORYNAME'} ) { installer::scriptitems::set_officedirectory_name($dirsinproductarrayref, $allvariableshashref->{'OFFICEDIRECTORYNAME'}); }


    installer::scriptitems::resolve_all_directory_names($dirsinproductarrayref);

    installer::logger::print_message( "... analyzing files ... \n" );

    my $filesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "File");

    if (( ! $installer::globals::isrpmbuild ) &&
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

    installer::logger::print_message( "... analyzing scpactions ... \n" );

    my $scpactionsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "ScpAction");

    if ( $installer::globals::languagepack ) { installer::scriptitems::use_langpack_copy_scpaction($scpactionsinproductarrayref); }
    elsif ( $installer::globals::helppack ) { installer::scriptitems::use_langpack_copy_scpaction($scpactionsinproductarrayref); }
    # TODO: why is this not done in scp2 based on the value of $(ENABLE_RELEASE_BUILD)?
    elsif ( $allvariableshashref->{'PRODUCTNAME'} eq "LibreOfficeDev" ) { installer::scriptitems::use_devversion_copy_scpaction($scpactionsinproductarrayref); }

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
    my $folderitempropertiesinproductarrayref;
    my $registryitemsinproductarrayref;
    my $windowscustomactionsarrayref;
    my $mergemodulesarrayref;

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
        my @setuplanguagesarray = @{ $languagesarrayref };

        my $languagestringref = installer::languages::get_language_string($languagesarrayref);
        installer::logger::print_message( "------------------------------------\n" );
        installer::logger::print_message( "... languages $$languagestringref ... \n" );

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
            @setuplanguagesarray = grep { $_ ne "en-US" } @setuplanguagesarray;
            unshift(@setuplanguagesarray, "en-US");
        }

        ############################################################
        # Beginning of language specific logging mechanism
        # Until now only global logging into default: logfile.txt
        ############################################################

        @installer::globals::logfileinfo = ();  # new logfile array and new logfile name
        installer::logger::copy_globalinfo_into_logfile();

        my $loglanguagestring = $$languagestringref;
        my $loglanguagestring_orig = $loglanguagestring;
        if (length($loglanguagestring) > $installer::globals::max_lang_length)
        {
            my $number_of_languages = installer::systemactions::get_number_of_langs($loglanguagestring);
            chomp(my $shorter = `echo $loglanguagestring | $ENV{'MD5SUM'} | sed -e "s/ .*//g"`);
            my $id = substr($shorter, 0, 8); # taking only the first 8 digits
            $loglanguagestring = "lang_" . $number_of_languages . "_id_" . $id;
        }

        $installer::globals::logfilename = "log_" . $installer::globals::build;
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

        ################################################
        # Resolving include paths (language dependent)
        ################################################

        my $includepatharrayref_lang = installer::ziplist::replace_languages_in_paths($includepatharrayref, \@setuplanguagesarray);

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

        installer::scriptitems::changing_name_of_language_dependent_keys($filesinproductlanguageresolvedarrayref);

        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_non_existent_languages_in_productlists($filesinproductlanguageresolvedarrayref, $languagestringref, "Name", "file");

        installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($filesinproductlanguageresolvedarrayref, $dirsinproductarrayref);

        installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist($filesinproductlanguageresolvedarrayref, $includepatharrayref_lang, $dirsinproductlanguageresolvedarrayref, "Files", $allvariableshashref);

        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_Without_Sourcedirectory($filesinproductlanguageresolvedarrayref);

        if ($installer::globals::languagepack)
        {
            $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_For_Languagepacks($filesinproductlanguageresolvedarrayref);
        }

        $filesinproductlanguageresolvedarrayref = installer::scriptitems::add_License_Files_into_Installdir($filesinproductlanguageresolvedarrayref, $dirsinproductlanguageresolvedarrayref, $languagesarrayref);

        installer::scriptitems::make_filename_language_specific($filesinproductlanguageresolvedarrayref);

        ######################################################################################
        # Processing files with flag FILELIST and putting listed files into the file list
        ######################################################################################

        installer::logger::print_message( "... analyzing files with flag FILELIST ...\n" );

        ($filesinproductlanguageresolvedarrayref, $unixlinksinproductarrayref) = installer::filelists::resolve_filelist_flag($filesinproductlanguageresolvedarrayref, $unixlinksinproductarrayref, $ENV{'INSTDIR'});

        # packed files sometimes contain a "$" in their name: HighlightText$1.class. For epm the "$" has to be quoted by "$$"

        installer::scriptitems::quoting_illegal_filenames($filesinproductlanguageresolvedarrayref);
        #####################################
        # Files with flag SCPZIP_REPLACE
        #####################################

        installer::logger::print_message( "... analyzing files with flag SCPZIP_REPLACE ...\n" );

        # Editing files with flag SCPZIP_REPLACE.

        installer::scpzipfiles::resolving_scpzip_replace_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);

        #########################################################
        # language dependent unix links part
        #########################################################

        installer::logger::print_message( "... analyzing unix links ...\n" );

        my $unixlinksinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($unixlinksinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($unixlinksinproductlanguageresolvedarrayref);

        installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($unixlinksinproductlanguageresolvedarrayref, $dirsinproductarrayref);

        ############################################
        # Collecting directories for epm list file
        ############################################

        installer::logger::print_message( "... analyzing all directories for this product ...\n" );

        # There are two ways for a directory to be included into the epm directory list:
        # 1. Looking for all destination paths in the files array
        # 2. Looking for directories with CREATE flag in the directory array
        # Advantage: Many paths are hidden in zip files, they are not defined in the setup script.
        # It will be possible, that in the setup script only those directories have to be defined,
        # that have a CREATE flag. All other directories are created, if they contain at least one file.

        my $alldirectoryhash = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref, $unixlinksinproductlanguageresolvedarrayref);
        my $directoriesforepmarrayref = installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($dirsinproductlanguageresolvedarrayref, $alldirectoryhash);

        #########################################################
        # language dependent scpactions part
        #########################################################

        my $scpactionsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($scpactionsinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($scpactionsinproductlanguageresolvedarrayref);

        installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist($scpactionsinproductlanguageresolvedarrayref, $includepatharrayref_lang, $dirsinproductlanguageresolvedarrayref, "ScpActions", $allvariableshashref);

        # Editing scpactions with flag SCPZIP_REPLACE.

        installer::scpzipfiles::resolving_scpzip_replace_flag($scpactionsinproductlanguageresolvedarrayref, $allvariableshashref, "ScpAction", $languagestringref);

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
        # language dependent part for profiles and profileitems
        #########################################################

        my $profilesinproductlanguageresolvedarrayref;
        my $profileitemsinproductlanguageresolvedarrayref;

        if ((!($installer::globals::is_copy_only_project)) && (!($installer::globals::languagepack)) && (!($installer::globals::helppack)))
        {
            installer::logger::print_message( "... creating profiles ...\n" );

            $profilesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($profilesinproductarrayref, $languagesarrayref);

            $profileitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($profileitemsinproductarrayref, $languagesarrayref);

            installer::scriptitems::changing_name_of_language_dependent_keys($profilesinproductlanguageresolvedarrayref);

            installer::scriptitems::changing_name_of_language_dependent_keys($profileitemsinproductlanguageresolvedarrayref);

            installer::scriptitems::replace_setup_variables($profileitemsinproductlanguageresolvedarrayref, $languagestringref, $allvariableshashref);

            # Note that patch_user_dir is not related to the killed
            # ancient (not MSP) "patch" thing, I think.
            if ( $installer::globals::patch_user_dir )
            {
                installer::scriptitems::replace_userdir_variable($profileitemsinproductlanguageresolvedarrayref, $allvariableshashref);
            }

            installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($profilesinproductlanguageresolvedarrayref, $dirsinproductarrayref);

            # Now the Profiles can be created

            installer::profiles::create_profiles($profilesinproductlanguageresolvedarrayref, $profileitemsinproductlanguageresolvedarrayref, $filesinproductlanguageresolvedarrayref, $languagestringref, $allvariableshashref);
        }

        my $folderitemsinproductlanguageresolvedarrayref;

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
                $directoriesforepmarrayref = installer::scriptitems::remove_not_required_spellcheckerlanguage_files($directoriesforepmarrayref);
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
            $alldirectoryhash = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref, $unixlinksinproductlanguageresolvedarrayref);
            $directoriesforepmarrayref = installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($dirsinproductlanguageresolvedarrayref, $alldirectoryhash);
            @$directoriesforepmarrayref = sort { $a->{"HostName"} cmp $b->{"HostName"} } @$directoriesforepmarrayref;

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
            $alldirectoryhash = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref, $unixlinksinproductlanguageresolvedarrayref);
            $directoriesforepmarrayref = installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($dirsinproductlanguageresolvedarrayref, $alldirectoryhash);
            @$directoriesforepmarrayref = sort { $a->{"HostName"} cmp $b->{"HostName"} } @$directoriesforepmarrayref;

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

        ####################################################
        # Writing log file before packages are packed
        ####################################################

        installer::logger::print_message( "... creating log file " . $loggingdir . $installer::globals::logfilename . "\n" );
        installer::files::save_file($loggingdir . $installer::globals::logfilename, \@installer::globals::logfileinfo);

        ####################################################
        # Creating directories
        ####################################################

        installer::download::set_download_filename($languagestringref, $allvariableshashref);

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
        @{$packages} = shuffle @{$packages}
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

            #################################
            # collecting items for package
            #################################

            my $filesinpackage = installer::packagelist::find_files_for_package($filesinproductlanguageresolvedarrayref, $onepackage);
            my $unixlinksinpackage = installer::packagelist::find_files_for_package($unixlinksinproductlanguageresolvedarrayref, $onepackage);
            my $linksinpackage = installer::packagelist::find_links_for_package($linksinproductlanguageresolvedarrayref, $onepackage);
            my $dirsinpackage = installer::packagelist::find_dirs_for_package($directoriesforepmarrayref, $onepackage);

            #############################################
            # copying the collectors for each package
            #############################################

            $filesinpackage = installer::converter::copy_collector($filesinpackage);
            $linksinpackage = installer::converter::copy_collector($linksinpackage);
            $unixlinksinpackage = installer::converter::copy_collector($unixlinksinpackage);
            $dirsinpackage = installer::converter::copy_collector($dirsinpackage);

            ###########################################
            # setting the root path for the packages
            ###########################################

            installer::scriptitems::add_rootpath_to_directories($dirsinpackage, $packagerootpath);
            installer::scriptitems::add_rootpath_to_files($filesinpackage, $packagerootpath);
            installer::scriptitems::add_rootpath_to_links($linksinpackage, $packagerootpath);
            installer::scriptitems::add_rootpath_to_files($unixlinksinpackage, $packagerootpath);

            ###############################################
            # nothing to do, if $filesinpackage is empty
            ###############################################

            if ( ! ( $#{$filesinpackage} > -1 ))
            {
                my $infoline = "\n\nNo file in package: $packagename \-\> Skipping\n\n";
                push(@installer::globals::logfileinfo, $infoline);
                next;   # next package, end of loop !
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

                # check if we have to create a debug info package
                my $dbg = 0;
                my $debugfilelist;
                if ( !($installer::globals::strip) )
                {
                    $debugfilelist = splitdbg_libraries($filesinpackage, $languagestringref);
                    if ( $#{$debugfilelist} > -1 )
                    {
                        $dbg = 1;
                    }
                }

                my $epmfilename = "epm_" . $onepackagename . ".lst";
                my $dbgepmfilename = "epm_" . $onepackagename . ".debug.lst";

                installer::logger::print_message( "... creating epm list file $epmfilename ... \n" );
                if ($dbg)
                {
                    installer::logger::print_message( "... creating epm list file $dbgepmfilename ... \n" );
                }

                my $completeepmfilename = $listfiledir . $installer::globals::separator . $epmfilename;
                my $completedbgepmfilename = $listfiledir . $installer::globals::separator . $dbgepmfilename;


                my @epmfile = ();
                my @dbgepmfile = ();

                my $epmheaderref = installer::epmfile::create_epm_header($allvariableshashref, $filesinproductlanguageresolvedarrayref, $languagesarrayref, $onepackage);
                installer::epmfile::adding_header_to_epm_file(\@epmfile, $epmheaderref);
                my $dbgepmheaderref;

                # adding directories, files and links into epm file

                installer::epmfile::put_directories_into_epmfile($dirsinpackage, \@epmfile, $allvariableshashref, $packagerootpath);
                installer::epmfile::put_files_into_epmfile($filesinpackage, \@epmfile );
                installer::epmfile::put_links_into_epmfile($linksinpackage, \@epmfile );
                installer::epmfile::put_unixlinks_into_epmfile($unixlinksinpackage, \@epmfile );

                if (!( $shellscriptsfilename eq "" )) { installer::epmfile::adding_shellscripts_to_epm_file(\@epmfile, $shellscriptsfilename, $packagerootpath, $allvariableshashref, $filesinpackage); }

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

                if ($dbg)
                {
                    $onepackage->{'packagename'} .= "-debuginfo";
                    $onepackage->{'description'} .= " (debug info)";
                    $dbgepmheaderref = installer::epmfile::create_epm_header($allvariableshashref, $filesinproductlanguageresolvedarrayref, $languagesarrayref, $onepackage);
                    installer::epmfile::adding_header_to_epm_file(\@dbgepmfile, $dbgepmheaderref);
                    installer::epmfile::put_directories_into_epmfile($dirsinpackage, \@dbgepmfile, $allvariableshashref, $packagerootpath);
                    installer::epmfile::put_files_into_dbgepmfile($debugfilelist, \@dbgepmfile );
                    installer::files::save_file($completedbgepmfilename ,\@dbgepmfile);
                }

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

                            # package the debug info if required

                            if ($dbg)
                            {
                                installer::epmfile::call_epm($epmexecutable, $completedbgepmfilename, $packagename . "-debuginfo", $includepatharrayref);
                                my $newdbgepmdir = installer::epmfile::prepare_packages($loggingdir, $packagename . "-debuginfo", $staticpath, $relocatablepath, $onepackage, $allvariableshashref, $debugfilelist, $languagestringref);
                                installer::epmfile::create_packages_without_epm($newdbgepmdir, $packagename . "-debuginfo", $includepatharrayref, $allvariableshashref, $languagestringref);
                                installer::epmfile::remove_temporary_epm_files($newdbgepmdir, $loggingdir, $packagename . "-debuginfo");
                                installer::epmfile::create_new_directory_structure($newdbgepmdir);
                            }
                            $installer::globals::postprocess_specialepm = 1;
                        }
                    }

                    else    # this is the standard epm (not relocatable) or ( nonlinux and nonsolaris )
                    {
                        installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "\$\$PRODUCTINSTALLLOCATION", $relocatablepath);
                        installer::files::save_file($completeepmfilename ,\@epmfile);   # Warning for pool, content of epm file is changed.

                        if ( $installer::globals::call_epm )
                        {
                            # ... now epm can be started, to create the installation sets

                            if ( $installer::globals::is_special_epm )
                            {
                                   installer::logger::print_message( "... starting patched epm ... \n" );
                            }
                            else
                            {
                                   installer::logger::print_message( "... starting unpatched epm ... \n" );
                            }

                            installer::epmfile::call_epm($epmexecutable, $completeepmfilename, $packagename, $includepatharrayref);
                            if ($dbg)
                            {
                                installer::epmfile::call_epm($epmexecutable, $completedbgepmfilename, $packagename . "-debuginfo", $includepatharrayref);
                            }

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

            # Adding license into installation set
            if ($installer::globals::addlicensefile) { installer::worker::put_scpactions_into_installset("."); }

            # Adding license file into setup
            if ( $allvariableshashref->{'PUT_LICENSE_INTO_SETUP'} ) { installer::worker::put_license_into_setup(".", $includepatharrayref); }

            # Creating installation set for Unix language packs, that are not part of multi lingual installation sets
            if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::languagepack::build_installer_for_languagepack($installer::globals::epmoutpath, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

            # Creating installation set for Unix help packs, that are not part of multi lingual installation sets
            if ( ( $installer::globals::helppack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::helppack::build_installer_for_helppack($installer::globals::epmoutpath, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

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

            # Adding license into installation set
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
            if (( $is_success ) && ( $create_download ) && ( $ENV{'ENABLE_DOWNLOADSETS'} ))
            {
                my $downloaddir = installer::download::create_download_sets($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref);
                installer::worker::analyze_and_save_logfile($loggingdir, $downloaddir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
            }
        }
        #################################################
        # Part 2b: The Windows platform
        #################################################

        #####################################################################
        # ... creating idt files ...
        # Only for Windows builds
        #####################################################################

        # saving file_info file for later analysis
        my $speciallogfilename = $loggingdir . "fileinfo_" . $installer::globals::product . "\.log";
        open my $log_fh, '>', $speciallogfilename
            or die "Could not open $speciallogfilename for writing: $!";
        print $log_fh Dumper($filesinproductlanguageresolvedarrayref);

    }   # end of iteration for one language group
}

sub cleanup_on_error {
    my $message = shift;

    # If an installation set is currently created, the directory name
    # is saved in $installer::globals::saveinstalldir.  If this
    # directory name contains "_inprogress", it has to be renamed to
    # "_witherror".
    if ( $installer::globals::saveinstalldir =~ /_inprogress/ ) {
        installer::systemactions::rename_string_in_directory($installer::globals::saveinstalldir, "_inprogress", "_witherror");
    }

    # Removing directories created in the output tree.
    installer::worker::clean_output_tree();

    $installer::globals::logfilename = $installer::globals::exitlog . $installer::globals::logfilename;

    my @log = (@installer::globals::logfileinfo, @installer::globals::globallogfileinfo);

    push(@log, "\n" . '*' x 65 . "\n");
    push(@log, $message);
    push(@log, '*' x 65 . "\n");

    installer::files::save_file($installer::globals::logfilename, \@log);

    print("ERROR, saved logfile $installer::globals::logfilename is:\n");
    open(my $log, "<", $installer::globals::logfilename);
    print ": $_" while (<$log>);
    print "\n";
    close($log);
}

1;
