#*************************************************************************
#
#   $RCSfile: scriptitems.pm,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: cp $ $Date: 2004-08-05 17:05:04 $
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

package installer::scriptitems;

use installer::converter;
use installer::existence;
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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::resolve_all_directory_names : $#{$directoryarrayref}"); }

    # After this procedure the hash shall contain the complete language
    # dependent path, not only the language dependent HostName.

    my ($key, $value, $parentvalue, $parentgid, $parentdirectoryhashref);

    for ( my $i = 0; $i <= $#{$directoryarrayref}; $i++ )
    {
        my $directoryhashref = ${$directoryarrayref}[$i];
        my $gid = $directoryhashref-> {'gid'};
        my $parentid = $directoryhashref-> {'ParentID'};

        if (!( $parentid eq "PREDEFINED_PROGDIR" ))
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
                    # the key ("HostName (49)") must be usable for both hashes

                    if ( $key =~ /\bHostName\b/ )
                    {
                        $value = $directoryhashref->{$key};
                        $parentvalue = $parentdirectoryhashref->{$key};
                        $directoryhashref->{$key} = $parentvalue . $installer::globals::separator . $value;
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
# Files with flag DELETE_ONLY do not need to be packed into installation set
#############################################################################

sub remove_delete_only_files_from_productlists
{
    my ($productarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_delete_only_files_from_productlists : $#{$productarrayref}"); }

    my @newitems = ();

    for ( my $i = 0; $i <= $#{$productarrayref}; $i++ )
    {
        my $oneitem = ${$productarrayref}[$i];
        my $styles = "";

        if ( $oneitem->{'Styles'} ) { $styles = $oneitem->{'Styles'}; }

        if (!($styles =~ /\bDELETE_ONLY\b/))
        {
            push(@newitems, $oneitem);
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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_uninstall_regitems_from_script : $#{$registryarrayref}"); }

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
# Removing all items in product lists which do not have the correct languages
##############################################################################

sub resolving_all_languages_in_productlists
{
    my ($productarrayref, $languagesarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::resolving_all_languages_in_productlists : $#{$productarrayref} : $#{$languagesarrayref}"); }

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

                push(@itemsinalllanguages, \%oneitemhash);
            }
        }
    }

    return \@itemsinalllanguages;
}

################################################################################
# Simplifying the name for language dependent items from "Name (xy)" to "Name"
################################################################################

sub changing_name_of_langugage_dependent_keys
{
    my ($itemsarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::changing_name_of_langugage_dependent_keys : $#{$itemsarrayref}"); }

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
# <productkey>, <buildid>, <sequence_languages>, <productcode>, <upgradecode>
################################################################################

sub replace_setup_variables
{
    my ($itemsarrayref, $languagestringref, $hashref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::replace_setup_variables : $#{$itemsarrayref} : $$languagestringref : $hashref->{'PRODUCTNAME'}"); }

    my $languagesstring = $$languagestringref;
    $languagesstring =~ s/\_/ /g;   # replacing underscore with whitespace
    # $languagesstring is "01 49" instead of "en-US de"
    installer::languages::fake_languagesstring(\$languagesstring);

    my $productname = $hashref->{'PRODUCTNAME'};
    my $productversion = $hashref->{'PRODUCTVERSION'};
    my $productkey = $productname . " " . $productversion;

    # string $buildid, which is used to replace the setup variable <buildid>

    # doesn't contain buildid if not built without "-m $LAST_MINOR"
    # my $buildidstring = $installer::globals::build . $installer::globals::minor . "(Build:" . $installer::globals::buildid . ")";
    my $buildidstring = $installer::globals::build . $ENV{LAST_MINOR} . "(Build:" . $installer::globals::buildid . ")";

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $value = $oneitem->{'Value'};

        $value =~ s/\<buildid\>/$buildidstring/;
        $value =~ s/\<sequence_languages\>/$languagesstring/;
        $value =~ s/\<productkey\>/$productkey/;
        $value =~ s/\<productcode\>/$installer::globals::productcode/;
        $value =~ s/\<upgradecode\>/$installer::globals::upgradecode/;

        $oneitem->{'Value'} = $value;
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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_non_existent_languages_in_productlists : $#{$itemsarrayref} : $$languagestringref : $searchkey : $itemtype"); }

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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::get_Directoryname_From_Directorygid : $#{$dirsarrayref} : $searchgid : $onelanguage"); }

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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::get_Destination_Directory_For_Item_From_Directorylist : $#{$itemarrayref} : $#{$dirsarrayref}"); }

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
# Input is one file name, output the complete absolute path of this file
##########################################################################

sub get_sourcepath_from_filename_and_includepath
{
    my ($searchfilenameref, $includepatharrayref, $write_logfile) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::get_sourcepath_from_filename_and_includepath : $$searchfilenameref : $#{$includepatharrayref} : $write_logfile"); }

    my ($onefile, $includepath, $infoline);

    my $foundsourcefile = 0;
    my $foundnewname = 0;

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

    if (!($foundsourcefile))    # testing with lowercase filename
    {
        # Attention: README01.html is copied for Windows to readme01.html, not case sensitive

        for ( my $j = 0; $j <= $#{$includepatharrayref}; $j++ )
        {
            $includepath = ${$includepatharrayref}[$j];
            installer::remover::remove_leading_and_ending_whitespaces(\$includepath);

            my $newfilename = $$searchfilenameref;

            $newfilename =~ s/readme/README/;       # special handling for readme files
            $newfilename =~ s/license/LICENSE/;     # special handling for license files

            $onefile = $includepath . $installer::globals::separator . $newfilename;

            if ( -f $onefile )
            {
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
            $infoline = "ERROR: Source for $$searchfilenameref not found!\n";   # Important error message in log file
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
# Getting all source pathes for all files to be packed
# $item can be "Files" or "ScpActions"
##############################################################

sub get_Source_Directory_For_Files_From_Includepathlist
{
    my ($filesarrayref, $includepatharrayref, $item) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::get_Source_Directory_For_Files_From_Includepathlist : $#{$filesarrayref} : $#{$includepatharrayref} : $item"); }

    installer::logger::include_header_into_logfile("$item:");

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $onelanguage = $onefile->{'specificlanguage'};

        my $onefilename = $onefile->{'Name'};

        $onefilename =~ s/^\s*\Q$installer::globals::separator\E//;     # filename begins with a slash, for instance /registry/schema/org/openoffice/VCL.xcs

        my $sourcepathref = get_sourcepath_from_filename_and_includepath(\$onefilename, $includepatharrayref, 1);

        $onefile->{'sourcepath'} = $$sourcepathref; # This $$sourcepathref is empty, if no source was found
    }

    my $infoline = "\n";    # empty line after listing of all files
    push( @installer::globals::logfileinfo, $infoline);
}

#################################################################################
# Removing files, that are not part of ada products
#################################################################################

sub remove_Files_For_Ada_Products
{
    my ($filesarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_Files_For_Ada_Products : $#{$filesarrayref}"); }

    my $infoline;

    my @newfilesarray = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];

        my $filename = $onefile->{'Name'};

        # only adabas.zip and license and readme files for Ada products.

        if (($filename eq "adabas.zip") || ($filename =~ /license/i) || ($filename =~ /readme/i) || ($filename =~ /services.bat/i))
        {
            push(@newfilesarray, $onefile);
        }
        else
        {
            $infoline = "Warning: Removing file $filename from file list for Ada product.\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

    return \@newfilesarray;
}

#################################################################################
# Files, whose source directory is not found, are removed now (this is an ERROR)
#################################################################################

sub remove_Files_Without_Sourcedirectory
{
    my ($filesarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_Files_Without_Sourcedirectory : $#{$filesarrayref}"); }

    my $infoline;

    my $error_occured = 0;
    my @missingfiles = ();
    push(@missingfiles, "ERROR: The following files could not be found: \n");

    my @newfilesarray = ();

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $sourcepath = $onefile->{'sourcepath'};

        if ($sourcepath eq "")
        {
            my $filename = $onefile->{'Name'};
            $infoline = "ERROR: Removing file $filename from file list.\n";
            push( @installer::globals::logfileinfo, $infoline);

            push(@missingfiles, "ERROR: File not found: $filename\n");
            $error_occured = 1;

            next;   # removing this file from list, if sourcepath is empty
        }

        push(@newfilesarray, $onefile);
    }

    $infoline = "\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $error_occured )
    {
        for ( my $i = 0; $i <= $#missingfiles; $i++ ) { print "$missingfiles[$i]"; }
        installer::exiter::exit_program("ERROR: Missing files", "remove_Files_Without_Sourcedirectory");
    }

    return \@newfilesarray;
}

############################################################################
# License and Readme files in the default language have to be installed
# in the installation root (next to the program dir). This is in scp
# project done by a post install basic script
############################################################################

sub add_License_Files_into_Installdir
{
    my ($filesarrayref, $languagesarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::add_License_Files_into_Installdir : $#{$filesarrayref} : $#{$languagesarrayref}"); }

    my $infoline;

    my @newfilesarray = ();

    my $defaultlanguage = installer::languages::get_default_language($languagesarrayref);

    # copy all files from directory share/readme, that contain the default language in their name
    # without default language into the installation root. This makes the settings of the correct
    # file names superfluous. On the other hand this requires a dependency to the directory
    # share/readme

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $destination = $onefile->{'destination'};

        if ( $destination =~ /^\s*share\Q$installer::globals::separator\Ereadme\Q$installer::globals::separator\E(\w+?)_?$defaultlanguage\.?(\w*)\s*/ )
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
            $newfile->{'Dir'} = "PREDEFINED_PROGDIR";
            $newfile->{'destination'} = $newfilename;
            $newfile->{'ismultilingual'} = "0";
            $newfile->{'specificlanguage'} = "";

            push(@newfilesarray, $newfile);

            $infoline = "New files: Adding file $newfilename for the installation root to the file list. Language: $defaultlanguage\n";
            push( @installer::globals::logfileinfo, $infoline);
        }

        push(@newfilesarray, $onefile);
    }

    return \@newfilesarray;
}

############################################################################
# Removing all scpactions, that have no name.
# See: FlatLoaderZip
############################################################################

sub remove_scpactions_without_name
{
    my ($itemsarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_scpactions_without_name : $#{$itemsarrayref}"); }

    my $infoline;

    my @newitemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $name = "";

        if ( $oneitem->{'Name'} ) { $name = $oneitem->{'Name'}; }

        if  ( $name eq "" )
        {
            $infoline = "ATTENTION: Removing scpaction $oneitem->{'gid'} from the installation set.\n";
            push( @installer::globals::logfileinfo, $infoline);
            next;
        }

        push(@newitemsarray, $oneitem);
    }

    return \@newitemsarray;
}

############################################################################
# Because of the item "File" the source name must be "Name". Therefore
# "Copy" is changed to "Name" and "Name" is changed to "DestinationName".
############################################################################

sub change_keys_of_scpactions
{
    my ($itemsarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::change_keys_of_scpactions : $#{$itemsarrayref}"); }

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];

        $oneitem->{'DestinationName'} = $oneitem->{'Name'};     # Name cannot be multilingual (!?)
        delete($oneitem->{'Name'});

        my $key;

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
# Removing all setup files from installation set.
# This can be removed, if the files and links are
# also removed in scp project.
# set-dll, set-res, setup-link
# gid_File_Lib_Set, gid_File_Res_Set, gid_Shortcut_Setup
############################################################################

sub remove_Setup_from_Installset
{
    my ($itemsarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_Setup_from_Installset : $#{$itemsarrayref}"); }

    my $infoline;

    my @newitemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsarrayref}; $i++ )
    {
        my $oneitem = ${$itemsarrayref}[$i];
        my $gid = $oneitem->{'gid'};

        # scp Todo: Remove asap after removal of old setup

        if (( $gid eq "gid_File_Lib_Set" ) ||
            ( $gid eq "gid_File_Res_Set" ) ||
            ( $gid eq "gid_File_Lib_Tplx" ) ||
            ( $gid eq "gid_File_Res_Tplx" ) ||
            ( $gid eq "gid_File_Lib_Jvm" ) ||
            ( $gid eq "gid_File_Res_Jvm" ) ||
            ( $gid eq "gid_File_Lib_Usp" ) ||
            ( $gid eq "gid_File_Bin_Setup" ) ||
            ( $gid eq "gid_File_Binary_Setup" ) ||
            ( $gid eq "gid_File_Bin_Jre117" ) ||
            ( $gid eq "gid_File_Exe_Regsvrex" ) ||
            ( $gid eq "GID_FILE_RDB_SETUP_SERVICES" ) ||
            ( $gid eq "gid_File_Rdb_Setup_Services" ) ||
            ( $gid eq "gid_File_Rdb_Setup_Services_Rdb" ) ||
            ( $gid eq "gid_Shortcut_Setup" ) ||
            ( $gid eq "gid_File_Extra_Cdemath" ) ||
            ( $gid eq "gid_File_Extra_Cdewriter" ) ||
            ( $gid eq "gid_File_Extra_Cdecalc" ) ||
            ( $gid eq "gid_File_Extra_Cdedraw" ) ||
            ( $gid eq "gid_File_Extra_Cdeimpress" ) ||
            ( $gid eq "gid_File_Images_Zip_Setup" ))
        {
            $infoline = "ATTENTION: Removing setup item $oneitem->{'gid'} from the installation set.\n";
            push( @installer::globals::logfileinfo, $infoline);

            next;
        }

        push(@newitemsarray, $oneitem);
    }

    return \@newitemsarray;
}

############################################################################
# FAKE: Some files cotain a $ in their name. epm conflicts with such files.
# Quick solution: Renaming this files, converting "$" to "_"
# Still a ToDo ! :-)
############################################################################

sub rename_illegal_filenames
{
    my ($filesarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::rename_illegal_filenames : $#{$filesarrayref}"); }

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

            $filename =~ s/\$/\_/g;
            $destpath =~ s/\$/\_/g;
            my $oldsourcepath = $sourcepath;
            $sourcepath =~ s/\$/\_/g;

            my $infoline = "ATTENTION: Files: Renaming $onefile->{'Name'} to $filename\n";
            push( @installer::globals::logfileinfo, $infoline);
            $infoline = "ATTENTION: Files: Renaming $onefile->{'sourcepath'} to $sourcepath\n";
            push( @installer::globals::logfileinfo, $infoline);
            $infoline = "ATTENTION: Files: Renaming $onefile->{'destination'} to $destpath\n";
            push( @installer::globals::logfileinfo, $infoline);

            $onefile->{'Name'} = $filename;
            $onefile->{'sourcepath'} = $sourcepath;
            $onefile->{'destination'} = $destpath;

            if (! -f $sourcepath)
            {
                installer::systemactions::copy_one_file($oldsourcepath, $sourcepath);
            }
        }
    }
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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::collect_directories_from_filesarray : $#{$filesarrayref}"); }

    my @alldirectories = ();

    my $predefinedprogdir_added = 0;
    my $searchkey = "HostName";

    # Preparing this already as hash, although the only needed value at the moment is the HostName
    # But also adding: "specificlanguage" and "Dir" (for instance gid_Dir_Program)

    for ( my $i = 0; $i <= $#{$filesarrayref}; $i++ )
    {
        my $onefile = ${$filesarrayref}[$i];
        my $destinationpath = $onefile->{'destination'};
        installer::pathanalyzer::get_path_from_fullqualifiedname(\$destinationpath);
        $destinationpath =~ s/\Q$installer::globals::separator\E\s*$//;     # removing ending slashes or backslashes

        my $alreadyincluded = installer::existence::exists_in_array_of_hashes($searchkey, $destinationpath, \@alldirectories);

        if (!($alreadyincluded))
        {
            my %directoryhash = ();
            $directoryhash{'HostName'} = $destinationpath;
            $directoryhash{'specificlanguage'} = $onefile->{'specificlanguage'};
            $directoryhash{'Dir'} = $onefile->{'Dir'};

            if ( $onefile->{'Dir'} eq "PREDEFINED_PROGDIR" ) { $predefinedprogdir_added = 1; }

            push(@alldirectories, \%directoryhash);

            # Problem: The $destinationpath can be share/registry/schema/org/openoffice
            # but not all directories contain files and will be added to this list.
            # Therefore the path has to be analyzed.

            while ( $destinationpath =~ /(^.*\S)\Q$installer::globals::separator\E(\S.*?)\s*$/ )    # as long as the path contains slashes
            {
                $destinationpath = $1;

                $alreadyincluded = installer::existence::exists_in_array_of_hashes($searchkey, $destinationpath, \@alldirectories);

                if (!($alreadyincluded))
                {
                    my %directoryhash = ();

                    $directoryhash{'HostName'} = $destinationpath;
                    $directoryhash{'specificlanguage'} = $onefile->{'specificlanguage'};
                    $directoryhash{'Dir'} = $onefile->{'Dir'};

                    push(@alldirectories, \%directoryhash);
                }
            }
        }
    }

    # if there is no file in the root directory PREDEFINED_PROGDIR, it has to be included into the directory array now
    # HostName= specificlanguage=   Dir=PREDEFINED_PROGDIR

    if (! $predefinedprogdir_added )
    {
        my %directoryhash = ();
        $directoryhash{'HostName'} = "";
        $directoryhash{'specificlanguage'} = "";
        $directoryhash{'Dir'} = "PREDEFINED_PROGDIR";

        push(@alldirectories, \%directoryhash);
    }

    return \@alldirectories;
}

##################################
# Collecting directories: Part 2
##################################

sub collect_directories_with_create_flag_from_directoryarray
{
    my ($directoriesforepmarrayref, $directoryarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::collect_directories_with_create_flag_from_directoryarray : $#{$directoriesforepmarrayref} : $#{$directoryarrayref}"); }

    my $searchkey = "HostName";

    for ( my $i = 0; $i <= $#{$directoryarrayref}; $i++ )
    {
        my $onedir = ${$directoryarrayref}[$i];
        my $styles = "";

        if ( $onedir->{'Styles'} ) { $styles = $onedir->{'Styles'}; }

        if ( $styles =~ /\bCREATE\b/ )
        {
            my $directoryname = $onedir->{'HostName'};

            my $alreadyincluded = installer::existence::exists_in_array_of_hashes($searchkey, $directoryname, $directoriesforepmarrayref);

            if (!($alreadyincluded))
            {
                my %directoryhash = ();
                $directoryhash{'HostName'} = $directoryname;
                $directoryhash{'specificlanguage'} = $onedir->{'specificlanguage'};
                # $directoryhash{'gid'} = $onedir->{'gid'};
                $directoryhash{'Dir'} = $onedir->{'gid'};
                $directoryhash{'Styles'} = $onedir->{'Styles'};

                push(@{$directoriesforepmarrayref}, \%directoryhash);

                # Problem: The $destinationpath can be share/registry/schema/org/openoffice
                # but not all directories contain files and will be added to this list.
                # Therefore the path has to be analyzed.

                while ( $directoryname =~ /(^.*\S)\Q$installer::globals::separator\E(\S.*?)\s*$/ )  # as long as the path contains slashes
                {
                    $directoryname = $1;

                    $alreadyincluded = installer::existence::exists_in_array_of_hashes($searchkey, $directoryname, $directoriesforepmarrayref);

                    if (!($alreadyincluded))
                    {
                        my %directoryhash = ();

                        $directoryhash{'HostName'} = $directoryname;
                        $directoryhash{'specificlanguage'} = $onedir->{'specificlanguage'};
                        # $directoryhash{'gid'} = $onedir->{'gid'};
                        $directoryhash{'Dir'} = $onedir->{'gid'};
                        $directoryhash{'Styles'} = $onedir->{'Styles'};

                        push(@{$directoriesforepmarrayref}, \%directoryhash);
                    }
                }
            }
        }
    }
}

#################################################
# Determining the destination file of a link
#################################################

sub get_destination_file_path_for_links
{
    my ($linksarrayref, $filesarrayref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::get_destination_file_path_for_links : $#{$linksarrayref} : $#{$filesarrayref}"); }

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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::get_destination_link_path_for_links : $#{$linksarrayref}"); }

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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::remove_workstation_only_items : $#{$itemarrayref}"); }

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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::resolve_links_with_flag_relative : $#{$linksarrayref}"); }

    # Before this step is:
    # destination=program/libsalhelperC52.so.3, this will be the name of the link
    # destinationfile=program/libsalhelperC52.so.3, this will be the linked file or name
    # If the flag RELATIVE is set, the pathes have to be analyzed. If the flag is not set
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
# Items like files do not know their modules
# This function is a helper of function "assigning_modules_to_items"
########################################################################

sub get_string_of_modulegids_for_itemgid
{
    my ($modulesref, $itemgid, $itemname) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::get_string_of_modulegids_for_itemgid : $#{$modulesref} : $itemgid : $itemname"); }

    my $allmodules = "";

    for ( my $i = 0; $i <= $#{$modulesref}; $i++ )
    {
        my $onemodule = ${$modulesref}[$i];
        my $allitems = "";

        if ( $onemodule->{$itemname} ) { $allitems = $onemodule->{$itemname}; }

        if ( $allitems =~ /\b$itemgid\b/i )
        {
            $allmodules = $allmodules . "," . $onemodule->{'gid'};
        }
    }

    $allmodules =~ s/^\s*\,//;  # removing leading comma

    return $allmodules;
}

########################################################
# Items like files do not know their modules
# This function add the {'modules'} to these items
########################################################

sub assigning_modules_to_items
{
    my ($modulesref, $itemsref, $itemname) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::assigning_modules_to_items : $#{$modulesref} : $#{$itemsref} : $itemname"); }

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];
        my $itemgid = $oneitem->{'gid'};

        if ( $itemgid eq "" )
        {
            installer::exiter::exit_program("ERROR in item collection: No gid for item $oneitem->{'Name'}", "assigning_modules_to_items");
        }

        # every item can belong to many modules

        my $modulegids = get_string_of_modulegids_for_itemgid($modulesref, $itemgid, $itemname);

        if ($modulegids eq "")
        {
            installer::exiter::exit_program("ERROR in file collection: No module found for file $oneitem->{'Name'}", "assigning_modules_to_items");
        }

        $oneitem->{'modules'} = $modulegids;
    }
}

#################################################################################################
# Root path (for instance /opt/openofficeorg20) needs to be added to directories, files and links
#################################################################################################

sub add_rootpath_to_directories
{
    my ($dirsref, $rootpath) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::add_rootpath_to_directories : $#{$dirsref} : $rootpath"); }

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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::add_rootpath_to_files : $#{$filesref} : $rootpath"); }

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

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::scriptitems::add_rootpath_to_links : $#{$linksref} : $rootpath"); }

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

1;
