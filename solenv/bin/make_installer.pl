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
use installer::followme;
use installer::globals;
use installer::javainstaller;
use installer::languagepack;
use installer::languages;
use installer::logger;
use installer::mail;
use installer::packagelist;
use installer::packagepool;
use installer::parameter;
use installer::pathanalyzer;
use installer::profiles;
use installer::scppatchsoname;
use installer::scpzipfiles;
use installer::scriptitems;
use installer::setupscript;
use installer::simplepackage;
use installer::sorter;
use installer::strip;
use installer::substfilenamefiles;
use installer::upx;
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
use installer::windows::strip;
use installer::windows::upgrade;
use installer::worker;
use installer::xpdinstaller;
use installer::ziplist;

use strict;

sub GetSetupScriptLines ($$$)
{
    my ($allsettingsarrayref, $allvariableshashref, $includepatharrayref) = @_;

    if ($installer::globals::setupscript_defined_in_productlist)
    {
        installer::setupscript::set_setupscript_name($allsettingsarrayref, $includepatharrayref);
    }

    $installer::logger::Info->print( "... analyzing script: $installer::globals::setupscriptname ... \n" );
    installer::logger::globallog("setup script file: $installer::globals::setupscriptname");
    $installer::logger::Info->print( "... analyzing script: $installer::globals::setupscriptname ... \n" );

    # Reading the setup script file
    my $setupscriptref = installer::files::read_file($installer::globals::setupscriptname);

    # Resolving variables defined in the zip list file into setup
    # script.  All the variables are defined in $allvariablesarrayref
    installer::scpzipfiles::replace_all_ziplistvariables_in_file($setupscriptref, $allvariableshashref);

    # Resolving %variables defined in the installation object
    my $allscriptvariablesref = installer::setupscript::get_all_scriptvariables_from_installation_object(
        $setupscriptref,
        $installer::globals::setupscriptname);
    installer::setupscript::add_lowercase_productname_setupscriptvariable($allscriptvariablesref);
    installer::setupscript::resolve_lowercase_productname_setupscriptvariable($allscriptvariablesref);

    $setupscriptref = installer::setupscript::replace_all_setupscriptvariables_in_script(
        $setupscriptref,
        $allscriptvariablesref);

    # Adding all variables defined in the installation object into the
    # hash of all variables.  This is needed if variables are defined
    # in the installation object, but not in the zip list file.  If
    # there is a definition in the zip list file and in the
    # installation object, the installation object is more important
    installer::setupscript::add_installationobject_to_variables($allvariableshashref, $allscriptvariablesref);

    # Adding also all variables, that must be included into the $allvariableshashref.
    installer::setupscript::add_forced_properties($allvariableshashref);

    # Replacing preset properties, not using the default mechanisms (for example for UNIXPRODUCTNAME)
    installer::setupscript::replace_preset_properties($allvariableshashref);

    # We did this already.  Can this or the other one be removed.
    installer::scpzipfiles::replace_all_ziplistvariables_in_file($setupscriptref, $allvariableshashref);

    return $setupscriptref;
}

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
$installer::logger::Global->add_timestamp("starting logging");

# While there is no language set and logger::Lang is not yet tied to a log file,
# forward its output to logger::Global.
$installer::logger::Lang->set_forward($installer::logger::Global);

#########################################
# Checking the environment and setting
# most important variables
#########################################

$installer::logger::Info->print( "... checking environment variables ...\n" );
my $environmentvariableshashref = installer::control::check_system_environment();

installer::environment::set_global_environment_variables($environmentvariableshashref);

#################################
# Check and output of parameter
#################################

installer::parameter::saveparameter();
installer::parameter::getparameter();

# debugging can start after function "getparameter"

installer::parameter::control_fundamental_parameter();
installer::parameter::setglobalvariables();
installer::parameter::control_required_parameter();

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
my $currentdir = cwd();
my $shipinstalldir = "";
my $current_install_number = "";

######################################
# Checking the system requirements
######################################

$installer::logger::Info->print( "... checking required files ...\n" );
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

$installer::logger::Info->print( "... analyzing $installer::globals::ziplistname ... \n" );

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

installer::ziplist::overwrite_ooovendor( $allvariableshashref );

########################################################
# Check if this is simple packaging mechanism
########################################################

installer::simplepackage::check_simple_packager_project($allvariableshashref);

####################################################################
# setting global variables
####################################################################

installer::control::set_addchildprojects($allvariableshashref);
installer::control::set_addjavainstaller($allvariableshashref);
installer::control::set_addsystemintegration($allvariableshashref);

########################################################
# Re-define logging dir, after all variables are set
########################################################

my $oldloggingdir = $loggingdir;
installer::systemactions::remove_empty_directory($oldloggingdir);
$loggingdir = installer::systemactions::create_directories("logging", "");
$loggingdir = $loggingdir . $installer::globals::separator;
$installer::globals::exitlog = $loggingdir;

# checking, whether this is an opensource product

if (!($installer::globals::is_copy_only_project)) { installer::ziplist::set_manufacturer($allvariableshashref); }

##############################################
# Checking version of makecab.exe
##############################################

if ( $installer::globals::iswindowsbuild ) { installer::control::check_makecab_version(); }

##########################################################
# Getting the include path from the settings in zip list
##########################################################

my $includepathref = installer::ziplist::getinfofromziplist($allsettingsarrayref, "include");
if ( $$includepathref eq "" )
{
    installer::exiter::exit_program("ERROR: Definition for \"include\" not found in $installer::globals::ziplistname", "Main");
}

my $includepatharrayref = installer::converter::convert_stringlist_into_array($includepathref, ",");

installer::ziplist::replace_all_variables_in_pathes($includepatharrayref, $pathvariableshashref);

installer::ziplist::replace_minor_in_pathes($includepatharrayref);

installer::ziplist::replace_packagetype_in_pathes($includepatharrayref);

installer::ziplist::resolve_relative_pathes($includepatharrayref);

installer::ziplist::remove_ending_separator($includepatharrayref);

##############################################
# Collecting all files from all include
# pathes in global hashes.
##############################################

installer::worker::collect_all_files_from_includepathes($includepatharrayref);

##############################################
# Analyzing languages in zip.lst if required
# Probably no longer used.
##############################################

if ($installer::globals::languages_defined_in_productlist) { installer::languages::get_info_about_languages($allsettingsarrayref); }

#####################################
# Windows requires the encoding list
#####################################

if ( $installer::globals::iswindowsbuild ) { installer::control::read_encodinglist($includepatharrayref); }

####################################################################
# MacOS dmg build requires special DS_Store file to arrange icons
####################################################################
if (($installer::globals::ismacdmgbuild) && ($installer::globals::product =~ /OpenOffice_Dev/)) { $installer::globals::devsnapshotbuild = 1; }

#####################################################################
# Including additional inc files for variable settings, if defined
#####################################################################

if ( $allvariableshashref->{'ADD_INCLUDE_FILES'} ) { installer::worker::add_variables_from_inc_to_hashref($allvariableshashref, $includepatharrayref); }

################################################
# Disable xpd installer, if SOLAR_JAVA not set
################################################

installer::control::check_java_for_xpd($allvariableshashref);

#####################################
# Analyzing the setup script
#####################################

my $setupscriptref = GetSetupScriptLines($allsettingsarrayref, $allvariableshashref, $includepatharrayref);

installer::logger::log_hashref($allvariableshashref);

$installer::logger::Info->print( "... analyzing directories ... \n" );

# Collect all directories in the script to get the destination dirs

my $dirsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Directory");

if ( $installer::globals::languagepack ) { installer::scriptitems::use_langpack_hostname($dirsinproductarrayref); }
if ( $installer::globals::patch ) { installer::scriptitems::use_patch_hostname($dirsinproductarrayref); }

if ( $allvariableshashref->{'SHIFT_BASIS_INTO_BRAND_LAYER'} ) { $dirsinproductarrayref = installer::scriptitems::shift_basis_directory_parents($dirsinproductarrayref); }
if ( $allvariableshashref->{'OFFICEDIRECTORYNAME'} ) { installer::scriptitems::set_officedirectory_name($dirsinproductarrayref, $allvariableshashref->{'OFFICEDIRECTORYNAME'}); }


installer::scriptitems::resolve_all_directory_names($dirsinproductarrayref);

$installer::logger::Info->print( "... analyzing files ... \n" );

my $filesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "File");

$filesinproductarrayref = installer::scriptitems::remove_delete_only_files_from_productlists($filesinproductarrayref);

if (( ! $installer::globals::iswindowsbuild ) &&
    ( ! $installer::globals::islinuxrpmbuild ) &&
    ( ! $installer::globals::islinuxdebbuild ) &&
    ( ! $installer::globals::issolarispkgbuild ) &&
    ( $installer::globals::packageformat ne "installed" ) &&
    ( $installer::globals::packageformat ne "dmg" ) &&
    ( $installer::globals::packageformat ne "archive" ))
    { installer::control::check_oxtfiles($filesinproductarrayref); }

if ($installer::globals::product =~ /suite/i ) { $filesinproductarrayref = installer::scriptitems::remove_notinsuite_files_from_productlists($filesinproductarrayref); }

if (! $installer::globals::languagepack)
{
    $filesinproductarrayref = installer::scriptitems::remove_Languagepacklibraries_from_Installset($filesinproductarrayref);
}

if (! $installer::globals::patch)
{
    $filesinproductarrayref = installer::scriptitems::remove_patchonlyfiles_from_Installset($filesinproductarrayref);
}

if (! $installer::globals::tab)
{
    $filesinproductarrayref = installer::scriptitems::remove_tabonlyfiles_from_Installset($filesinproductarrayref);
}

if (( $installer::globals::packageformat ne "installed" ) && ( $installer::globals::packageformat ne "archive" ))
{
    $filesinproductarrayref = installer::scriptitems::remove_installedproductonlyfiles_from_Installset($filesinproductarrayref);
}

$installer::logger::Info->print( "... analyzing scpactions ... \n" );

my $scpactionsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "ScpAction");

if (( ! $allvariableshashref->{'XPDINSTALLER'} ) || ( ! $installer::globals::isxpdplatform ))
{
    $scpactionsinproductarrayref = installer::scriptitems::remove_Xpdonly_Items($scpactionsinproductarrayref);
}

if ( $installer::globals::languagepack ) { installer::scriptitems::use_langpack_copy_scpaction($scpactionsinproductarrayref); }
if ( $installer::globals::patch ) { installer::scriptitems::use_patch_copy_scpaction($scpactionsinproductarrayref); }
if (($installer::globals::devsnapshotbuild)) { installer::scriptitems::use_dev_copy_scpaction($scpactionsinproductarrayref); }

# $scpactionsinproductarrayref = installer::scriptitems::remove_scpactions_without_name($scpactionsinproductarrayref);

installer::scriptitems::change_keys_of_scpactions($scpactionsinproductarrayref);

$installer::logger::Info->print( "... analyzing shortcuts ... \n" );

my $linksinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Shortcut");

$installer::logger::Info->print( "... analyzing unix links ... \n" );

my $unixlinksinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Unixlink");

# $unixlinksinproductarrayref = installer::scriptitems::filter_layerlinks_from_unixlinks($unixlinksinproductarrayref);

$installer::logger::Info->print( "... analyzing profile ... \n" );

my $profilesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Profile");

$installer::logger::Info->print( "... analyzing profileitems ... \n" );

my $profileitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "ProfileItem");

my $folderinproductarrayref;
my $folderitemsinproductarrayref;
my $registryitemsinproductarrayref;
my $windowscustomactionsarrayref;

if ( $installer::globals::iswindowsbuild )  # Windows specific items: Folder, FolderItem, RegistryItem, WindowsCustomAction
{
    $installer::logger::Info->print( "... analyzing folders ... \n" );

    $folderinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Folder");

    $installer::logger::Info->print( "... analyzing folderitems ... \n" );

    $folderitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "FolderItem");

    installer::setupscript::add_predefined_folder($folderitemsinproductarrayref, $folderinproductarrayref);

    installer::setupscript::prepare_non_advertised_files($folderitemsinproductarrayref, $filesinproductarrayref);

    $installer::logger::Info->print( "... analyzing registryitems ... \n" );

    $registryitemsinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "RegistryItem");

    $registryitemsinproductarrayref = installer::scriptitems::remove_uninstall_regitems_from_script($registryitemsinproductarrayref);

    $installer::logger::Info->print( "... analyzing Windows custom actions ... \n" );

    $windowscustomactionsarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "WindowsCustomAction");
}

my $modulesinproductarrayref;

if (!($installer::globals::is_copy_only_project))
{
    $installer::logger::Info->print( "... analyzing modules ... \n" );

    $modulesinproductarrayref = installer::setupscript::get_all_items_from_script($setupscriptref, "Module");

    if (( ! $allvariableshashref->{'XPDINSTALLER'} ) || ( ! $installer::globals::isxpdplatform ))
    {
        $modulesinproductarrayref = installer::scriptitems::remove_Xpdonly_Items($modulesinproductarrayref);
    }

    installer::scriptitems::resolve_assigned_modules($modulesinproductarrayref);

    $modulesinproductarrayref = installer::scriptitems::remove_template_modules($modulesinproductarrayref);

    installer::scriptitems::set_children_flag($modulesinproductarrayref);

    installer::scriptitems::collect_all_languagemodules($modulesinproductarrayref);

    # Assigning the modules to the items

    installer::scriptitems::assigning_modules_to_items($modulesinproductarrayref, $filesinproductarrayref, "Files");

    installer::scriptitems::assigning_modules_to_items($modulesinproductarrayref, $unixlinksinproductarrayref, "Unixlinks");

    installer::scriptitems::assigning_modules_to_items($modulesinproductarrayref, $dirsinproductarrayref, "Dirs");
}


# saving debug info, before staring part 1b
if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

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


# Run the following code block exactly once.
# This strange version of a do{}while(false) loop exists only to allow (legacy) next statements.
for (;1;last)
{
    my $languagesarrayref = installer::languages::get_all_languages_for_one_product($installer::globals::languageproduct, $allvariableshashref);

    $installer::globals::alllanguagesinproductarrayref = $languagesarrayref;
    my $languagestringref = installer::languages::get_language_string($languagesarrayref);
    $installer::logger::Info->print( "------------------------------------\n" );
    $installer::logger::Info->print( "... languages $$languagestringref ... \n" );

    if ( $installer::globals::patch )
    {
        $installer::globals::addlicensefile = 0;    # no license files for patches
        $installer::globals::makedownload = 0;
        $installer::globals::makejds = 0;
    }

    if ( $installer::globals::languagepack )
    {
        $installer::globals::addchildprojects = 0;
        $installer::globals::addsystemintegration = 0;
        $installer::globals::makejds = 0;
        $installer::globals::addlicensefile = 0;

        if ( $allvariableshashref->{'OPENSOURCE'} ) { $installer::globals::makedownload = 1; }
        else { $installer::globals::makedownload = 0; }
    }

    ############################################################
    # Beginning of language specific logging mechanism
    # Until now only global logging into default: logfile.txt
    ############################################################

    installer::logger::copy_globalinfo_into_logfile();
    $installer::globals::globalinfo_copied = 1;

    my $logminor = "";
    if ( $installer::globals::updatepack ) { $logminor = $installer::globals::lastminor; }
    else { $logminor = $installer::globals::minor; }

    my $loglanguagestring = $$languagestringref;
    my $loglanguagestring_orig = $loglanguagestring;
    if (length($loglanguagestring) > $installer::globals::max_lang_length)
    {
        my $number_of_languages = installer::systemactions::get_number_of_langs($loglanguagestring);
        chomp(my $shorter = `echo $loglanguagestring | md5sum | sed -e "s/ .*//g"`);
        my $id = substr($shorter, 0, 8); # taking only the first 8 digits
        $loglanguagestring = "lang_" . $number_of_languages . "_id_" . $id;
    }

    # Setup the directory where the language dependent log file will be stored.
    $loggingdir = $loggingdir . $loglanguagestring . $installer::globals::separator;
    installer::systemactions::create_directory($loggingdir);

    # Set language dependent logging.
    $installer::globals::logfilename = sprintf("log_%s%s_%s.log",
        $installer::globals::build,
        $logminor ne "" ? "_" . $logminor : "",
        $loglanguagestring);
    $installer::logger::Lang->set_filename($loggingdir . $installer::globals::logfilename);
    $installer::logger::Lang->copy_lines_from($installer::logger::Global);
    $installer::logger::Lang->set_filter(\&installer::control::filter_log_error);
    installer::control::prepare_error_processing();
    # All logging to the console is also forwarded to the language dependen log.
    $installer::logger::Lang->set_forward(undef);
    $installer::logger::Info->set_forward($installer::logger::Lang);
    # Scan all log lines for error messages.
    $installer::logger::Lang->add_timestamp("starting log for language ".$loglanguagestring);

    if ($loglanguagestring ne $loglanguagestring_orig) {
        (my $dir = $loggingdir) =~ s!/$!!;
        open(my $F1, "> $dir.dir");
        open(my $F2, "> " . $loggingdir . $installer::globals::logfilename . '.file');
        my @s = map { "$_\n" } split('_', $loglanguagestring_orig);
        print $F1 @s;
        print $F2 @s;
    }

    $installer::globals::exitlog = $loggingdir;

    ##############################################################
    # Determining the ship location, if this is an update pack
    ##############################################################

    if ( $installer::globals::updatepack ) { $shipinstalldir = installer::control::determine_ship_directory($languagestringref); }

    ###################################################################
    # Reading an existing msi database, to prepare update and patch
    ###################################################################

    my $refdatabase = "";
    my $uniquefilename = "";
    my $revuniquefilename = "";
    my $revshortfilename = "";
    my $allupdatecomponentorder = "";
    my $shortdirname = "";
    my $componentid = "";
    my $componentidkeypath = "";
    my $alloldproperties = "";

    ##############################################
    # Setting global code variables for Windows
    ##############################################

    if (!($installer::globals::is_copy_only_project))
    {
        if (( $installer::globals::iswindowsbuild ) && ( $installer::globals::packageformat ne "archive" ) && ( $installer::globals::packageformat ne "installed" ))
        {
            installer::windows::msiglobal::set_global_code_variables($languagesarrayref, $languagestringref, $allvariableshashref, $alloldproperties);
        }
    }

    ################################################
    # Resolving include paths (language dependent)
    ################################################

    my $includepatharrayref_lang = installer::ziplist::replace_languages_in_pathes($includepatharrayref, $languagesarrayref);

    if ( $installer::globals::refresh_includepathes ) { installer::worker::collect_all_files_from_includepathes($includepatharrayref_lang); }

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

    $installer::logger::Info->print( "... analyzing files ...\n" );

    my $filesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($filesinproductarrayref, $languagesarrayref);

    if ( ! $installer::globals::set_office_start_language )
    {
        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_office_start_language_files($filesinproductlanguageresolvedarrayref);
    }

    installer::scriptitems::changing_name_of_language_dependent_keys($filesinproductlanguageresolvedarrayref);

    if ( $installer::globals::iswin and $^O =~ /MSWin/i ) { installer::converter::convert_slash_to_backslash($filesinproductlanguageresolvedarrayref); }

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_non_existent_languages_in_productlists($filesinproductlanguageresolvedarrayref, $languagestringref, "Name", "file");

    installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist($filesinproductlanguageresolvedarrayref, $includepatharrayref_lang, $dirsinproductlanguageresolvedarrayref, "Files");

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::add_bundled_extension_blobs( $filesinproductlanguageresolvedarrayref);
    ($filesinproductlanguageresolvedarrayref,$dirsinproductarrayref) = installer::scriptitems::add_bundled_prereg_extensions(
        $filesinproductlanguageresolvedarrayref, $dirsinproductarrayref);

    installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($filesinproductlanguageresolvedarrayref, $dirsinproductarrayref);

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_Without_Sourcedirectory($filesinproductlanguageresolvedarrayref);

    if ($installer::globals::languagepack)
    {
        $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_Files_For_Languagepacks($filesinproductlanguageresolvedarrayref);
    }


    if ( ! $allvariableshashref->{'NO_README_IN_ROOTDIR'} )
    {
        $filesinproductlanguageresolvedarrayref = installer::scriptitems::add_License_Files_into_Installdir($filesinproductlanguageresolvedarrayref, $dirsinproductlanguageresolvedarrayref, $languagesarrayref);
    }

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_onlyasialanguage_files_from_productlists($filesinproductlanguageresolvedarrayref);

    $filesinproductlanguageresolvedarrayref = installer::scriptitems::remove_onlywesternlanguage_files_from_productlists($filesinproductlanguageresolvedarrayref);

    installer::scriptitems::make_filename_language_specific($filesinproductlanguageresolvedarrayref);

    ######################################################################################
    # Unzipping files with flag ARCHIVE and putting all included files into the file list
    ######################################################################################

    $installer::logger::Info->print( "... analyzing files with flag ARCHIVE ...\n" );

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

    $installer::logger::Info->print( "... analyzing files with flag SUBST_FILENAME ...\n" );

    installer::substfilenamefiles::resolving_subst_filename_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, $languagestringref);

    #####################################
    # Files with flag SCPZIP_REPLACE
    #####################################

    $installer::logger::Info->print( "... analyzing files with flag SCPZIP_REPLACE ...\n" );

    # Editing files with flag SCPZIP_REPLACE.

    installer::scpzipfiles::resolving_scpzip_replace_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);

    #####################################
    # Files with flag PATCH_SO_NAME
    #####################################

    $installer::logger::Info->print( "... analyzing files with flag PATCH_SO_NAME ...\n" );

    # Editing files with flag PATCH_SO_NAME.

    installer::scppatchsoname::resolving_patchsoname_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);

    #####################################
    # Files with flag HIDDEN
    #####################################

    $installer::logger::Info->print( "... analyzing files with flag HIDDEN ...\n" );

    installer::worker::resolving_hidden_flag($filesinproductlanguageresolvedarrayref, $allvariableshashref, "File", $languagestringref);

    ############################################
    # Collecting directories for epm list file
    ############################################

    $installer::logger::Info->print( "... analyzing all directories for this product ...\n" );

    # There are two ways for a directory to be included into the epm directory list:
    # 1. Looking for all destination paths in the files array
    # 2. Looking for directories with CREATE flag in the directory array
    # Advantage: Many pathes are hidden in zip files, they are not defined in the setup script.
    # It will be possible, that in the setup script only those directoies have to be defined,
    # that have a CREATE flag. All other directories are created, if they contain at least one file.

    my ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_from_filesarray($filesinproductlanguageresolvedarrayref);

    ($directoriesforepmarrayref, $alldirectoryhash) = installer::scriptitems::collect_directories_with_create_flag_from_directoryarray($dirsinproductlanguageresolvedarrayref, $alldirectoryhash);

    # installer::sorter::sorting_array_of_hashes($directoriesforepmarrayref, "HostName");

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

    $installer::logger::Info->print( "... analyzing links ...\n" );

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

    $installer::logger::Info->print( "... analyzing unix links ...\n" );

    my $unixlinksinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($unixlinksinproductarrayref, $languagesarrayref);

    installer::scriptitems::changing_name_of_language_dependent_keys($unixlinksinproductlanguageresolvedarrayref);

    installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist($unixlinksinproductlanguageresolvedarrayref, $dirsinproductarrayref);

    #########################################################
    # language dependent part for profiles and profileitems
    #########################################################

    my $profilesinproductlanguageresolvedarrayref;
    my $profileitemsinproductlanguageresolvedarrayref;

    if ((!($installer::globals::is_copy_only_project)) && (!($installer::globals::product =~ /ada/i )) && (!($installer::globals::languagepack)))
    {
        $installer::logger::Info->print( "... creating profiles ...\n" );

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

        $installer::logger::Info->print( "... analyzing folder ...\n" );

        $folderinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($folderinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($folderinproductlanguageresolvedarrayref);

        #########################################################
        # language dependent part for folderitems
        #########################################################

        $installer::logger::Info->print( "... analyzing folderitems ...\n" );

        $folderitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($folderitemsinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($folderitemsinproductlanguageresolvedarrayref);

        #########################################################
        # language dependent part for registryitems
        #########################################################

        $installer::logger::Info->print( "... analyzing registryitems ...\n" );

        $registryitemsinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($registryitemsinproductarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($registryitemsinproductlanguageresolvedarrayref);
    }

    #########################################################
    # language dependent part for modules
    #########################################################

    my $modulesinproductlanguageresolvedarrayref;

    if (!($installer::globals::is_copy_only_project))
    {
        $installer::logger::Info->print( "... analyzing modules ...\n" );

        $modulesinproductlanguageresolvedarrayref = installer::scriptitems::resolving_all_languages_in_productlists($modulesinproductarrayref, $languagesarrayref);

        $modulesinproductlanguageresolvedarrayref = installer::scriptitems::remove_not_required_language_modules($modulesinproductlanguageresolvedarrayref, $languagesarrayref);

        installer::scriptitems::changing_name_of_language_dependent_keys($modulesinproductlanguageresolvedarrayref);

        # installer::scriptitems::collect_language_specific_names($modulesinproductlanguageresolvedarrayref);
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
        installer::sorter::sorting_array_of_hashes($directoriesforepmarrayref, "HostName");

        if ( $installer::globals::iswindowsbuild )
        {
            $registryitemsinproductlanguageresolvedarrayref = installer::worker::select_langpack_items($registryitemsinproductlanguageresolvedarrayref, "RegistryItem");
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

            installer::sorter::sorting_array_of_hashes($directoriesforepmarrayref, "HostName");
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
        $installer::logger::Info->print( "... creating inf files ...\n" );
        installer::worker::create_inf_file($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $folderinproductlanguageresolvedarrayref, $folderitemsinproductlanguageresolvedarrayref, $modulesinproductlanguageresolvedarrayref, $languagesarrayref, $languagestringref, $allvariableshashref);
    }

    ###########################################
    # Using upx, to decrease file size
    # Currently only for Windows.
    ###########################################

    if ( $allvariableshashref->{'UPXPRODUCT'} )
    {
        installer::upx::upx_on_libraries($filesinproductlanguageresolvedarrayref, $languagestringref);
    }

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

    $installer::logger::Info->print( "... analyzing package list ...\n" );

    my $packages = installer::packagelist::collectpackages($modulesinproductlanguageresolvedarrayref, $languagesarrayref);
    installer::packagelist::check_packagelist($packages);

    $packages = installer::packagelist::analyze_list($packages, $modulesinproductlanguageresolvedarrayref);
    installer::packagelist::remove_multiple_modules_packages($packages);

    # printing packages content:
    installer::packagelist::log_packages_content($packages);
    installer::packagelist::create_module_destination_hash($packages, $allvariableshashref);


    # saving debug info, before starting part 2
    if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

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
        # Creating directories
        ####################################################

        $installdir = installer::worker::create_installation_directory($shipinstalldir, $languagestringref, \$current_install_number);

        my $listfiledir = installer::systemactions::create_directories("listfile", $languagestringref);
        my $installlogdir = installer::systemactions::create_directory_next_to_directory($installdir, "log");

        ####################################################
        # Reading for Solaris all package descriptions
        # from file defined in property PACKAGEMAP
        ####################################################

        if (  $installer::globals::issolarisbuild ) { installer::epmfile::read_packagemap($allvariableshashref, $includepatharrayref, $languagesarrayref); }

        my $epmexecutable = "";
        my $found_epm = 0;

        # shuffle array to reduce parallel packaging process in pool
        installer::worker::shuffle_array($packages);

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
                installer::packagelist::resolve_packagevariables(\$packagename, $allvariableshashref, 0);
            }

            # Debian allows no underline in package name
            if ( $installer::globals::debian ) { $packagename =~ s/_/-/g; }

            # Debian allows no underline in package name
            if ( $installer::globals::debian ) { $packagename =~ s/_/-/g; }

            my $linkaddon = "";
            my $linkpackage = 0;
            $installer::globals::add_required_package = "";
            $installer::globals::linuxlinkrpmprocess = 0;

            if ( $installer::globals::makelinuxlinkrpm )
            {
                my $oldpackagename = $packagename;
                $installer::globals::add_required_package = $oldpackagename;    # the link rpm requires the non-linked version
                if ( $installer::globals::languagepack ) { $packagename = $packagename . "_u"; }
                else { $packagename = $packagename . "u"; }
                my $savestring = $oldpackagename . "\t" . $packagename;
                push(@installer::globals::linkrpms, $savestring);
                $linkaddon = "_links";
                $installer::globals::linuxlinkrpmprocess = 1;
                $linkpackage = 1;
            }

            ####################################################
            # Header for this package into log file
            ####################################################

            installer::logger::include_header_into_logfile("Creating package: $packagename ($k)");

            ####################################################
            # Pool check: If package is created at the moment
            # try it again later.
            ####################################################

            if (( $installer::globals::patch ) ||
                ( $installer::globals::languagepack ) ||
                ( $installer::globals::packageformat eq "native" ) ||
                ( $installer::globals::packageformat eq "portable" ) ||
                ( $installer::globals::packageformat eq "osx" )) { $allvariableshashref->{'POOLPRODUCT'} = 0; }

            if ( $allvariableshashref->{'POOLPRODUCT'} )
            {
                if ( ! $installer::globals::sessionidset ) { installer::packagepool::set_sessionid(); }
                if ( ! $installer::globals::poolpathset ) { installer::packagepool::set_pool_path(); }
                if (( ! $installer::globals::getuidpathset ) && ( $installer::globals::issolarisbuild )) { installer::worker::set_getuid_path($includepatharrayref); }

                my $package_is_creatable = installer::packagepool::check_package_availability($packagename);

                if (( ! $package_is_creatable ) && ( ! exists($installer::globals::poolshiftedpackages{$packagename}) ))
                {
                    splice(@{$packages}, $k, 1);    # removing package ...
                    push(@{$packages}, $onepackage);  # ... and adding it to the end
                    $installer::globals::poolshiftedpackages{$packagename} = 1; # only shifting each package once
                    $k--;                                                       # decreasing the counter
                    $installer::logger::Info->printf("Pool: Package \"%s\" cannot be created at the moment. Trying again later (1).\n", $packagename);
                    $installer::logger::Lang->printf("Pool: Package \"%s\" cannot be created at the moment. Trying again later (1).\n", $packagename);
                    next;                                                       # repeating this iteration with new package
                }
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
                push(@installer::globals::emptypackages, $packagename);
                $installer::logger::Lang->print("\n");
                $installer::logger::Lang->print("\n");
                $installer::logger::Lang->print("No file in package: %s \-\> Skipping\n\n", $packagename);
                next;   # next package, end of loop !
            }

            #################################################################
            # nothing to do for Linux patches, if no file has flag PATCH
            #################################################################

            # Linux Patch: The complete RPM has to be built, if one file in the RPM has the flag PATCH (also for DEBs)
            if (( $installer::globals::patch ) && (( $installer::globals::islinuxrpmbuild ) || ( $installer::globals::islinuxdebbuild )))
            {
                my $patchfiles = installer::worker::collect_all_items_with_special_flag($filesinpackage ,"PATCH");
                if ( ! ( $#{$patchfiles} > -1 ))
                {
                    $installer::logger::Lang->print("\n");
                    $installer::logger::Lang->print("\n");
                    $installer::logger::Lang->print("Linux Patch: No patch file in package: %s \-\> Skipping\n\n",
                        $packagename);
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
                installer::strip::strip_libraries($filesinpackage, $languagestringref);
            }

            ###############################################################
            # Searching for files in $filesinpackage with flag LINUXLINK
            ###############################################################

            if (( $installer::globals::islinuxbuild ) && ( ! $installer::globals::simple )) # for rpms and debian packages
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

                    }

                    if (( ! $installer::globals::makelinuxlinkrpm ) && ( ! $run ))
                    {
                        $filesinpackage = installer::worker::prepare_linuxlinkfiles($filesinpackage);
                        $linksinpackage = installer::worker::prepare_forced_linuxlinkfiles($linksinpackage);
                        $installer::globals::makelinuxlinkrpm = 1;
                        if ( $allvariableshashref->{'OPENSOURCE'} ) { $installer::globals::add_required_package = $packagename . "u"; }
                        if ( $installer::globals::patch ) { $installer::globals::call_epm = 0; }     # no packing of core module in patch
                        $shellscriptsfilename = ""; # shell scripts only need to be included into the link rpm
                        $run = 1;

                    }
                }
            }

            ###########################################
            # Simple installation mechanism
            ###########################################

            if ( $installer::globals::simple ) { installer::worker::install_simple($onepackagename, $$languagestringref, $dirsinpackage, $filesinpackage, $linksinpackage, $unixlinksinpackage); }

            ###########################################
            # Checking epm state
            ###########################################

            if (( $installer::globals::call_epm ) && ( ! $found_epm ))
            {
                $epmexecutable = installer::epmfile::find_epm_on_system($includepatharrayref);
                installer::epmfile::set_patch_state($epmexecutable);    # setting $installer::globals::is_special_epm
                $found_epm = 1; # searching only once
            }

            ###########################################
            # Creating epm list file
            ###########################################

            if ( ! $installer::globals::simple )
            {
                # epm list file format:
                # type mode owner group destination source options
                # Example for a file: f 755 root sys /usr/bin/foo foo
                # Example for a directory: d 755 root sys /var/spool/foo -
                # Example for a link: l 000 root sys /usr/bin/linkname filename
                # The source field specifies the file to link to

                my $epmfilename = "epm_" . $onepackagename . $linkaddon . ".lst";

                $installer::logger::Info->print( "... creating epm list file $epmfilename ... \n" );

                my $completeepmfilename = $listfiledir . $installer::globals::separator . $epmfilename;

                my @epmfile = ();

                my $epmheaderref = installer::epmfile::create_epm_header($allvariableshashref, $filesinproductlanguageresolvedarrayref, $languagesarrayref, $onepackage);
                installer::epmfile::adding_header_to_epm_file(\@epmfile, $epmheaderref);

                if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild ))
                {
                    $filesinpackage = installer::worker::analyze_patch_files($filesinpackage);

                    if ( ! ( $#{$filesinpackage} > -1 ))
                    {
                        push(@installer::globals::emptypackages, $packagename);
                        $installer::logger::Lang->print("\n");
                        $installer::logger::Lang->print("No file in package: %s \-\> Skipping\n", $packagename);
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
                # installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "BASISDIRECTORYVERSION", $allvariableshashref->{'OOOBASEVERSION'});
                installer::files::save_file($completeepmfilename ,\@epmfile);

                #######################################################
                # Now the complete content of the package is known,
                # including variables and shell scripts.
                # Create the package or using the package pool?
                #######################################################

                my $use_package_from_pool = 0;
                if ( $allvariableshashref->{'POOLPRODUCT'} ) { $use_package_from_pool = installer::packagepool::package_is_up_to_date($allvariableshashref, $onepackage, $packagename, \@epmfile, $filesinpackage, $installdir, $installer::globals::epmoutpath, $languagestringref); }

                if ( $use_package_from_pool == 3 ) # repeat this package later
                {
                    my $package_is_creatable = installer::packagepool::check_package_availability($packagename);

                    if (( ! $package_is_creatable ) && ( ! exists($installer::globals::poolshiftedpackages{$packagename}) ))
                    {
                        splice(@{$packages}, $k, 1);    # removing package ...
                        push(@{$packages}, $onepackage);  # ... and adding it to the end
                        $installer::globals::poolshiftedpackages{$packagename} = 1; # only shifting each package once
                        $k--;                                                       # decreasing the counter
                        $installer::logger::Info->print("\n");
                        $installer::logger::Info->print("Pool: Package \"%s\" cannot be created at the moment. Trying again later (2).\n", $packagename);
                        $installer::logger::Lang->print("\n");
                        $installer::logger::Lang->print("Pool: Package \"%s\" cannot be created at the moment. Trying again later (2).\n", $packagename);
                        next;                                                       # repeating this iteration with new package
                    }
                }

                if ( $use_package_from_pool == 4 ) # There was a problem with pooling. Repeat this package immediately.
                {
                        $k--;                                                       # decreasing the counter
                        $installer::logger::Info->print("\n");
                        $installer::logger::Info->print("Pool: Package \"%s\" had pooling problems. Repeating packaging immediately (3).\n", $packagename);
                        $installer::logger::Lang->print("\n");
                        $installer::logger::Lang->print("Pool: Package \"%s\" had pooling problems. Repeating packaging immediately (3).\n", $packagename);
                        next;                                                       # repeating this iteration
                }

                if ( $use_package_from_pool == 0 )
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

                    if (( $installer::globals::is_special_epm ) && ( ($installer::globals::islinuxrpmbuild) || ($installer::globals::issolarispkgbuild) ))  # special handling only for Linux RPMs and Solaris Packages
                    {
                        if ( $installer::globals::call_epm )    # only do something, if epm is really executed
                        {
                            # ... now epm can be started, to create the installation sets

                            $installer::logger::Info->print( "... starting patched epm ... \n" );

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
                            # if (( $installer::globals::patch ) && ( $installer::globals::issolarisx86build )) { installer::worker::fix2_solaris_x86_patch($packagename, $installer::globals::epmoutpath); }
                        }
                    }
                    else    # this is the standard epm (not relocatable) or ( nonlinux and nonsolaris )
                    {
                        installer::epmfile::resolve_path_in_epm_list_before_packaging(\@epmfile, $completeepmfilename, "\$\$PRODUCTINSTALLLOCATION", $relocatablepath);
                        installer::files::save_file($completeepmfilename ,\@epmfile);   # Warning for pool, content of epm file is changed.

                        if ( $installer::globals::call_epm )
                        {
                            # ... now epm can be started, to create the installation sets

                            $installer::logger::Info->print( "... starting unpatched epm ... \n" );

                            if ( $installer::globals::call_epm ) { installer::epmfile::call_epm($epmexecutable, $completeepmfilename, $packagename, $includepatharrayref); }

                            if (($installer::globals::islinuxrpmbuild) || ($installer::globals::issolarispkgbuild) || ($installer::globals::debian))
                            {
                                $installer::globals::postprocess_standardepm = 1;
                            }
                        }
                    }

                    if ( $allvariableshashref->{'POOLPRODUCT'} ) { installer::packagepool::put_content_into_pool($packagename, $installdir, $installer::globals::epmoutpath, $filesinpackage, \@epmfile); }

                    chdir($currentdir); # changing back into start directory

                } # end of "if ( ! $use_package_from_pool )

            } # end of "if ( ! $installer::globals::simple )

            ###########################################
            # xpd installation mechanism
            ###########################################

            # Creating the xpd file for the package. This has to happen always, not determined by $use_package_from_pool

            if ( $installer::globals::isxpdplatform )
            {
                if (( ! $installer::globals::languagepack ) && ( ! $installer::globals::patch ))
                {
                    if (( $allvariableshashref->{'XPDINSTALLER'} ) && ( $installer::globals::call_epm != 0 ))
                    {
                        installer::xpdinstaller::create_xpd_file($onepackage, $packages, $languagestringref, $allvariableshashref, $modulesinproductarrayref, $installdir, $installer::globals::epmoutpath, $linkpackage, \%installer::globals::xpdpackageinfo);
                        $installer::globals::xpd_files_prepared = 1;
                        %installer::globals::xpdpackageinfo = ();
                    }
                }
            }

            if ( $installer::globals::makelinuxlinkrpm ) { $k--; }  # decreasing the counter to create the link rpm!

        }   # end of "for ( my $k = 0; $k <= $#{$packages}; $k++ )"

        installer::packagepool::log_pool_statistics();

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
            # if ($installer::globals::addlicensefile) { installer::epmfile::put_installsetfiles_into_installset($installer::globals::epmoutpath); }
            if ($installer::globals::addlicensefile) { installer::worker::put_scpactions_into_installset("."); }

            # Adding child projects to installation dynamically
            if ($installer::globals::addchildprojects) { installer::epmfile::put_childprojects_into_installset($installer::globals::epmoutpath, $allvariableshashref, $modulesinproductarrayref, $includepatharrayref); }

            # Adding license file into setup
            if ( $allvariableshashref->{'PUT_LICENSE_INTO_SETUP'} ) { installer::worker::put_license_into_setup(".", $includepatharrayref); }

            # Creating installation set for Unix language packs, that are not part of multi lingual installation sets
            if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::languagepack::build_installer_for_languagepack($installer::globals::epmoutpath, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

            # Finalizing patch installation sets
            if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild )) { installer::epmfile::finalize_patch($installer::globals::epmoutpath, $allvariableshashref); }
            if (( $installer::globals::patch ) && ( $installer::globals::islinuxrpmbuild )) { installer::epmfile::finalize_linux_patch($installer::globals::epmoutpath, $allvariableshashref, $includepatharrayref); }

            # Copying the xpd installer into the installation set
            if (( $allvariableshashref->{'XPDINSTALLER'} ) && ( $installer::globals::isxpdplatform ) && ( $installer::globals::xpd_files_prepared ))
            {
                installer::xpdinstaller::create_xpd_installer($installdir, $allvariableshashref, $languagestringref);
                $installer::globals::addjavainstaller = 0;  # only one java installer possible
            }

            # Copying the java installer into the installation set
            chdir($currentdir); # changing back into start directory
            if ( $installer::globals::addjavainstaller ) { installer::javainstaller::create_java_installer($installdir, $installer::globals::epmoutpath, $languagestringref, $languagesarrayref, $allvariableshashref, $includepatharrayref, $modulesinproductarrayref); }
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
            # if ($installer::globals::addlicensefile) { installer::epmfile::put_installsetfiles_into_installset($newepmdir); }
            if ($installer::globals::addlicensefile) { installer::worker::put_scpactions_into_installset("."); }

            # Adding license file into setup
            if ( $allvariableshashref->{'PUT_LICENSE_INTO_SETUP'} ) { installer::worker::put_license_into_setup(".", $includepatharrayref); }

            # Creating installation set for Unix language packs, that are not part of multi lingual installation sets
            if ( ( $installer::globals::languagepack ) && ( ! $installer::globals::debian ) && ( ! $installer::globals::makedownload ) ) { installer::languagepack::build_installer_for_languagepack($newepmdir, $allvariableshashref, $includepatharrayref, $languagesarrayref, $languagestringref); }

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
        if ( $is_success ) { installer::followme::save_followme_info($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref, $current_install_number, $loggingdir, $installlogdir); }

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

        #######################################################
        # Creating jds installation set
        #######################################################

        if ( $installer::globals::makejds )
        {
            my $create_jds = 0;

            if ( $allvariableshashref->{'JDSBUILD'} ) { $create_jds = 1; }
            if (! $installer::globals::issolarispkgbuild ) { $create_jds = 0; }

            if (( $is_success ) && ( $create_jds ))
            {
                if ( ! $installer::globals::jds_language_controlled )
                {
                    my $correct_language = installer::worker::check_jds_language($allvariableshashref, $languagestringref);
                    $installer::globals::correct_jds_language = $correct_language;
                    $installer::globals::jds_language_controlled = 1;
                }

                if ( $installer::globals::correct_jds_language )
                {
                    my $jdsdir = installer::worker::create_jds_sets($finalinstalldir, $allvariableshashref, $languagestringref, $languagesarrayref, $includepatharrayref);
                    installer::worker::clean_jds_temp_dirs();
                    installer::worker::analyze_and_save_logfile($loggingdir, $jdsdir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
                }
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
        # my $installchecksumdir = installer::systemactions::create_directory_next_to_directory($installdir, "checksum");

        #################################################################################
        # Preparing cabinet files from package definitions
        #################################################################################

        # installer::packagelist::prepare_cabinet_files($packages, $allvariableshashref, $$languagestringref);
        installer::packagelist::prepare_cabinet_files($packages, $allvariableshashref);
        # printing packages content:
        installer::packagelist::log_cabinet_assignments();

        #################################################################################
        # Begin of functions that are used for the creation of idt files (Windows only)
        #################################################################################

        $installer::logger::Info->print( "... creating idt files ...\n" );

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

        $filesinproductlanguageresolvedarrayref = installer::windows::file::create_files_table($filesinproductlanguageresolvedarrayref, \@allfilecomponents, $newidtdir, $allvariableshashref, $uniquefilename);

        installer::windows::directory::create_directory_table($directoriesforepmarrayref, $newidtdir, $allvariableshashref, $shortdirname, $loggingdir);

        # Attention: The table "Registry.idt" contains language specific strings -> parameter: $languagesarrayref !
        installer::windows::registry::create_registry_table($registryitemsinproductlanguageresolvedarrayref, \@allregistrycomponents, $newidtdir, $languagesarrayref, $allvariableshashref);

        installer::windows::component::create_component_table($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $directoriesforepmarrayref, \@allfilecomponents, \@allregistrycomponents, $newidtdir, $componentid, $componentidkeypath, $allvariableshashref);

        # Attention: The table "Feature.idt" contains language specific strings -> parameter: $languagesarrayref !
        installer::windows::feature::add_uniquekey($modulesinproductlanguageresolvedarrayref);
        $modulesinproductlanguageresolvedarrayref = installer::windows::feature::sort_feature($modulesinproductlanguageresolvedarrayref);
        installer::windows::feature::create_feature_table($modulesinproductlanguageresolvedarrayref, $newidtdir, $languagesarrayref, $allvariableshashref);

        installer::windows::featurecomponent::create_featurecomponent_table($filesinproductlanguageresolvedarrayref, $registryitemsinproductlanguageresolvedarrayref, $newidtdir);

        installer::windows::media::create_media_table($filesinproductlanguageresolvedarrayref, $newidtdir, $allvariableshashref);

        installer::windows::font::create_font_table($filesinproductlanguageresolvedarrayref, $newidtdir);

        # Attention: The table "Shortcut.idt" contains language specific strings -> parameter: $languagesarrayref !
        # Attention: Shortcuts (Folderitems) have icon files, that have to be copied into the Icon directory (last parameter)
        my @iconfilecollector = ();

        installer::windows::shortcut::create_shortcut_table($filesinproductlanguageresolvedarrayref, $linksinproductlanguageresolvedarrayref, $folderinproductlanguageresolvedarrayref, $folderitemsinproductlanguageresolvedarrayref, $directoriesforepmarrayref, $newidtdir, $languagesarrayref, $includepatharrayref, \@iconfilecollector);

        installer::windows::inifile::create_inifile_table($inifiletableentries, $filesinproductlanguageresolvedarrayref, $newidtdir);

        installer::windows::icon::create_icon_table(\@iconfilecollector, $newidtdir);    # creating the icon table with all iconfiles used as shortcuts (FolderItems)

        installer::windows::createfolder::create_createfolder_table($directoriesforepmarrayref, $filesinproductlanguageresolvedarrayref, $newidtdir, $allvariableshashref);

        installer::windows::upgrade::create_upgrade_table($newidtdir, $allvariableshashref);

        if ( ! $installer::globals::languagepack )   # the following tables not for language packs
        {
            installer::windows::removefile::create_removefile_table($folderitemsinproductlanguageresolvedarrayref, $newidtdir);

            installer::windows::selfreg::create_selfreg_table($filesinproductlanguageresolvedarrayref, $newidtdir);

            # Adding Assemblies into the tables MsiAssembly and MsiAssemblyName dynamically
            installer::windows::assembly::create_msiassembly_table($filesinproductlanguageresolvedarrayref, $newidtdir);
            installer::windows::assembly::create_msiassemblyname_table($filesinproductlanguageresolvedarrayref, $newidtdir);
            installer::windows::assembly::add_assembly_condition_into_component_table($filesinproductlanguageresolvedarrayref, $newidtdir);
        }

        $installer::logger::Lang->print("\n");

        # Localizing the language dependent idt files
        # For every language there will be a localized msi database
        # For multilingual installation sets, the differences of this
        # databases have to be stored in transforms.

        for ( my $m = 0; $m <= $#{$languagesarrayref}; $m++ )
        {
            my $onelanguage = ${$languagesarrayref}[$m];

            my $is_bidi = 0;
            if ( installer::existence::exists_in_array($onelanguage, \@installer::globals::bidilanguages) ) { $is_bidi = 1; }

            my $languageidtdir = $idtdirbase . $installer::globals::separator . $onelanguage;
            if ( -d $languageidtdir ) { installer::systemactions::remove_complete_directory($languageidtdir, 1); }
            installer::systemactions::create_directory($languageidtdir);

            # Copy the template idt files and the new created idt files into this language directory

            $installer::logger::Info->print( "... copying idt files ...\n" );

            installer::logger::include_header_into_logfile("Copying idt files to $languageidtdir:");

            installer::windows::idtglobal::prepare_language_idt_directory($languageidtdir, $newidtdir, $onelanguage, $filesinproductlanguageresolvedarrayref, \@iconfilecollector, $binarytablefiles, $allvariableshashref);

            if (( ! $installer::globals::languagepack ) && ( ! $allvariableshashref->{'NOLANGUAGESELECTIONPRODUCT'} ))
            {
                # For multilingual installation sets, the dialog for the language selection can now be prepared, with
                # a checkbox for each available language. This has to happen before the following translation.
                # The new controls have to be added into the Control.idt

                my $controlidttablename = $languageidtdir . $installer::globals::separator . "Control.idt";
                my $controlidttable = installer::files::read_file($controlidttablename);
                installer::windows::idtglobal::add_language_checkboxes_to_database($controlidttable, $languagesarrayref);
                installer::files::save_file($controlidttablename, $controlidttable);
                $installer::logger::Lang->printf("Added checkboxes for language selection dialog into table %s\n",
                    $controlidttablename);
            }

            # Now all files are copied into a language specific directory
            # The template idt files can be translated

            $installer::logger::Info->print( "... localizing idt files (language: $onelanguage) ...\n" );

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

                $installer::logger::Lang->printf("Translated idt file: %s into language %s\n",
                    $oneidtfilename,
                    $onelanguage);
                $installer::logger::Lang->printf("Used languagefile: %s\n", $languagefilename);
            }

            # setting the encoding in every table (replacing WINDOWSENCODINGTEMPLATE)

            installer::windows::idtglobal::setencoding($languageidtdir, $onelanguage);

            # setting bidi attributes, if required

            if ( $is_bidi ) { installer::windows::idtglobal::setbidiattributes($languageidtdir, $onelanguage); }

            # setting the encoding in every table (replacing WINDOWSENCODINGTEMPLATE)
            installer::windows::idtglobal::set_multilanguageonly_condition($languageidtdir);

            # include the license text into the table Control.idt

            if ( ! $allvariableshashref->{'HIDELICENSEDIALOG'} )
            {
                my $licensefilesource = installer::windows::idtglobal::get_rtflicensefilesource($onelanguage, $includepatharrayref_lang);
                my $licensefile = installer::files::read_file($licensefilesource);
                installer::scpzipfiles::replace_all_ziplistvariables_in_rtffile($licensefile, $allvariablesarrayref, $onelanguage, $loggingdir);
                my $controltablename = $languageidtdir . $installer::globals::separator . "Control.idt";
                my $controltable = installer::files::read_file($controltablename);
                installer::windows::idtglobal::add_licensefile_to_database($licensefile, $controltable);
                installer::files::save_file($controltablename, $controltable);

                $installer::logger::Lang->printf("Added licensefile %s into database %s\n",
                    $licensefilesource,
                    $controltablename);
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

            # setting patch codes to detect installed products

            if (( $installer::globals::patch ) || ( $installer::globals::languagepack ) || ( $allvariableshashref->{'PDFCONVERTER'} )) { installer::windows::patch::update_patch_tables($languageidtdir, $allvariableshashref); }

            # Adding Windows Installer CustomActions

            installer::windows::idtglobal::addcustomactions($languageidtdir, $windowscustomactionsarrayref, $filesinproductlanguageresolvedarrayref);

            # Adding child projects if specified

            if ($installer::globals::addchildprojects)
            {
                # Adding child projects to installation dynamically (also in feature table)
                installer::windows::idtglobal::add_childprojects($languageidtdir, $filesinproductlanguageresolvedarrayref, $allvariableshashref);
                # setting Java variables for Java products
                if ( $allvariableshashref->{'JAVAPRODUCT'} ) { installer::windows::java::update_java_tables($languageidtdir, $allvariableshashref); }
            }

            # Then the language specific msi database can be created

            if ( $installer::globals::iswin )   # only possible on a Windows platform
            {
                my $msidatabasename = installer::windows::msiglobal::get_msidatabasename($allvariableshashref, $onelanguage);
                my $msifilename = $languageidtdir . $installer::globals::separator . $msidatabasename;

                $installer::logger::Info->print( "... creating msi database (language $onelanguage) ... \n" );

                installer::windows::msiglobal::set_uuid_into_component_table($languageidtdir, $allvariableshashref);    # setting new GUID for the components using the tool uuidgen.exe
                installer::windows::msiglobal::prepare_64bit_database($languageidtdir, $allvariableshashref);   # making last 64 bit changes
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

            if ( $allvariableshashref->{'ADDLANGUAGEINDATABASENAME'} ) { installer::windows::msiglobal::add_language_to_msi_database($defaultlanguage, $installdir, $allvariableshashref); }

            $installer::logger::Info->print( "... generating setup.ini ...\n" );

            if ( ! $allvariableshashref->{'NOLOADERREQUIRED'} ) { installer::windows::msiglobal::create_setup_ini($languagesarrayref, $defaultlanguage, $installdir, $allvariableshashref); }
        }

        # Analyzing the ScpActions and copying the files into the installation set
        # At least the loader.exe

        $installer::logger::Info->print( "... copying files into installation set ...\n" );

        # installer::windows::msiglobal::copy_scpactions_into_installset($defaultlanguage, $installdir, $scpactionsinproductlanguageresolvedarrayref);
        installer::worker::put_scpactions_into_installset($installdir);

        # ... copying the setup.exe

        installer::windows::msiglobal::copy_windows_installer_files_into_installset($installdir, $includepatharrayref, $allvariableshashref);

        # ... copying the child projects

        if ($installer::globals::addchildprojects)
        {
            installer::windows::msiglobal::copy_child_projects_into_installset($installdir, $allvariableshashref);
        }

        $installer::logger::Info->print( "... creating ddf files ...\n" );

        # Creating all needed ddf files and generating a list
        # for the package process containing all system calls

        my $ddfdir = installer::systemactions::create_directories("ddf", $languagestringref);

        $installer::globals::packjobref = installer::windows::msiglobal::generate_cab_file_list($filesinproductlanguageresolvedarrayref, $installdir, $ddfdir, $allvariableshashref);

        # Update and patch reasons the pack order needs to be saved
        installer::windows::msiglobal::save_packorder();

        $installer::logger::Info->print("\n");

        #######################################################
        # Finally really create the installation packages,
        # Only for Windows and only on a windows platform.
        #######################################################

        if ( $installer::globals::iswin )   # only possible on a Windows platform
        {
            $installer::logger::Info->print( "... packaging installation set ... \n" );
            installer::windows::msiglobal::execute_packaging($installer::globals::packjobref, $loggingdir, $allvariableshashref);
            if ( $installer::globals::include_cab_in_msi ) { installer::windows::msiglobal::include_cabs_into_msi($installdir); }
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

        if ( $is_success ) { installer::followme::save_followme_info($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref, $current_install_number, $loggingdir, $installlogdir); }

        if ( $$downloadname ne "" ) { $create_download = 1; }
        if (( $is_success ) && ( $create_download ) && ( $ENV{'ENABLE_DOWNLOADSETS'} ))
        {
            my $downloaddir = installer::download::create_download_sets($finalinstalldir, $includepatharrayref, $allvariableshashref, $$downloadname, $languagestringref, $languagesarrayref);
            installer::worker::analyze_and_save_logfile($loggingdir, $downloaddir, $installlogdir, $allsettingsarrayref, $languagestringref, $current_install_number);
        }

    }    # end of "if ( $installer::globals::iswindowsbuild )"


    # saving file_info file for later analysis
    my $speciallogfilename = "fileinfo_" . $installer::globals::product . "\.log";
    installer::files::save_array_of_hashes($loggingdir . $speciallogfilename, $filesinproductlanguageresolvedarrayref);
}

# saving debug info at end
if ( $installer::globals::debug ) { installer::logger::savedebug($installer::globals::exitlog); }

#######################################################
# Stopping time
#######################################################

installer::logger::stoptime();

####################################
# Main program end
####################################
