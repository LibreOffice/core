#*************************************************************************
#
#   $RCSfile: make_installer.pl,v $
#
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

#################
# use
#################

use lib ("$ENV{SOLARENV}/bin/modules");

use Cwd;
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
use installer::javainstaller;
use installer::languagepack;
use installer::languages;
use installer::logger;
use installer::mail;
use installer::packagelist;
use installer::parameter;
use installer::pathanalyzer;
use installer::profiles;
use installer::regmerge;
use installer::scppatchsoname;
use installer::scpzipfiles;
use installer::scriptitems;
use installer::servicesfile;
use installer::setupscript;
use installer::sorter;
use installer::strip;
use installer::worker;
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
use installer::windows::java;
use installer::windows::media;
use installer::windows::msiglobal;
use installer::windows::patch;
use installer::windows::property;
use installer::windows::removefile;
use installer::windows::registry;
use installer::windows::selfreg;
use installer::windows::shortcut;
use installer::windows::upgrade;
use installer::ziplist;

#################################################
# Main program
#################################################

#################################################
# Part 1: The platform independent part
#################################################

#################################################
# Part 1a: The language independent part
#################################################

installer::logger::starttime();

#########################################
# Checking the environment and setting
# most important variables
#########################################

installer::logger::print_message( "... checking environment variables ...\n" );
my $environmentvariableshashref = installer::control::check_system_environment();

installer::environment::set_global_environment_variables($environmentvariableshashref);

#################################
# Check and output of parameter
#################################

installer::parameter::saveparameter();
installer::parameter::getparameter();

# debugging can start after function "getparameter"
if ( $installer::globals::debug ) { installer::logger::debuginfo("\nPart 1: The platform independent part\n"); }
if ( $installer::globals::debug ) { installer::logger::debuginfo("\nPart 1a: The language independent part\n"); }

installer::parameter::control_fundamental_parameter();
installer::parameter::setglobalvariables();
installer::parameter::control_required_parameter();
installer::parameter::set_childproductnames();

if (!($installer::globals::languages_defined_in_productlist)) { installer::languages::analyze_languagelist(); }
installer::parameter::outputparameter();

installer::control::check_updatepack();

$installer::globals::build = uc($installer::globals::build);    # using "SRC680" instead of "src680"

######################################
# Creating the log directory
######################################

my $loggingdir = installer::systemactions::create_directories("logging", "");
$loggingdir = $loggingdir . $installer::globals::separator;
$installer::globals::exitlog = $loggingdir;

my $installdir = "";
my $currentdir = "";
my $shipinstalldir = "";
my $current_install_number = "";

######################################
# Checking the package list
######################################

my $packages;

if ((!($installer::globals::iswindowsbuild)) && (!($installer::globals::is_copy_only_project)))
{
    $packages = installer::control::read_packagelist($installer::globals::packagelist);
    installer::control::check_packagelist($packages);
}

######################################
# Checking the system requirements
######################################

installer::logger::print_message( "... checking required files ...\n" );
installer::control::check_system_path();

my $pathvariableshashref = installer::environment::create_pathvariables($environmentvariableshashref);

###################################################
# Analyzing the settings and variables in zip.lst
###################################################

# if the ziplistname is not defined on the command line, it is defaulted

if ($installer::globals::use_default_ziplist) { installer::ziplist::set_ziplist_name($pathvariableshashref); }

installer::logger::globallog("zip list file: $installer::globals::ziplistname");

my $ziplistref = installer::files::read_file($installer::globals::ziplistname);

installer::logger::print_message( "... analyzing $installer::globals::ziplistname ... \n" );

my $productblockref = installer::ziplist::getproductblock($ziplistref, $installer::globals::product);       # product block from zip.lst
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "productblock.log" ,$productblockref); }

my $settingsblockref = installer::ziplist::getproductblock($productblockref, "Settings");       # settings block from zip.lst
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "settingsblock1.log" ,$settingsblockref); }

$settingsblockref = installer::ziplist::analyze_settings_block($settingsblockref);              # select data from settings block in zip.lst
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "settingsblock2.log" ,$settingsblockref); }

my $allsettingsarrayref = installer::ziplist::get_settings_from_ziplist($settingsblockref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allsettings1.log" ,$allsettingsarrayref); }

my $allvariablesarrayref = installer::ziplist::get_variables_from_ziplist($settingsblockref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allvariables1.log" ,$allvariablesarrayref); }

$allsettingsarrayref = installer::ziplist::remove_multiples_from_ziplist($allsettingsarrayref); # the settings from the zip.lst
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allsettings2.log" ,$allsettingsarrayref); }

$allvariablesarrayref = installer::ziplist::remove_multiples_from_ziplist($allvariablesarrayref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allvariables2.log" ,$allvariablesarrayref); }

installer::ziplist::replace_variables_in_ziplist_variables($allvariablesarrayref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allvariables2a.log" ,$allvariablesarrayref); }

my $allvariableshashref = installer::converter::convert_array_to_hash($allvariablesarrayref);   # the variables from the zip.lst
if ( $installer::globals::globallogging ) { installer::files::save_hash($loggingdir . "allvariables3.log", $allvariableshashref); }

installer::ziplist::set_default_productversion_if_required($allvariableshashref);
if ( $installer::globals::globallogging ) { installer::files::save_hash($loggingdir . "allvariables3a.log", $allvariableshashref); }

installer::ziplist::add_variables_to_allvariableshashref($allvariableshashref);
if ( $installer::globals::globallogging ) { installer::files::save_hash($loggingdir . "allvariables3b.log", $allvariableshashref); }

# checking, whether this is an opensource product

if (!($installer::globals::is_copy_only_project)) { installer::ziplist::set_manufacturer($allvariableshashref); }

##########################################################
# Getting the include path from the settings in zip list
##########################################################

my $includepathref = installer::ziplist::getinfofromziplist($allsettingsarrayref, "include");
if ( $$includepathref eq "" )
{
    installer::exiter::exit_program("ERROR: Definition for \"include\" not found in $installer::globals::ziplistname", "Main");
}

my $includepatharrayref = installer::converter::convert_stringlist_into_array($includepathref, ",");
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allpatharray1.log" ,$includepatharrayref); }

installer::ziplist::replace_all_variables_in_pathes($includepatharrayref, $pathvariableshashref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allpatharray2.log" ,$includepatharrayref); }

installer::ziplist::replace_minor_in_pathes($includepatharrayref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allpatharray3.log" ,$includepatharrayref); }

installer::ziplist::resolve_relative_pathes($includepatharrayref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allpatharray3b.log" ,$includepatharrayref); }

installer::ziplist::remove_ending_separator($includepatharrayref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allpatharray3c.log" ,$includepatharrayref); }

##############################################
# Analyzing languages in zip.lst if required
##############################################

# Now get info about the languages and the setup script (defined on command line or in product list)

if ($installer::globals::languages_defined_in_productlist) { installer::languages::get_info_about_languages($allsettingsarrayref); }

#####################################
# Windows requires the encoding list
#####################################

if ( $installer::globals::iswindowsbuild ) { installer::control::read_encodinglist($includepatharrayref); }

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
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "setupscript1.log" ,$setupscriptref); }

# Resolving %variables defined in the installation object

my $allscriptvariablesref = installer::setupscript::get_all_scriptvariables_from_installation_object($setupscriptref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "setupscriptvariables1.log" ,$allscriptvariablesref); }

installer::setupscript::add_lowercase_productname_setupscriptvariable($allscriptvariablesref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "setupscriptvariables2.log" ,$allscriptvariablesref); }

installer::setupscript::replace_all_setupscriptvariables_in_script($setupscriptref, $allscriptvariablesref);
if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "setupscript2.log" ,$setupscriptref); }

# Adding all variables defined in the installation object into the hash of all variables.
# This is needed if variables are defined in the installation object, but not in the zip list file.
# If there is a definition in the zip list file and in the installation object, the installation object is more important

installer::setupscript::add_installationobject_to_variables($allvariableshashref, $allscriptvariablesref);
if ( $installer::globals::globallogging ) { installer::files::save_hash($loggingdir . "allvariables4.log", $allvariableshashref); }

installer::logger::log_hashref($allvariableshashref);

installer::logger::print_message( "... analyzing directories ... \n" );

# Collect all directories in the script to get the destination dirs

my $dirsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Directory");
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productdirectories1.log", $dirsinproductarrayref); }

installer::scriptitems::resolve_all_directory_names($dirsinproductarrayref);
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productdirectories2.log", $dirsinproductarrayref); }

installer::logger::print_message( "... analyzing files ... \n" );

my $filesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "File");
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles1.log", $filesinproductarrayref); }

$filesinproductarrayref = installer::scriptitems::remove_delete_only_files_from_productlists($filesinproductarrayref);
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles2.log", $filesinproductarrayref); }

if ($installer::globals::product =~ /suite/i ) { $filesinproductarrayref = installer::scriptitems::remove_notinsuite_files_from_productlists($filesinproductarrayref); }
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles2aa.log", $filesinproductarrayref); }

if (! $installer::globals::languagepack)
{
    $filesinproductarrayref = installer::scriptitems::remove_Languagepacklibraries_from_Installset($filesinproductarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles2b.log", $filesinproductarrayref); }
}

if (! $installer::globals::patch)
{
    $filesinproductarrayref = installer::scriptitems::remove_patchonlyfiles_from_Installset($filesinproductarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles2c.log", $filesinproductarrayref); }
}

installer::logger::print_message( "... analyzing scpactions ... \n" );

my $scpactionsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "ScpAction");
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions1.log", $scpactionsinproductarrayref); }

# $scpactionsinproductarrayref = installer::scriptitems::remove_scpactions_without_name($scpactionsinproductarrayref);
# if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions2.log", $scpactionsinproductarrayref); }

installer::scriptitems::change_keys_of_scpactions($scpactionsinproductarrayref);
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions2.log", $scpactionsinproductarrayref); }

installer::logger::print_message( "... analyzing shortcuts ... \n" );

my $linksinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Shortcut");
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks1.log", $linksinproductarrayref); }

installer::logger::print_message( "... analyzing profile ... \n" );

my $profilesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Profile");
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profiles1.log", $profilesinproductarrayref); }

installer::logger::print_message( "... analyzing profileitems ... \n" );

my $profileitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "ProfileItem");
if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profileitems1.log", $profileitemsinproductarrayref); }

my $folderinproductarrayref;
my $folderitemsinproductarrayref;
my $registryitemsinproductarrayref;

if ( $installer::globals::iswindowsbuild )  # Windows specific items: Folder, FolderItem, RegistryItem
{
    installer::logger::print_message( "... analyzing folders ... \n" );

    $folderinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Folder");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folder1.log", $folderinproductarrayref); }

    installer::logger::print_message( "... analyzing folderitems ... \n" );

    $folderitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "FolderItem");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folderitems1.log", $folderitemsinproductarrayref); }

    installer::setupscript::add_predefined_folder($folderitemsinproductarrayref, $folderinproductarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folder1b.log", $folderinproductarrayref); }

    installer::logger::print_message( "... analyzing registryitems ... \n" );

    $registryitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "RegistryItem");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems1.log", $registryitemsinproductarrayref); }

    $registryitemsinproductarrayref = installer::scriptitems::remove_uninstall_regitems_from_script($registryitemsinproductarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems1b.log", $registryitemsinproductarrayref); }
}

my $modulesinproductarrayref;

if (!($installer::globals::is_copy_only_project))
{
    installer::logger::print_message( "... analyzing modules ... \n" );

    $modulesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Module");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "modules1.log", $modulesinproductarrayref); }

    installer::scriptitems::set_children_flag($modulesinproductarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "modules1b.log", $modulesinproductarrayref); }

    # Assigning the modules to the items

    installer::scriptitems::assigning_modules_to_items($modulesinproductarrayref, $filesinproductarrayref, "Files");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles3.log", $filesinproductarrayref); }
}

if ( $installer::globals::debug ) { installer::logger::debuginfo("\nEnd of part 1a: The language independent part\n"); }

# saving debug info, before staring part 1b
if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

#################################################
# Part 1b: The language dependent part
# (still platform independent)
#################################################

# Now starts the language dependent part, if more than one product is defined on the command line
# Example -l 01,33,49#81,82,86,88 defines two multilingual products

###############################################################################
# Beginning of language dependent part
# The for iterates over all products, separated by an # in the language list
###############################################################################

if ( $installer::globals::debug ) { installer::logger::debuginfo("\nPart 1b: The language dependent part\n"); }

for ( my $n = 0; $n <= $#installer::globals::languageproducts; $n++ )
{
    my $languagesarrayref = installer::languages::get_all_languages_for_one_product($installer::globals::languageproducts[$n], $allvariableshashref);
    if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "languages.log" ,$languagesarrayref); }

    if ( ! $installer::globals::is_unix_multi ) { $installer::globals::alllanguagesinproductarrayref = $languagesarrayref; }

    my $languagestringref = installer::languages::get_language_string($languagesarrayref);
    installer::logger::print_message( "------------------------------------\n" );
    installer::logger::print_message( "... languages $$languagestringref ... \n" );

    ############################################################
    # Special handling for Unix multi installation sets.
    # Creating one base office product with the first language
    # and for all other languages languagepacks.
    ############################################################

    my $isfirstrun = 0;
    my $islastrun = 0;

    if ( $installer::globals::is_unix_multi )
    {
        if ( $n == 0 )
        {
            @installer::globals::logfileinfo = ();  # new logfile array and new logfile name
            installer::logger::copy_globalinfo_into_logfile();
            $isfirstrun = 1;
        }
        else    # switching from office installation to language pack
        {
            my $infoline = "... creating language pack: $$languagestringref !\n";
            installer::logger::include_header_into_logfile("$infoline");
            installer::logger::print_message( "$infoline" );

            $packages = installer::control::read_packagelist($installer::globals::addpackagelist); # reading the addon packagelist (which should be the languagepack package list)
            installer::control::check_packagelist($packages);

            if ( $n == 1 )  # packing the language packs, after the office is created
            {
                $installer::globals::languagepack = 1;      # !!! Setting languagepack variable after finishing the first language
                $installer::globals::islanguagepackinunixmulti = 1;
            }
        }

        if ( $n == $#installer::globals::languageproducts )
        {
            $islastrun = 1;
        }
    }

    if ( ! $installer::globals::is_unix_multi ) { $islastrun = 1; }

    if ( $installer::globals::patch )
    {
        $installer::globals::addchildprojects = 0;  # no child projects for patches
        $installer::globals::addlicensefile = 0;    # no license files for patches
        $installer::globals::makedownload = 0;
        $installer::globals::makejds = 0;
    }

    if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::is_unix_multi ) )
    {
        $installer::globals::addchildprojects = 0;
        $installer::globals::addsystemintegration = 0;
        $installer::globals::makedownload = 0;
        $installer::globals::makejds = 0;
        $installer::globals::addlicensefile = 0;
    }

    $installer::globals::fontpackageexists{$$languagestringref} = 0;

    ############################################################
    # Beginning of language specific logging mechanism
    # Until now only global logging into default: logfile.txt
    ############################################################

    if ( ! $installer::globals::is_unix_multi )
    {
        @installer::globals::logfileinfo = ();  # new logfile array and new logfile name
        installer::logger::copy_globalinfo_into_logfile();
    }

    my $logminor = "";
    if ( $installer::globals::updatepack ) { $logminor = $installer::globals::lastminor; }
    else { $logminor = $installer::globals::minor; }

    my $loglanguagestring = $$languagestringref;
    if ( $installer::globals::is_unix_multi ) { $loglanguagestring = $installer::globals::unixmultipath; }

    $installer::globals::logfilename = "log_" . $installer::globals::build . "_" . $logminor . "_" . $loglanguagestring . ".log";

    if (( ! $installer::globals::is_unix_multi ) || ( $isfirstrun )) { $loggingdir = $loggingdir . $loglanguagestring . $installer::globals::separator; }

    installer::systemactions::create_directory($loggingdir);
    $installer::globals::exitlog = $loggingdir;

    ##############################################################
    # Determining the ship location, if this is an update pack
    ##############################################################

    if ( $installer::globals::updatepack )
    {
        if (( ! $installer::globals::is_unix_multi ) || ( $isfirstrun ))
        {
            $shipinstalldir = installer::control::determine_ship_directory($languagestringref);
        }
    }

    ##############################################
    # Setting global code variables for Windows
    ##############################################

    if (!($installer::globals::is_copy_only_project))
    {
        if ( $installer::globals::iswindowsbuild )
        {
            installer::windows::msiglobal::set_global_code_variables($languagesarrayref, $languagestringref, $allvariableshashref);
        }
    }

    ################################################
    # Resolving include paths (language dependent)
    ################################################

    $includepatharrayref_lang = installer::ziplist::replace_languages_in_pathes($includepatharrayref, $languagesarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allpatharray4.log" ,$includepatharrayref_lang); }

    # Now all include paths are evaluated.
    # All files in this include paths can be collected.

    my $allfilesinincludepatharrayref = installer::ziplist::collect_all_files_from_include_path($includepatharrayref_lang); # needed for include path logging
    if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "allfiles.log" ,$allfilesinincludepatharrayref); }

    #####################################
    # Language dependent directory part
    #####################################

    my $dirsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($dirsinproductarrayref, $languagesarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productdirectories3.log", $dirsinproductlanguageresolvedarrayref); }
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productdirectories2a.log", $dirsinproductarrayref); }

    # A new directory array is needed ($dirsinproductlanguageresolvedarrayref instead of $dirsinproductarrayref)
    # because $dirsinproductarrayref is needed in get_Destination_Directory_For_Item_From_Directorylist

    installer::scriptitems::changing_name_of_language_dependent_keys($dirsinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productdirectories4.log", $dirsinproductlanguageresolvedarrayref); }

    installer::scriptitems::checking_directories_with_corrupt_hostname($dirsinproductlanguageresolvedarrayref, $languagesarrayref);

    #####################################
    # files part, language dependent
    #####################################

    installer::logger::print_message( "... analyzing files ...\n" );

    my $filesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($filesinproductarrayref, $languagesarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles4.log", $filesinproductlanguageresolvedarrayref); }

    if ( ! $installer::globals::set_office_start_language )
    {
        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_office_start_language_files($filesinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles4a.log", $filesinproductlanguageresolvedarrayref); }
    }

    installer::scriptitems::changing_name_of_language_dependent_keys($filesinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles5.log", $filesinproductlanguageresolvedarrayref); }

    if ( $installer::globals::iswin and $^O =~ /MSWin/i ) { installer::converter::convert_slash_to_backslash($filesinproductlanguageresolvedarrayref); }
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles6.log", $filesinproductlanguageresolvedarrayref); }

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_non_existent_languages_in_productlists($filesinproductlanguageresolvedarrayref, $languagestringref, "Name", "file");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles7.log", $filesinproductlanguageresolvedarrayref); }

    installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($filesinproductlanguageresolvedarrayref, $dirsinproductarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles8.log", $filesinproductlanguageresolvedarrayref); }

    installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist($filesinproductlanguageresolvedarrayref, $includepatharrayref_lang, $dirsinproductlanguageresolvedarrayref, "Files");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles9.log", $filesinproductlanguageresolvedarrayref); }

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_Without_Sourcedirectory($filesinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles10.log", $filesinproductlanguageresolvedarrayref); }

    if ($installer::globals::product =~ /ada/i )
    {
        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_For_Ada_Products($filesinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles10a.log", $filesinproductlanguageresolvedarrayref); }
    }

    if ($installer::globals::languagepack)
    {
        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_For_Languagepacks($filesinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles10c.log", $filesinproductlanguageresolvedarrayref); }
    }

    if ( ! $installer::globals::islanguagepackinunixmulti )
    {
        $filesinproductlanguageresolvedarrayref = installer::scriptitems::add_License_Files_into_Installdir($filesinproductlanguageresolvedarrayref, $languagesarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles10b.log", $filesinproductlanguageresolvedarrayref); }
    }

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_onlyasialanguage_files_from_productlists($filesinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles10d.log", $filesinproductlanguageresolvedarrayref); }

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_onlywesternlanguage_files_from_productlists($filesinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles10e.log", $filesinproductlanguageresolvedarrayref); }

    installer::scriptitems::make_filename_language_specific($filesinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles10f.log", $filesinproductlanguageresolvedarrayref); }

    # print "... calculating checksums ...\n";
    # my $checksumfile = installer::worker::make_checksum_file($filesinproductlanguageresolvedarrayref, $includepatharrayref);
    # if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . $installer::globals::checksumfilename, $checksumfile); }

    ######################################################################################
    # Unzipping files with flag ARCHIVE and putting all included files into the file list
    ######################################################################################

    installer::logger::print_message( "... analyzing files with flag ARCHIVE ...\n" );

    my @additional_paths_from_zipfiles = ();

    $filesinproductlanguageresolvedarrayref = installer::archivefiles::resolving_archive_flag($filesinproductlanguageresolvedarrayref, \@additional_paths_from_zipfiles, $languagestringref, $loggingdir);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles11.log", $filesinproductlanguageresolvedarrayref); }
    if ( $installer::globals::globallogging ) { installer::files::save_file($loggingdir . "additional_paths.log" ,\@additional_paths_from_zipfiles); }

    # packed files sometimes contain a $ in their name: HighlightText$1.class. epm conflicts with such files.
    # Therefore these files are renamed for non-Windows builds at the moment:

    if (!( $installer::globals::iswindowsbuild ))
    {
        installer::scriptitems::rename_illegal_filenames($filesinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles12.log", $filesinproductlanguageresolvedarrayref); }
    }

    #####################################
    # Files with flag SCPZIP_REPLACE
    #####################################

    installer::logger::print_message( "... analyzing files with flag SCPZIP_REPLACE ...\n" );

    # Editing files with flag SCPZIP_REPLACE.

    installer::scpzipfiles::resolving_scpzip_replace_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles13.log", $filesinproductlanguageresolvedarrayref); }

    #####################################
    # Files with flag PATCH_SO_NAME
    #####################################

    installer::logger::print_message( "... analyzing files with flag PATCH_SO_NAME ...\n" );

    # Editing files with flag PATCH_SO_NAME.

    installer::scppatchsoname::resolving_patchsoname_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles13b.log", $filesinproductlanguageresolvedarrayref); }

    #####################################
    # Creating services.rdb
    #####################################

    if ( $allvariableshashref->{'SERVICESPROJEKT'} )
    {
        if (! $installer::globals::languagepack)
        {
            # ATTENTION: For creating the services.rdb it is necessary to execute the native file
            # "regcomp" or "regcomp.exe". Therefore this function can only be executed on the
            # corresponding platform.

            if ( $installer::globals::servicesrdb_can_be_created )
            {
                installer::logger::print_message( "... creating preregistered services.rdb ...\n" );

                installer::servicesfile::create_services_rdb($filesinproductlanguageresolvedarrayref, $includepatharrayref, $languagestringref);
                if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles14.log", $filesinproductlanguageresolvedarrayref); }
            }
        }
    }

    #####################################
    # Calls of regmerge
    #####################################

    if (!($installer::globals::is_copy_only_project))
    {
        if (! $installer::globals::languagepack)
        {
            installer::logger::print_message( "... merging files into registry database ...\n" );

            installer::regmerge::merge_registration_files($filesinproductlanguageresolvedarrayref, $includepatharrayref, $languagestringref);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles14b.log", $filesinproductlanguageresolvedarrayref); }
        }
    }

    ############################################
    # Collecting directories for epm list file
    ############################################

    installer::logger::print_message( "... analyzing all directories for this product ...\n" );

    # There are two ways for a directory to be included into the epm directory list:
    # 1. Looking for all destination paths in the files array
    # 2. Looking for directories with CREATE flag in the directory array
    # Advantage: Many pathes are hidden in zip files, they are not defined in the setup script.
    # It will be possible, that in the setup script only those directoies have to be defined,
    # that have a CREATE flag. All other directories are created, if they contain at least one file.

    my $directoriesforepmarrayref = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist1.log", $directoriesforepmarrayref); }

    installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($directoriesforepmarrayref, $dirsinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist2.log", $directoriesforepmarrayref); }

    installer::sorter::sorting_array_of_hashes($directoriesforepmarrayref, "HostName");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist3.log", $directoriesforepmarrayref); }

    #########################################################
    # language dependent scpactions part
    #########################################################

    my $scpactionsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($scpactionsinproductarrayref, $languagesarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions3.log", $scpactionsinproductlanguageresolvedarrayref); }

    installer::scriptitems::changing_name_of_language_dependent_keys($scpactionsinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions4.log", $scpactionsinproductlanguageresolvedarrayref); }

    installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist($scpactionsinproductlanguageresolvedarrayref, $includepatharrayref_lang, $dirsinproductlanguageresolvedarrayref, "ScpActions");
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions5.log", $scpactionsinproductlanguageresolvedarrayref); }

    # Editing scpactions with flag SCPZIP_REPLACE.

    installer::scpzipfiles::resolving_scpzip_replace_flag($scpactionsinproductlanguageresolvedarrayref, $allvariableshashref, "ScpAction", $languagestringref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions6.log", $scpactionsinproductlanguageresolvedarrayref); }

    #########################################################
    # language dependent links part
    #########################################################

    installer::logger::print_message( "... analyzing links ...\n" );

    my $linksinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($linksinproductarrayref, $languagesarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks2.log", $linksinproductlanguageresolvedarrayref); }

    installer::scriptitems::changing_name_of_language_dependent_keys($linksinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks3.log", $linksinproductlanguageresolvedarrayref); }

    installer::scriptitems::get_destination_file_path_for_links($linksinproductlanguageresolvedarrayref, $filesinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks4.log", $linksinproductlanguageresolvedarrayref); }

    installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($linksinproductlanguageresolvedarrayref, $dirsinproductarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks5.log", $linksinproductlanguageresolvedarrayref); }

    # Now taking all links that have no FileID but a ShortcutID, linking to another link

    installer::scriptitems::get_destination_link_path_for_links($linksinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks6.log", $linksinproductlanguageresolvedarrayref); }

    $linksinproductlanguageresolvedarrayref = installer::scriptitems::remove_workstation_only_items($linksinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks7.log", $linksinproductlanguageresolvedarrayref); }

    installer::scriptitems::resolve_links_with_flag_relative($linksinproductlanguageresolvedarrayref);
    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks8.log", $linksinproductlanguageresolvedarrayref); }

    #########################################################
    # language dependent part for profiles and profileitems
    #########################################################

    my $profilesinproductlanguageresolvedarrayref;
    my $profileitemsinproductlanguageresolvedarrayref;

    if ((!($installer::globals::is_copy_only_project)) && (!($installer::globals::product =~ /ada/i )))
    {
        installer::logger::print_message( "... creating profiles ...\n" );

        $profilesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($profilesinproductarrayref, $languagesarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profiles2.log", $profilesinproductlanguageresolvedarrayref); }

        $profileitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($profileitemsinproductarrayref, $languagesarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profileitems2.log", $profilesinproductlanguageresolvedarrayref); }

        installer::scriptitems::changing_name_of_language_dependent_keys($profilesinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profiles3.log", $profilesinproductlanguageresolvedarrayref); }

        installer::scriptitems::changing_name_of_language_dependent_keys($profileitemsinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profileitems3.log", $profileitemsinproductlanguageresolvedarrayref); }

        installer::scriptitems::replace_setup_variables($profileitemsinproductlanguageresolvedarrayref, $languagestringref, $allvariableshashref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profileitems4.log", $profileitemsinproductlanguageresolvedarrayref); }

        installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($profilesinproductlanguageresolvedarrayref, $dirsinproductarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "profiles4.log", $profilesinproductlanguageresolvedarrayref); }

        # Now the Profiles can be created

        installer::profiles::create_profiles($profilesinproductlanguageresolvedarrayref, $profileitemsinproductlanguageresolvedarrayref, $filesinproductlanguageresolvedarrayref, $languagestringref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles15.log", $filesinproductlanguageresolvedarrayref); }
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
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folder2.log", $folderinproductlanguageresolvedarrayref); }

        installer::scriptitems::changing_name_of_language_dependent_keys($folderinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folder3.log", $folderinproductlanguageresolvedarrayref); }

        #########################################################
        # language dependent part for folderitems
        #########################################################

        installer::logger::print_message( "... analyzing folderitems ...\n" );

        $folderitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($folderitemsinproductarrayref, $languagesarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folderitems2.log", $folderitemsinproductlanguageresolvedarrayref); }

        installer::scriptitems::changing_name_of_language_dependent_keys($folderitemsinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folderitems3.log", $folderitemsinproductlanguageresolvedarrayref); }

        #########################################################
        # language dependent part for registryitems
        #########################################################

        installer::logger::print_message( "... analyzing registryitems ...\n" );

        $registryitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($registryitemsinproductarrayref, $languagesarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems2.log", $registryitemsinproductlanguageresolvedarrayref); }

        installer::scriptitems::changing_name_of_language_dependent_keys($registryitemsinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems3.log", $registryitemsinproductlanguageresolvedarrayref); }
    }

    #########################################################
    # language dependent part for modules
    #########################################################

    my $modulesinproductlanguageresolvedarrayref;

    if (!($installer::globals::is_copy_only_project))
    {
        installer::logger::print_message( "... analyzing modules ...\n" );

        $modulesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($modulesinproductarrayref, $languagesarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "modules2.log", $modulesinproductlanguageresolvedarrayref); }

        installer::scriptitems::changing_name_of_language_dependent_keys($modulesinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "modules3.log", $modulesinproductlanguageresolvedarrayref); }
    }

    # Copy-only projects can now start to copy all items File and ScpAction
    if ( $installer::globals::is_copy_only_project ) { installer::copyproject::copy_project($filesinproductlanguageresolvedarrayref, $scpactionsinproductlanguageresolvedarrayref, $loggingdir, $languagestringref, $shipinstalldir, $allsettingsarrayref); }

    # Language pack projects can now start to select the required information
    if ( $installer::globals::languagepack )
    {
        $filesinproductlanguageresolvedarrayref = installer::languagepack::select_language_items($filesinproductlanguageresolvedarrayref, $languagesarrayref, "File");
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles16b.log", $filesinproductlanguageresolvedarrayref); }
        $scpactionsinproductlanguageresolvedarrayref = installer::languagepack::select_language_items($scpactionsinproductlanguageresolvedarrayref, $languagesarrayref, "ScpAction");
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions6b.log", $scpactionsinproductlanguageresolvedarrayref); }
        $linksinproductlanguageresolvedarrayref = installer::languagepack::select_language_items($linksinproductlanguageresolvedarrayref, $languagesarrayref, "Shortcut");
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks8b.log", $linksinproductlanguageresolvedarrayref); }
        @{$folderitemsinproductlanguageresolvedarrayref} = (); # no folderitems in languagepacks

        # Collecting the directories again, to include only the language specific directories
        $directoriesforepmarrayref = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist3alangpack.log", $directoriesforepmarrayref); }
        installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($directoriesforepmarrayref, $dirsinproductlanguageresolvedarrayref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist3blangpack.log", $directoriesforepmarrayref); }
        installer::sorter::sorting_array_of_hashes($directoriesforepmarrayref, "HostName");
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist3clangpack.log", $directoriesforepmarrayref); }

        if ( $installer::globals::iswindowsbuild )
        {
            $registryitemsinproductlanguageresolvedarrayref = installer::worker::select_langpack_items($registryitemsinproductlanguageresolvedarrayref, "RegistryItem");
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems3aa.log", $registryitemsinproductlanguageresolvedarrayref); }
        }

    }

    # Patch projects can now start to select the required information
    if (( $installer::globals::patch ) && (( $installer::globals::issolarispkgbuild ) || ( $installer::globals::iswindowsbuild )))
    {
        $filesinproductlanguageresolvedarrayref = installer::worker::select_patch_items($filesinproductlanguageresolvedarrayref, "File");
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles16patch.log", $filesinproductlanguageresolvedarrayref); }
        $scpactionsinproductlanguageresolvedarrayref = installer::worker::select_patch_items($scpactionsinproductlanguageresolvedarrayref, "ScpAction");
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productscpactions6patch.log", $scpactionsinproductlanguageresolvedarrayref); }
        $linksinproductlanguageresolvedarrayref = installer::worker::select_patch_items($linksinproductlanguageresolvedarrayref, "Shortcut");
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productlinks8patch.log", $linksinproductlanguageresolvedarrayref); }
        @{$folderitemsinproductlanguageresolvedarrayref} = (); # no folderitems in languagepacks

        if ( $installer::globals::iswindowsbuild )
        {
            $registryitemsinproductlanguageresolvedarrayref = installer::worker::select_patch_items_without_name($registryitemsinproductlanguageresolvedarrayref, "RegistryItem");
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems3a.log", $registryitemsinproductlanguageresolvedarrayref); }

            installer::worker::prepare_windows_patchfiles($filesinproductlanguageresolvedarrayref, $languagestringref);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles16bpatch.log", $filesinproductlanguageresolvedarrayref); }

            # For Windows patches, the directories can now be collected again
            $directoriesforepmarrayref = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist4_patch.log", $directoriesforepmarrayref); }

            installer::sorter::sorting_array_of_hashes($directoriesforepmarrayref, "HostName");
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforepmlist5_patch.log", $directoriesforepmarrayref); }
        }
    }

    #########################################################
    # Collecting all scp actions
    #########################################################

    installer::worker::collect_scpactions($scpactionsinproductlanguageresolvedarrayref);

    #########################################################
    # creating inf files for user system integration
    #########################################################

    if (( $installer::globals::iswindowsbuild ) && ( ! $installer::globals::patch ))    # Windows specific items: Folder, FolderItem, RegistryItem
    {
        installer::logger::print_message( "... creating inf files ...\n" );
        installer::worker::create_inf_file($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $folderinproductlanguageresolvedarrayref, $folderitemsinproductlanguageresolvedarrayref, $modulesinproductlanguageresolvedarrayref, $languagesarrayref, $languagestringref, $allvariableshashref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles16c.log", $filesinproductlanguageresolvedarrayref); }
    }

    if ( $installer::globals::debug ) { installer::logger::debuginfo("\nEnd of part 1b: The language dependent part\n"); }

    # saving debug info, before starting part 2
    if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

    #################################################
    # Part 2: The platform dependent part
    #################################################

    if ( $installer::globals::debug ) { installer::logger::debuginfo("\nPart 2: The platform dependent part\n"); }

    #################################################
    # Part 2a: All non-Windows platforms
    #################################################

    if ( $installer::globals::debug ) { installer::logger::debuginfo("\nPart 2a: All non-Windows platforms\n"); }

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

        if (( ! $installer::globals::is_unix_multi ) || ( $isfirstrun ))
        {
            $installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);
        }

        my $listfiledir = installer::systemactions::create_directories("listfile", $languagestringref);
        my $installlogdir = installer::systemactions::create_directory_next_to_directory($installdir, "log");
        # my $installchecksumdir = installer::systemactions::create_directory_next_to_directory($installdir, "checksum");

        ############################################################################
        # Investigating the different RPMs, Packages, ... that shall be created.
        # The module GIDs are defined in the input file
        # Only for non-Windows platforms
        ############################################################################

        installer::logger::print_message( "... analyzing package list ...\n" );

        if ( ! $installer::globals::languagepack )  # language pack has its own module structure
        {
            $packages = installer::packagelist::analyze_list($packages, $modulesinproductlanguageresolvedarrayref);

            installer::packagelist::remove_multiple_modules_packages($packages);
        }

        if ( $installer::globals::languagepack )    # language pack has its own module structure
        {
            $packages = installer::packagelist::analyze_list_languagepack($packages);
        }

        my $epmexecutable = "";
        my $found_epm = 0;

        # iterating over all packages

        for ( my $k = 0; $k <= $#{$packages}; $k++ )
        {
            my $onepackage = ${$packages}[$k];

            # checking, if this is a language pack or a project pack.
            # Creating language packs only, if $installer::globals::languagepack is set. Parameter: -languagepack

            if ( $installer::globals::languagepack ) { installer::languagepack::replace_languagestring_variable($onepackage, $languagestringref); }

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
                installer::packagelist::resolve_packagevariables(\$packagename, $allvariableshashref, 1);
            }

            my $linkaddon = "";
            $installer::globals::add_required_package = "";

            if ( $installer::globals::makelinuxlinkrpm )
            {
                my $oldpackagename = $packagename;
                $installer::globals::add_required_package = $oldpackagename;    # the link rpm requires the non-linked version
                if ( $installer::globals::languagepack ) { $packagename = $packagename . "_u"; }
                else { $packagename = $packagename . "u"; }
                my $savestring = $oldpackagename . "\t" . $packagename;
                push(@installer::globals::linkrpms, $savestring);
                $linkaddon = "_links";
            }

            ###########################################
            # Root path, can be defined as parameter
            ###########################################

            my $packagerootpath = "";

            if ($installer::globals::rootpath eq "")
            {
                $packagerootpath = $onepackage->{'destpath'};
                installer::packagelist::resolve_packagevariables(\$packagerootpath, $allvariableshashref, 1);
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
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "files1_" . $packagename . ".log", $filesinpackage); }
            my $linksinpackage = installer::converter::copy_collector($linksinproductlanguageresolvedarrayref);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "links1_" . $packagename . ".log", $linksinpackage); }
            my $dirsinpackage = installer::converter::copy_collector($directoriesforepmarrayref);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "dirs1_" . $packagename . ".log", $dirsinpackage); }

            ###########################################
            # setting the root path for the packages
            ###########################################

            installer::scriptitems::add_rootpath_to_directories($dirsinpackage, $packagerootpath);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "dirs2_" . $packagename . ".log", $dirsinpackage); }
            installer::scriptitems::add_rootpath_to_files($filesinpackage, $packagerootpath);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "files2_" . $packagename . ".log", $filesinpackage); }
            installer::scriptitems::add_rootpath_to_links($linksinpackage, $packagerootpath);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "links2_" . $packagename . ".log", $linksinpackage); }

            #################################
            # collecting items for package
            #################################

            $filesinpackage = installer::packagelist::find_files_for_package($filesinpackage, $onepackage);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "files3_" . $packagename . ".log", $filesinpackage); }
            $linksinpackage = installer::packagelist::find_links_for_package($linksinpackage, $filesinpackage);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "links3_" . $packagename . ".log", $linksinpackage); }
            $dirsinpackage = installer::packagelist::find_dirs_for_package($dirsinpackage, $filesinpackage, $linksinpackage, $onepackagename);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "dirs3_" . $packagename . ".log", $dirsinpackage); }

            ###############################################
            # nothing to do, if $filesinpackage is empty
            ###############################################

            if ( ! ( $#{$filesinpackage} > -1 ))
            {
                push(@installer::globals::emptypackages, $packagename);
                $infoline = "\n\nNo file in package: $packagename \-\> Skipping\n\n";
                push(@installer::globals::logfileinfo, $infoline);
                next;   # next package, end of loop !
            }

            ###############################################
            # Setting the font flag for language packs
            ###############################################

            if (( $installer::globals::languagepack ) && ( $packagename =~ /-fonts/ ))
            {
                $installer::globals::fontpackageexists{$$languagestringref} = 1;
                $infoline = "\nFont package for language $$languagestringref exists!\n\n";
                push(@installer::globals::logfileinfo, $infoline);
            }

            #################################################################
            # nothing to do for Linux patches, if no file has flag PATCH
            #################################################################

            # Linux Patch: The complete RPM has to be built, if one file in the RPM has the flag PATCH
            if (( $installer::globals::patch ) && ( $installer::globals::islinuxrpmbuild ))
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

            installer::strip::strip_libraries($filesinpackage, $languagestringref);
            if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . $packagename ."_files.log", $filesinpackage); }

            ###############################################################
            # Searching for files in $filesinpackage with flag LINUXLINK
            ###############################################################

            if (( $installer::globals::islinuxrpmbuild ) && ( ! $installer::globals::simple ))
            {
                # special handling for all RPMs in $installer::globals::linuxlinkrpms

                # if (( $installer::globals::linuxlinkrpms =~ /\b$onepackagename\b/ ) || ( $installer::globals::languagepack ))
                if ( $installer::globals::linuxlinkrpms =~ /\b$onepackagename\b/ )
                {
                    my $run = 0;

                    if (( $installer::globals::makelinuxlinkrpm ) && ( ! $run ))
                    {
                        $filesinpackage = \@installer::globals::linuxpatchfiles;
                        $linksinpackage = \@installer::globals::linuxlinks;
                        $installer::globals::makelinuxlinkrpm = 0;
                        if ( $installer::globals::patch ) { $installer::globals::call_epm = 1; }     # enabling packing again
                        $run = 1;

                        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "files3b_" . $packagename . ".log", $filesinpackage); }
                        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "links3b_" . $packagename . ".log", $linksinpackage); }
                        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "dirs3b_" . $packagename . ".log", $dirsinpackage); }
                    }

                    if (( ! $installer::globals::makelinuxlinkrpm ) && ( ! $run ))
                    {
                        $filesinpackage = installer::worker::prepare_linuxlinkfiles($filesinpackage);
                        $installer::globals::makelinuxlinkrpm = 1;
                        if ( $installer::globals::patch ) { $installer::globals::call_epm = 0; }     # no packing of core module in patch
                        $shellscriptsfilename = ""; # shell scripts only need to be included into the link rpm
                        $run = 1;

                        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "files3a_" . $packagename . ".log", $filesinpackage); }
                        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "links3a_" . $packagename . ".log", $linksinpackage); }
                        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "dirs3a_" . $packagename . ".log", $dirsinpackage); }
                    }
                }
            }

            ###########################################
            # Simple installation mechanism
            ###########################################

            if ( $installer::globals::simple ) { installer::worker::install_simple($onepackagename, $$languagestringref, $dirsinpackage, $filesinpackage, $linksinpackage); }

            ###########################################
            # Creating epm list file
            ###########################################

            if (! $installer::globals::simple)
            {
                # epm list file format:
                # type mode owner group destination source options
                # Example for a file: f 755 root sys /usr/bin/foo foo
                # Example for a directory: d 755 root sys /var/spool/foo -
                # Example for a link: l 000 root sys /usr/bin/linkname filename
                # The source field specifies the file to link to

                my $epmfilename = "epm_" . $installer::globals::product . "_" . $onepackagename . $linkaddon . "_" . $installer::globals::compiler . "_" . $installer::globals::build . "_" . $installer::globals::minor . "_" . $$languagestringref . ".lst";

                installer::logger::print_message( "... creating epm list file $epmfilename ... \n" );

                my $completeepmfilename = $listfiledir . $installer::globals::separator . $epmfilename;

                my @epmfile = ();

                my $epmheaderref = installer::epmfile::create_epm_header($allvariableshashref, $filesinproductlanguageresolvedarrayref, $languagesarrayref, $onepackage);
                installer::epmfile::adding_header_to_epm_file(\@epmfile, $epmheaderref);

                if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild ))
                {
                    $filesinpackage = installer::worker::analyze_patch_files($filesinpackage);
                    if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "files4_" . $packagename . ".log", $filesinpackage); }

                    if ( ! ( $#{$filesinpackage} > -1 ))
                    {
                        push(@installer::globals::emptypackages, $packagename);
                        $infoline = "\nNo file in package: $packagename \-\> Skipping\n";
                        push(@installer::globals::logfileinfo, $infoline);
                        next;   # next package, end of loop !
                    }
                }

                # adding directories, files and links into epm file

                installer::epmfile::put_directories_into_epmfile($dirsinpackage, \@epmfile );
                installer::epmfile::put_files_into_epmfile($filesinpackage, \@epmfile );
                installer::epmfile::put_links_into_epmfile($linksinpackage, \@epmfile );

                if ((!( $shellscriptsfilename eq "" )) && (!($installer::globals::iswindowsbuild))) { installer::epmfile::adding_shellscripts_to_epm_file(\@epmfile, $shellscriptsfilename, $packagerootpath, $allvariableshashref); }

                installer::files::save_file($completeepmfilename ,\@epmfile);

                # ... splitting the rootpath into a relocatable part and a static part, if possible

                my $staticpath = "";
                my $relocatablepath = "";
                installer::epmfile::analyze_rootpath($packagerootpath, \$staticpath, \$relocatablepath);

                # ... replacing the variable PRODUCTDIRECTORYNAME in the shellscriptfile by $staticpath

                installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "PRODUCTDIRECTORYNAME", $staticpath);
                installer::files::save_file($completeepmfilename ,\@epmfile);

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

                if (( $installer::globals::call_epm ) && ( ! $found_epm ))
                {
                    $epmexecutable = installer::epmfile::find_epm_on_system($includepatharrayref);
                    installer::epmfile::set_patch_state($epmexecutable);    # setting $installer::globals::is_special_epm
                    $found_epm = 1; # searching only once
                }

                if (( $installer::globals::is_special_epm ) && ( ($installer::globals::islinuxrpmbuild) || ($installer::globals::issolarispkgbuild) ))  # special handling only for Linux RPMs and Solaris Packages
                {
                    if ( $installer::globals::call_epm )    # only do something, if epm is really executed
                    {
                        # ... now epm can be started, to create the installation sets

                        installer::logger::print_message( "... starting patched epm ... \n" );

                        installer::epmfile::call_epm($epmexecutable, $completeepmfilename, $packagename);

                        my $newepmdir = installer::epmfile::prepare_packages($loggingdir, $packagename, $staticpath, $relocatablepath, $onepackage, $allvariableshashref, $filesinpackage, $languagestringref); # adding the line for Prefix / Basedir, include rpmdir

                        installer::epmfile::create_packages_without_epm($newepmdir, $packagename, $includepatharrayref);    # start to package

                        # finally removing all temporary files

                        installer::epmfile::remove_temporary_epm_files($newepmdir, $loggingdir, $packagename);

                        # Installation:
                        # Install: pkgadd -a myAdminfile -d ./SUNWso8m34.pkg
                        # Install: rpm -i --prefix=/opt/special --nodeps so8m35.rpm

                        # $installer::globals::subdir is only "RPMS" or "packages"
                        $installer::globals::subdir = installer::epmfile::create_new_directory_structure($newepmdir);
                        $installer::globals::postprocess_specialepm = 1;
                    }
                }
                else    # this is the standard epm (not relocatable) or ( nonlinux and nonsolaris )
                {
                    installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "\$\$PRODUCTINSTALLLOCATION", $relocatablepath);
                    installer::files::save_file($completeepmfilename ,\@epmfile);

                    if ( $installer::globals::call_epm )
                    {
                        # ... now epm can be started, to create the installation sets

                        installer::logger::print_message( "... starting unpatched epm ... \n" );

                        if ( $installer::globals::call_epm ) { installer::epmfile::call_epm($epmexecutable, $completeepmfilename, $packagename); }

                        if (($installer::globals::islinuxrpmbuild) || ($installer::globals::issolarispkgbuild))
                        {
                            $installer::globals::postprocess_standardepm = 1;
                        }
                    }
                }

                chdir($currentdir); # changing back into start directory

            }  # end of "if (! $installer::globals::simple)"

            if ( $installer::globals::makelinuxlinkrpm ) { $k--; }  # decreasing the counter to create the link rpm!

        }   # end of "for ( my $k = 0; $k <= $#{$allpackages}; $k++ )"

        ##############################################################
        # Post epm functionality, after the last package is packed
        ##############################################################

        if (( $installer::globals::postprocess_specialepm ) && ( $islastrun ))
        {
            installer::logger::include_header_into_logfile("Post EPM processes (Patched EPM):");

            chdir($installdir);

            # Copying the cde, kde and gnome packages into the installation set
            if ( $installer::globals::addsystemintegration ) { installer::epmfile::put_systemintegration_into_installset($installer::globals::subdir, $includepatharrayref, $allvariableshashref); }

            # Adding license and readme into installation set
            # if ($installer::globals::addlicensefile) { installer::epmfile::put_installsetfiles_into_installset($installer::globals::subdir); }
            if ($installer::globals::addlicensefile) { installer::worker::put_scpactions_into_installset("."); }

            # Adding child projects to installation dynamically
            if ($installer::globals::addchildprojects) { installer::epmfile::put_childprojects_into_installset($installer::globals::subdir, $allvariableshashref); }

            # Creating installation set for Unix language packs, that are not part of multi lingual installation sets
            if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::is_unix_multi ) ) { installer::languagepack::build_installer_for_languagepack($installer::globals::subdir, $allvariableshashref, $includepatharrayref, $languagesarrayref); }

            # Finalizing patch installation sets
            if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild )) { installer::epmfile::finalize_patch($installer::globals::subdir, $allvariableshashref); }
            if (( $installer::globals::patch ) && ( $installer::globals::islinuxrpmbuild )) { installer::epmfile::finalize_linux_patch($installer::globals::subdir, $allvariableshashref, $includepatharrayref); }

            # Copying the java installer into the installation set
            chdir($currentdir); # changing back into start directory
            if ( $installer::globals::addjavainstaller ) { installer::javainstaller::create_java_installer($installdir, $installer::globals::subdir, $languagestringref, $languagesarrayref, $allvariableshashref, $includepatharrayref, $modulesinproductarrayref); }
        }

        if (( $installer::globals::postprocess_standardepm ) && ( $islastrun ))
        {
            installer::logger::include_header_into_logfile("Post EPM processes (Standard EPM):");

            chdir($installdir);

            # determine the destination directory
            my $newepmdir = installer::epmfile::determine_installdir_ooo();

            # Copying the cde, kde and gnome packages into the installation set
            if ( $installer::globals::addsystemintegration ) { installer::epmfile::put_systemintegration_into_installset($newepmdir, $includepatharrayref, $allvariableshashref); }

            # Adding license and readme into installation set
            # if ($installer::globals::addlicensefile) { installer::epmfile::put_installsetfiles_into_installset($newepmdir); }
            if ($installer::globals::addlicensefile) { installer::worker::put_scpactions_into_installset("."); }

            # Creating installation set for Unix language packs, that are not part of multi lingual installation sets
            if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::is_unix_multi ) ) { installer::languagepack::build_installer_for_languagepack($newepmdir, $allvariableshashref, $includepatharrayref, $languagesarrayref); }

            chdir($currentdir); # changing back into start directory
        }

        #######################################################
        # Analyzing the log file
        #######################################################

        my $is_success = 0;
        my $finalinstalldir = "";

        if ( $islastrun )
        {
            installer::worker::clean_output_tree(); # removing directories created in the output tree
            ($is_success, $finalinstalldir) = installer::worker::analyze_and_save_logfile($loggingdir, $installdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
        }

        #######################################################
        # Creating download installation set
        #######################################################

        if (( $islastrun ) && ( $installer::globals::makedownload ))
        {
            my $create_download = 0;
            my $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "downloadname");
            if ( $$downloadname ne "" ) { $create_download = 1; }
            if (( $is_success ) && ( $create_download ))
            {
                $downloaddir = installer::download::create_download_sets($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref);
                installer::worker::analyze_and_save_logfile($loggingdir, $downloaddir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
            }
        }

        #######################################################
        # Creating jds installation set
        #######################################################

        if (( $islastrun ) && ( $installer::globals::makejds ))
        {
            my $create_jds = 0;

            if ( $allvariableshashref->{'JDSBUILD'} ) { $create_jds = 1; }
            if (( ! $installer::globals::islinuxrpmbuild ) && ( ! $installer::globals::issolarispkgbuild )) { $create_jds = 0; }

            if (( $is_success ) && ( $create_jds ))
            {
                my $correct_language = installer::worker::check_jds_language($allvariableshashref, $languagestringref);

                if ( $correct_language )
                {
                    my $jdsdir = installer::worker::create_jds_sets($finalinstalldir, $allvariableshashref, $languagestringref, $languagesarrayref, $includepatharrayref);
                    installer::worker::analyze_and_save_logfile($loggingdir, $jdsdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
                }
            }
        }

    }   # end of "if (!( $installer::globals::iswindowsbuild ))"

    if ( $installer::globals::debug ) { installer::logger::debuginfo("\nEnd of part 2a: All non-Windows platforms\n"); }

    #################################################
    # Part 2b: The Windows platform
    #################################################

    if ( $installer::globals::debug ) { installer::logger::debuginfo("\nPart 2b: The Windows platform\n"); }

    #####################################################################
    # ... creating idt files ...
    # Only for Windows builds ($installer::globals::compiler is wntmsci)
    #####################################################################

    if ( $installer::globals::iswindowsbuild )
    {
        $installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);

         my $idtdirbase = installer::systemactions::create_directories("idt_files", $languagestringref);
         $installer::globals::infodirectory = installer::systemactions::create_directories("info_files", $languagestringref);
        my $installlogdir = installer::systemactions::create_directory_next_to_directory($installdir, "log");
        # my $installchecksumdir = installer::systemactions::create_directory_next_to_directory($installdir, "checksum");

        ############################################################################
        # Begin of functions that are used for the creation of idt files
        # This is only for Windows ($installer::globals::compiler, $installer::globals::iswindowsbuild)
        # The following tables of the msi-database have to be created dynamically:
        #  1. ActionTe.idt (localization)
        #  2. Componen.idt  (All components)
        #  3. Control.idt (localization and license text)
        #  4. Director.idt  (All directories)
        #  5. Error.idt (localization)
        #  6. Feature.idt  (All features)
        #  7. FeatureC.idt ! (Features <-> Components)
        #  8. File.idt  (All files)
        #  9. Font.idt
        # 10. Icon.idt
        # 11. Media.idt
        # 12. Property.idt (defining some properties)
        # 13. RadioBut.idt (localization)
        # 14. Registry.idt  Windows registry
        # 15. Shortcut.idt  (non advertised)
        # 16. Shortcut.idt  (advertised)
        # 17. CreateFo.idt  (empty directories)
        # 18. UIText.idt (localization)
        # 19. RemoveFi.idt   (removal of OfficeMenuFolder)
        # 10. Upgrade.idt
        # All other tables are static!
        ############################################################################

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

        installer::windows::file::create_files_table($filesinproductlanguageresolvedarrayref, \@allfilecomponents, $newidtdir);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles17.log", $filesinproductlanguageresolvedarrayref); }

        installer::windows::directory::create_directory_table($directoriesforepmarrayref, $newidtdir, $allvariableshashref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles18.log", $filesinproductlanguageresolvedarrayref); }
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "directoriesforidt1.log", $directoriesforepmarrayref); }

        # Attention: The table "Registry.idt" contains language specific strings -> parameter: $languagesarrayref !
        installer::windows::registry::create_registry_table($registryitemsinproductlanguageresolvedarrayref, \@allregistrycomponents, $newidtdir, $languagesarrayref, $allvariableshashref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems4.log", $registryitemsinproductlanguageresolvedarrayref); }

        installer::windows::component::create_component_table($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $directoriesforepmarrayref, \@allfilecomponents, \@allregistrycomponents, $newidtdir);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles19.log", $filesinproductlanguageresolvedarrayref); }
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "registryitems5.log", $registryitemsinproductlanguageresolvedarrayref); }

        # Advising language specific files and component to the corresponding feature for multilingual installation sets
        if (( $installer::globals::ismultilingual ) && ( ! $installer::globals::languagepack )) { installer::windows::feature::change_modules_in_filescollector($filesinproductlanguageresolvedarrayref); }
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles19b.log", $filesinproductlanguageresolvedarrayref); }

        # Attention: The table "Feature.idt" contains language specific strings -> parameter: $languagesarrayref !
        installer::windows::feature::create_feature_table($modulesinproductlanguageresolvedarrayref, $newidtdir, $languagesarrayref, $allvariableshashref);

        installer::windows::featurecomponent::create_featurecomponent_table($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $newidtdir);

        installer::windows::media::create_media_table($filesinproductlanguageresolvedarrayref, $newidtdir, $allvariableshashref);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "productfiles20.log", $filesinproductlanguageresolvedarrayref); }

        installer::windows::font::create_font_table($filesinproductlanguageresolvedarrayref, $newidtdir);

        # Attention: The table "Shortcut.idt" contains language specific strings -> parameter: $languagesarrayref !
        # Attention: Shortcuts (Folderitems) have icon files, that have to be copied into the Icon directory (last parameter)
        my @iconfilecollector = ();

        installer::windows::shortcut::create_shortcut_table($filesinproductlanguageresolvedarrayref, $linksinproductlanguageresolvedarrayref, $folderinproductlanguageresolvedarrayref, $folderitemsinproductlanguageresolvedarrayref, $directoriesforepmarrayref, $newidtdir, $languagesarrayref, $includepatharrayref, \@iconfilecollector);
        if ( $installer::globals::globallogging ) { installer::files::save_array_of_hashes($loggingdir . "folderitems4.log", $folderitemsinproductlanguageresolvedarrayref); }

        installer::windows::inifile::create_inifile_table($inifiletableentries, $filesinproductlanguageresolvedarrayref, $newidtdir);

        installer::windows::icon::create_icon_table(\@iconfilecollector, $newidtdir);    # creating the icon table with all iconfiles used as shortcuts (FolderItems)

        installer::windows::createfolder::create_createfolder_table($directoriesforepmarrayref, $filesinproductlanguageresolvedarrayref, $newidtdir, $allvariableshashref);

        if ( ! $installer::globals::languagepack )   # the following tables not for language packs
        {
            # installer::windows::removefile::create_removefile_table($folderitemsinproductlanguageresolvedarrayref, $newidtdir);

            installer::windows::selfreg::create_selfreg_table($filesinproductlanguageresolvedarrayref, $newidtdir);

            installer::windows::upgrade::create_upgrade_table($newidtdir, $allvariableshashref);

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

            my $languageidtdir = $idtdirbase . $installer::globals::separator . $onelanguage;
            if ( -d $languageidtdir ) { installer::systemactions::remove_complete_directory($languageidtdir, 1); }
            installer::systemactions::create_directory($languageidtdir);

            # Copy the template idt files and the new created idt files into this language directory

            installer::logger::print_message( "... copying idt files ...\n" );

            installer::logger::include_header_into_logfile("Copying idt files to $languageidtdir:");

            installer::windows::idtglobal::prepare_language_idt_directory($languageidtdir, $newidtdir, $onelanguage, $filesinproductlanguageresolvedarrayref, \@iconfilecollector, $binarytablefiles);

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

            # include the license text into the table Control.idt

            # my $licensefilesource = installer::windows::idtglobal::get_licensefilesource($onelanguage, $filesinproductlanguageresolvedarrayref);
            my $licensefilesource = installer::windows::idtglobal::get_rtflicensefilesource($onelanguage, $includepatharrayref_lang);
            my $licensefile = installer::files::read_file($licensefilesource);
            installer::scpzipfiles::replace_all_ziplistvariables_in_rtffile($licensefile, $allvariablesarrayref, $onelanguage, $loggingdir);
            my $controltablename = $languageidtdir . $installer::globals::separator . "Control.idt";
            my $controltable = installer::files::read_file($controltablename);
            installer::windows::idtglobal::add_licensefile_to_database($licensefile, $controltable);
            installer::files::save_file($controltablename, $controltable);

            $infoline = "Added licensefile $licensefilesource into database $controltablename\n";
            push(@installer::globals::logfileinfo, $infoline);

            # include the ProductCode and the UpgradeCode from codes-file into the Property.idt

            installer::windows::property::set_codes_in_property_table($languageidtdir);

            # the language specific properties can now be set in the Property.idt

            installer::windows::property::update_property_table($languageidtdir, $onelanguage, $allvariableshashref, $languagestringref);

            # adding language specific properties for multilingual installation sets

            installer::windows::property::set_languages_in_property_table($languageidtdir, $languagesarrayref);

            # adding the files from the binary directory into the binary table
            installer::windows::binary::update_binary_table($languageidtdir, $filesinproductlanguageresolvedarrayref, $binarytablefiles);

            # setting Java variables for Java products

            if ( $allvariableshashref->{'JAVAPRODUCT'} ) { installer::windows::java::update_java_tables($languageidtdir, $allvariableshashref); }

            # setting patch codes to detect installed products

            if (( $installer::globals::patch ) || ( $installer::globals::languagepack )) { installer::windows::patch::update_patch_tables($languageidtdir, $allvariableshashref); }

            # Adding Windows Installer CustomActions dynamically

            my $customactionidttablename = $languageidtdir . $installer::globals::separator . "CustomAc.idt";
            my $customactionidttable = installer::files::read_file($customactionidttablename);
            my $installexecutetablename = $languageidtdir . $installer::globals::separator . "InstallE.idt";
            my $installexecutetable = installer::files::read_file($installexecutetablename);
            my $installuitablename = $languageidtdir . $installer::globals::separator . "InstallU.idt";
            my $installuitable = installer::files::read_file($installuitablename);
            my $binarytablename = $languageidtdir . $installer::globals::separator . "Binary.idt";
            my $binarytable = installer::files::read_file($binarytablename);
            my $controleventtablename = $languageidtdir . $installer::globals::separator . "ControlE.idt";
            my $controleventtable = installer::files::read_file($controleventtablename);
            my $controlconditiontablename = $languageidtdir . $installer::globals::separator . "ControlC.idt";
            my $controlconditiontable = installer::files::read_file($controlconditiontablename);

            # The following addition of Custom Actions has to be done by scp as soon as old setup is removed

            # adding the custom action for the quickstarter into the product (CustomAc.idt and InstallE.idt)
            # $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "ExecuteQuickstart", "82", "install_quickstart.exe", "", 0, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            # if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "install_quickstart.exe", "ExecuteQuickstart", "\&FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the reg4msdoc in uisequence table into the product (CustomAc.idt and InstallU.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Regmsdocmsidll1", "65", "reg4msdocmsi.dll", "InstallUiSequenceEntry", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction )
            {
                # conneting the custom action to a control in the controlevent table
                installer::windows::idtglobal::connect_custom_action_to_control($controleventtable, $controleventtablename, "SetupType", "Next", "DoAction", "Regmsdocmsidll1", "_IsSetupTypeMin = \"Typical\"", "1");
                installer::windows::idtglobal::connect_custom_action_to_control($controleventtable, $controleventtablename, "CustomSetup", "Next", "DoAction", "Regmsdocmsidll1", "1", "1");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "FileTypeDialog", "CheckBox1", "Disable", "(\!gm_p_Wrt_Bin=2 And \&gm_p_Wrt_Bin=-1) Or (\!gm_p_Wrt_Bin=3 And \&gm_p_Wrt_Bin=2)");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "FileTypeDialog", "CheckBox2", "Disable", "(\!gm_p_Calc_Bin=2 And \&gm_p_Calc_Bin=-1) Or (\!gm_p_Calc_Bin=3 And \&gm_p_Calc_Bin=2)");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "FileTypeDialog", "CheckBox3", "Disable", "(\!gm_p_Impress_Bin=2 And \&gm_p_Impress_Bin=-1) Or (\!gm_p_Impress_Bin=3 And \&gm_p_Impress_Bin=2)");
            }

            # adding the custom action for the reg4msdoc in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Regmsdocmsidll2", "65", "reg4msdocmsi.dll", "InstallExecSequenceEntry", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "reg4msdocmsi.dll", "Regmsdocmsidll2", "Not REMOVE=\"ALL\" And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the reg4msdoc in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Regmsdocmsidll3", "65", "reg4msdocmsi.dll", "DeinstallExecSequenceEntry", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "reg4msdocmsi.dll",  "Regmsdocmsidll3", "REMOVE\<\>\"\" And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the pythonmsi in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Pythonmsidll1", "65", "pythonmsi.dll", "InstallExecSequenceEntry", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "pythonmsi.dll",  "Pythonmsidll1", "\&FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the pythonmsi in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Pythonmsidll2", "65", "pythonmsi.dll", "DeinstallExecSequenceEntry", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "pythonmsi.dll",  "Pythonmsidll2", "\&FEATURETEMPLATE=2 And \!FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the regactivex in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Regactivexdll1", "65", "regactivex.dll", "InstallActiveXControl", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "regactivex.dll", "Regactivexdll1", "\&FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the regactivex in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Regactivexdll2", "65", "regactivex.dll", "DeinstallActiveXControl", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "regactivex.dll",  "Regactivexdll2", "\&FEATURETEMPLATE=2 And \!FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the javafilter in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Jfregcadll1", "65", "jfregca.dll", "install_jf", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "jfregca.dll", "Jfregcadll1", "\&FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the javafilter in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Jfregcadll2", "65", "jfregca.dll", "uninstall_jf", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "jfregca.dll",  "Jfregcadll2", "\&FEATURETEMPLATE=2 And \!FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for shutting down the quickstarter in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "sdqsmsidll", "65", "sdqsmsi.dll", "ShutDownQuickstarter", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "sdqsmsi.dll", "sdqsmsidll", "REMOVE=\"ALL\" And Not PATCH", "InstallInitialize", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the removal of the startup folder link in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "qslnkmsidll", "65", "qslnkmsi.dll", "RemoveQuickstarterLink", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "qslnkmsi.dll",  "qslnkmsidll", "REMOVE=\"ALL\" And Not PATCH", "sdqsmsidll", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the winexplorerext in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Shellextensionsdll1", "65", "shlxtmsi.dll", "InstallExecSequenceEntry", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "shlxtmsi.dll", "Shellextensionsdll1", "\&FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for the winexplorerext in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Shellextensionsdll2", "65", "shlxtmsi.dll", "DeinstallExecSequenceEntry", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "shlxtmsi.dll",  "Shellextensionsdll2", "\&FEATURETEMPLATE=2 And \!FEATURETEMPLATE=3 And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for restarting the indexing service, necessary for the installation of ooofilt.dll in executesequence table into the product (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Instooofiltmsidll", "65", "instooofiltmsi.dll", "RestartIndexingService", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "instooofiltmsi.dll",  "Instooofiltmsidll", "Not REMOVE=\"ALL\" And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for adding the icon to the office folder in start menu (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Shellextensionsdll3", "65", "shlxtmsi.dll", "InstallStartmenuFolderIcon", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "shlxtmsi.dll", "Shellextensionsdll3", "Not REMOVE=\"ALL\" And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for removing the icon from the office folder in start menu (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Shellextensionsdll4", "65", "shlxtmsi.dll", "DeinstallStartmenuFolderIcon", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "shlxtmsi.dll",  "Shellextensionsdll4", "REMOVE=\"ALL\" And Not PATCH", "qslnkmsidll", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for setting the correct ALLUSERS value (CustomAc.idt and InstallE.idt and InstallU.idt )
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Shellextensionsdll5", "321", "shlxtmsi.dll", "SetProductInstallMode", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "shlxtmsi.dll",  "Shellextensionsdll5", "Not REMOVE=\"ALL\" And Not PATCH", "FindRelatedProducts", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installuitable, "shlxtmsi.dll", "Shellextensionsdll5", "Not REMOVE=\"ALL\" And Not PATCH", "FindRelatedProducts", $filesinproductlanguageresolvedarrayref, $installuitablename); }

            # adding the custom action for rebuilding the icon cache (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Shellextensionsdll6", "65", "shlxtmsi.dll", "RebuildShellIconCache", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "shlxtmsi.dll",  "Shellextensionsdll6", "Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action for starting an inf file in deinstallation process (CustomAc.idt and InstallE.idt)
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "Shellextensionsdll7", "65", "shlxtmsi.dll", "ExecutePostUninstallScript", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "shlxtmsi.dll",  "Shellextensionsdll7", "REMOVE=\"ALL\" And Not PATCH", "InstallValidate", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            # adding the custom action to remove old Windows registry (CustomAc.idt and InstallE.idt )
            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "RegCleanOld", "65", "regcleanold.dll", "CleanCurUserOldSystemRegistryFromSetup", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "regcleanold.dll",  "RegCleanOld", "Not REMOVE=\"ALL\" And Not PATCH And Not ALLUSERS=\"\"", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }

            if ( $installer::globals::tab )
            {
                # adding the tab custom action (CustomAc.idt and InstallE.idt)
                $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "InstallTab", "65", "tabaction.dll", "TabSetup", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
                if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "tabaction.dll", "InstallTab", "Not REMOVE=\"ALL\"  And Not PATCH", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }
            }

            if ( $installer::globals::patch )
            {
                # adding the patch custom action (CustomAc.idt and InstallE.idt (install and deinstall))
                $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "InstallExchangeFiles", "65", "patchmsi.dll", "InstallPatchedFiles", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
                if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "patchmsi.dll", "InstallExchangeFiles", "Not REMOVE=\"ALL\"", "end", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }
                $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "DeinstallExchangeFiles", "65", "patchmsi.dll", "UninstallPatchedFiles", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
                if ( $added_customaction ) { installer::windows::idtglobal::add_custom_action_to_install_table($installexecutetable, "patchmsi.dll", "DeinstallExchangeFiles", "REMOVE=\"ALL\"", "InstallValidate", $filesinproductlanguageresolvedarrayref, $installexecutetablename); }
                $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "SetUserInstallMode", "65", "patchmsi.dll", "GetUserInstallMode", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
                if ( $added_customaction )
                {
                    # conneting the custom action to a control in the controlevent table
                    installer::windows::idtglobal::connect_custom_action_to_control($controleventtable, $controleventtablename, "LicenseAgreement", "Next", "DoAction", "SetUserInstallMode", "1", "1");
                    installer::windows::idtglobal::connect_custom_action_to_control($controleventtable, $controleventtablename, "InstallChangeFolder", "OK", "DoAction", "SetUserInstallMode", "1", "4");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "Next", "Enable", "NOT INVALIDDIRECTORY And NOT PATCHISOLDER And NOT ISWRONGPRODUCT");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "Next", "Disable", "INVALIDDIRECTORY Or PATCHISOLDER Or ISWRONGPRODUCT");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelStartInstall", "Show", "NOT INVALIDDIRECTORY And NOT PATCHISOLDER And NOT ISWRONGPRODUCT");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelInvalidDir", "Show", "INVALIDDIRECTORY");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelPatchOlder", "Show", "PATCHISOLDER");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelWrongProduct", "Show", "ISWRONGPRODUCT");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelStartInstall2", "Show", "NOT INVALIDDIRECTORY And NOT PATCHISOLDER And NOT ISWRONGPRODUCT");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelInvalidDir2", "Show", "INVALIDDIRECTORY");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelPatchOlder2", "Show", "PATCHISOLDER");
                    installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelWrongProduct2", "Show", "ISWRONGPRODUCT");
                }
            }

            # custom actions for language packs

            $added_customaction = installer::windows::idtglobal::set_custom_action($customactionidttable, $binarytable, "SetUserInstallMode", "65", "lngpckinsthlp.dll", "GetUserInstallMode", 1, $filesinproductlanguageresolvedarrayref, $customactionidttablename);
            if ( $added_customaction )
            {
                # conneting the custom action to a control in the controlevent table
                installer::windows::idtglobal::connect_custom_action_to_control($controleventtable, $controleventtablename, "LicenseAgreement", "Next", "DoAction", "SetUserInstallMode", "1", "1");
                installer::windows::idtglobal::connect_custom_action_to_control($controleventtable, $controleventtablename, "InstallChangeFolder", "OK", "DoAction", "SetUserInstallMode", "1", "4");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "Next", "Enable", "NOT INVALIDDIRECTORY And NOT ISWRONGPRODUCT");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "Next", "Disable", "INVALIDDIRECTORY Or ISWRONGPRODUCT");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelStartInstall", "Show", "NOT INVALIDDIRECTORY And NOT ISWRONGPRODUCT");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelInvalidDir", "Show", "INVALIDDIRECTORY");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelWrongProduct", "Show", "ISWRONGPRODUCT");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelStartInstall2", "Show", "NOT INVALIDDIRECTORY And NOT ISWRONGPRODUCT");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelInvalidDir2", "Show", "INVALIDDIRECTORY");
                installer::windows::idtglobal::connect_condition_to_control($controlconditiontable, $controlconditiontablename, "DestinationFolder", "LabelWrongProduct2", "Show", "ISWRONGPRODUCT");
            }

            installer::files::save_file($customactionidttablename, $customactionidttable);
            installer::files::save_file($installexecutetablename, $installexecutetable);
            installer::files::save_file($installuitablename, $installuitable);
            installer::files::save_file($controleventtablename, $controleventtable);
            installer::files::save_file($controlconditiontablename, $controlconditiontable);

            # Adding child projects to installation dynamically (also in feature table)

            if ($installer::globals::addchildprojects)
            {
                my $customactiontablename = $languageidtdir . $installer::globals::separator . "CustomAc.idt";
                my $customactiontable = installer::files::read_file($customactiontablename);
                my $installuitablename = $languageidtdir . $installer::globals::separator . "InstallU.idt";
                my $installuitable = installer::files::read_file($installuitablename);
                my $featuretablename = $languageidtdir . $installer::globals::separator . "Feature.idt";
                my $featuretable = installer::files::read_file($featuretablename);
                my $directorytablename = $languageidtdir . $installer::globals::separator . "Director.idt";
                my $directorytable = installer::files::read_file($directorytablename);
                my $componenttablename = $languageidtdir . $installer::globals::separator . "Componen.idt";
                my $componenttable = installer::files::read_file($componenttablename);

                installer::windows::idtglobal::add_childprojects($customactiontable, $installuitable, $featuretable, $directorytable, $componenttable, $customactiontablename, $installuitablename, $featuretablename, $directorytablename, $componenttablename, $filesinproductlanguageresolvedarrayref, $allvariableshashref);

                installer::files::save_file($customactiontablename, $customactiontable);
                installer::files::save_file($installuitablename, $installuitable);
                installer::files::save_file($featuretablename, $featuretable);
                installer::files::save_file($directorytablename, $directorytable);
                installer::files::save_file($componenttablename, $componenttable);
            }

            # Then the language specific msi database can be created

            if ( $installer::globals::iswin )   # only possible on a Windows platform
            {
                my $msidatabasename = installer::windows::msiglobal::get_msidatabasename($allvariableshashref, $onelanguage);
                my $msifilename = $languageidtdir . $installer::globals::separator . $msidatabasename;

                installer::logger::print_message( "... creating msi database (language $onelanguage) ... \n" );

                installer::windows::msiglobal::set_uuid_into_component_table($languageidtdir);  # setting new GUID for the components using the tool uuidgen.exe
                installer::windows::msiglobal::create_msi_database($languageidtdir ,$msifilename);

                # validating the database   # ToDo

                my $languagefile = installer::files::read_file($installer::globals::idtlanguagepath . $installer::globals::separator . "SIS.mlf");
                # my $languagefile = installer::files::read_file($installer::globals::idtlanguagepath . $installer::globals::separator . "SIS.ulf");

                installer::windows::msiglobal::write_summary_into_msi_database($msifilename, $onelanguage, $languagefile, $allvariableshashref);

                # copy msi database into installation directory

                my $msidestfilename = $installdir . $installer::globals::separator . $msidatabasename;
                installer::systemactions::copy_one_file($msifilename, $msidestfilename);
            }
        }

        # Creating transforms, if the installation set has more than one language
        # renaming the msi database and generating the setup.ini file

        my $defaultlanguage = installer::languages::get_default_language($languagesarrayref);

        if ( $installer::globals::iswin )   # only possible on a Windows platform
        {
            if  ( $#{$languagesarrayref} > 0 )
            {
                installer::windows::msiglobal::create_transforms($languagesarrayref, $defaultlanguage, $installdir, $allvariableshashref);
            }

            installer::windows::msiglobal::rename_msi_database_in_installset($defaultlanguage, $installdir, $allvariableshashref);

            installer::logger::print_message( "... generating setup.ini ...\n" );

            installer::windows::msiglobal::create_setup_ini($languagesarrayref, $defaultlanguage, $installdir, $allvariableshashref);
        }

        # Analyzing the ScpActions and copying the files into the installation set
        # At least the loader, instmsia.exe and instmsiw.exe

        installer::logger::print_message( "... copying files into installation set ...\n" );

        # installer::windows::msiglobal::copy_scpactions_into_installset($defaultlanguage, $installdir, $scpactionsinproductlanguageresolvedarrayref);
        installer::worker::put_scpactions_into_installset($installdir);

        # ... copying the setup.exe, instmsia.exe and instmsiw.exe

        installer::windows::msiglobal::copy_windows_installer_files_into_installset($installdir, $includepatharrayref);

        # ... copying the child projects

        if ($installer::globals::addchildprojects)
        {
            installer::windows::msiglobal::copy_child_projects_into_installset($installdir, $allvariableshashref);
        }

        installer::logger::print_message( "... creating ddf files ...\n" );

        # Creating all needed ddf files and generating a list
        # for the package process containing all system calls

        my $ddfdir = installer::systemactions::create_directories("ddf", $languagestringref);

        $installer::globals::packjobref = installer::windows::msiglobal::generate_cab_file_list($filesinproductlanguageresolvedarrayref, $installdir, $ddfdir);

        # Update and patch reasons the pack order needs to be saved
        installer::windows::msiglobal::save_packorder($filesinproductlanguageresolvedarrayref);

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

        if ( $installer::globals::iswin )   # only possible on a Windows platform
        {
            installer::logger::print_message( "... packaging installation set ... \n" );
            installer::windows::msiglobal::execute_packaging($installer::globals::packjobref, $loggingdir);
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
        # Creating download installation set
        #######################################################

        my $create_download = 0;
        my $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "downloadname");
        if ( $installer::globals::languagepack ) { $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "langpackdownloadname"); }
        if ( $installer::globals::patch ) { $downloadname = installer::ziplist::getinfofromziplist($allsettingsarrayref, "patchdownloadname"); }

        if ( $$downloadname ne "" ) { $create_download = 1; }
        if (( $is_success ) && ( $create_download ))
        {
            $downloaddir = installer::download::create_download_sets($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref);
            installer::worker::analyze_and_save_logfile($loggingdir, $downloaddir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
        }

    }    # end of "if ( $installer::globals::iswindowsbuild )"

    if ( $installer::globals::debug ) { installer::logger::debuginfo("\nEnd of part 2b: The Windows platform\n"); }


}   # end of iteration for one language group

# saving debug info at end
if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

#######################################################
# Stopping time
#######################################################

installer::logger::stoptime();

####################################
# Main program end
####################################
