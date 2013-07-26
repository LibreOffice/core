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

package installer::windows::idtglobal;

use Cwd;
use installer::converter;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::pathanalyzer;
use installer::remover;
use installer::scriptitems;
use installer::systemactions;
use installer::windows::language;

##############################################################
# Shorten the gid for a feature.
# Attention: Maximum length is 38
##############################################################

sub shorten_feature_gid
{
    my ($stringref) = @_;

    $$stringref =~ s/gid_Module_/gm_/;
    $$stringref =~ s/_Extension_/_ex_/;
    $$stringref =~ s/_Root_/_r_/;
    $$stringref =~ s/_Prg_/_p_/;
    $$stringref =~ s/_Optional_/_o_/;
    $$stringref =~ s/_Tools_/_tl_/;
    $$stringref =~ s/_Wrt_Flt_/_w_f_/;
    $$stringref =~ s/_Productivity_/_pr_/;
#   $$stringref =~ s/_Replacement_/_rpl_/; # native373 fix
}

############################################
# Getting the next free number, that
# can be added.
# Sample: 01-44-~1.DAT, 01-44-~2.DAT, ...
############################################

sub get_next_free_number
{
    my ($name, $shortnamesref) = @_;

    my $counter = 0;
    my $dontsave = 0;
    my $alreadyexists;
    my ($newname, $shortname);

    do
    {
        $alreadyexists = 0;
        $counter++;
        $newname = $name . $counter;

        for ( my $i = 0; $i <= $#{$shortnamesref}; $i++ )
        {
            $shortname = ${$shortnamesref}[$i];

            if ( uc($shortname) eq uc($newname) )   # case insensitive
            {
                $alreadyexists = 1;
                last;
            }
        }
    }
    until (!($alreadyexists));

    if (( $counter > 9 ) && ( length($name) > 6 )) { $dontsave = 1; }
    if (( $counter > 99 ) && ( length($name) > 5 )) { $dontsave = 1; }

    if (!($dontsave))
    {
        push(@{$shortnamesref}, $newname);  # adding the new shortname to the array of shortnames
    }

    return $counter
}

############################################
# Getting the next free number, that
# can be added.
# Sample: 01-44-~1.DAT, 01-44-~2.DAT, ...
############################################

sub get_next_free_number_with_hash
{
    my ($name, $shortnamesref, $ext) = @_;

    my $counter = 0;
    my $dontsave = 0;
    my $saved = 0;
    my $alreadyexists;
    my ($newname, $shortname);

    do
    {
        $alreadyexists = 0;
        $counter++;
        $newname = $name . $counter;
        $newname = uc($newname);    # case insensitive, always upper case
        if ( exists($shortnamesref->{$newname}) ||
             exists($installer::globals::savedrev83mapping{$newname.$ext}) )
        {
            $alreadyexists = 1;
        }
    }
    until (!($alreadyexists));

    if (( $counter > 9 ) && ( length($name) > 6 )) { $dontsave = 1; }
    if (( $counter > 99 ) && ( length($name) > 5 )) { $dontsave = 1; }

    if (!($dontsave))
    {
        $shortnamesref->{$newname} = 1; # adding the new shortname to the array of shortnames, always uppercase
        $saved = 1;
    }

    return ( $counter, $saved )
}

#########################################
# 8.3 for filenames and directories
#########################################

sub make_eight_three_conform
{
    my ($inputstring, $pattern, $shortnamesref) = @_;

    # all shortnames are collected in $shortnamesref, because of uniqueness

    my ($name, $namelength, $number);
    my $conformstring = "";
    my $changed = 0;

    if (( $inputstring =~ /^\s*(.*?)\.(.*?)\s*$/ ) && ( $pattern eq "file" ))   # files with a dot
    {
        $name = $1;
        my $extension = $2;

        $namelength = length($name);
        my $extensionlength = length($extension);

        if ( $extensionlength > 3 )
        {
            # simply taking the first three letters
            $extension = substr($extension, 0, 3);  # name, offset, length
        }

        # Attention: readme.html -> README~1.HTM

        if (( $namelength > 8 ) || ( $extensionlength > 3 ))
        {
            # taking the first six letters
            $name = substr($name, 0, 6);    # name, offset, length
            $name =~ s/\s*$//; # removing ending whitespaces
            $name = $name . "\~";
            $number = get_next_free_number($name, $shortnamesref);

            # if $number>9 the new name would be "abcdef~10.xyz", which is 9+3, and therefore not allowed

            if ( $number > 9 )
            {
                $name = substr($name, 0, 5);    # name, offset, length
                $name =~ s/\s*$//; # removing ending whitespaces
                $name = $name . "\~";
                $number = get_next_free_number($name, $shortnamesref);

                if ( $number > 99 )
                {
                    $name = substr($name, 0, 4);    # name, offset, length
                    $name =~ s/\s*$//; # removing ending whitespaces
                    $name = $name . "\~";
                    $number = get_next_free_number($name, $shortnamesref);
                }
            }

            $name = $name . "$number";

            $changed = 1;
        }

        $conformstring = $name . "\." . $extension;

        if ( $changed ) { $conformstring= uc($conformstring); }
    }
    else        # no dot in filename or directory (also used for shortcuts)
    {
        $name = $inputstring;
        $namelength = length($name);

        if ( $namelength > 8 )
        {
            # taking the first six letters
            $name = substr($name, 0, 6);    # name, offset, length
            $name =~ s/\s*$//; # removing ending whitespaces
            $name = $name . "\~";
            $number = get_next_free_number($name, $shortnamesref);

            # if $number>9 the new name would be "abcdef~10.xyz", which is 9+3, and therefore not allowed

            if ( $number > 9 )
            {
                $name = substr($name, 0, 5);    # name, offset, length
                $name =~ s/\s*$//; # removing ending whitespaces
                $name = $name . "\~";
                $number = get_next_free_number($name, $shortnamesref);

                if ( $number > 99 )
                {
                    $name = substr($name, 0, 4);    # name, offset, length
                    $name =~ s/\s*$//; # removing ending whitespaces
                    $name = $name . "\~";
                    $number = get_next_free_number($name, $shortnamesref);
                }
            }

            $name = $name . "$number";
            $changed = 1;
            if ( $pattern eq "dir" ) { $name =~ s/\./\_/g; }    # in directories replacing "." with "_"
        }

        $conformstring = $name;

        if ( $changed ) { $conformstring = uc($name); }
    }

    return $conformstring;
}

#########################################
# 8.3 for filenames and directories
# $shortnamesref is a hash in this case
# -> performance reasons
#########################################

sub make_eight_three_conform_with_hash
{
    my ($inputstring, $pattern, $shortnamesref) = @_;

    # all shortnames are collected in $shortnamesref, because of uniqueness (a hash!)

    my ($name, $namelength, $number);
    my $conformstring = "";
    my $changed = 0;
    my $saved;

    if (( $inputstring =~ /^\s*(.*)\.(.*?)\s*$/ ) && ( $pattern eq "file" ))    # files with a dot
    {
        # extension has to be non-greedy, but name is. This is important to find the last dot in the filename
        $name = $1;
        my $extension = $2;

        if ( $name =~ /^\s*(.*?)\s*$/ ) { $name = $1; } # now the name is also non-greedy
        $name =~ s/\.//g; # no dots in 8+3 conform filename

        $namelength = length($name);
        my $extensionlength = length($extension);

        if ( $extensionlength > 3 )
        {
            # simply taking the first three letters
            $extension = substr($extension, 0, 3);  # name, offset, length
            $changed = 1;
        }

        # Attention: readme.html -> README~1.HTM

        if (( $namelength > 8 ) || ( $extensionlength > 3 ))
        {
            # taking the first six letters, if filename is longer than 6 characters
            if ( $namelength > 6 )
            {
                $name = substr($name, 0, 6);    # name, offset, length
                $name =~ s/\s*$//; # removing ending whitespaces
                $name = $name . "\~";
                ($number, $saved) = get_next_free_number_with_hash($name, $shortnamesref, '.'.uc($extension));

                # if $number>9 the new name would be "abcdef~10.xyz", which is 9+3, and therefore not allowed

                if ( ! $saved )
                {
                    $name = substr($name, 0, 5);    # name, offset, length
                    $name =~ s/\s*$//; # removing ending whitespaces
                    $name = $name . "\~";
                    ($number, $saved) = get_next_free_number_with_hash($name, $shortnamesref, '.'.uc($extension));

                    # if $number>99 the new name would be "abcde~100.xyz", which is 9+3, and therefore not allowed

                    if ( ! $saved )
                    {
                        $name = substr($name, 0, 4);    # name, offset, length
                        $name =~ s/\s*$//; # removing ending whitespaces
                        $name = $name . "\~";
                        ($number, $saved) = get_next_free_number_with_hash($name, $shortnamesref, '.'.uc($extension));

                        if ( ! $saved )
                        {
                            installer::exiter::exit_program("ERROR: Could not set 8+3 conform name for $inputstring !", "make_eight_three_conform_with_hash");
                        }
                    }
                }

                $name = $name . "$number";
                $changed = 1;
            }
        }

        $conformstring = $name . "\." . $extension;

        if ( $changed ) { $conformstring= uc($conformstring); }
    }
    else        # no dot in filename or directory (also used for shortcuts)
    {
        $name = $inputstring;
        $namelength = length($name);

        if ( $namelength > 8 )
        {
            # taking the first six letters
            $name = substr($name, 0, 6);    # name, offset, length
            $name =~ s/\s*$//; # removing ending whitespaces
            $name = $name . "\~";
            ( $number, $saved ) = get_next_free_number_with_hash($name, $shortnamesref, '');

            # if $number>9 the new name would be "abcdef~10", which is 9+0, and therefore not allowed

            if ( ! $saved )
            {
                $name = substr($name, 0, 5);    # name, offset, length
                $name =~ s/\s*$//; # removing ending whitespaces
                $name = $name . "\~";
                ( $number, $saved ) = get_next_free_number_with_hash($name, $shortnamesref, '');

                # if $number>99 the new name would be "abcde~100", which is 9+0, and therefore not allowed

                if ( ! $saved )
                {
                    $name = substr($name, 0, 4);    # name, offset, length
                    $name =~ s/\s*$//; # removing ending whitespaces
                    $name = $name . "\~";
                    ( $number, $saved ) = get_next_free_number_with_hash($name, $shortnamesref, '');

                    if ( ! $saved ) { installer::exiter::exit_program("ERROR: Could not set 8+3 conform name for $inputstring !", "make_eight_three_conform_with_hash"); }
                }
            }

            $name = $name . "$number";
            $changed = 1;
            if ( $pattern eq "dir" ) { $name =~ s/\./\_/g; }    # in directories replacing "." with "_"
        }

        $conformstring = $name;

        if ( $changed ) { $conformstring = uc($name); }
    }

    return $conformstring;
}

#########################################
# Writing the header for idt files
#########################################

sub write_idt_header
{
    my ($idtref, $definestring) = @_;

    my $oneline;

    if ( $definestring eq "file" )
    {
        $oneline = "File\tComponent_\tFileName\tFileSize\tVersion\tLanguage\tAttributes\tSequence\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts72\tl255\ti4\tS72\tS20\tI2\ti4\n";
        push(@{$idtref}, $oneline);
        $oneline = "File\tFile\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "filehash" )
    {
        $oneline = "File_\tOptions\tHashPart1\tHashPart2\tHashPart3\tHashPart4\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ti2\ti4\ti4\ti4\ti4\n";
        push(@{$idtref}, $oneline);
        $oneline = "MsiFileHash\tFile_\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "directory" )
    {
        $oneline = "Directory\tDirectory_Parent\tDefaultDir\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\tS72\tl255\n";
        push(@{$idtref}, $oneline);
        $oneline = "Directory\tDirectory\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "component" )
    {
        $oneline = "Component\tComponentId\tDirectory_\tAttributes\tCondition\tKeyPath\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\tS38\ts72\ti2\tS255\tS72\n";
        push(@{$idtref}, $oneline);
        $oneline = "Component\tComponent\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "feature" )
    {
        $oneline = "Feature\tFeature_Parent\tTitle\tDescription\tDisplay\tLevel\tDirectory_\tAttributes\n";
        push(@{$idtref}, $oneline);
        $oneline = "s38\tS38\tL64\tL255\tI2\ti2\tS72\ti2\n";
        push(@{$idtref}, $oneline);
        $oneline = "65001\tFeature\tFeature\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "featurecomponent" )
    {
        $oneline = "Feature_\tComponent_\n";
        push(@{$idtref}, $oneline);
        $oneline = "s38\ts72\n";
        push(@{$idtref}, $oneline);
        $oneline = "FeatureComponents\tFeature_\tComponent_\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "media" )
    {
        $oneline = "DiskId\tLastSequence\tDiskPrompt\tCabinet\tVolumeLabel\tSource\n";
        push(@{$idtref}, $oneline);
        $oneline = "i2\ti4\tL64\tS255\tS32\tS72\n";
        push(@{$idtref}, $oneline);
        $oneline = "Media\tDiskId\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "font" )
    {
        $oneline = "File_\tFontTitle\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\tS128\n";
        push(@{$idtref}, $oneline);
        $oneline = "Font\tFile_\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "shortcut" )
    {
        $oneline = "Shortcut\tDirectory_\tName\tComponent_\tTarget\tArguments\tDescription\tHotkey\tIcon_\tIconIndex\tShowCmd\tWkDir\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts72\tl128\ts72\ts72\tS255\tL255\tI2\tS72\tI2\tI2\tS72\n";
        push(@{$idtref}, $oneline);
        $oneline = "65001\tShortcut\tShortcut\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "msishortcutproperty" )
    {
        $oneline = "MsiShortcutProperty\tShortcut_\tPropertyKey\tPropVariantValue\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts72\ts255\ts255\n";
        push(@{$idtref}, $oneline);
        $oneline = "MsiShortcutProperty\tMsiShortcutProperty\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "registry" )
    {
        $oneline = "Registry\tRoot\tKey\tName\tValue\tComponent_\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ti2\tl255\tL255\tL0\ts72\n";
        push(@{$idtref}, $oneline);
        $oneline = "Registry\tRegistry\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "createfolder" )
    {
        $oneline = "Directory_\tComponent_\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts72\n";
        push(@{$idtref}, $oneline);
        $oneline = "CreateFolder\tDirectory_\tComponent_\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "removefile" )
    {
        $oneline = "FileKey\tComponent_\tFileName\tDirProperty\tInstallMode\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts72\tL255\ts72\ti2\n";
        push(@{$idtref}, $oneline);
        $oneline = "RemoveFile\tFileKey\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "upgrade" )
    {
        $oneline = "UpgradeCode\tVersionMin\tVersionMax\tLanguage\tAttributes\tRemove\tActionProperty\n";
        push(@{$idtref}, $oneline);
        $oneline = "s38\tS20\tS20\tS255\ti4\tS255\ts72\n";
        push(@{$idtref}, $oneline);
        $oneline = "Upgrade\tUpgradeCode\tVersionMin\tVersionMax\tLanguage\tAttributes\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "icon" )
    {
        $oneline = "Name\tData\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\tv0\n";
        push(@{$idtref}, $oneline);
        $oneline = "Icon\tName\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "inifile" )
    {
        $oneline = "IniFile\tFileName\tDirProperty\tSection\tKey\tValue\tAction\tComponent_\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\tl255\tS72\tl96\tl128\tl255\ti2\ts72\n";
        push(@{$idtref}, $oneline);
        $oneline = "IniFile\tIniFile\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "msiassembly" )
    {
        $oneline = "Component_\tFeature_\tFile_Manifest\tFile_Application\tAttributes\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts38\tS72\tS72\tI2\n";
        push(@{$idtref}, $oneline);
        $oneline = "MsiAssembly\tComponent_\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "msiassemblyname" )
    {
        $oneline = "Component_\tName\tValue\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts255\ts255\n";
        push(@{$idtref}, $oneline);
        $oneline = "MsiAssemblyName\tComponent_\tName\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "appsearch" )
    {
        $oneline = "Property\tSignature_\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts72\n";
        push(@{$idtref}, $oneline);
        $oneline = "AppSearch\tProperty\tSignature_\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "reglocat" )
    {
        $oneline = "Signature_\tRoot\tKey\tName\tType\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ti2\ts255\tS255\tI2\n";
        push(@{$idtref}, $oneline);
        $oneline = "RegLocator\tSignature_\n";
        push(@{$idtref}, $oneline);
    }

    if ( $definestring eq "signatur" )
    {
        $oneline = "Signature\tFileName\tMinVersion\tMaxVersion\tMinSize\tMaxSize\tMinDate\tMaxDate\tLanguages\n";
        push(@{$idtref}, $oneline);
        $oneline = "s72\ts255\tS20\tS20\tI4\tI4\tI4\tI4\tS255\n";
        push(@{$idtref}, $oneline);
        $oneline = "Signature\tSignature\n";
        push(@{$idtref}, $oneline);
    }

}

##############################################################
# Returning the name of the rranslation file for a
# given language.
# Sample: "01" oder "en-US" -> "1033.txt"
##############################################################

sub get_languagefilename
{
    my ($idtfilename, $basedir) = @_;

    $idtfilename =~ s/\.idt/\.ulf/;

    my $languagefilename = $basedir . $installer::globals::separator . $idtfilename;

    return $languagefilename;
}

##############################################################
# Returning the complete block in all languages
# for a specified string
##############################################################

sub get_language_block_from_language_file
{
    my ($searchstring, $languagefile) = @_;

    my @language_block = ();

    for ( my $i = 0; $i <= $#{$languagefile}; $i++ )
    {
        if ( ${$languagefile}[$i] =~ /^\s*\[\s*$searchstring\s*\]\s*$/ )
        {
            my $counter = $i;

            push(@language_block, ${$languagefile}[$counter]);
            $counter++;

            while (( $counter <= $#{$languagefile} ) && (!( ${$languagefile}[$counter] =~ /^\s*\[/ )))
            {
                push(@language_block, ${$languagefile}[$counter]);
                $counter++;
            }

            last;
        }
    }

    return \@language_block;
}

##############################################################
# Returning a specific language string from the block
# of all translations
##############################################################

sub get_language_string_from_language_block
{
    my ($language_block, $language, $oldstring) = @_;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$language_block}; $i++ )
    {
        if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
        {
            $newstring = $1;
            $newstring =~ s/\\\"/\"/g; #un-escape quotes, fdo#59321
            last;
        }
    }

    if ( $newstring eq "" )
    {
        $language = "en-US";    # defaulting to english

        for ( my $i = 0; $i <= $#{$language_block}; $i++ )
        {
            if ( ${$language_block}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
            {
                $newstring = $1;
                last;
            }
        }
    }

    return $newstring;
}

##############################################################
# Returning a specific code from the block
# of all codes. No defaulting to english!
##############################################################

sub get_code_from_code_block
{
    my ($codeblock, $language) = @_;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$codeblock}; $i++ )
    {
        if ( ${$codeblock}[$i] =~ /^\s*$language\s*\=\s*\"(.*)\"\s*$/ )
        {
            $newstring = $1;
            last;
        }
    }

    return $newstring;
}

##############################################################
# Translating an idt file
##############################################################

sub translate_idtfile
{
    my ($idtfile, $languagefile, $onelanguage) = @_;

    for ( my $i = 0; $i <= $#{$idtfile}; $i++ )
    {
        my @allstrings = ();

        my $oneline = ${$idtfile}[$i];

        while ( $oneline =~ /\b(OOO_\w+)\b/ )
        {
            my $replacestring = $1;
            push(@allstrings, $replacestring);
            $oneline =~ s/$replacestring//;
        }

        my $oldstring;

        foreach $oldstring (@allstrings)
        {
            my $language_block = get_language_block_from_language_file($oldstring, $languagefile);
            my $newstring = get_language_string_from_language_block($language_block, $onelanguage, $oldstring);

            ${$idtfile}[$i] =~ s/$oldstring/$newstring/;    # always substitute, even if $newstring eq "" (there are empty strings for control.idt)
        }
    }
}

##############################################################
# Copying all needed files to create a msi database
# into one language specific directory
##############################################################

sub prepare_language_idt_directory
{
    my ($destinationdir, $newidtdir, $onelanguage, $filesref, $iconfilecollector, $binarytablefiles, $allvariables) = @_;

    # Copying all idt-files from the source $installer::globals::idttemplatepath to the destination $destinationdir
    # Copying all files in the subdirectory "Binary"
    # Copying all files in the subdirectory "Icon"

    my $infoline = "";

    installer::systemactions::copy_directory($installer::globals::idttemplatepath, $destinationdir);

    if ( -d $installer::globals::idttemplatepath . $installer::globals::separator . "Binary")
    {
        installer::systemactions::create_directory($destinationdir . $installer::globals::separator . "Binary");
        installer::systemactions::copy_directory($installer::globals::idttemplatepath . $installer::globals::separator . "Binary", $destinationdir . $installer::globals::separator . "Binary");
    }

    installer::systemactions::create_directory($destinationdir . $installer::globals::separator . "Icon");

    if ( -d $installer::globals::idttemplatepath . $installer::globals::separator . "Icon")
    {
        installer::systemactions::copy_directory($installer::globals::idttemplatepath . $installer::globals::separator . "Icon", $destinationdir . $installer::globals::separator . "Icon");
    }

    # Copying all files in $iconfilecollector, that describe icons of folderitems

    for ( my $i = 0; $i <= $#{$iconfilecollector}; $i++ )
    {
        my $iconfilename = ${$iconfilecollector}[$i];
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$iconfilename);
        installer::systemactions::copy_one_file(${$iconfilecollector}[$i], $destinationdir . $installer::globals::separator . "Icon" . $installer::globals::separator . $iconfilename);
    }

    # Copying all files in $binarytablefiles in the binary directory

    for ( my $i = 0; $i <= $#{$binarytablefiles}; $i++ )
    {
        my $binaryfile = ${$binarytablefiles}[$i];
        my $binaryfilepath = $binaryfile->{'sourcepath'};
        my $binaryfilename = $binaryfilepath;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$binaryfilename);
        installer::systemactions::copy_one_file($binaryfilepath, $destinationdir . $installer::globals::separator . "Binary" . $installer::globals::separator . $binaryfilename);
    }

    # Copying all new created and language independent idt-files to the destination $destinationdir.
    # Example: "File.idt"

    installer::systemactions::copy_directory_with_fileextension($newidtdir, $destinationdir, "idt");

    # Copying all new created and language dependent idt-files to the destination $destinationdir.
    # Example: "Feature.idt.01"

    installer::systemactions::copy_directory_with_fileextension($newidtdir, $destinationdir, $onelanguage);
    installer::systemactions::rename_files_with_fileextension($destinationdir, $onelanguage);

}

##############################################################
# Returning the source path of the rtf licensefile for
# a specified language
##############################################################

sub get_rtflicensefilesource
{
    my ($language, $includepatharrayref) = @_;

    my $licensefilename = "license_" . $language . ".rtf";

    my $sourcefileref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, $includepatharrayref, 1);

    if ($$sourcefileref eq "") { installer::exiter::exit_program("ERROR: Could not find $licensefilename!", "get_rtflicensefilesource"); }

    my $infoline = "Using licensefile: $$sourcefileref\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $$sourcefileref;
}

##############################################################
# A simple converter to create a license txt string from
# the rtf format
##############################################################

sub make_string_licensetext
{
    my ($licensefile) = @_;

    my $rtf_licensetext = "";

    for ( my $i = 0; $i <= $#{$licensefile}; $i++ )
    {
        my $oneline = ${$licensefile}[$i];
        $oneline =~ s/\s*$//g;      # no whitespace at line end

        $rtf_licensetext = $rtf_licensetext .  $oneline . " ";
    }

    return $rtf_licensetext;
}

##############################################################
# Including the license text into the table control.idt
##############################################################

sub add_licensefile_to_database
{
    my ($licensefile, $controltable) = @_;

    # Nine tabs before the license text and two tabs after it
    # The license text has to be included into the dialog
    # LicenseAgreement into the control Memo.

    my $foundlicenseline = 0;
    my ($number, $line);

    for ( my $i = 0; $i <= $#{$controltable}; $i++ )
    {
        $line = ${$controltable}[$i];

        if ( $line =~ /^\s*\bLicenseAgreement\b\t\bMemo\t/ )
        {
            $foundlicenseline = 1;
            $number = $i;
            last;
        }
    }

    if (!($foundlicenseline))
    {
        installer::exiter::exit_program("ERROR: Line for license file in Control.idt not found!", "add_licensefile_to_database");
    }
    else
    {
        my %control = ();

        if ( $line =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            $control{'Dialog_'} = $1;
            $control{'Control'} = $2;
            $control{'Type'} = $3;
            $control{'X'} = $4;
            $control{'Y'} = $5;
            $control{'Width'} = $6;
            $control{'Height'} = $7;
            $control{'Attributes'} = $8;
            $control{'Property'} = $9;
            $control{'Text'} = $10;
            $control{'Control_Next'} = $11;
            $control{'Help'} = $12;
        }
        else
        {
            installer::exiter::exit_program("ERROR: Could not split line correctly!", "add_licensefile_to_database");
        }

        my $licensetext = make_string_licensetext($licensefile);

        $control{'Text'} = $licensetext;

        my $newline = $control{'Dialog_'} . "\t" . $control{'Control'} . "\t" . $control{'Type'} . "\t" .
                        $control{'X'} . "\t" . $control{'Y'} . "\t" . $control{'Width'} . "\t" .
                        $control{'Height'} . "\t" . $control{'Attributes'} . "\t" . $control{'Property'} . "\t" .
                        $control{'Text'} . "\t" . $control{'Control_Next'} . "\t" . $control{'Help'} . "\n";

        ${$controltable}[$number] = $newline
    }
}

###################################################################
# Determining the last position in a sequencetable
# into the tables CustomAc.idt and InstallE.idt.
###################################################################

sub get_last_position_in_sequencetable
{
    my ($sequencetable) = @_;

    my $position = 0;

    for ( my $i = 0; $i <= $#{$sequencetable}; $i++ )
    {
        my $line = ${$sequencetable}[$i];

        if ( $line =~ /^\s*\w+\t.*\t\s*(\d+)\s$/ )
        {
            my $newposition = $1;
            if ( $newposition > $position ) { $position = $newposition; }
        }
    }

    return $position;
}

#########################################################################
# Determining the position of a specified Action in the sequencetable
#########################################################################

sub get_position_in_sequencetable
{
    my ($action, $sequencetable) = @_;

    my $position = 0;

    $action =~ s/^\s*behind_//;

    for ( my $i = 0; $i <= $#{$sequencetable}; $i++ )
    {
        my $line = ${$sequencetable}[$i];

        if ( $line =~ /^\s*(\w+)\t.*\t\s*(\d+)\s$/ )
        {
            my $compareaction = $1;
            $position = $2;
            if ( $compareaction eq $action ) { last; }
        }
    }

    return $position;
}

################################################################################################
# Including the CustomAction for the configuration
# into the tables CustomAc.idt and InstallE.idt.
#
# CustomAc.idt: ExecutePkgchk 82 pkgchk.exe -s
# InstallE.idt: ExecutePkgchk Not REMOVE="ALL" 3175
#
# CustomAc.idt: ExecuteQuickstart 82 install_quickstart.exe
# InstallE.idt: ExecuteQuickstart &gm_o_Quickstart=3 3200
#
# CustomAc.idt: ExecuteInstallRegsvrex 82 regsvrex.exe shlxthdl.dll
# InstallE.idt: ExecuteInstallRegsvrex Not REMOVE="ALL" 3225
#
# CustomAc.idt: ExecuteUninstallRegsvrex 82 regsvrex.exe /u shlxthdl.dll
# InstallE.idt: ExecuteUninstallRegsvrex REMOVE="ALL" 690
#
# CustomAc.idt: Regmsdocmsidll1 1 reg4msdocmsidll Reg4MsDocEntry
# InstallU.idt: Regmsdocmsidll1 Not REMOVE="ALL" 610
#
# CustomAc.idt: Regmsdocmsidll2 1 reg4msdocmsidll Reg4MsDocEntry
# InstallE.idt: Regmsdocmsidll2 Not REMOVE="ALL" 3160
################################################################################################

sub set_custom_action
{
    my ($customactionidttable, $actionname, $actionflags, $exefilename, $actionparameter, $inbinarytable, $filesref, $customactionidttablename, $styles) = @_;

    my $included_customaction = 0;
    my $infoline = "";
    my $customaction_exefilename = $exefilename;
    my $uniquename = "";

    # when the style NO_FILE is set, no searching for the file is needed, no filtering is done, we can add that custom action
    if ( $styles =~ /\bNO_FILE\b/ )
    {
        my $line = $actionname . "\t" . $actionflags . "\t" . $customaction_exefilename . "\t" . $actionparameter . "\n";
        push(@{$customactionidttable}, $line);

        $infoline = "Added $actionname CustomAction into table $customactionidttablename (NO_FILE has been set)\n";
        push(@installer::globals::logfileinfo, $infoline);

        $included_customaction = 1;
        return $included_customaction;
    }

    # is the $exefilename a library that is included into the binary table

    if ( $inbinarytable ) { $customaction_exefilename =~ s/\.//; }  # this is the entry in the binary table ("abc.dll" -> "abcdll")

    # is the $exefilename included into the product?

    my $contains_file = 0;

    # All files are located in $filesref and in @installer::globals::binarytableonlyfiles.
    # Both must be added together
    my $localfilesref = [@installer::globals::binarytableonlyfiles, @{$filesref}];

    for ( my $i = 0; $i <= $#{$localfilesref}; $i++ )
    {
        my $onefile = ${$localfilesref}[$i];
        my $filename = "";
        if ( exists($onefile->{'Name'}) )
        {
            $filename = $onefile->{'Name'};

            if ( $filename eq $exefilename )
            {
                $contains_file = 1;
                $uniquename = ${$localfilesref}[$i]->{'uniquename'};
                last;
            }
        }
        else
        {
            installer::exiter::exit_program("ERROR: Did not find \"Name\" for file \"$onefile->{'uniquename'}\" ($onefile->{'gid'})!", "set_custom_action");
        }
    }

    if ( $contains_file )
    {
        # Now the CustomAction can be included into the CustomAc.idt

        if ( ! $inbinarytable ) { $customaction_exefilename = $uniquename; }    # the unique file name has to be added to the custom action table

        my $line = $actionname . "\t" . $actionflags . "\t" . $customaction_exefilename . "\t" . $actionparameter . "\n";
        push(@{$customactionidttable}, $line);

        $included_customaction = 1;
    }

    if ( $included_customaction ) { $infoline = "Added $actionname CustomAction into table $customactionidttablename\n"; }
    else { $infoline = "Did not add $actionname CustomAction into table $customactionidttablename\n"; }
    push(@installer::globals::logfileinfo, $infoline);

    return $included_customaction;
}

####################################################################
# Adding a Custom Action to InstallExecuteTable or InstallUITable
####################################################################

sub add_custom_action_to_install_table
{
    my ($installtable, $exefilename, $actionname, $actioncondition, $position, $filesref, $installtablename, $styles) = @_;

    my $included_customaction = 0;
    my $feature = "";
    my $infoline = "";

    # when the style NO_FILE is set, no searching for the file is needed, no filtering is done, we can add that custom action
    if ( $styles =~ /\bNO_FILE\b/ )
    {
        # then the InstallE.idt.idt or InstallU.idt.idt
        $actioncondition =~ s/FEATURETEMPLATE/$feature/g;   # only execute Custom Action, if feature of the file is installed

        my $actionposition = 0;

        if ( $position eq "end" ) { $actionposition = get_last_position_in_sequencetable($installtable) + 25; }
        elsif ( $position =~ /^\s*behind_/ ) { $actionposition = get_position_in_sequencetable($position, $installtable) + 2; }
        else { $actionposition = get_position_in_sequencetable($position, $installtable) - 2; }

        my $line = $actionname . "\t" . $actioncondition . "\t" . $actionposition . "\n";
        push(@{$installtable}, $line);

        $infoline = "Added $actionname CustomAction into table $installtablename (NO_FILE has been set)\n";
        push(@installer::globals::logfileinfo, $infoline);
        return;
    }

    my $contains_file = 0;

    # All files are located in $filesref and in @installer::globals::binarytableonlyfiles.
    # Both must be added together
    my $localfilesref = [@installer::globals::binarytableonlyfiles, @{$filesref}];

    for ( my $i = 0; $i <= $#{$localfilesref}; $i++ )
    {
        my $filename = ${$localfilesref}[$i]->{'Name'};

        if ( $filename eq $exefilename )
        {
            $contains_file = 1;

            # Determining the feature of the file

            if ( ${$localfilesref}[$i] ) { $feature = ${$localfilesref}[$i]->{'modules'}; }

            # If modules contains a list of modules, only taking the first one.
            if ( $feature =~ /^\s*(.*?)\,/ ) { $feature = $1; }
            # Attention: Maximum feature length is 38!
            shorten_feature_gid(\$feature);

            last;
        }
    }

    if ( $contains_file )
    {
        # then the InstallE.idt.idt or InstallU.idt.idt

        $actioncondition =~ s/FEATURETEMPLATE/$feature/g;   # only execute Custom Action, if feature of the file is installed

#       my $actionposition = 0;
#       if ( $position eq "end" ) { $actionposition = get_last_position_in_sequencetable($installtable) + 25; }
#       elsif ( $position =~ /^\s*behind_/ ) { $actionposition = get_position_in_sequencetable($position, $installtable) + 2; }
#       else { $actionposition = get_position_in_sequencetable($position, $installtable) - 2; }
#       my $line = $actionname . "\t" . $actioncondition . "\t" . $actionposition . "\n";

        my $positiontemplate = "";
        if ( $position =~ /^\s*\d+\s*$/ ) { $positiontemplate = $position; }    # setting the position directly, number defined in scp2
        else { $positiontemplate = "POSITIONTEMPLATE_" . $position; }

        my $line = $actionname . "\t" . $actioncondition . "\t" . $positiontemplate . "\n";
        push(@{$installtable}, $line);

        $included_customaction = 1;
    }

    if ( $included_customaction ) { $infoline = "Added $actionname CustomAction into table $installtablename\n"; }
    else { $infoline = "Did not add $actionname CustomAction into table $installtablename\n"; }
    push(@installer::globals::logfileinfo, $infoline);

}

##################################################################
# A line in the table ControlEvent connects a Control
# with a Custom Action
#################################################################

sub connect_custom_action_to_control
{
    my ( $table, $tablename, $dialog, $control, $event, $argument, $condition, $ordering) = @_;

    my $line = $dialog . "\t" . $control. "\t" . $event. "\t" . $argument. "\t" . $condition. "\t" . $ordering . "\n";

    push(@{$table}, $line);

    $line =~ s/\s*$//g;

    $infoline = "Added line \"$line\" into table $tablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

##################################################################
# A line in the table ControlCondition connects a Control state
# with a condition
##################################################################

sub connect_condition_to_control
{
    my ( $table, $tablename, $dialog, $control, $event, $condition) = @_;

    my $line = $dialog . "\t" . $control. "\t" . $event. "\t" . $condition. "\n";

    push(@{$table}, $line);

    $line =~ s/\s*$//g;

    $infoline = "Added line \"$line\" into table $tablename\n";
    push(@installer::globals::logfileinfo, $infoline);
}

##################################################################
# Searching for a sequencenumber in InstallUISequence table
# "ExecuteAction" must be the last action
##################################################################

sub get_free_number_in_uisequence_table
{
    my ( $installuitable ) = @_;

    # determining the sequence of "ExecuteAction"

    my $executeactionnumber = 0;

    for ( my $i = 0; $i <= $#{$installuitable}; $i++ )
    {
        if ( ${$installuitable}[$i] =~ /^\s*(\w+)\t\w*\t(\d+)\s*$/ )
        {
            my $actionname = $1;
            my $actionnumber = $2;

            if ( $actionname eq "ExecuteAction" )
            {
                $executeactionnumber = $actionnumber;
                last;
            }
        }
    }

    if ( $executeactionnumber == 0 ) { installer::exiter::exit_program("ERROR: Did not find \"ExecuteAction\" in InstallUISequence table!", "get_free_number_in_uisequence_table"); }

    # determining the sequence of the action before "ExecuteAction"

    my $lastactionnumber = 0;

    for ( my $i = 0; $i <= $#{$installuitable}; $i++ )
    {
        if ( ${$installuitable}[$i] =~ /^\s*\w+\t\w*\t(\d+)\s*$/ )
        {
            my $actionnumber = $1;

            if (( $actionnumber > $lastactionnumber ) && ( $actionnumber != $executeactionnumber ))
            {
                $lastactionnumber = $actionnumber;
            }
        }
    }

    # the new number can now be calculated

    my $newnumber = 0;

    if ((( $lastactionnumber + $executeactionnumber ) % 2 ) == 0 ) { $newnumber = ( $lastactionnumber + $executeactionnumber ) / 2; }
    else { $newnumber = ( $lastactionnumber + $executeactionnumber -1 ) / 2; }

    return $newnumber;
}

#############################################################
# Including the new subdir into the directory table
#############################################################

sub include_subdirname_into_directory_table
{
    my ($dirname, $directorytable, $directorytablename, $onefile) = @_;

    my $subdir = "";
    if ( $onefile->{'Subdir'} ) { $subdir = $onefile->{'Subdir'}; }
    if ( $subdir eq "" ) { installer::exiter::exit_program("ERROR: No \"Subdir\" defined for $onefile->{'Name'}", "include_subdirname_into_directory_table"); }

    # program INSTALLLOCATION program -> subjava INSTALLLOCATION program:java

    my $uniquename = "";
    my $parent = "";
    my $name = "";

    my $includedline = 0;

    my $newdir = "";

    for ( my $i = 0; $i <= $#{$directorytable}; $i++ )
    {

        if ( ${$directorytable}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            $uniquename = $1;
            $parent = $2;
            $name = $3;

            if ( $dirname eq $name )
            {
                my $newuniquename = "sub" . $subdir;
                $newdir = $newuniquename;
                my $newparent = "INSTALLLOCATION";
                my $newname = $name . "\:" . $subdir;
                my $newline =
                $line = "$newuniquename\t$newparent\t$newname\n";
                push(@{$directorytable}, $line);
                installer::remover::remove_leading_and_ending_whitespaces(\$line);
                $infoline = "Added $line into directory table $directorytablename\n";
                push(@installer::globals::logfileinfo, $infoline);

                $includedline = 1;
                last;
            }
        }
    }

    if ( ! $includedline ) { installer::exiter::exit_program("ERROR: Could not include new subdirectory into directory table for file $onefile->{'Name'}!", "include_subdirname_into_directory_table"); }

    return $newdir;
}

##################################################################
# Including the new sub directory into the component table
##################################################################

sub include_subdir_into_componenttable
{
    my ($subdir, $onefile, $componenttable) = @_;

    my $componentname = $onefile->{'componentname'};

    my $changeddirectory = 0;

    for ( my $i = 0; $i <= $#{$componenttable}; $i++ )
    {
        if ( ${$componenttable}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $localcomponentname = $1;
            my $directory = $3;

            if ( $componentname eq $localcomponentname )
            {
                my $oldvalue = ${$componenttable}[$i];
                ${$componenttable}[$i] =~ s/\b\Q$directory\E\b/$subdir/;
                my $newvalue = ${$componenttable}[$i];

                installer::remover::remove_leading_and_ending_whitespaces(\$oldvalue);
                installer::remover::remove_leading_and_ending_whitespaces(\$newvalue);
                $infoline = "Change in Component table: From \"$oldvalue\" to \"$newvalue\"\n";
                push(@installer::globals::logfileinfo, $infoline);

                $changeddirectory = 1;
                last;
            }
        }
    }

    if ( ! $changeddirectory ) { installer::exiter::exit_program("ERROR: Could not change directory for component: $onefile->{'Name'}!", "include_subdir_into_componenttable"); }

}

##################################################################
# Setting the condition, that at least one module is selected.
# All modules with flag SHOW_MULTILINGUAL_ONLY were already
# collected. In table ControlE.idt, the string
# LANGUAGECONDITIONINSTALL needs to be replaced.
# Also for APPLICATIONCONDITIONINSTALL for the applications
# with flag APPLICATIONMODULE.
##################################################################

sub set_multilanguageonly_condition
{
    my ( $languageidtdir ) = @_;

    my $onefilename = $languageidtdir . $installer::globals::separator . "ControlE.idt";
    my $onefile = installer::files::read_file($onefilename);

    # Language modules

    my $condition = "";

    foreach my $module ( sort keys %installer::globals::multilingual_only_modules )
    {
        $condition = $condition . " &$module=3 Or";
    }

    $condition =~ s/^\s*//;
    $condition =~ s/\s*Or\s*$//;    # removing the ending "Or"

    if ( $condition eq "" ) { $condition = "1"; }

    for ( my $j = 0; $j <= $#{$onefile}; $j++ )
    {
        ${$onefile}[$j] =~ s/LANGUAGECONDITIONINSTALL/$condition/;
    }

    # Application modules

    $condition = "";

    foreach my $module ( sort keys %installer::globals::application_modules )
    {
        $condition = $condition . " &$module=3 Or";
    }

    $condition =~ s/^\s*//;
    $condition =~ s/\s*Or\s*$//;    # removing the ending "Or"

    if ( $condition eq "" ) { $condition = "1"; }

    for ( my $j = 0; $j <= $#{$onefile}; $j++ )
    {
        ${$onefile}[$j] =~ s/APPLICATIONCONDITIONINSTALL/$condition/;
    }

    installer::files::save_file($onefilename, $onefile);
}

#############################################
# Putting array values into hash
#############################################

sub fill_assignment_hash
{
    my ($gid, $name, $key, $assignmenthashref, $parameter, $tablename, $assignmentarray) = @_;

    my $max = $parameter - 1;

    if ( $max != $#{$assignmentarray} )
    {
        my $definedparameter = $#{$assignmentarray} + 1;
        installer::exiter::exit_program("ERROR: gid: $gid, key: $key ! Wrong parameter in scp. For table $tablename $parameter parameter are required ! You defined: $definedparameter", "fill_assignment_hash");
    }

    for ( my $i = 0; $i <= $#{$assignmentarray}; $i++ )
    {
        my $counter = $i + 1;
        my $key = "parameter". $counter;

        my $localvalue = ${$assignmentarray}[$i];
        installer::remover::remove_leading_and_ending_quotationmarks(\$localvalue);
        $localvalue =~ s/\\\"/\"/g;
        $localvalue =~ s/\\\!/\!/g;
        $localvalue =~ s/\\\&/\&/g;
        $localvalue =~ s/\\\</\</g;
        $localvalue =~ s/\\\>/\>/g;
        $assignmenthashref->{$key} = $localvalue;
    }
}

##########################################################################
# Checking the assignment of a Windows CustomAction and putting it
# into a hash
##########################################################################

sub create_customaction_assignment_hash
{
    my ($gid, $name, $key, $assignmentarray) = @_;

    my %assignment = ();
    my $assignmenthashref = \%assignment;

    my $tablename = ${$assignmentarray}[0];
    installer::remover::remove_leading_and_ending_quotationmarks(\$tablename);

    my $tablename_defined = 0;
    my $parameter = 0;

    if ( $tablename eq "InstallUISequence" )
    {
        $tablename_defined = 1;
        $parameter = 3;
        fill_assignment_hash($gid, $name, $key, $assignmenthashref, $parameter, $tablename, $assignmentarray);
    }

    if ( $tablename eq "InstallExecuteSequence" )
    {
        $tablename_defined = 1;
        $parameter = 3;
        fill_assignment_hash($gid, $name, $key, $assignmenthashref, $parameter, $tablename, $assignmentarray);
    }

    if ( $tablename eq "AdminExecuteSequence" )
    {
        $tablename_defined = 1;
        $parameter = 3;
        fill_assignment_hash($gid, $name, $key, $assignmenthashref, $parameter, $tablename, $assignmentarray);
    }

    if ( $tablename eq "ControlEvent" )
    {
        $tablename_defined = 1;
        $parameter = 7;
        fill_assignment_hash($gid, $name, $key, $assignmenthashref, $parameter, $tablename, $assignmentarray);
    }

    if ( $tablename eq "ControlCondition" )
    {
        $tablename_defined = 1;
        $parameter = 5;
        fill_assignment_hash($gid, $name, $key, $assignmenthashref, $parameter, $tablename, $assignmentarray);
    }

    if ( ! $tablename_defined )
    {
        installer::exiter::exit_program("ERROR: gid: $gid, key: $key ! Unknown Windows CustomAction table: $tablename ! Currently supported: InstallUISequence, InstallExecuteSequence, ControlEvent, ControlCondition", "create_customaction_assignment_hash");
    }

    return $assignmenthashref;
}

##########################################################################
# Finding the position of a specified CustomAction.
# If the CustomAction is not found, the return value is "-1".
# If the CustomAction position is not defined yet,
# the return value is also "-1".
##########################################################################

sub get_customaction_position
{
    my ($action, $sequencetable) = @_;

    my $position = -1;

    for ( my $i = 0; $i <= $#{$sequencetable}; $i++ )
    {
        my $line = ${$sequencetable}[$i];

        if ( $line =~ /^\s*([\w\.]+)\t.*\t\s*(\d+)\s$/ )    # matching only, if position is a number!
        {
            my $compareaction = $1;
            my $localposition = $2;

            if ( $compareaction eq $action )
            {
                $position = $localposition;
                last;
            }
        }
    }

    return $position;
}

##########################################################################
# Setting the position of CustomActions in sequence tables.
# Replacing all occurrences of "POSITIONTEMPLATE_"
##########################################################################

sub set_positions_in_table
{
    my ( $sequencetable, $tablename ) = @_;

    my $infoline = "\nSetting positions in table \"$tablename\".\n";
    push(@installer::globals::logfileinfo, $infoline);

    # Step 1: Resolving all occurrences of "POSITIONTEMPLATE_end"

    my $lastposition = get_last_position_in_sequencetable($sequencetable);

    for ( my $i = 0; $i <= $#{$sequencetable}; $i++ )
    {
        if ( ${$sequencetable}[$i] =~ /^\s*([\w\.]+)\t.*\t\s*POSITIONTEMPLATE_end\s*$/ )
        {
            my $customaction = $1;
            $lastposition = $lastposition + 25;
            ${$sequencetable}[$i] =~ s/POSITIONTEMPLATE_end/$lastposition/;
            $infoline = "Setting position \"$lastposition\" for custom action \"$customaction\".\n";
            push(@installer::globals::logfileinfo, $infoline);
        }
    }

    # Step 2: Resolving all occurrences of "POSITIONTEMPLATE_abc" or "POSITIONTEMPLATE_behind_abc"
    # where abc is the name of the reference Custom Action.
    # This has to be done, until there is no more occurrence of POSITIONTEMPLATE (success)
    # or there is no replacement in one circle (failure).

    my $template_exists = 0;
    my $template_replaced = 0;
    my $counter = 0;

    do
    {
        $template_exists = 0;
        $template_replaced = 0;
        $counter++;

        for ( my $i = 0; $i <= $#{$sequencetable}; $i++ )
        {
            if ( ${$sequencetable}[$i] =~ /^\s*([\w\.]+)\t.*\t\s*(POSITIONTEMPLATE_.*?)\s*$/ )
            {
                my $onename = $1;
                my $templatename = $2;
                my $positionname = $templatename;
                my $customaction = $templatename;
                $customaction =~ s/POSITIONTEMPLATE_//;
                $template_exists = 1;

                # Trying to find the correct number.
                # This can fail, if the custom action has no number

                my $setbehind = 0;
                if ( $customaction =~ /^\s*behind_(.*?)\s*$/ )
                {
                    $customaction = $1;
                    $setbehind = 1;
                }

                my $position = get_customaction_position($customaction, $sequencetable);

                if ( $position >= 0 )   # Found CustomAction and is has a position. Otherwise return value is "-1".
                {
                    my $newposition = 0;
                    if ( $setbehind ) { $newposition = $position + 2; }
                    else { $newposition = $position - 2; }
                    ${$sequencetable}[$i] =~ s/$templatename/$newposition/;
                    $template_replaced = 1;
                    $infoline = "Setting position \"$newposition\" for custom action \"$onename\" (scp: \"$positionname\" at position $position).\n";
                    push(@installer::globals::logfileinfo, $infoline);
                }
                else
                {
                    $infoline = "Could not assign position for custom action \"$onename\" yet (scp: \"$positionname\").\n";
                    push(@installer::globals::logfileinfo, $infoline);
                }
            }
        }
    } while (( $template_exists ) && ( $template_replaced ));

    # An error occurred, because templates still exist, but could not be replaced.
    # Reason:
    # 1. Wrong name of CustomAction in scp2 (typo?)
    # 2. Circular dependencies of CustomActions (A after B and B after A)

    # Problem: It is allowed, that a CustomAction is defined in scp2 in a library that is
    # part of product ABC, but this CustomAction is not used in this product
    # and the reference CustomAction is not part of this product.
    # Therefore this cannot be an error, but only produce a warning. The assigned number
    # must be the last sequence number.

    if (( $template_exists ) && ( ! $template_replaced ))
    {
        for ( my $i = 0; $i <= $#{$sequencetable}; $i++ )
        {
            if ( ${$sequencetable}[$i] =~ /^\s*([\w\.]+)\t.*\t\s*(POSITIONTEMPLATE_.*?)\s*$/ )
            {
                my $customactionname = $1;
                my $fulltemplate = $2;
                my $template = $fulltemplate;
                $template =~ s/POSITIONTEMPLATE_//;
                $lastposition = $lastposition + 25;
                ${$sequencetable}[$i] =~ s/$fulltemplate/$lastposition/;
                $infoline = "WARNING: Setting position \"$lastposition\" for custom action \"$customactionname\". Could not find CustomAction \"$template\".\n";
                push(@installer::globals::logfileinfo, $infoline);
            }
        }
    }
}

##########################################################################
# Setting the Windows custom actions into different tables
# CustomAc.idt, InstallE.idt, InstallU.idt, ControlE.idt, ControlC.idt
##########################################################################

sub addcustomactions
{
    my ($languageidtdir, $customactions, $filesarray) = @_;

    installer::logger::include_timestamp_into_logfile("\nPerformance Info: addcustomactions start\n");

    my $customactionidttablename = $languageidtdir . $installer::globals::separator . "CustomAc.idt";
    my $customactionidttable = installer::files::read_file($customactionidttablename);
    my $installexecutetablename = $languageidtdir . $installer::globals::separator . "InstallE.idt";
    my $installexecutetable = installer::files::read_file($installexecutetablename);
    my $adminexecutetablename = $languageidtdir . $installer::globals::separator . "AdminExe.idt";
    my $adminexecutetable = installer::files::read_file($adminexecutetablename);
    my $installuitablename = $languageidtdir . $installer::globals::separator . "InstallU.idt";
    my $installuitable = installer::files::read_file($installuitablename);
    my $controleventtablename = $languageidtdir . $installer::globals::separator . "ControlE.idt";
    my $controleventtable = installer::files::read_file($controleventtablename);
    my $controlconditiontablename = $languageidtdir . $installer::globals::separator . "ControlC.idt";
    my $controlconditiontable = installer::files::read_file($controlconditiontablename);

    # Iterating over all Windows custom actions

    for ( my $i = 0; $i <= $#{$customactions}; $i++ )
    {
        my $customaction = ${$customactions}[$i];
        my $name = $customaction->{'Name'};
        my $typ = $customaction->{'Typ'};
        my $source = $customaction->{'Source'};
        my $target = $customaction->{'Target'};
        my $inbinarytable = $customaction->{'Inbinarytable'};
        my $gid = $customaction->{'gid'};

        my $styles = "";
        if ( $customaction->{'Styles'} ) { $styles = $customaction->{'Styles'}; }

        my $added_customaction = set_custom_action($customactionidttable, $name, $typ, $source, $target, $inbinarytable, $filesarray, $customactionidttablename, $styles);

        if ( $added_customaction )
        {
            # If the CustomAction was added into the CustomAc.idt, it can be connected to the installation.
            # There are currently two different ways for doing this:
            # 1. Using "add_custom_action_to_install_table", which adds the CustomAction to the install sequences,
            #    which are saved in InstallE.idt and InstallU.idt
            # 2. Using "connect_custom_action_to_control" and "connect_custom_action_to_control". The first method
            #    connects a CustomAction to a control in ControlE.idt. The second method sets a condition for a control,
            #    which might be influenced by the CustomAction. This happens in ControlC.idt.

            # Any Windows CustomAction can have a lot of different assignments.

            for ( my $j = 1; $j <= 50; $j++ )
            {
                my $key = "Assignment" . $j;
                my $value = "";
                if ( $customaction->{$key} )
                {
                    $value = $customaction->{$key};
                }
                else { last; }

                # $value is now a comma separated list
                if ( $value =~ /^\s*\(\s*(.*)\s*\);?\s*$/ ) { $value = $1; }
                my $assignmentarray = installer::converter::convert_stringlist_into_array(\$value, ",");
                my $assignment = create_customaction_assignment_hash($gid, $name, $key, $assignmentarray);

                if ( $assignment->{'parameter1'} eq "InstallExecuteSequence" )
                {
                    add_custom_action_to_install_table($installexecutetable, $source, $name, $assignment->{'parameter2'}, $assignment->{'parameter3'}, $filesarray, $installexecutetablename, $styles);
                }
                elsif ( $assignment->{'parameter1'} eq "AdminExecuteSequence" )
                {
                    add_custom_action_to_install_table($adminexecutetable, $source, $name, $assignment->{'parameter2'}, $assignment->{'parameter3'}, $filesarray, $adminexecutetablename, $styles);
                }
                elsif ( $assignment->{'parameter1'} eq "InstallUISequence" )
                {
                    add_custom_action_to_install_table($installuitable, $source, $name, $assignment->{'parameter2'}, $assignment->{'parameter3'}, $filesarray, $installuitablename, $styles);
                }
                elsif ( $assignment->{'parameter1'} eq "ControlEvent" )
                {
                    connect_custom_action_to_control($controleventtable, $controleventtablename, $assignment->{'parameter2'}, $assignment->{'parameter3'}, $assignment->{'parameter4'}, $assignment->{'parameter5'}, $assignment->{'parameter6'}, $assignment->{'parameter7'});
                }
                elsif ( $assignment->{'parameter1'} eq "ControlCondition" )
                {
                    connect_condition_to_control($controlconditiontable, $controlconditiontablename, $assignment->{'parameter2'}, $assignment->{'parameter3'}, $assignment->{'parameter4'}, $assignment->{'parameter5'});
                }
                else
                {
                    installer::exiter::exit_program("ERROR: gid: $gid, key: $key ! Unknown Windows CustomAction table: $assignmenthashref->{'parameter1'} ! Currently supported: InstallUISequence, InstallESequence, ControlEvent, ControlCondition", "addcustomactions");
                }
            }
        }
    }

    # Setting the positions in the tables

    set_positions_in_table($installexecutetable, $installexecutetablename);
    set_positions_in_table($installuitable, $installuitablename);
    set_positions_in_table($adminexecutetable, $adminexecutetablename);

    # Saving the files

    installer::files::save_file($customactionidttablename, $customactionidttable);
    installer::files::save_file($installexecutetablename, $installexecutetable);
    installer::files::save_file($adminexecutetablename, $adminexecutetable);
    installer::files::save_file($installuitablename, $installuitable);
    installer::files::save_file($controleventtablename, $controleventtable);
    installer::files::save_file($controlconditiontablename, $controlconditiontable);

    my $infoline = "Updated idt file: $customactionidttablename\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Updated idt file: $installexecutetablename\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Updated idt file: $adminexecutetablename\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Updated idt file: $installuitablename\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Updated idt file: $controleventtablename\n";
    push(@installer::globals::logfileinfo, $infoline);
    $infoline = "Updated idt file: $controlconditiontablename\n";
    push(@installer::globals::logfileinfo, $infoline);

    installer::logger::include_timestamp_into_logfile("\nPerformance Info: addcustomactions end\n");
}

##########################################################################
# Setting bidi attributes in idt tables
##########################################################################

sub setbidiattributes
{
    my ($languageidtdir, $onelanguage) = @_;

    # Editing the files Dialog.idt and Control.idt

    my $dialogfilename = $languageidtdir . $installer::globals::separator . "Dialog.idt";
    my $controlfilename = $languageidtdir . $installer::globals::separator . "Control.idt";

    my $dialogfile = installer::files::read_file($dialogfilename);
    my $controlfile = installer::files::read_file($controlfilename);

    # Searching attributes in Dialog.idt and adding "896".
    # Attributes are in column 6 (from 10).

    my $bidiattribute = 896;
    for ( my $i = 0; $i <= $#{$dialogfile}; $i++ )
    {
        if ( $i < 3 ) { next; }
        if ( ${$dialogfile}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $one = $1;
            my $two = $2;
            my $three = $3;
            my $four = $4;
            my $five = $5;
            my $attribute = $6;
            my $seven = $7;
            my $eight = $8;
            $attribute = $attribute + $bidiattribute;
            ${$dialogfile}[$i] = "$one\t$two\t$three\t$four\t$five\t$attribute\t$seven\t$eight\n";
        }
    }

    # Searching attributes in Control.idt and adding "224".
    # Attributes are in column 8 (from 12).

    $bidiattribute = 224;
    for ( my $i = 0; $i <= $#{$controlfile}; $i++ )
    {
        if ( $i < 3 ) { next; }
        if ( ${$controlfile}[$i] =~ /^\s*(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\t(.*?)\s*$/ )
        {
            my $one = $1;
            my $two = $2;
            my $three = $3;
            my $four = $4;
            my $five = $5;
            my $six = $6;
            my $seven = $7;
            my $attribute = $8;
            my $nine = $9;
            my $ten = $10;
            my $eleven = $11;
            my $twelve = $12;
            $attribute = $attribute + $bidiattribute;
            ${$controlfile}[$i] = "$one\t$two\t$three\t$four\t$five\t$six\t$seven\t$attribute\t$nine\t$ten\t$eleven\t$twelve\n";
        }
    }

    # Saving the file

    installer::files::save_file($dialogfilename, $dialogfile);
    $infoline = "Set bidi support in idt file \"$dialogfilename\" for language $onelanguage\n";
    push(@installer::globals::logfileinfo, $infoline);

    installer::files::save_file($controlfilename, $controlfile);
    $infoline = "Set bidi support in idt file \"$controlfilename\" for language $onelanguage\n";
    push(@installer::globals::logfileinfo, $infoline);
}

1;
