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

package installer::scriptitems;

use installer::converter;
use installer::exiter;
use installer::globals;
use installer::languages;
use installer::logger;
use installer::pathanalyzer;
use installer::remover;
use installer::systemactions;

################################################################
# Resolving the GID for the directories defined in setup script
################################################################

sub resolve_all_directory_names
{
    my ($directoryarrayref) = @_;

    # After this procedure the hash shall contain the complete language
    # dependent path, not only the language dependent HostName.

    my ($key, $value, $parentvalue, $parentgid, $parentdirectoryhashref);

    for ( my $i = 0; $i <= $#{$directoryarrayref}; $i++ )
    {
        my $directoryhashref = ${$directoryarrayref}[$i];
        my $gid = $directoryhashref-> {'gid'};
        my $parentid = $directoryhashref-> {'ParentID'};

        if ( $parentid ne "PREDEFINED_PROGDIR" )
        {
            # find the array of the parentid, which has to be defined before in setup script
            # and is therefore listed before in this array

            for ( my $j = 0; $j <= $i; $j++ )
            {
                $parentdirectoryhashref = ${$directoryarrayref}[$j];
                $parentgid = $parentdirectoryhashref->{'gid'};

                if ( $parentid eq $parentgid)
                {
                    last;
                }
            }

            # and now we can put the path together
            # But take care of the languages!

            my $dirismultilingual = $directoryhashref->{'ismultilingual'};
            my $parentismultilingual = $parentdirectoryhashref->{'ismultilingual'};

            # First: Both directories are language independent or both directories are language dependent

            if ((( ! $dirismultilingual ) && ( ! $parentismultilingual )) ||
                (( $dirismultilingual ) && ( $parentismultilingual )))
            {
                foreach $key (keys %{$directoryhashref})
                {
                    # the key ("HostName (en-US)") must be usable for both hashes

                    if ( $key =~ /\bHostName\b/ )
                    {
                        $parentvalue = "";
                        $value = $directoryhashref->{$key};
                        if ( $parentdirectoryhashref->{$key} ) { $parentvalue = $parentdirectoryhashref->{$key}; }

                        # It is possible, that in scp project, a directory is defined in more languages than
                        # the directory parent (happened after automatic generation of macros.inc).
                        # Therefore this is checked now and written with a warning into the logfile.
                        # This is no error, because (in most cases) the concerned language is not build.

                        if ($parentvalue eq "")
                        {
                            $directoryhashref->{$key} = "FAILURE";
                            my $infoline = "WARNING: No hostname for $parentid with \"$key\". Needed by child directory $gid !\n";
                            push( @installer::globals::globallogfileinfo, $infoline);
                        }
                        else
                        {
                            $directoryhashref->{$key} = $parentvalue . $installer::globals::separator . $value;
                        }
                    }
                }
            }

            # Second: The directory is language dependent, the parent not

            if (( $dirismultilingual ) && ( ! $parentismultilingual ))
            {
                $parentvalue = $parentdirectoryhashref->{'HostName'};       # there is only one

                foreach $key (keys %{$directoryhashref})        # the current directory
                {
                    if ( $key =~ /\bHostName\b/ )
                    {
                        $value = $directoryhashref->{$key};
                        $directoryhashref->{$key} = $parentvalue . $installer::globals::separator . $value;
                    }
                }
            }

            # Third: The directory is not language dependent, the parent is language dependent

            if (( ! $dirismultilingual ) && ( $parentismultilingual ))
            {
                $value = $directoryhashref->{'HostName'};       # there is only one
                delete($directoryhashref->{'HostName'});

                foreach $key (keys %{$parentdirectoryhashref})      # the parent directory
                {
                    if ( $key =~ /\bHostName\b/ )
                    {
                        $parentvalue = $parentdirectoryhashref->{$key};     # there is only one
                        $directoryhashref->{$key} = $parentvalue . $installer::globals::separator . $value;
                    }
                }

                $directoryhashref->{'ismultilingual'} = 1;  # now this directory is also language dependent
            }
        }
    }
}

#############################################################################
# Files with flag NOT_IN_SUITE do not need to be packed into
# Suite installation sets
#############################################################################

sub remove_office_start_language_files
{
    my ($productarrayref) = @_;

    my @newitems = ();

    for ( my $i = 0; $i <= $#{$productarrayref}; $i++ )
    {
        my $oneitem = ${$productarrayref}[$i];
        my $styles = "";

        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }

        if (!($styles =~ /\bSET_OFFICE_LANGUAGE\b/))
        {
            push(@newitems, $oneitem);
        }
        else
        {
            my $infoline = "INFO: Flag SET_OFFICE_LANGUAGE \-\> Removing $oneitem->{'gid'} from file list.\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    return \@newitems;
}

#############################################################################
# Registryitems for Uninstall have to be removed
#############################################################################

sub remove_uninstall_regitems_from_script
{
    my ($registryarrayref) = @_;

    my @newitems = ();

    for ( my $i = 0; $i <= $#{$registryarrayref}; $i++ )
    {
        my $oneitem = ${$registryarrayref}[$i];
        my $subkey = "";

        if ( $oneitem->{'Subkey'} ) { $subkey = $oneitem->{'Subkey'}; }

        if ( $subkey =~ /Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall/ ) { next; }

        push(@newitems, $oneitem);
    }

    return \@newitems;
}

##############################################################################
# Searching the language module for a specified language
##############################################################################

sub get_languagespecific_module
{
    my ( $lang, $modulestring ) = @_;

    my $langmodulestring = "";

    my $module;
    foreach $module ( keys %installer::globals::alllangmodules )
    {
        if (( $installer::globals::alllangmodules{$module} eq $lang ) && ( $modulestring =~ /\b$module\b/ ))
        {
            $langmodulestring = "$langmodulestring,$module";
        }
    }

    $langmodulestring =~ s/^\s*,//;

    if ( $langmodulestring eq "" ) { installer::exiter::exit_program("ERROR: No language pack module found for language $lang in string \"$modulestring\"!", "get_languagespecific_module");  }

    return $langmodulestring;
}

##############################################################################
# Removing all items in product lists which do not have the correct languages
##############################################################################

sub resolving_all_languages_in_productlists
{
    my ($productarrayref, $languagesarrayref) = @_;

    my @itemsinalllanguages = ();

    my ($key, $value);

    for ( my $i = 0; $i <= $#{$productarrayref}; $i++ )
    {
        my $oneitem = ${$productarrayref}[$i];

        my $ismultilingual = $oneitem->{'ismultilingual'};

        if (!($ismultilingual)) # nothing to do with single language items
        {
            $oneitem->{'specificlanguage'} = "";
            push(@itemsinalllanguages, $oneitem);
        }
        else    #all language dependent files
        {
            for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ )   # iterating over all languages
            {
                my $onelanguage = ${$languagesarrayref}[$j];

                my %oneitemhash = ();

                foreach $key (keys %{$oneitem})
                {
                    if ( $key =~ /\(\S+\)/ )    # this are the language dependent keys
                    {
                        if ( $key =~ /\(\Q$onelanguage\E\)/ )
                        {
                            $value = $oneitem->{$key};
                            $oneitemhash{$key} = $value;
                        }
                    }
                    else
                    {
                        $value = $oneitem->{$key};
                        $oneitemhash{$key} = $value;
                    }
                }

                $oneitemhash{'specificlanguage'} = $onelanguage;

                if ( $oneitemhash{'haslanguagemodule'} )
                {
                    my $langmodulestring = get_languagespecific_module($onelanguage, $oneitemhash{'modules'});
                    $oneitemhash{'modules'} = $langmodulestring;
                }

                push(@itemsinalllanguages, \%oneitemhash);
            }
        }
    }

    return \@itemsinalllanguages;
}

################################################################################
# Removing all modules, that have the flag LANGUAGEMODULE, but do not
# have the correct language
################################################################################

sub remove_not_required_language_modules
{
    my ($modulesarrayref, $languagesarrayref) = @_;

    my @allmodules = ();

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $module = ${$modulesarrayref}[$i];
        my $styles = "";
        if ( $module->{'Styles'} ) { $styles = $module->{'Styles'}; }

        if ( $styles =~ /\bLANGUAGEMODULE\b/ )
        {
            if ( ! exists($module->{'Language'}) ) { installer::exiter::exit_program("ERROR: \"$module->{'gid'}\" has flag LANGUAGEMODULE, but does not know its language!", "remove_not_required_language_modules"); }
            my $modulelanguage = $module->{'Language'};
            # checking, if language is required
            my $doinclude = 0;
            for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ )
            {
                my $onelanguage = ${$languagesarrayref}[$j];
                if ( $onelanguage eq $modulelanguage )
                {
                    $doinclude = 1;
                    last;
                }
            }

            if ( $doinclude ) { push(@allmodules, $module); }
        }
        else
        {
            push(@allmodules, $module);
        }
    }

    return \@allmodules;
}

################################################################################
# Removing all modules, that have a spellchecker language that is not
# required for this product (spellchecker selection).
# All required spellchecker languages are stored in
# %installer::globals::spellcheckerlanguagehash
################################################################################

sub remove_not_required_spellcheckerlanguage_modules
{
    my ($modulesarrayref) = @_;

    my $infoline = "";
    my @allmodules = ();

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $module = ${$modulesarrayref}[$i];
        if ( $module->{'Spellcheckerlanguage'} )    # selecting modules with Spellcheckerlanguage
        {
            if ( exists($installer::globals::spellcheckerlanguagehash{$module->{'Spellcheckerlanguage'}}) )
            {
                push(@allmodules, $module);
            }
            else
            {
                $infoline = "Spellchecker selection: Removing module $module->{'gid'}\n";
                push( @installer::globals::logfileinfo, $infoline);

                # Collecting all files at modules that are removed

                if ( $module->{'Files'} )
                {
                    if ( $module->{'Files'} =~ /^\s*\((.*?)\)\s*$/ )
                    {
                        my $filelist = $1;

                        my $filelisthash = installer::converter::convert_stringlist_into_hash(\$filelist, ",");
                        foreach my $onefile ( keys %{$filelisthash} ) { $installer::globals::spellcheckerfilehash{$onefile} = 1; }
                    }
                }
            }
        }
        else
        {
            push(@allmodules, $module);
        }
    }

    return \@allmodules;
}

################################################################################
# Removing all modules, that belong to a module that was removed
# in "remove_not_required_spellcheckerlanguage_modules" because of the
# spellchecker language. The files belonging to the modules are collected
# in %installer::globals::spellcheckerfilehash.
################################################################################

sub remove_not_required_spellcheckerlanguage_files
{
    my ($filesarrayref) = @_;

    my @filesarray = ();
    my $infoline = "";

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        if ( exists($installer::globals::spellcheckerfilehash{$onefile->{'gid'}}) )
        {
            $infoline = "Spellchecker selection: Removing file $onefile->{'gid'}\n";
            push( @installer::globals::logfileinfo, $infoline);
            next;
        }
        push(@filesarray, $onefile);
    }

    return \@filesarray;
}

################################################################################
# Looking for directories without correct HostName
################################################################################

sub checking_directories_with_corrupt_hostname
{
    my ($dirsref, $languagesarrayref) = @_;

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];

        my $hostname = "";

        if ( $onedir->{'HostName'} ) { $hostname = $onedir->{'HostName'}; }

        if ( $hostname eq "" )
        {
            my $langstring = "";
            for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ ) { $langstring .= ${$languagesarrayref}[$j] . " "; }
            installer::exiter::exit_program("ERROR: HostName not defined for $onedir->{'gid'} for specified language. Probably you wanted to create an installation set, in a language not defined in scp2 project. You selected the following language(s): $langstring", "checking_directories_with_corrupt_hostname");
        }

        if ( $hostname eq "FAILURE" )
        {
            installer::exiter::exit_program("ERROR: Could not create HostName for $onedir->{'gid'} (missing language at parent). See logfile warning for more info!", "checking_directories_with_corrupt_hostname");
        }
    }
}

################################################################################
# Setting global properties
################################################################################

sub set_global_directory_hostnames
{
    my ($dirsref, $allvariables) = @_;

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];
        my $styles = "";
        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        if ( $styles =~ /\bOFFICEDIRECTORY\b/ )
        {
            $installer::globals::officedirhostname = $onedir->{'HostName'};
            $installer::globals::officedirgid = $onedir->{'gid'};
            $allvariables->{'OFFICEDIRECTORYHOSTNAME'} = $installer::globals::officedirhostname;
        }
    }
}

########################################################
# Recursively defined procedure to order
# modules and directories
########################################################

sub get_children
{
    my ($allitems, $startparent, $newitemorder) = @_;

    for ( my $i = 0; $i <= $#{$allitems}; $i++ )
    {
        my $gid = ${$allitems}[$i]->{'gid'};
        my $parent = "";
        if ( ${$allitems}[$i]->{'ParentID'} ) { $parent = ${$allitems}[$i]->{'ParentID'}; }

        if ( $parent eq $startparent )
        {
            push(@{$newitemorder}, ${$allitems}[$i]);
            my $parent = $gid;
            get_children($allitems, $parent, $newitemorder);    # recursive!
        }
    }
}

################################################################################
# Using different HostName for language packs
################################################################################

sub use_langpack_hostname
{
    my ($dirsref) = @_;

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];
        if (( $onedir->{'LangPackHostName'} ) && ( $onedir->{'LangPackHostName'} ne "" )) { $onedir->{'HostName'} = $onedir->{'LangPackHostName'}; }
    }
}

################################################################################
# Using different HostName for language packs
################################################################################

sub use_patch_hostname
{
    my ($dirsref) = @_;

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];
        if (( $onedir->{'PatchHostName'} ) && ( $onedir->{'PatchHostName'} ne "" )) { $onedir->{'HostName'} = $onedir->{'PatchHostName'}; }
    }
}

################################################################################
# Using different HostName for language packs
################################################################################

sub use_langpack_copy_scpaction
{
    my ($scpactionsref) = @_;

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onescpaction = ${$scpactionsref}[$i];
        if (( $onescpaction->{'LangPackCopy'} ) && ( $onescpaction->{'LangPackCopy'} ne "" )) { $onescpaction->{'Copy'} = $onescpaction->{'LangPackCopy'}; }
    }
}

################################################################################
# Using different HostName for dev builds
################################################################################

sub use_devversion_copy_scpaction
{
    my ($scpactionsref) = @_;

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onescpaction = ${$scpactionsref}[$i];
        if (( $onescpaction->{'DevVersionCopy'} ) && ( $onescpaction->{'DevVersionCopy'} ne "" )) { $onescpaction->{'Copy'} = $onescpaction->{'DevVersionCopy'}; }
    }
}

################################################################################
# Using different HostName for language packs
################################################################################

sub use_patch_copy_scpaction
{
    my ($scpactionsref) = @_;

    for ( my $i = 0; $i <= $#{$scpactionsref}; $i++ )
    {
        my $onescpaction = ${$scpactionsref}[$i];
        if (( $onescpaction->{'PatchCopy'} ) && ( $onescpaction->{'PatchCopy'} ne "" )) { $onescpaction->{'Copy'} = $onescpaction->{'PatchCopy'}; }
    }
}

################################################################################
# Shifting parent directories of URE and Basis layer, so that
# these directories are located below the Brand layer.
# Style: SHIFT_BASIS_INTO_BRAND_LAYER
################################################################################

sub shift_basis_directory_parents
{
    my ($dirsref) = @_;

    my @alldirs = ();
    my @savedirs = ();
    my @shifteddirs = ();

    my $officedirgid = "";

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];
        my $styles = "";
        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        if ( $styles =~ /\bOFFICEDIRECTORY\b/ ) { $officedirgid = $onedir->{'gid'}; }
    }

    if ( $officedirgid ne "" )
    {
        for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
        {
            my $onedir = ${$dirsref}[$i];
            my $styles = "";
            if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

            if (( $styles =~ /\bBASISDIRECTORY\b/ ) || ( $styles =~ /\bUREDIRECTORY\b/ ))
            {
                $onedir->{'ParentID'} = $officedirgid;
            }
        }

        # Sorting directories
        my $startgid = "PREDEFINED_PROGDIR";
        get_children($dirsref, $startgid, \@alldirs);
    }

    return \@alldirs;
}

################################################################################
# Setting the name of the directory with style OFFICEDIRECTORY.
# The name can be defined in property OFFICEDIRECTORYNAME.
################################################################################

sub set_officedirectory_name
{
    my ($dirsref, $officedirname) = @_;

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];
        my $styles = "";
        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }
        if ( $styles =~ /\bOFFICEDIRECTORY\b/ )
        {
            $onedir->{'HostName'} = $officedirname;
            last;
        }
    }
}

################################################################################
# Simplifying the name for language dependent items from "Name (xy)" to "Name"
################################################################################

sub changing_name_of_language_dependent_keys
{
    my ($itemsarrayref) = @_;

    # Changing key for multilingual items from "Name ( )" to "Name" or "HostName ( )" to "HostName"

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $onelanguage = $oneitem->{'specificlanguage'};

        if (!($onelanguage eq "" ))                 # language dependent item
        {
            my $itemkey;

            foreach $itemkey (keys %{$oneitem})
            {
                if ( $itemkey =~ /^\s*(\S+?)\s+\(\S+\)\s*$/ )
                {
                    my $newitemkey = $1;
                    my $itemvalue = $oneitem->{$itemkey};
                    $oneitem->{$newitemkey} = $itemvalue;
                    delete($oneitem->{$itemkey});
                }
            }
        }
    }
}

################################################################################
# Replacement of setup variables in ConfigurationItems and ProfileItems
# <productkey>, <buildid>, <sequence_languages>, <productcode>, <upgradecode>, <productupdate>
################################################################################

sub replace_setup_variables
{
    my ($itemsarrayref, $languagestringref, $hashref) = @_;

    my $languagesstring = $$languagestringref;
    $languagesstring =~ s/\_/ /g;   # replacing underscore with whitespace

    my $productname = $hashref->{'PRODUCTNAME'};
    my $productversion = $hashref->{'PRODUCTVERSION'};
    my $userdirproductversion = "";
    if ( $hashref->{'USERDIRPRODUCTVERSION'} ) { $userdirproductversion = $hashref->{'USERDIRPRODUCTVERSION'}; }
    my $productkey = $productname . " " . $productversion;

    # string $buildid, which is used to replace the setup variable <buildid>

    my $localminor = "flat";
    if ( $installer::globals::minor ne "" ) { $localminor = $installer::globals::minor; }
    else { $localminor = $installer::globals::lastminor; }

    my $localbuild = $installer::globals::build;

    if ( $localbuild =~ /^\s*(\w+?)(\d+)\s*$/ ) { $localbuild = $2; }   # using "680" instead of "src680"

    my $buildidstring = join( '-', split( ' ', `$ENV{'SRC_ROOT'}/g -s log -n 1 --pretty=format:"%h "` ));
    if ($? || !$buildidstring) {
        $buildidstring = $localbuild . $localminor . "(Build:" . $installer::globals::buildid . ")";
    }
    else {
        $buildidstring = substr($buildidstring, 0, -1);
    }

    if ( $localminor =~ /^\s*\w(\d+)\w*\s*$/ ) { $localminor = $1; }

    my $updateid = $productname . "_" . $userdirproductversion . "_" . $$languagestringref;
    $updateid =~ s/ /_/g;

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $value = $oneitem->{'Value'};

        $value =~ s/\<buildid\>/$buildidstring/;
        $value =~ s/\<sequence_languages\>/$languagesstring/;
        $value =~ s/\<productkey\>/$productkey/;
        $value =~ s/\<productcode\>/$installer::globals::productcode/;
        $value =~ s/\<upgradecode\>/$installer::globals::upgradecode/;
        $value =~ s/\<alllanguages\>/$languagesstring/;
        $value =~ s/\<productmajor\>/$localbuild/;
        $value =~ s/\<productminor\>/$localminor/;
        $value =~ s/\<productbuildid\>/$installer::globals::buildid/;
        $value =~ s/\<sourceid\>/$installer::globals::build/;
        $value =~ s/\<updateid\>/$updateid/;
        $value =~ s/\<pkgformat\>/$installer::globals::packageformat/;
        $ENV{'OOO_VENDOR'} = "" if !defined $ENV{'OOO_VENDOR'};
        $value =~ s/\<vendor\>/$ENV{'OOO_VENDOR'}/;
        $ENV{'BUILD_VER_STRING'} = "" if !defined $ENV{'BUILD_VER_STRING'};
        $value =~ s/\<buildversion\>/$ENV{'BUILD_VER_STRING'}/;

        $oneitem->{'Value'} = $value;
    }
}

################################################################################
# By defining variable LOCALUSERDIR in *.lst it is possible to change
# the standard destination of user directory defined in scp2 ($SYSUSERCONFIG).
################################################################################

sub replace_userdir_variable
{
    my ($itemsarrayref) = @_;

    my $userdir = "";
    if ( $allvariableshashref->{'LOCALUSERDIR'} ) { $userdir = $allvariableshashref->{'LOCALUSERDIR'}; }
    else { $userdir = $installer::globals::simpledefaultuserdir; }

    if ( $userdir ne "" )
    {
        for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
        {
            my $oneitem = ${$itemsarrayref}[$i];
            $oneitem->{'Value'} =~ s/\$SYSUSERCONFIG/$userdir/;
        }
    }
}

#####################################################################################
# Files and ConfigurationItems are not included for all languages.
# For instance asian fonts. These can be removed, if no "Name" is found.
# ConfigurationItems are not always defined in the linguistic configuration file.
# The "Key" cannot be found for them.
#####################################################################################

sub remove_non_existent_languages_in_productlists
{
    my ($itemsarrayref, $languagestringref, $searchkey, $itemtype) = @_;

    # Removing of all non existent files, for instance asian fonts

    installer::logger::include_header_into_logfile("Removing for this language $$languagestringref:");

    my @allexistentitems = ();

    my $infoline;

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $oneitemname = "";       # $searchkey is "Name" for files and "Key" for ConfigurationItems

        if ( $oneitem->{$searchkey} ) { $oneitemname = $oneitem->{$searchkey} }

        my $itemtoberemoved = 0;

        if ($oneitemname eq "")                     # for instance asian font in english installation set
        {
            $itemtoberemoved = 1;
        }

        if ($itemtoberemoved)
        {
            $infoline = "WARNING: Language $$languagestringref: No $itemtype packed for $oneitem->{'gid'}!\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
        else
        {
            push(@allexistentitems, $oneitem);
        }
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

    return \@allexistentitems;
}

########################################################################
# Input is the directory gid, output the "HostName" of the directory
########################################################################

sub get_Directoryname_From_Directorygid
{
    my ($dirsarrayref ,$searchgid, $onelanguage, $oneitemgid) = @_;

    my $directoryname = "";
    my $onedirectory;
    my $foundgid = 0;

    for ( my $i = 0; $i <= $#{$dirsarrayref}; $i++ )
    {
        $onedirectory = ${$dirsarrayref}[$i];
        my $directorygid = $onedirectory->{'gid'};

        if ($directorygid eq $searchgid)
        {
            $foundgid = 1;
            last;
        }
    }

    if (!($foundgid))
    {
        installer::exiter::exit_program("ERROR: Gid $searchgid not defined in $installer::globals::setupscriptname", "get_Directoryname_From_Directorygid");
    }

    if ( ! ( $onedirectory->{'ismultilingual'} ))   # the directory is not language dependent
    {
         $directoryname = $onedirectory->{'HostName'};
    }
    else
    {
        $directoryname = $onedirectory->{"HostName ($onelanguage)"};
    }

    # gid_Dir_Template_Wizard_Letter is defined as language dependent directory, but the file gid_Dir_Template_Wizard_Letter
    # is not language dependent. Therefore $onelanguage is not defined. But which language is the correct language for the
    # directory?
    # Perhaps better solution: In scp it must be forbidden to have a language independent file in a language dependent directory.

    if (( ! $directoryname ) && ( $onelanguage eq "" ))
    {
        installer::exiter::exit_program("ERROR (in scp): Directory $searchgid is language dependent, but not $oneitemgid inside this directory", "get_Directoryname_From_Directorygid");
    }

    return \$directoryname;
}

##################################################################
# Getting destination direcotory for links, files and profiles
##################################################################

sub get_Destination_Directory_For_Item_From_Directorylist       # this is used for Files, Profiles and Links
{
    my ($itemarrayref, $dirsarrayref) = @_;

    for ( my $i = 0; $i <= $#{$itemarrayref}; $i++ )
    {
        my $oneitem = ${$itemarrayref}[$i];
        my $oneitemgid = $oneitem->{'gid'};
        my $directorygid = $oneitem->{'Dir'};       # for instance gid_Dir_Program
        my $netdirectorygid = "";
        my $onelanguage = $oneitem->{'specificlanguage'};
        my $ispredefinedprogdir = 0;
        my $ispredefinedconfigdir = 0;

        my $oneitemname = $oneitem->{'Name'};

        if ( $oneitem->{'NetDir'} ) { $netdirectorygid = $oneitem->{'NetDir'}; }

        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$oneitemname);    # making /registry/schema/org/openoffice/VCL.xcs to VCL.xcs

        my $searchdirgid;

        if ( $netdirectorygid eq "" )   # if NetDir is defined, it is privileged
        {
            $searchdirgid = $directorygid
        }
        else
        {
            $searchdirgid = $netdirectorygid
        }

        if ($searchdirgid =~ /PREDEFINED_PROGDIR/)  # the root directory is not defined in setup script
        {
            $ispredefinedprogdir = 1;
        }

        if ($searchdirgid =~ /PREDEFINED_CONFIGDIR/)    # the root directory is not defined in setup script
        {
            $ispredefinedconfigdir = 1;
        }

        my $destfilename;

        if ((!( $ispredefinedprogdir )) && (!( $ispredefinedconfigdir )))
        {
            my $directorynameref = get_Directoryname_From_Directorygid($dirsarrayref, $searchdirgid, $onelanguage, $oneitemgid);
            $destfilename = $$directorynameref . $installer::globals::separator . $oneitemname;
        }
        else
        {
            $destfilename = $oneitemname;
        }

        $oneitem->{'destination'} = $destfilename;
    }
}

##########################################################################
# Searching a file in a list of paths
##########################################################################

sub get_sourcepath_from_filename_and_includepath_classic
{
    my ($searchfilenameref, $includepatharrayref, $write_logfile) = @_;

    my ($onefile, $includepath, $infoline);

    my $foundsourcefile = 0;

    for ( my $j = 0; $j <= $#{$includepatharrayref}; $j++ )
    {
        $includepath = ${$includepatharrayref}[$j];
        installer::remover::remove_leading_and_ending_whitespaces(\$includepath);

        $onefile = $includepath . $installer::globals::separator . $$searchfilenameref;

        if ( -f $onefile )
        {
            $foundsourcefile = 1;
            last;
        }
    }

    if (!($foundsourcefile))
    {
        $onefile = "";  # the sourcepath has to be empty
        if ( $write_logfile)
        {
            if ( $ENV{'DEFAULT_TO_ENGLISH_FOR_PACKING'} )
            {
                $infoline = "WARNING: Source for $$searchfilenameref not found!\n";  # Important message in log file
            }
            else
            {
                $infoline = "ERROR: Source for $$searchfilenameref not found!\n";    # Important message in log file
            }

            push( @installer::globals::logfileinfo, $infoline);
        }
    }
    else
    {
        if ( $write_logfile)
        {
            $infoline = "SUCCESS: Source for $$searchfilenameref: $onefile\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    return \$onefile;
}

##########################################################################
# Input is one file name, output the complete absolute path of this file
##########################################################################

sub get_sourcepath_from_filename_and_includepath
{
    my ($searchfilenameref, $unused, $write_logfile) = @_;

    my ($onefile, $includepath, $infoline);

    my $foundsourcefile = 0;
    my $foundnewname = 0;

    for ( my $j = 0; $j <= $#installer::globals::allincludepaths; $j++ )
    {
        my $allfiles = $installer::globals::allincludepaths[$j];

        if ( exists( $allfiles->{$$searchfilenameref} ))
        {
            $onefile = $allfiles->{'includepath'} . $installer::globals::separator . $$searchfilenameref;
            $foundsourcefile = 1;
            last;
        }
    }

    if (!($foundsourcefile))    # testing with lowercase filename
    {
        # Attention: README01.html is copied for Windows to readme01.html, not case sensitive

        for ( my $j = 0; $j <= $#installer::globals::allincludepaths; $j++ )
        {
            my $allfiles = $installer::globals::allincludepaths[$j];

            my $newfilename = $$searchfilenameref;
            $newfilename =~ s/readme/README/;       # special handling for readme files
            $newfilename =~ s/license/LICENSE/;     # special handling for license files

            if ( exists( $allfiles->{$newfilename} ))
            {
                $onefile = $allfiles->{'includepath'} . $installer::globals::separator . $newfilename;
                $foundsourcefile = 1;
                $foundnewname = 1;
                last;
            }
        }
    }

    if (!($foundsourcefile))
    {
        $onefile = "";  # the sourcepath has to be empty
        if ( $write_logfile)
        {
            if ( $ENV{'DEFAULT_TO_ENGLISH_FOR_PACKING'} )
            {
                $infoline = "WARNING: Source for $$searchfilenameref not found!\n";  # Important message in log file
            }
            else
            {
                $infoline = "ERROR: Source for $$searchfilenameref not found!\n";    # Important message in log file
            }

            push( @installer::globals::logfileinfo, $infoline);
        }
    }
    else
    {
        if ( $write_logfile)
        {
            if (!($foundnewname))
            {
                $infoline = "SUCCESS: Source for $$searchfilenameref: $onefile\n";
            }
            else
            {
                $infoline = "SUCCESS/WARNING: Special handling for $$searchfilenameref: $onefile\n";
            }
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    return \$onefile;
}

##############################################################
# Determining, whether a specified directory is language
# dependent
##############################################################

sub determine_directory_language_dependency
{
    my($directorygid, $dirsref) = @_;

    my $is_multilingual = 0;

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];
        my $gid = $onedir->{'gid'};

        if ( $gid eq $directorygid )
        {
            $is_multilingual = $onedir->{'ismultilingual'};
            last;
        }
    }

    return $is_multilingual;
}

##############################################################
# Getting all source paths for all files to be packed
# $item can be "Files" or "ScpActions"
##############################################################

sub get_Source_Directory_For_Files_From_Includepathlist
{
    my ($filesarrayref, $includepatharrayref, $dirsref, $item) = @_;

    installer::logger::include_header_into_logfile("$item:");

    my $infoline = "";

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $onelanguage = $onefile->{'specificlanguage'};

        if ( ! $onefile->{'Name'} ) { installer::exiter::exit_program("ERROR: $item without name ! GID: $onefile->{'gid'} ! Language: $onelanguage", "get_Source_Directory_For_Files_From_Includepathlist"); }

        my $onefilename = $onefile->{'Name'};
        if ( $item eq "ScpActions" ) { $onefilename =~ s/\//$installer::globals::separator/g; }
        $onefilename =~ s/^\s*\Q$installer::globals::separator\E//;     # filename begins with a slash, for instance /registry/schema/org/openoffice/VCL.xcs

        my $styles = "";
        my $file_can_miss = 0;
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
        if ( $styles =~ /\bFILE_CAN_MISS\b/ ) { $file_can_miss = 1; }

        if (( $installer::globals::languagepack ) && ( ! $onefile->{'ismultilingual'} ) && ( ! ( $styles =~ /\bFORCELANGUAGEPACK\b/ ))) { $file_can_miss = 1; }
        if (( $installer::globals::helppack ) && ( ! $onefile->{'ismultilingual'} ) && ( ! ( $styles =~ /\bFORCEHELPPACK\b/ ))) { $file_can_miss = 1; }

        my $sourcepathref = "";

        if ( $file_can_miss ) { $sourcepathref = get_sourcepath_from_filename_and_includepath(\$onefilename, $includepatharrayref, 0); }
        else { $sourcepathref = get_sourcepath_from_filename_and_includepath(\$onefilename, $includepatharrayref, 1); }

        $onefile->{'sourcepath'} = $$sourcepathref; # This $$sourcepathref is empty, if no source was found

        # defaulting to english for multilingual files if DEFAULT_TO_ENGLISH_FOR_PACKING is set

        if ( $ENV{'DEFAULT_TO_ENGLISH_FOR_PACKING'} )
        {
            if (( ! $onefile->{'sourcepath'} ) && ( $onefile->{'ismultilingual'} ))
            {
                my $oldname = $onefile->{'Name'};
                my $oldlanguage = $onefile->{'specificlanguage'};
                my $newlanguage = "en-US";
                $onefilename = $onefile->{'Name'};
                $onefilename =~ s/$oldlanguage\./$newlanguage\./;   # Example: tplwizfax_it.zip -> tplwizfax_en-US.zip
                $onefilename =~ s/^\s*\Q$installer::globals::separator\E//;     # filename begins with a slash, for instance /registry/schema/org/openoffice/VCL.xcs
                $sourcepathref = get_sourcepath_from_filename_and_includepath(\$onefilename, $includepatharrayref, 1);
                $onefile->{'sourcepath'} = $$sourcepathref;                     # This $$sourcepathref is empty, if no source was found

                if ($onefile->{'sourcepath'})   # defaulting to english was successful
                {
                    $infoline = "WARNING: Using $onefilename instead of $oldname\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    print "    $infoline";

                    # If the directory, in which the new file is installed, is not language dependent,
                    # the filename has to be changed to avoid installation conflicts
                    # No mechanism for resource files!
                    # -> implementing for the content of ARCHIVE files

                    if ( $onefile->{'Styles'} =~ /\bARCHIVE\b/ )
                    {
                        my $directorygid = $onefile->{'Dir'};
                        my $islanguagedependent = determine_directory_language_dependency($directorygid, $dirsref);

                        if ( ! $islanguagedependent )
                        {
                            $onefile->{'Styles'} =~ s/\bARCHIVE\b/ARCHIVE, RENAME_TO_LANGUAGE/; # Setting new flag RENAME_TO_LANGUAGE
                            $infoline = "Setting flag RENAME_TO_LANGUAGE: File $onefile->{'Name'} in directory: $directorygid\n";
                            push( @installer::globals::logfileinfo, $infoline);
                        }
                    }
                }
                else
                {
                    $infoline = "WARNING: Using $onefile->{'Name'} instead of $oldname was not successful\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $onefile->{'Name'} = $oldname;  # Switching back to old file name
                }
            }
        }
    }

    $infoline = "\n";   # empty line after listing of all files
    push( @installer::globals::logfileinfo, $infoline);
}

#################################################################################
# Removing files, that shall not be included into languagepacks
# (because of rpm conflicts)
#################################################################################

sub remove_Files_For_Languagepacks
{
    my ($itemsarrayref) = @_;

    my $infoline;

    my @newitemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $gid = $oneitem->{'gid'};

        # scp Todo: Remove asap after removal of old setup

        if (( $gid eq "gid_File_Extra_Fontunxpsprint" ) ||
            ( $gid eq "gid_File_Extra_Migration_Lang" ))
        {
            $infoline = "ATTENTION: Removing item $oneitem->{'gid'} from the installation set.\n";
            push( @installer::globals::logfileinfo, $infoline);

            next;
        }

        push(@newitemsarray, $oneitem);
    }

    return \@newitemsarray;
}

#################################################################################
# Files, whose source directory is not found, are removed now (this is an ERROR)
#################################################################################

sub remove_Files_Without_Sourcedirectory
{
    my ($filesarrayref) = @_;

    my $infoline;

    my $error_occurred = 0;
    my @missingfiles = ();
    push(@missingfiles, "ERROR: The following files could not be found: \n");

    my @newfilesarray = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $sourcepath = $onefile->{'sourcepath'};

        if ($sourcepath eq "")
        {
            my $styles = $onefile->{'Styles'};
            my $filename = $onefile->{'Name'};

            if ( ! $installer::globals::languagepack && !$installer::globals::helppack)
            {
                $infoline = "ERROR: Removing file $filename from file list.\n";
                push( @installer::globals::logfileinfo, $infoline);

                push(@missingfiles, "ERROR: File not found: $filename\n");
                $error_occurred = 1;

                next;   # removing this file from list, if sourcepath is empty
            }
            elsif ( $installer::globals::languagepack ) # special case for language packs
            {
                if (( $onefile->{'ismultilingual'} ) || ( $styles =~ /\bFORCELANGUAGEPACK\b/ ))
                {
                    $infoline = "ERROR: Removing file $filename from file list.\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    push(@missingfiles, "ERROR: File not found: $filename\n");
                    $error_occurred = 1;

                    next;   # removing this file from list, if sourcepath is empty
                }
                else
                {
                    $infoline = "INFO: Removing file $filename from file list. It is not language dependent.\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $infoline = "INFO: It is not language dependent and can be ignored in language packs.\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    next;   # removing this file from list, if sourcepath is empty
                }
            }
            else # special case for help packs
            {
                if (( $onefile->{'ismultilingual'} ) || ( $styles =~ /\bFORCEHELPPACK\b/ ))
                {
                    $infoline = "ERROR: Removing file $filename from file list.\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    push(@missingfiles, "ERROR: File not found: $filename\n");
                    $error_occurred = 1;

                    next;   # removing this file from list, if sourcepath is empty
                }
                else
                {
                    $infoline = "INFO: Removing file $filename from file list. It is not language dependent.\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    $infoline = "INFO: It is not language dependent and can be ignored in help packs.\n";
                    push( @installer::globals::logfileinfo, $infoline);

                    next;   # removing this file from list, if sourcepath is empty
                }
           }
        }

        push(@newfilesarray, $onefile);
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $error_occurred )
    {
        for ( my $i = 0; $i <= $#missingfiles; $i++ ) { print "$missingfiles[$i]"; }
        installer::exiter::exit_program("ERROR: Missing files", "remove_Files_Without_Sourcedirectory");
    }

    return \@newfilesarray;
}

############################################################################
# License and Readme files in the default language have to be installed
# in the directory with flag OFFICEDIRECTORY. If this is not defined
# they have to be installed in the installation root.
############################################################################

sub get_office_directory_gid_and_hostname
{
    my ($dirsarrayref) = @_;

    my $foundofficedir = 0;
    my $gid = "";
    my $hostname = "";

    for ( my $i = 0; $i <= $#{$dirsarrayref}; $i++ )
    {
        my $onedir = ${$dirsarrayref}[$i];
        if ( $onedir->{'Styles'} )
        {
            my $styles = $onedir->{'Styles'};

            if ( $styles =~ /\bOFFICEDIRECTORY\b/ )
            {
                $foundofficedir = 1;
                $gid = $onedir->{'gid'};
                $hostname = $onedir->{'HostName'};
                last;
            }
        }
    }

    return ($foundofficedir, $gid, $hostname);
}

############################################################################
# License and Readme files in the default language have to be installed
# in the installation root (next to the program dir). This is in scp
# project done by a post install basic script
############################################################################

sub add_License_Files_into_Installdir
{
    my ($filesarrayref, $dirsarrayref, $languagesarrayref) = @_;

    my $infoline;

    my @newfilesarray = ();

    my $defaultlanguage = installer::languages::get_default_language($languagesarrayref);

    my ($foundofficedir, $officedirectorygid, $officedirectoryhostname) = get_office_directory_gid_and_hostname($dirsarrayref);

    # copy all files from directory share/readme, that contain the default language in their name
    # without default language into the installation root. This makes the settings of the correct
    # file names superfluous. On the other hand this requires a dependency to the directory
    # share/readme

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $destination = $onefile->{'destination'};
        my $styles = "";
        if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }

        if ( ( $destination =~ /share\Q$installer::globals::separator\Ereadme\Q$installer::globals::separator\E(\w+?)_?$defaultlanguage\.?(\w*)\s*/ )
            || (( $styles =~ /\bROOTLICENSEFILE\b/ ) && ( $destination =~ /\Q$installer::globals::separator\E?(\w+?)_?$defaultlanguage\.?(\w*?)\s*$/ )) )
        {
            my $filename = $1;
            my $extension = $2;

            my $newfilename;

            if ( $extension eq "" ) { $newfilename = $filename; }
            else { $newfilename = $filename . "\." . $extension; }

            my %newfile = ();
            my $newfile = \%newfile;

            installer::converter::copy_item_object($onefile, $newfile);

            $newfile->{'gid'} = $onefile->{'gid'} . "_Copy";
            $newfile->{'Name'} = $newfilename;
            $newfile->{'ismultilingual'} = "0";
            $newfile->{'specificlanguage'} = "";
            $newfile->{'haslanguagemodule'} = "0";

            if ( defined $newfile->{'InstallName'} )
            {
                if ( $newfile->{'InstallName'} =~ /^\s*(.*?)_$defaultlanguage\.?(\w*?)\s*$/ )
                {
                    my $localfilename = $1;
                    my $localextension = $2;

                    if ( $localextension eq "" ) { $newfile->{'InstallName'} = $localfilename; }
                    else { $newfile->{'InstallName'} = $localfilename . "\." . $localextension; }
                }
            }

            $newfile->{'removelangfromfile'} = "1"; # Important for files with an InstallName, because language also has to be removed there.

            if ( $foundofficedir )
            {
                $newfile->{'Dir'} = $officedirectorygid;
                $newfile->{'destination'} = $officedirectoryhostname . $installer::globals::separator . $newfilename;
            }
            else
            {
                $newfile->{'Dir'} = "PREDEFINED_PROGDIR";
                $newfile->{'destination'} = $newfilename;
            }

            # Also setting "modules=gid_Module_Root_Brand" (module with style: ROOT_BRAND_PACKAGE)
            if ( $installer::globals::rootbrandpackageset )
            {
                $newfile->{'modules'} = $installer::globals::rootbrandpackage;
            }

            push(@newfilesarray, $newfile);

            $infoline = "New files: Adding file $newfilename for the installation root to the file list. Language: $defaultlanguage\n";
            push( @installer::globals::logfileinfo, $infoline);

            if ( defined $newfile->{'InstallName'} )
            {
                $infoline = "New files: Using installation name: $newfile->{'InstallName'}\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }

        push(@newfilesarray, $onefile);
    }

    return \@newfilesarray;
}

############################################################################
# Some files are included for more than one language and have the same
# name and the same destination directory for all languages. This would
# lead to conflicts, if the filenames are not changed.
# In scp project this files must have the flag MAKE_LANG_SPECIFIC
# For this files, the language is included into the filename.
############################################################################

sub make_filename_language_specific
{
    my ($filesarrayref) = @_;

    my $infoline = "";

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];

        if ( $onefile->{'ismultilingual'} )
        {
            my $styles = "";
            if ( $onefile->{'Styles'} ) { $styles = $onefile->{'Styles'}; }
            if ( $styles =~ /\bMAKE_LANG_SPECIFIC\b/ )
            {
                my $language = $onefile->{'specificlanguage'};
                my $olddestination = $onefile->{'destination'};
                my $oldname = $onefile->{'Name'};

                # Including the language into the file name.
                # But be sure, to include the language before the file extension.

                my $fileextension = "";

                if ( $onefile->{'Name'} =~ /(\.\w+?)\s*$/ ) { $fileextension = $1; }
                if ( $fileextension ne "" )
                {
                    $onefile->{'Name'} =~ s/\Q$fileextension\E\s*$/_$language$fileextension/;
                    $onefile->{'destination'} =~ s/\Q$fileextension\E\s*$/_$language$fileextension/;
                }

                $infoline = "Flag MAKE_LANG_SPECIFIC:\n";
                push( @installer::globals::logfileinfo, $infoline);
                $infoline = "Changing name from $oldname to $onefile->{'Name'} !\n";
                push( @installer::globals::logfileinfo, $infoline);
                $infoline = "Changing destination from $olddestination to $onefile->{'destination'} !\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
    }
}

############################################################################
# Because of the item "File" the source name must be "Name". Therefore
# "Copy" is changed to "Name" and "Name" is changed to "DestinationName".
############################################################################

sub change_keys_of_scpactions
{
    my ($itemsarrayref) = @_;

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];

        my $key;

        # First Name to DestinationName, then deleting Name
        foreach $key (keys %{$oneitem})
        {
            if ( $key =~ /\bName\b/ )
            {
                my $value = $oneitem->{$key};
                my $oldkey = $key;
                $key =~ s/Name/DestinationName/;
                $oneitem->{$key} = $value;
                delete($oneitem->{$oldkey});
            }
        }

        # Second Copy to Name, then deleting Copy
        foreach $key (keys %{$oneitem})
        {
            if ( $key =~ /\bCopy\b/ )
            {
                my $value = $oneitem->{$key};
                my $oldkey = $key;
                $key =~ s/Copy/Name/;
                $oneitem->{$key} = $value;
                delete($oneitem->{$oldkey});
            }
        }
    }
}

############################################################################
# Removing all language pack files from installation set (files with
# the style LANGUAGEPACK), except this is a language pack.
############################################################################

sub remove_Languagepacklibraries_from_Installset
{
    my ($itemsarrayref) = @_;

    my $infoline;

    my @newitemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }

        if ( $styles =~ /\bLANGUAGEPACK\b/ )
        {
            $infoline = "Removing language pack file $oneitem->{'gid'} from the installation set.\n";
            push( @installer::globals::globallogfileinfo, $infoline);

            next;
        }

        push(@newitemsarray, $oneitem);
    }

    $infoline = "\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    return \@newitemsarray;
}

############################################################################
# Removing all help pack files from installation set (files with
# the style HELPPACK), except this is a help pack.
############################################################################

sub remove_Helppacklibraries_from_Installset
{
    my ($itemsarrayref) = @_;

    my $infoline;

    my @newitemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }

        if ( $styles =~ /\bHELPPACK\b/ )
        {
            $infoline = "Removing help pack file $oneitem->{'gid'} from the installation set.\n";
            push( @installer::globals::globallogfileinfo, $infoline);

            next;
        }

        push(@newitemsarray, $oneitem);
    }

    $infoline = "\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    return \@newitemsarray;
}

############################################################################
# Removing all files with flag PATCH_ONLY from installation set.
# This function is not called during patch creation.
############################################################################

sub remove_patchonlyfiles_from_Installset
{
    my ($itemsarrayref) = @_;

    my $infoline;

    my @newitemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }

        if ( $styles =~ /\bPATCH_ONLY\b/ )
        {
            $infoline = "Removing file with flag PATCH_ONLY $oneitem->{'gid'} from the installation set.\n";
            push( @installer::globals::globallogfileinfo, $infoline);

            next;
        }

        push(@newitemsarray, $oneitem);
    }

    $infoline = "\n";
    push( @installer::globals::globallogfileinfo, $infoline);

    return \@newitemsarray;
}

############################################################################
# Some files cotain a $ in their name. epm conflicts with such files.
# Solution: Renaming this files, converting "$" to "$$"
############################################################################

sub quoting_illegal_filenames
{
    my ($filesarrayref) = @_;

    # This function has to be removed as soon as possible!

    installer::logger::include_header_into_logfile("Renaming illegal filenames:");

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $filename = $onefile->{'Name'};

        if ( $filename =~ /\$/ )
        {
            my $sourcepath = $onefile->{'sourcepath'};
            my $destpath = $onefile->{'destination'};

            # sourcepath and destination have to be quoted for epm list file

            $destpath =~ s/\$/\$\$/g;
            $sourcepath =~ s/\$/\$\$/g;

            my $infoline = "ATTENTION: Files: Quoting sourcepath $onefile->{'sourcepath'} to $sourcepath\n";
            push( @installer::globals::logfileinfo, $infoline);
            $infoline = "ATTENTION: Files: Quoting destination path $onefile->{'destination'} to $destpath\n";
            push( @installer::globals::logfileinfo, $infoline);

            $onefile->{'sourcepath'} = $sourcepath;
            $onefile->{'destination'} = $destpath;
        }
    }
}

############################################################################
# Removing multiple occurrences of same module.
############################################################################

sub optimize_list
{
    my ( $longlist ) = @_;
    my %tmpHash;

    $longlist =~ s/^\s+//;
    $longlist =~ s/\s+$//;
    $longlist =~ s/\s*,\s*/,/g;

    @tmpHash{split /,/, $longlist} = ();
    return join(",", sort keys %tmpHash);
}

#######################################################################
# Collecting all directories needed for the epm list
# 1. Looking for all destination paths in the files array
# 2. Looking for directories with CREATE flag in the directory array
#######################################################################

##################################
# Collecting directories: Part 1
##################################

sub collect_directories_from_filesarray
{
    my ($filesarrayref) = @_;

    my @alldirectories = ();
    my %alldirectoryhash = ();

    my $predefinedprogdir_added = 0;

    # Preparing this already as hash, although the only needed value at the moment is the HostName
    # But also adding: "specificlanguage" and "Dir" (for instance gid_Dir_Program)

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $destinationpath = $onefile->{'destination'};
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationpath);
        $destinationpath =~ s/\Q$installer::globals::separator\E\s*$//;     # removing ending slashes or backslashes

        do
        {
            if (!exists($alldirectoryhash{$destinationpath}))
            {
                my %directoryhash = ();
                $directoryhash{'HostName'} = $destinationpath;
                $directoryhash{'specificlanguage'} = $onefile->{'specificlanguage'};
                $directoryhash{'Dir'} = $onefile->{'Dir'};
                $directoryhash{'modules'} = $onefile->{'modules'}; # NEW, saving modules

                $predefinedprogdir_added ||= $onefile->{'Dir'} eq "PREDEFINED_PROGDIR";

                $alldirectoryhash{$destinationpath} = \%directoryhash;
            }
            else
            {
                # Adding the modules to the module list!
                $alldirectoryhash{$destinationpath}->{'modules'} .= "," . $onefile->{'modules'};
            }
        } while ($destinationpath =~ s/(^.*\S)\Q$installer::globals::separator\E(\S.*?)\s*$/$1/);  # as long as the path contains slashes
    }

    # if there is no file in the root directory PREDEFINED_PROGDIR, it has to be included into the directory array now
    # HostName= specificlanguage=   Dir=PREDEFINED_PROGDIR

    if (! $predefinedprogdir_added )
    {
        my %directoryhash = ();
        $directoryhash{'HostName'} = "";
        $directoryhash{'specificlanguage'} = "";
        $directoryhash{'modules'} = ""; # ToDo?
        $directoryhash{'Dir'} = "PREDEFINED_PROGDIR";

        push(@alldirectories, \%directoryhash);
    }

    # Creating directory array
    foreach my $destdir ( sort keys %alldirectoryhash )
    {
        $alldirectoryhash{$destdir}->{'modules'} = optimize_list($alldirectoryhash{$destdir}->{'modules'});
        push(@alldirectories, $alldirectoryhash{$destdir});
    }

    return (\@alldirectories, \%alldirectoryhash);
}

##################################
# Collecting directories: Part 2
##################################

sub collect_directories_with_create_flag_from_directoryarray
{
    my ($directoryarrayref, $alldirectoryhash) = @_;

    my $alreadyincluded = 0;
    my @alldirectories = ();

    for ( my $i = 0; $i <= $#{$directoryarrayref}; $i++ )
    {
        my $onedir = ${$directoryarrayref}[$i];
        my $styles = "";
        $newdirincluded = 0;

        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        if ( $styles =~ /\bCREATE\b/ )
        {
            my $directoryname = "";

            if ( $onedir->{'HostName'} ) { $directoryname = $onedir->{'HostName'}; }
            else { installer::exiter::exit_program("ERROR: No directory name (HostName) set for specified language in gid $onedir->{'gid'}", "collect_directories_with_create_flag_from_directoryarray"); }

            $alreadyincluded = 0;
            if ( exists($alldirectoryhash->{$directoryname}) ) { $alreadyincluded = 1; }

            if (!($alreadyincluded))
            {
                my %directoryhash = ();
                $directoryhash{'HostName'} = $directoryname;
                $directoryhash{'specificlanguage'} = $onedir->{'specificlanguage'};
                $directoryhash{'Dir'} = $onedir->{'gid'};
                $directoryhash{'Styles'} = $onedir->{'Styles'};

                # saving also the modules
                if ( ! $onedir->{'modules'} ) { installer::exiter::exit_program("ERROR: No assigned modules found for directory $onedir->{'gid'}", "collect_directories_with_create_flag_from_directoryarray"); }
                $directoryhash{'modules'} = $onedir->{'modules'};

                $alldirectoryhash->{$directoryname} = \%directoryhash;
                $newdirincluded = 1;

                # Problem: The $destinationpath can be share/registry/schema/org/openoffice
                # but not all directories contain files and will be added to this list.
                # Therefore the path has to be analyzed.

                while ( $directoryname =~ /(^.*\S)\Q$installer::globals::separator\E(\S.*?)\s*$/ )  # as long as the path contains slashes
                {
                    $directoryname = $1;

                    $alreadyincluded = 0;
                    if ( exists($alldirectoryhash->{$directoryname}) ) { $alreadyincluded = 1; }

                    if (!($alreadyincluded))
                    {
                        my %directoryhash = ();

                        $directoryhash{'HostName'} = $directoryname;
                        $directoryhash{'specificlanguage'} = $onedir->{'specificlanguage'};
                        $directoryhash{'Dir'} = $onedir->{'gid'};
                        if ( ! $installer::globals::iswindowsbuild ) { $directoryhash{'Styles'} = "(CREATE)"; } # Exeception for Windows?

                        # saving also the modules
                        $directoryhash{'modules'} = $onedir->{'modules'};

                        $alldirectoryhash->{$directoryname} = \%directoryhash;
                        $newdirincluded = 1;
                    }
                    else
                    {
                        # Adding the modules to the module list!
                        $alldirectoryhash->{$directoryname}->{'modules'} = $alldirectoryhash->{$directoryname}->{'modules'} . "," . $onedir->{'modules'};
                    }
                }
            }
            else
            {
                # Adding the modules to the module list!
                $alldirectoryhash->{$directoryname}->{'modules'} = $alldirectoryhash->{$directoryname}->{'modules'} . "," . $onedir->{'modules'};

                while ( $directoryname =~ /(^.*\S)\Q$installer::globals::separator\E(\S.*?)\s*$/ )  # as long as the path contains slashes
                {
                    $directoryname = $1;
                    # Adding the modules to the module list!
                    $alldirectoryhash->{$directoryname}->{'modules'} = $alldirectoryhash->{$directoryname}->{'modules'} . "," . $onedir->{'modules'};
                }
            }
        }

        # Saving the styles for already added directories in function collect_directories_from_filesarray

        if (( ! $newdirincluded ) && ( $styles ne "" ))
        {
            $styles =~ s/\bWORKSTATION\b//;
            $styles =~ s/\bCREATE\b//;

            if (( ! ( $styles =~ /^\s*\(\s*\)\s*$/ )) && ( ! ( $styles =~ /^\s*\(\s*\,\s*\)\s*$/ )) && ( ! ( $styles =~ /^\s*$/ ))) # checking, if there are styles left
            {
                my $directoryname = "";
                if ( $onedir->{'HostName'} ) { $directoryname = $onedir->{'HostName'}; }
                else { installer::exiter::exit_program("ERROR: No directory name (HostName) set for specified language in gid $onedir->{'gid'}", "collect_directories_with_create_flag_from_directoryarray"); }

                if ( exists($alldirectoryhash->{$directoryname}) )
                {
                    $alldirectoryhash->{$directoryname}->{'Styles'} = $styles;
                }
            }
        }
    }

    # Creating directory array
    foreach my $destdir ( sort keys %{$alldirectoryhash} )
    {
        $alldirectoryhash->{$destdir}->{'modules'} = optimize_list($alldirectoryhash->{$destdir}->{'modules'});
        push(@alldirectories, $alldirectoryhash->{$destdir});
    }

    return (\@alldirectories, \%alldirectoryhash);
}

#################################################
# Determining the destination file of a link
#################################################

sub get_destination_file_path_for_links
{
    my ($linksarrayref, $filesarrayref) = @_;

    my $infoline;

    for ( my $i = 0; $i <= $#{$linksarrayref}; $i++ )
    {
        my $fileid = "";
        my $onelink = ${$linksarrayref}[$i];
        if ( $onelink->{'FileID'} ) { $fileid = $onelink->{'FileID'}; }

        if (!( $fileid eq "" ))
        {
            my $foundfile = 0;

            for ( my $j = 0; $j <= $#{$filesarrayref}; $j++ )
            {
                my $onefile = ${$filesarrayref}[$j];
                my $filegid = $onefile->{'gid'};

                if ( $filegid eq $fileid )
                {
                    $foundfile = 1;
                    $onelink->{'destinationfile'} = $onefile->{'destination'};
                    last;
                }
            }

            if (!($foundfile))
            {
                $infoline = "Warning: FileID $fileid for Link $onelink->{'gid'} not found!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#################################################
# Determining the destination link of a link
#################################################

sub get_destination_link_path_for_links
{
    my ($linksarrayref) = @_;

    my $infoline;

    for ( my $i = 0; $i <= $#{$linksarrayref}; $i++ )
    {
        my $shortcutid = "";
        my $onelink = ${$linksarrayref}[$i];
        if ( $onelink->{'ShortcutID'} ) { $shortcutid = $onelink->{'ShortcutID'}; }

        if (!( $shortcutid eq "" ))
        {
            my $foundlink = 0;

            for ( my $j = 0; $j <= $#{$linksarrayref}; $j++ )
            {
                my $destlink = ${$linksarrayref}[$j];
                $shortcutgid = $destlink->{'gid'};

                if ( $shortcutgid eq $shortcutid )
                {
                    $foundlink = 1;
                    $onelink->{'destinationfile'} = $destlink->{'destination'};     # making key 'destinationfile'
                    last;
                }
            }

            if (!($foundlink))
            {
                $infoline = "Warning: ShortcutID $shortcutid for Link $onelink->{'gid'} not found!\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);
}

###################################################################################
# Items with flag WORKSTATION are not needed (here: links and configurationitems)
###################################################################################

sub remove_workstation_only_items
{
    my ($itemarrayref) = @_;

    my @newitemarray = ();

    for ( my $i = 0; $i <= $#{$itemarrayref}; $i++ )
    {
        my $oneitem = ${$itemarrayref}[$i];
        my $styles = $oneitem->{'Styles'};

        if (( $styles =~ /\bWORKSTATION\b/ ) &&
            (!( $styles =~ /\bNETWORK\b/ )) &&
            (!( $styles =~ /\bSTANDALONE\b/ )))
        {
            next;   # removing this link, it is only needed for a workstation installation
        }

        push(@newitemarray, $oneitem);
    }

    return \@newitemarray;
}

################################################
# Resolving relative path in links
################################################

sub resolve_links_with_flag_relative
{
    my ($linksarrayref) = @_;

    # Before this step is:
    # destination=program/libsalhelperC52.so.3, this will be the name of the link
    # destinationfile=program/libsalhelperC52.so.3, this will be the linked file or name
    # If the flag RELATIVE is set, the paths have to be analyzed. If the flag is not set
    # (this will not occur in the future?) destinationfile has to be an absolute path name

    for ( my $i = 0; $i <= $#{$linksarrayref}; $i++ )
    {
        my $onelink = ${$linksarrayref}[$i];
        my $styles = $onelink->{'Styles'};

        if ( $styles =~ /\bRELATIVE\b/ )
        {
            # ToDo: This is only a simple not sufficient mechanism

            my $destination = $onelink->{'destination'};
            my $destinationfile = $onelink->{'destinationfile'};

            my $destinationpath = $destination;

            installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationpath);

            my $destinationfilepath = $destinationfile;

            # it is possible, that the destinationfile is no longer part of the files collector
            if ($destinationfilepath) { installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationfilepath); }
            else { $destinationfilepath = ""; }

            if ( $destinationpath eq $destinationfilepath )
            {
                # link and file are in the same directory
                # Therefore the path of the file can be removed

                my $newdestinationfile = $destinationfile;
                installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$newdestinationfile);

                $onelink->{'destinationfile'} = $newdestinationfile;
            }
        }
    }
}

########################################################################
# This function is a helper of function "assigning_modules_to_items"
########################################################################

sub insert_for_item ($$$)
{
    my ($hash, $item, $id) = @_;

    if (!defined $hash->{$item})
    {
        my @gids = ();
        $hash->{$item} = \@gids;
    }
    my $gid_list = $hash->{$item};
    push @{$gid_list}, $id;
    $hash->{$item} = $gid_list;
}

sub build_modulegids_table
{
    my ($modulesref, $itemname) = @_;

    my %module_lookup_table = ();

    # build map of item names to list of respective module gids
    # containing these items
    for my $onemodule (@{$modulesref})
    {
        next if ( ! defined $onemodule->{$itemname} );
        # these are the items contained in this module
        # eg. Files = (gid_a_b_c,gid_d_e_f)
        my $module_gids = $onemodule->{$itemname};

        # prune outer brackets
        $module_gids =~ s|^\s*\(||g;
        $module_gids =~ s|\)\s*$||g;
        for my $id (split (/,/, $module_gids))
        {
            chomp $id;
            insert_for_item(\%module_lookup_table, lc ($id), $onemodule->{'gid'});
        }
    }

    return \%module_lookup_table;
}

########################################################################
# Items like files do not know their modules
# This function is a helper of function "assigning_modules_to_items"
########################################################################

sub get_string_of_modulegids_for_itemgid
{
    my ($module_lookup_table, $modulesref, $itemgid, $itemname) = @_;

    my $allmodules = "";
    my $haslanguagemodule = 0;
    my %foundmodules = ();

    my $gid_list = $module_lookup_table->{lc($itemgid)};

    for my $gid (@{$gid_list})
    {
        $foundmodules{$gid} = 1;
        $allmodules = $allmodules . "," . $gid;
        # Is this module a language module? This info should be stored at the file.
        if ( exists($installer::globals::alllangmodules{$gid}) ) { $haslanguagemodule = 1; }
     }

    $allmodules =~ s/^\s*\,//;  # removing leading comma

    # Check: All modules or no module must have flag LANGUAGEMODULE
    if ( $haslanguagemodule )
    {
        my $isreallylanguagemodule = installer::worker::key_in_a_is_also_key_in_b(\%foundmodules, \%installer::globals::alllangmodules);
        if ( ! $isreallylanguagemodule ) { installer::exiter::exit_program("ERROR: \"$itemgid\" is assigned to modules with flag \"LANGUAGEMODULE\" and also to modules without this flag! Modules: $allmodules", "get_string_of_modulegids_for_itemgid");  }
    }

    return ($allmodules, $haslanguagemodule);
}

########################################################
# Items like files do not know their modules
# This function add the {'modules'} to these items
########################################################

sub assigning_modules_to_items
{
    my ($modulesref, $itemsref, $itemname) = @_;

    my $infoline = "";
    my $languageassignmenterror = 0;
    my @languageassignmenterrors = ();

    my $module_lookup_table = build_modulegids_table($modulesref, $itemname);

    for my $oneitem (@{$itemsref})
    {
        my $itemgid = $oneitem->{'gid'};

        my $styles = "";
        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }
        if (( $itemname eq "Dirs" ) && ( ! ( $styles =~ /\bCREATE\b/ ))) { next; }

        if ( $itemgid eq "" )
        {
            installer::exiter::exit_program("ERROR in item collection: No gid for item $oneitem->{'Name'}", "assigning_modules_to_items");
        }

        # every item can belong to many modules

        my ($modulegids, $haslanguagemodule) = get_string_of_modulegids_for_itemgid($module_lookup_table, $modulesref, $itemgid, $itemname);

        if ($modulegids eq "")
        {
            installer::exiter::exit_program("ERROR in file collection: No module found for $itemname $itemgid", "assigning_modules_to_items");
        }

        $oneitem->{'modules'} = $modulegids;
        $oneitem->{'haslanguagemodule'} = $haslanguagemodule;

        # Important check: "ismultilingual" and "haslanguagemodule" must have the same value !
        if (( $oneitem->{'ismultilingual'} ) && ( ! $oneitem->{'haslanguagemodule'} ))
        {
            $infoline = "Error: \"$oneitem->{'gid'}\" is multi lingual, but not in language pack (Assigned module: $modulegids)!\n";
            push( @installer::globals::globallogfileinfo, $infoline);
            push( @languageassignmenterrors, $infoline );
            $languageassignmenterror = 1;
        }
        if (( $oneitem->{'haslanguagemodule'} ) && ( ! $oneitem->{'ismultilingual'} ))
        {
            $infoline = "Error: \"$oneitem->{'gid'}\" is in language pack, but not multi lingual (Assigned module: $modulegids)!\n";
            push( @installer::globals::globallogfileinfo, $infoline);
            push( @languageassignmenterrors, $infoline );
            $languageassignmenterror = 1;
        }
    }

    if ($languageassignmenterror)
    {
        for ( my $i = 0; $i <= $#languageassignmenterrors; $i++ ) { print "$languageassignmenterrors[$i]"; }
        installer::exiter::exit_program("ERROR: Incorrect assignments for language packs.", "assigning_modules_to_items");
    }

}

#################################################################################################
# Root path (for instance /opt/openofficeorg20) needs to be added to directories, files and links
#################################################################################################

sub add_rootpath_to_directories
{
    my ($dirsref, $rootpath) = @_;

    for ( my $i = 0; $i <= $#{$dirsref}; $i++ )
    {
        my $onedir = ${$dirsref}[$i];
        my $dir = "";

        if ( $onedir->{'Dir'} ) { $dir = $onedir->{'Dir'}; }

        if (!($dir =~ /\bPREDEFINED_/ ))
        {
            my $hostname = $onedir->{'HostName'};
            $hostname = $rootpath . $installer::globals::separator . $hostname;
            $onedir->{'HostName'} = $hostname;
        }

        # added

        if ( $dir =~ /\bPREDEFINED_PROGDIR\b/ )
        {
            my $hostname = $onedir->{'HostName'};
            if ( $hostname eq "" ) { $onedir->{'HostName'} = $rootpath; }
            else { $onedir->{'HostName'} = $rootpath . $installer::globals::separator . $hostname; }
        }
    }
}

sub add_rootpath_to_files
{
    my ($filesref, $rootpath) = @_;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $destination = $onefile->{'destination'};
        $destination = $rootpath . $installer::globals::separator . $destination;
        $onefile->{'destination'} = $destination;
    }
}

sub add_rootpath_to_links
{
    my ($linksref, $rootpath) = @_;

    for ( my $i = 0; $i <= $#{$linksref}; $i++ )
    {
        my $onelink = ${$linksref}[$i];
        my $styles = $onelink->{'Styles'};

        my $destination = $onelink->{'destination'};
        $destination = $rootpath . $installer::globals::separator . $destination;
        $onelink->{'destination'} = $destination;

        if (!($styles =~ /\bRELATIVE\b/ )) # for absolute links
        {
            my $destinationfile = $onelink->{'destinationfile'};
            $destinationfile = $rootpath . $installer::globals::separator . $destinationfile;
            $onelink->{'destinationfile'} = $destinationfile;
        }
    }
}

#################################################################################
# Collecting all parent gids
#################################################################################

sub collect_all_parent_feature
{
    my ($modulesref) = @_;

    my @allparents = ();

    my $found_root_module = 0;

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];

        my $parentgid = "";
        if ( $onefeature->{'ParentID'} )
        {
            $parentgid = $onefeature->{'ParentID'};
        }

        if ( $parentgid ne "" )
        {
            if (! grep {$_ eq $parentgid} @allparents)
            {
                push(@allparents, $parentgid);
            }
        }

        # Setting the global root module

        if ( $parentgid eq "" )
        {
            if ( $found_root_module ) { installer::exiter::exit_program("ERROR: Only one module without ParentID or with empty ParentID allowed ($installer::globals::rootmodulegid, $onefeature->{'gid'}).", "collect_all_parent_feature"); }
            $installer::globals::rootmodulegid = $onefeature->{'gid'};
            $found_root_module = 1;
            $infoline = "Setting Root Module: $installer::globals::rootmodulegid\n";
            push( @installer::globals::globallogfileinfo, $infoline);
        }

        if ( ! $found_root_module ) { installer::exiter::exit_program("ERROR: Could not define root module. No module without ParentID or with empty ParentID exists.", "collect_all_parent_feature"); }

    }

    return \@allparents;
}

#################################################################################
# Checking for every feature, whether it has children
#################################################################################

sub set_children_flag
{
    my ($modulesref) = @_;

    my $allparents = collect_all_parent_feature($modulesref);

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];
        my $gid = $onefeature->{'gid'};

        # is this gid a parent?

        if ( grep {$_ eq $gid} @{$allparents} )
        {
            $onefeature->{'has_children'} = 1;
        }
        else
        {
            $onefeature->{'has_children'} = 0;
        }
    }
}

#################################################################################
# All modules, that use a template module, do now get the assignments of
# the template module.
#################################################################################

sub resolve_assigned_modules
{
    my ($modulesref) = @_;

    # collecting all template modules

    my %directaccess = ();

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];
        my $styles = "";
        if ( $onefeature->{'Styles'} ) { $styles = $onefeature->{'Styles'}; }
        if ( $styles =~ /\bTEMPLATEMODULE\b/ ) { $directaccess{$onefeature->{'gid'}} = $onefeature; }

        # also looking for module with flag ROOT_BRAND_PACKAGE, to save is for further usage
        if ( $styles =~ /\bROOT_BRAND_PACKAGE\b/ )
        {
            $installer::globals::rootbrandpackage = $onefeature->{'gid'};
            $installer::globals::rootbrandpackageset = 1;
        }
    }

    # looking, where template modules are assigned

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];
        if ( $onefeature->{'Assigns'} )
        {
            my $templategid = $onefeature->{'Assigns'};

            if ( ! exists($directaccess{$templategid}) )
            {
                installer::exiter::exit_program("ERROR: Did not find definition of assigned template module \"$templategid\"", "resolve_assigned_modules");
            }

            # Currently no merging of Files, Dirs, ...
            # This has to be included here, if it is required
            my $item;
            foreach $item (@installer::globals::items_at_modules)
            {
                if ( exists($directaccess{$templategid}->{$item}) ) { $onefeature->{$item} = $directaccess{$templategid}->{$item}; }
            }
        }
    }
}

#################################################################################
# Removing the template modules from the list, after all
# assignments are transferred to the "real" modules.
#################################################################################

sub remove_template_modules
{
    my ($modulesref) = @_;

    my @modules = ();

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];
        my $styles = "";
        if ( $onefeature->{'Styles'} ) { $styles = $onefeature->{'Styles'}; }
        if ( $styles =~ /\bTEMPLATEMODULE\b/ ) { next; }

        push(@modules, $onefeature);
    }

    return \@modules;
}

#################################################################################
# Collecting all modules with flag LANGUAGEMODULE in a global
# collector.
#################################################################################

sub collect_all_languagemodules
{
    my ($modulesref) = @_;

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];
        my $styles = "";
        if ( $onefeature->{'Styles'} ) { $styles = $onefeature->{'Styles'}; }
        if ( $styles =~ /\bLANGUAGEMODULE\b/ )
        {
            if ( ! exists($onefeature->{'Language'}) ) { installer::exiter::exit_program("ERROR: \"$onefeature->{'gid'}\" has flag LANGUAGEMODULE, but does not know its language!", "collect_all_languagemodules"); }
            $installer::globals::alllangmodules{$onefeature->{'gid'}} = $onefeature->{'Language'};
            # Collecting also the english names, that are used for nsis unpack directory for language packs
            my $lang = $onefeature->{'Language'};
            my $name = "";
            foreach my $localkey ( keys %{$onefeature} )
            {
                if ( $localkey =~ /^\s*Name\s*\(\s*en-US\s*\)\s*$/ )
                {
                    $installer::globals::all_english_languagestrings{$lang} = $onefeature->{$localkey};
                }
            }
        }
    }
}

#################################################################################
# Selecting from all collected english language strings those, that are really
# required in this installation set.
#################################################################################

sub select_required_language_strings
{
    my ($modulesref) = @_;

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onefeature = ${$modulesref}[$i];
        my $styles = "";
        if ( $onefeature->{'Styles'} ) { $styles = $onefeature->{'Styles'}; }
        if ( $styles =~ /\bLANGUAGEMODULE\b/ )
        {
            if ( ! exists($onefeature->{'Language'}) ) { installer::exiter::exit_program("ERROR: \"$onefeature->{'gid'}\" has flag LANGUAGEMODULE, but does not know its language!", "select_required_language_strings"); }
            my $lang = $onefeature->{'Language'};

            if (( exists($installer::globals::all_english_languagestrings{$lang}) ) && ( ! exists($installer::globals::all_required_english_languagestrings{$lang}) ))
            {
                $installer::globals::all_required_english_languagestrings{$lang} = $installer::globals::all_english_languagestrings{$lang};
            }
        }
    }
}

1;
