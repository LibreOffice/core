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
package installer::javainstaller;

use Cwd;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::languages;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::systemactions;
use installer::worker;
use installer::logger;

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

#######################################################
# Searching for the module name and description in the
# modules collector
#######################################################

sub get_module_name_description
{
    my ($modulesarrayref, $onelanguage, $gid, $type) = @_;

    my $found = 0;

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $onemodule = ${$modulesarrayref}[$i];

        if ( $onemodule->{'gid'} eq $gid )
        {
            my $typestring = $type . " " . "(" . $onelanguage . ")";
            if ( $onemodule->{$typestring} ) { $newstring = $onemodule->{$typestring}; }
            $found = 1;
        }

        if ( $found ) { last; }
    }

    # defaulting to english

    if ( ! $found )
    {
        my $defaultlanguage = "en-US";

        for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
        {
            my $onemodule = ${$modulesarrayref}[$i];

            if ( $onemodule->{'gid'} eq $gid )
            {
                my $typestring = $type . " " . "(" . $defaultlanguage . ")";
                if ( $onemodule->{$typestring} ) { $newstring = $onemodule->{$typestring}; }
                $found = 1;
            }

            if ( $found ) { last; }
        }
    }

    return $newstring;
}

#######################################################
# Setting the productname and productversion
#######################################################

sub set_productname_and_productversion
{
    my ($templatefile, $variableshashref) = @_;

    my $infoline = "\nSetting product name and product version in Java template file\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $productname = $variableshashref->{'PRODUCTNAME'};
    my $productversion = $variableshashref->{'PRODUCTVERSION'};

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        ${$templatefile}[$i] =~ s/\{PRODUCTNAME\}/$productname/g;
        ${$templatefile}[$i] =~ s/\{PRODUCTVERSION\}/$productversion/g;
    }

    $infoline = "End of: Setting product name and product version in Java template file\n\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#######################################################
# Setting the localized Module name and description
#######################################################

sub set_component_name_and_description
{
    my ($templatefile, $modulesarrayref, $onelanguage) = @_;

    my $infoline = "\nSetting component names and description in Java template file\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        # OOO_gid_Module_Prg_Wrt_Name
        # OOO_gid_Module_Prg_Wrt_Description

        my $oneline = ${$templatefile}[$i];
        my $oldstring = "";
        my $gid = "";
        my $type = "";

        if ( $oneline =~ /\b(OOO_gid_\w+)\b/ )
        {
            $oldstring = $1;

            $infoline = "Found: $oldstring\n";
            push( @installer::globals::logfileinfo, $infoline);

            if ( $oldstring =~ /^\s*OOO_(gid_\w+)_(\w+?)\s*$/ )
            {
                $gid = $1;
                $type = $2;
            }

            my $newstring = get_module_name_description($modulesarrayref, $onelanguage, $gid, $type);

            $infoline = "\tReplacing (language $onelanguage): OLDSTRING: $oldstring NEWSTRING $newstring\n";
            push( @installer::globals::logfileinfo, $infoline);

            ${$templatefile}[$i] =~ s/$oldstring/$newstring/;   # always substitute, even if $newstring eq ""
        }
    }

    $infoline = "End of: Setting component names and description in Java template file\n\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#######################################################
# Translating the Java file
#######################################################

sub translate_javafile
{
    my ($templatefile, $languagefile, $onelanguage) = @_;

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        my @allstrings = ();

        my $oneline = ${$templatefile}[$i];

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

            $newstring =~ s/\"/\\\"/g;  # masquerading the "
            $newstring =~ s/\\\\\"/\\\"/g;  # unmasquerading if \" was converted to \\" (because " was already masked)

            ${$templatefile}[$i] =~ s/$oldstring/$newstring/;   # always substitute, even if $newstring eq ""
        }
    }
}

###########################################################
# Returning the license file name for a defined language
###########################################################

sub get_licensefilesource
{
    my ($language, $includepatharrayref) = @_;

    my $licensefilename = "LICENSE_" . $language;

    my $licenseref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, $includepatharrayref, 0);
    if ($$licenseref eq "") { installer::exiter::exit_program("ERROR: Could not find License file $licensefilename!", "get_licensefilesource"); }

    my $infoline = "Found licensefile $licensefilename: $$licenseref \n";
    push( @installer::globals::logfileinfo, $infoline);

    return $$licenseref;
}

#######################################################
# Converting the license string into the
# Java specific encoding.
#######################################################

sub convert_licenstring
{
    my ($licensefile, $includepatharrayref, $javadir, $onelanguage) = @_;

    my $licensedir = $javadir . $installer::globals::separator . "license";
    installer::systemactions::create_directory($licensedir);

    # saving the original license file

    my $licensefilename = $licensedir . $installer::globals::separator . "licensefile.txt";
    installer::files::save_file($licensefilename, $licensefile);

    # creating the ulf file from the license file

    $licensefilename = $licensedir . $installer::globals::separator . "licensefile.ulf";
    my @licensearray = ();

    my $section = "\[TRANSLATE\]\n";
    push(@licensearray, $section);

    for ( my $i = 0; $i <= $#{$licensefile}; $i++ )
    {
        my $oneline = ${$licensefile}[$i];

        if ($i == 0) { $oneline =~ s/^\s*\ï\»\¿//; }

        $oneline =~ s/\s*$//;
        $oneline =~ s/\"/\\\"/g;    # masquerading the "
        $oneline =~ s/\'/\\\'/g;    # masquerading the '

        $oneline =~ s/\$\{/\{/g;    # replacement of variables, only {PRODUCTNAME}, not ${PRODUCTNAME}

        my $ulfstring = $onelanguage . " = " . "\"" . $oneline . "\"\n";
        push(@licensearray, $ulfstring);
    }

    installer::files::save_file($licensefilename, \@licensearray);

    # converting the ulf file to the jlf file with ulfconv

    @licensearray = ();

    my $converter = "ulfconv";

    my $converterref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$converter, $includepatharrayref, 0);
    if ($$converterref eq "") { installer::exiter::exit_program("ERROR: Could not find converter $converter!", "convert_licenstring"); }

    my $infoline = "Found converter file $converter: $$converterref \n";
    push( @installer::globals::logfileinfo, $infoline);

    my $systemcall = "$$converterref $licensefilename |";
    open (CONV, "$systemcall");
    @licensearray = <CONV>;
    close (CONV);

    $licensefilename = $licensedir . $installer::globals::separator . "licensefile.jlf";
    installer::files::save_file($licensefilename, \@licensearray);

    # creating the license string from the jlf file

    $licensestring = "";

    for ( my $i = 1; $i <= $#licensearray; $i++ )   # not the first line!
    {
        my $oneline = $licensearray[$i];
        $oneline =~ s/^\s*$onelanguage\s*\=\s*\"//;
        $oneline =~ s/\"\s*$//;
        $licensestring = $licensestring . $oneline . "\\n";
    }

    $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $licensestring eq "" )
    {
        $infoline = "ERROR: Could not convert $licensefilename !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $licensestring;
}

#######################################################
# Adding the license file into the java file
# In the template java file there are two
# occurrences of INSTALLSDK_GUI_LICENSE
# and INSTALLSDK_CONSOLE_LICENSE
#######################################################

sub add_license_file_into_javafile
{
    my ( $templatefile, $licensefile, $includepatharrayref, $javadir, $onelanguage ) = @_;

    my $licensestring = convert_licenstring($licensefile, $includepatharrayref, $javadir, $onelanguage);

    # saving the licensestring in an ulf file
    # converting the file using "ulfconv license.ulf"
    # including the new string into the java file

    for ( my $i = 0; $i <= $#{$templatefile}; $i++ )
    {
        ${$templatefile}[$i] =~ s/INSTALLSDK_GUI_LICENSE/$licensestring/;
        ${$templatefile}[$i] =~ s/INSTALLSDK_CONSOLE_LICENSE/$licensestring/;
    }
}

#######################################################
# Executing one system call
#######################################################

sub make_systemcall
{
    my ( $systemcall, $logreturn ) = @_;

    my @returns = ();

    installer::logger::print_message( "... $systemcall ...\n" );

    open (REG, "$systemcall");
    while (<REG>) {push(@returns, $_); }
    close (REG);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ( $logreturn )
    {
        for ( my $j = 0; $j <= $#returns; $j++ ) { push( @installer::globals::logfileinfo, "$returns[$j]"); }
    }

    if ($returnvalue)
    {
        $infoline = "ERROR: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
        $error_occurred = 1;
    }
    else
    {
        $infoline = "SUCCESS: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return \@returns;
}

#######################################################
# Setting the class path for the Installer SDK
#######################################################

sub set_classpath_for_install_sdk
{
    my ( $directory ) = @_;

    my $installsdk = "";
    my $solarVersion = "";
    my $inPath = "";
    my $updMinorExt = "";

    if ( defined( $ENV{ 'SOLARVERSION' } ) ) { $solarVersion =  $ENV{'SOLARVERSION'}; }
    else { installer::exiter::exit_program("ERROR: Environment variable \"SOLARVERSION\" not set!", "set_classpath_for_install_sdk"); }

    if ( defined( $ENV{ 'INPATH' } ) ) { $inPath =  $ENV{'INPATH'}; }
    else { installer::exiter::exit_program("ERROR: Environment variable \"INPATH\" not set!", "set_classpath_for_install_sdk"); }

    if ( defined( $ENV{ 'UPDMINOREXT' } ) ) { $updMinorExt =  $ENV{'UPDMINOREXT'}; }

    $installsdk = $solarVersion .  $installer::globals::separator . $inPath . $installer::globals::separator . "bin" . $updMinorExt;
    $installsdk = $installsdk . $installer::globals::separator . "javainstaller";

    if ( $ENV{'INSTALLSDK_SOURCE'} ) { $installsdk = $ENV{'INSTALLSDK_SOURCE'}; }   # overriding the Install SDK with INSTALLSDK_SOURCE

    # The variable CLASSPATH has to contain:
    # $installsdk/classes:$installsdk/classes/setupsdk.jar:
    # $installsdk/classes/parser.jar:$installsdk/classes/jaxp.jar:
    # $installsdk/classes/ldapjdk.jar:$directory

    my @additional_classpath = ();
    push(@additional_classpath, "$installsdk\/classes");
    push(@additional_classpath, "$installsdk\/installsdk.jar");
    push(@additional_classpath, "$installsdk\/classes\/parser.jar");
    push(@additional_classpath, "$installsdk\/classes\/jaxp.jar");
    push(@additional_classpath, "$directory");

    my $newclasspathstring = "";
    my $oldclasspathstring = "";
    if ( $ENV{'CLASSPATH'} ) { $oldclasspathstring = $ENV{'CLASSPATH'}; }
    else { $oldclasspathstring = "\."; }

    for ( my $i = 0; $i <= $#additional_classpath; $i++ )
    {
        $newclasspathstring = $newclasspathstring . $additional_classpath[$i] . ":";
    }

    $newclasspathstring = $newclasspathstring . $oldclasspathstring;

    $ENV{'CLASSPATH'} = $newclasspathstring;

    my $infoline = "Setting CLASSPATH to $ENV{'CLASSPATH'}\n";
    push( @installer::globals::logfileinfo, $infoline);
}

#######################################################
# Setting the class file name in the Java locale file
#######################################################

sub set_classfilename
{
    my ($templatefile, $classfilename, $searchstring) = @_;

    for ( my $j = 0; $j <= $#{$templatefile}; $j++ )
    {
        if ( ${$templatefile}[$j] =~ /\Q$searchstring\E/ )
        {
            ${$templatefile}[$j] =~ s/$searchstring/$classfilename/;
            last;
        }
    }
}

#######################################################
# Substituting one variable in the xml file
#######################################################

sub replace_one_variable
{
    my ($xmlfile, $variable, $searchstring) = @_;

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        ${$xmlfile}[$i] =~ s/\$\{$searchstring\}/$variable/g;
    }
}

#######################################################
# Substituting the variables in the xml file
#######################################################

sub substitute_variables
{
    my ($xmlfile, $variableshashref) = @_;

    my $key;

    foreach $key (keys %{$variableshashref})
    {
        my $value = $variableshashref->{$key};
        replace_one_variable($xmlfile, $value, $key);
    }
}

##########################################################
# Finding the line number in xml file of a special
# component
##########################################################

sub find_component_line
{
    my ($xmlfile, $componentname) = @_;

    my $linenumber = 0;

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        if ( ${$xmlfile}[$i] =~ /name\s*\=\'\s*$componentname/ )
        {
            $linenumber = $i;
            last;
        }
    }

    return $linenumber;
}

##########################################################
# Removing one package from the xml file
##########################################################

sub remove_package
{
    my ($xmlfile, $packagename) = @_;

    my $searchstring = $packagename;
    if ( $searchstring =~ /\-(\S+?)\s*$/ ) { $searchstring = $1; } # "SUNW%PRODUCTNAME-mailcap" -> "mailcap"

    my $packagestring = "";
    my $namestring = "";
    my $infoline = "";

    if ( $installer::globals::issolarispkgbuild )
    {
        $packagestring = "\<pkgunit";
        $namestring = "pkgName";
    }
    elsif ( $installer::globals::isrpmbuild )
    {
        $packagestring = "\<rpmunit";
        $namestring = "rpmUniqueName";
    }

    my $removed_packge = 0;

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        if ( ${$xmlfile}[$i] =~ /^\s*\Q$packagestring\E/ )
        {
            # this is a package, but is it the correct one?

            my $do_delete = 0;
            my $linecounter = 1;
            my $startline = $i+1;
            my $line = ${$xmlfile}[$startline];
            if (($line =~ /^\s*\Q$namestring\E\s*\=/) && ($line =~ /\-\Q$searchstring\E/)) { $do_delete = 1; }

            # but not deleting fonts package in language packs
            if ( $line =~ /-ONELANGUAGE-/ ) { $do_delete = 0; }

            my $endcounter = 0;

            while ((!( $line =~ /\/\>/ )) && ( $startline <= $#{$xmlfile} ))
            {
                $linecounter++;
                $startline++;
                $line = ${$xmlfile}[$startline];
                if (($line =~ /^\s*\Q$namestring\E\s*\=/) && ($line =~ /\-\Q$searchstring\E/)) { $do_delete = 1; }
            }

            $linecounter = $linecounter + 1;

            if ( $do_delete )
            {
                my $infoline = "\tReally removing package $packagename from xml file.\n";
                push( @installer::globals::logfileinfo, $infoline);
                splice(@{$xmlfile},$i, $linecounter);   # removing $linecounter lines, beginning in line $i
                $removed_packge = 1;
                last;
            }
        }
    }

    if ( $removed_packge )
    {
        $infoline = "Package $packagename successfully removed from xml file.\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Did not find package $packagename in xml file.\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

}

##########################################################
# Removing one component from the xml file
##########################################################

sub remove_component
{
    my ($xmlfile, $componentname) = @_;

    my @removed_lines = ();

    push(@removed_lines, "\n");

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        if ( ${$xmlfile}[$i] =~ /name\s*\=\'\s*$componentname/ )
        {
            # Counting the lines till the second "</component>"

            push(@removed_lines, ${$xmlfile}[$i]);
            my $linecounter = 1;
            my $startline = $i+1;
            my $line = ${$xmlfile}[$startline];
            push(@removed_lines, $line);
            my $endcounter = 0;

            while ((!( $line =~ /^\s*\<\/component\>\s*$/ )) && ( $startline <= $#{$xmlfile} ))
            {
                $linecounter++;
                $startline++;
                $line = ${$xmlfile}[$startline];
                push(@removed_lines, $line);
            }

            $linecounter = $linecounter + 2;     # last line and following empty line

            splice(@{$xmlfile},$i, $linecounter);   # removing $linecounter lines, beginning in line $i
            last;
        }
    }

    return \@removed_lines;
}

##########################################################
# If this is an installation set without language packs
# the language pack module can be removed
##########################################################

sub remove_languagepack_from_xmlfile
{
    my ($xmlfile) = @_;

    # Component begins with "<component selected="true" name='module_languagepacks' componentVersion="${PRODUCTVERSION}">"
    # and ends with "</component>" (the second "</component>" !)

    remove_component($xmlfile, "languagepack_DEFAULT");
    remove_component($xmlfile, "languagepack_ONELANGUAGE");
    remove_component($xmlfile, "module_languagepacks");
}

##########################################################
# Duplicating a component
##########################################################

sub duplicate_component
{
    my ( $arrayref ) = @_;

    @newarray = ();

    for ( my $i = 0; $i <= $#{$arrayref}; $i++ )
    {
        push(@newarray, ${$arrayref}[$i]);
    }

    return \@newarray;
}

##########################################################
# Including a component into the xml file
# at a specified line
##########################################################

sub include_component_at_specific_line
{
    my ($xmlfile, $unit, $line) = @_;

    splice(@{$xmlfile},$line, 0, @{$unit});
}

##########################################################
# Font packages do not exist for all languages.
##########################################################

sub remove_font_package_from_unit
{
    my ( $unitcopy, $onelanguage ) = @_;

    my $searchstring = "-fonts";

    my $packagestring = "";
    my $namestring = "";

    if ( $installer::globals::issolarispkgbuild )
    {
        $packagestring = "\<pkgunit";
        $namestring = "pkgName";
    }
    elsif ( $installer::globals::isrpmbuild )
    {
        $packagestring = "\<rpmunit";
        $namestring = "rpmUniqueName";
    }

    for ( my $i = 0; $i <= $#{$unitcopy}; $i++ )
    {
        if ( ${$unitcopy}[$i] =~ /^\s*\Q$packagestring\E/ )
        {
            # this is a package, but is it the correct one?

            my $do_delete = 0;
            my $linecounter = 1;
            my $startline = $i+1;
            my $line = ${$unitcopy}[$startline];
            if (($line =~ /^\s*\Q$namestring\E\s*\=/) && ($line =~ /\Q$searchstring\E/)) { $do_delete = 1; }

            my $endcounter = 0;

            while ((!( $line =~ /\/\>/ )) && ( $startline <= $#{$unitcopy} ))
            {
                $linecounter++;
                $startline++;
                $line = ${$unitcopy}[$startline];
                if (($line =~ /^\s*\Q$namestring\E\s*\=/) && ($line =~ /\Q$searchstring\E/)) { $do_delete = 1; }
            }

            $linecounter = $linecounter + 1;

            if ( $do_delete )
            {
                splice(@{$unitcopy},$i, $linecounter);  # removing $linecounter lines, beginning in line $i
                last;
            }
        }
    }
}

##########################################################
# If this is an installation set with language packs,
# modules for each language pack have to be created
# dynamically
##########################################################

sub duplicate_languagepack_in_xmlfile
{
    my ($xmlfile, $languagesarrayref) = @_;

    my $unit = remove_component($xmlfile, "languagepack_ONELANGUAGE");
    my $startline = find_component_line($xmlfile, "module_languagepacks");
    my $infoline = "";
    $startline = $startline + 1;

    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ )
    {
        my $onelanguage = ${$languagesarrayref}[$i];
        my $unitcopy = duplicate_component($unit);

        # replacing string ONELANGUAGE in the unit copy
        for ( my $j = 0; $j <= $#{$unitcopy}; $j++ ) { ${$unitcopy}[$j] =~ s/ONELANGUAGE/$onelanguage/g; }

        # including the unitcopy into the xml file
        include_component_at_specific_line($xmlfile, $unitcopy, $startline);
        $startline = $startline + $#{$unitcopy} + 1;
    }

    # adding the default language as language pack, too
    $unit = remove_component($xmlfile, "languagepack_DEFAULT");
    $startline = find_component_line($xmlfile, "module_languagepacks");
    $startline = $startline + 1;

    $onelanguage = ${$languagesarrayref}[0];
    $unitcopy = duplicate_component($unit);

    # replacing string DEFAULT in the unit copy
    for ( my $j = 0; $j <= $#{$unitcopy}; $j++ ) { ${$unitcopy}[$j] =~ s/DEFAULT/$onelanguage/g; }

    # including the unitcopy into the xml file
    include_component_at_specific_line($xmlfile, $unitcopy, $startline);
    $startline = $startline + $#{$unitcopy} + 1;
}

#######################################################
# Removing empty packages from xml file. The names
# are stored in @installer::globals::emptypackages
#######################################################

sub remove_empty_packages_in_xmlfile
{
    my ($xmlfile) = @_;

    for ( my $i = 0; $i <= $#installer::globals::emptypackages; $i++ )
    {
        my $packagename = $installer::globals::emptypackages[$i];
        my $infoline = "Try to remove package $packagename from xml file.\n";
        push( @installer::globals::logfileinfo, $infoline);
        remove_package($xmlfile, $packagename);
    }
}

#######################################################
# Preparing the language packs in the xml file
#######################################################

sub prepare_language_pack_in_xmlfile
{
    my ($xmlfile, $languagesarrayref) = @_;

        duplicate_languagepack_in_xmlfile($xmlfile, $languagesarrayref);
}

#######################################################
# Returning a rpm unit from a xml file
#######################################################

sub get_rpm_unit_from_xmlfile
{
    my ($rpmname, $xmlfile) = @_;

    my $infoline = "Searching for $rpmname in xml file.\n";
    push( @installer::globals::logfileinfo, $infoline);

    my @rpmunit = ();
    my $includeline = 0;
    my $record = 0;
    my $foundrpm = 0;

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        my $oneline = ${$xmlfile}[$i];

        if ( $oneline =~ /^\s*\<rpmunit/ ) { $record = 1; }

        if ( $record ) { push(@rpmunit, $oneline); }

        if ( $oneline =~ /^\s*rpmUniqueName\s*=\s*\"\Q$rpmname\E\"\s*$/ ) { $foundrpm = 1; }

        if (( $record ) && ( $oneline =~ /\/\>\s*$/ )) { $record = 0; }

        if (( ! $foundrpm ) && ( ! $record )) { @rpmunit = (); }

        if (( $foundrpm ) && ( ! $record )) { $includeline = $i + 1; }

        if (( $foundrpm ) && ( ! $record )) { last; }
    }

    if ( ! $foundrpm ) { installer::exiter::exit_program("ERROR: Did not find rpmunit $rpmname in xml file!", "get_rpm_unit_from_xmlfile"); }

    $infoline = "Found $rpmname in xml file. Returning block lines: $#rpmunit + 1. Includeline: $includeline \n";
    push( @installer::globals::logfileinfo, $infoline);

    return (\@rpmunit, $includeline);
}

#######################################################
# Exchanging package names in xml file
#######################################################

sub exchange_name_in_rpmunit
{
    my ($rpmunit, $oldpackagename, $newpackagename) = @_;

    for ( my $i = 0; $i <= $#{$rpmunit}; $i++ )
    {
        ${$rpmunit}[$i] =~ s/$oldpackagename/$newpackagename/;
    }
}

#######################################################
# Preparing link RPMs in the xml file
#######################################################

sub prepare_linkrpm_in_xmlfile
{
    my ($xmlfile, $rpmlist) = @_;

    for ( my $i = 0; $i <= $#{$rpmlist}; $i++ )
    {
        my $oldpackagename = "";
        my $newpackagename = "";

        my $rpmline = ${$rpmlist}[$i];

        my $infoline = "Preparing link/patch RPM: $rpmline\n";
        push( @installer::globals::logfileinfo, $infoline);

        if ( $rpmline =~ /^\s*(\S.*?\S)\s+(\S.*?\S)\s*$/ )
        {
            $oldpackagename = $1;
            $newpackagename = $2;
        }

        my ($rpmunit, $includeline) = get_rpm_unit_from_xmlfile($oldpackagename, $xmlfile);
        exchange_name_in_rpmunit($rpmunit, $oldpackagename, $newpackagename);
        include_component_at_specific_line($xmlfile, $rpmunit, $includeline);
    }
}

#######################################################################
# Removing w4w filter module from xml file for Solaris x86 and Linux
#######################################################################

sub remove_w4w_from_xmlfile
{
    my ($xmlfile) = @_;

    # Component begins with "<component selected='true' name='gid_Module_Prg_Wrt_Flt_W4w' componentVersion="8">"
    # and ends with "</component>"

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        if ( ${$xmlfile}[$i] =~ /name\s*\=\'\s*gid_Module_Prg_Wrt_Flt_W4w/ )
        {
            # Counting the lines till "</component>"

            my $linecounter = 1;
            my $startline = $i+1;
            my $line = ${$xmlfile}[$startline];

            while ((!( $line =~ /^\s*\<\/component\>\s*$/ )) && ( $startline <= $#{$xmlfile} ))
            {
                $linecounter++;
                $startline++;
                $line = ${$xmlfile}[$startline];
            }

            $linecounter = $linecounter + 2;     # last line and following empty line

            splice(@{$xmlfile},$i, $linecounter);   # removing $linecounter lines, beginning in line $i
            last;
        }
    }
}

#######################################################################
# Removing module from xml file, if not defined in scp
#######################################################################

sub remove_scpgid_from_xmlfile
{
    my ($xmlfile, $scpgid) = @_;

    # Component begins with "<component selected='true' name='$scpgid' componentVersion="8">"
    # and ends with "</component>"

    my $successfully_removed = 0;

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        if ( ${$xmlfile}[$i] =~ /name\s*\=\'\s*\Q$scpgid\E/ )
        {
            # Counting the lines till "</component>"

            my $linecounter = 1;
            my $startline = $i+1;
            my $line = ${$xmlfile}[$startline];

            while ((!( $line =~ /^\s*\<\/component\>\s*$/ )) && ( $startline <= $#{$xmlfile} ))
            {
                $linecounter++;
                $startline++;
                $line = ${$xmlfile}[$startline];
            }

            $linecounter = $linecounter + 2;     # last line and following empty line

            splice(@{$xmlfile},$i, $linecounter);   # removing $linecounter lines, beginning in line $i
            $successfully_removed = 1;
            last;
        }
    }

    my $infoline = "";
    if ($successfully_removed)
    {
        $infoline = "Module $scpgid successfully removed from xml file.\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Module $scpgid not found in xml file (no problem).\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#######################################################################
# Special mechanism for removing modules for xml file, if they are
# not defined in scp (introduced for onlineupdate module).
#######################################################################

sub remove_module_if_not_defined
{
    my ($xmlfile, $modulesarrayref, $scpgid) = @_;

    my $infoline = "Checking existence of $scpgid in scp definition\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $found = 0;

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $onemodule = ${$modulesarrayref}[$i];
        if ( $onemodule->{'gid'} eq $scpgid ) { $found = 1; }
        if ( $found ) { last; }
    }

    if ( ! $found )
    {
        $infoline = "Module $scpgid not found -> Removing from xml file.\n";
        push( @installer::globals::logfileinfo, $infoline);
        remove_scpgid_from_xmlfile($xmlfile, $scpgid);
    }
}

###########################################################
# Preparing the package subdirectory
###########################################################

sub create_empty_packages
{
    my ( $xmlfile ) = @_;

    if ( $installer::globals::issolarispkgbuild )
    {
        my $path = "";

        for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
        {
            if ( ${$xmlfile}[$i] =~ /pkgRelativePath\s*\=\s*\'(.*?)\'\s*$/ )
            {
                $path = $1;
                installer::systemactions::create_directory_structure($path);
                last;   # only creating one path
            }
        }

        for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
        {
            if ( ${$xmlfile}[$i] =~ /pkgName\s*\=\s*\'(.*?)\'\s*$/ )
            {
                my $pkgname = $1;
                if ( $path ne "" ) { $pkgname = $path . $installer::globals::separator . $pkgname; }
                installer::systemactions::create_directory_structure($pkgname);
            }
        }
    }

    # "-novalidate" does not work for Linux RPMs

    if ( $installer::globals::isrpmbuild )
    {
        for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
        {
            if ( ${$xmlfile}[$i] =~ /rpmPath\s*\=\s*\"(.*?)\"\s*$/ )
            {
                my $rpmpath = $1;
                my $path = "";

                if ( $rpmpath =~ /^\s*(.*)\/(.*?)\s*$/ )
                {
                    $path = $1;
                }

                if ( $path ne "" ) { installer::systemactions::create_directory_structure($path); }

                my $systemcall = "touch $rpmpath";  # creating empty rpm
                system($systemcall);
            }
        }
    }
}

###########################################################
# Reading the archive file name from the xml file
###########################################################

sub get_archivefilename
{
    my ( $xmlfile ) = @_;

    my $archivefilename = "";

    for ( my $j = 0; $j <= $#{$xmlfile}; $j++ )
    {
        if ( ${$xmlfile}[$j] =~ /archiveFileName\s*=\s*\'(.*?)\'/ )
        {
            $archivefilename = $1;
            last;
        }
    }

    return $archivefilename;
}

#######################################################
# Copying the loader locally
#######################################################

sub copy_setup_locally
{
    my ($includepatharrayref, $loadername, $newname) = @_;

    my $loadernameref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$loadername, $includepatharrayref, 0);

    if ($$loadernameref eq "") { installer::exiter::exit_program("ERROR: Could not find Java loader $loadername!", "copy_setup_locally"); }

    installer::systemactions::copy_one_file($$loadernameref, $newname);
    chmod 0775, $newname;
}


#######################################################
# Copying the loader into the installation set
#######################################################

sub put_loader_into_installset
{
    my ($installdir, $filename) = @_;

    my $installname = $installdir . $installer::globals::separator . $filename;

    installer::systemactions::copy_one_file($filename, $installname);

    chmod 0775, $installname;
}

#################################################################
# Setting for Solaris the package names in the Java translation
# file. The name is used for the
# This name is displayed tools like prodreg.
# Unfortunately this name in the component is also used
# in the translation template file for the module name
# and module description translations.
#################################################################

sub replace_component_name_in_java_file
{
    my ($alljavafiles, $oldname, $newname) = @_;

    # The new name must not contain white spaces

    $newname =~ s/ /\_/g;

    for ( my $i = 0; $i <= $#{$alljavafiles}; $i++ )
    {
        my $javafilename = ${$alljavafiles}[$i];
        my $javafile = installer::files::read_file($javafilename);

        my $oldstring = "ComponentDescription-" . $oldname;
        my $newstring = "ComponentDescription-" . $newname;

        for ( my $j = 0; $j <= $#{$javafile}; $j++ ) { ${$javafile}[$j] =~ s/\b$oldstring\b/$newstring/; }

        $oldstring = $oldname . "-install-DisplayName";
        $newstring = $newname . "-install-DisplayName";

        for ( my $j = 0; $j <= $#{$javafile}; $j++ ) { ${$javafile}[$j] =~ s/\b$oldstring\b/$newstring/; }

        $oldstring = $oldname . "-uninstall-DisplayName";
        $newstring = $newname . "-uninstall-DisplayName";

        for ( my $j = 0; $j <= $#{$javafile}; $j++ ) { ${$javafile}[$j] =~ s/\b$oldstring\b/$newstring/; }

        installer::files::save_file($javafilename, $javafile);
        $infoline = "Changes in Java file: $javafilename : $oldname \-\> $newname\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#################################################################
# Some module names are not defined in the scp project.
# The names for this modules are searched in the base Java
# translation file.
#################################################################

sub get_module_name_from_basejavafile
{
    my ($componentname, $javatemplateorigfile, $ulffile) = @_;

    my $searchname = $componentname . "-install-DisplayName";
    my $modulename = "";
    my $replacename = "";

    # line content: { "coremodule-install-DisplayName", "OOO_INSTALLSDK_117" },

    for ( my $i = 0; $i <= $#{$javatemplateorigfile}; $i++ )
    {
        if ( ${$javatemplateorigfile}[$i] =~ /\"\s*\Q$searchname\E\s*\"\s*\,\s*\"\s*(.*?)\s*\"\s*\}\s*\,\s*$/ )
        {
            $replacename = $1;
            last;
        }
    }

    if ( $replacename ne "" )
    {
        my $language_block = get_language_block_from_language_file($replacename, $ulffile);
        $modulename = get_language_string_from_language_block($language_block, "en-US", $replacename);
    }

    return $modulename;
}

#################################################################
# Setting for Solaris the package names in the xml file.
# This name is displayed tools like prodreg.
# Unfortunately this name in the component is also used
# in the translation template file for the module name
# and module description translations.
#################################################################

sub replace_component_names
{
    my ($xmlfile, $templatefilename, $modulesarrayref, $javatemplateorigfile, $ulffile) = @_;

    # path in which all java languages files are located

    my $javafilesdir = $templatefilename;
    installer::pathanalyzer::get_path_from_fullqualifiedname(\$javafilesdir);
    my $alljavafiles = installer::systemactions::find_file_with_file_extension("java", $javafilesdir);
    for ( my $i = 0; $i <= $#{$alljavafiles}; $i++ ) { ${$alljavafiles}[$i] = $javafilesdir . ${$alljavafiles}[$i]; }

    # analyzing the xml file

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        my $newstring = "";
        my $componentname = "";

        if ( ${$xmlfile}[$i] =~ /\bcomponent\b.*\bname\s*\=\'\s*(.*?)\s*\'/ )
        {
            $componentname = $1;

            # Getting module name from the scp files in $modulesarrayref

            my $onelanguage = "en-US";
            my $gid = $componentname;
            my $type = "Name";

            my $modulename = "";
            $modulename = get_module_name_description($modulesarrayref, $onelanguage, $gid, $type);

            if ( $modulename eq "" )
            {
                $infoline = "Info: Modulename for $gid not defined in modules collector. Looking in Java ulf file.\n";
                push( @installer::globals::logfileinfo, $infoline);
            }

            if ( $modulename eq "" ) # the modulename can also be set in the Java ulf file
            {
                $modulename = get_module_name_from_basejavafile($componentname, $javatemplateorigfile, $ulffile);
            }

            if ( $modulename ne "" )    # only do something, if the modulename was found
            {
                ${$xmlfile}[$i] =~ s/$componentname/$modulename/;

                $infoline = "Replacement in xml file (Solaris): $componentname \-\> $modulename\n";
                push( @installer::globals::logfileinfo, $infoline);

                # Replacement has to be done in all Java language files
                replace_component_name_in_java_file($alljavafiles, $componentname, $modulename);
            }

            if ( $modulename eq "" ) # the modulename can also be set in the Java ulf file
            {
                $infoline = "WARNING: No replacement in xml file for component: $componentname\n";
                push( @installer::globals::logfileinfo, $infoline);
            }
        }
    }
}

#############################################################################
# Collecting all packages or rpms located in the installation directory
#############################################################################

sub get_all_packages_in_installdir
{
    my ($installdir, $subdir) = @_;

    my $infoline = "";

    my @allrpms = ();   # not needed for Solaris at the moment
    my $allrpms = \@allrpms;

    $installdir =~ s/\Q$installer::globals::separator\E\s*$//;
    my $directory = $installdir . $installer::globals::separator . $subdir;
    $directory =~ s/\Q$installer::globals::separator\E\s*$//;

    if ( $installer::globals::isrpmbuild )
    {
        $allrpms = installer::systemactions::find_file_with_file_extension("rpm", $directory);

        # collecting rpms with the complete path

        for ( my $i = 0; $i <= $#{$allrpms}; $i++ )
        {
            ${$allrpms}[$i] = $directory . $installer::globals::separator . ${$allrpms}[$i];
            $infoline = "Found RPM: ${$allrpms}[$i]\n";
            push( @installer::globals::logfileinfo, $infoline);
        }
    }

    return $allrpms;
}

#######################################################
# Adding the values of the array
#######################################################

sub do_sum
{
    my ( $allnumbers ) = @_;

    my $sum = 0;

    for ( my $i = 0; $i <= $#{$allnumbers}; $i++ )
    {
        $sum = $sum + ${$allnumbers}[$i];
    }

    return $sum;
}

#######################################################
# Setting the filesize for the RPMs in the xml file
#######################################################

sub set_filesize_in_xmlfile
{
    my ($filesize, $rpmname, $xmlfile) = @_;

    my $infoline = "";
    my $foundrpm = 0;
    my $filesizeset = 0;

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        my $line = ${$xmlfile}[$i];

        # searching for "rpmPath="RPMS/${UNIXPRODUCTNAME}-core01-${PACKAGEVERSION}-${PACKAGEREVISION}.i586.rpm""

        if (( $line =~ /rpmPath\s*=/ ) && ( $line =~ /\Q$rpmname\E\"\s*$/ ))
        {
            $foundrpm = 1;

            my $number = $i;
            $number++;

            while ( ! ( ${$xmlfile}[$number] =~ /\/\>\s*$/ ))
            {
                if ( ${$xmlfile}[$number] =~ /FILESIZEPLACEHOLDER/ )
                {
                    ${$xmlfile}[$number] =~ s/FILESIZEPLACEHOLDER/$filesize/;
                    $filesizeset = 1;
                    $infoline = "Setting filesize for $rpmname : $filesize\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    last;
                }

                $number++;
            }

            last;
        }
    }

    if ( ! $foundrpm )
    {
        $infoline = "ERROR: Did not find $rpmname in xml file !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    if ( ! $filesizeset )
    {
        $infoline = "ERROR: Did not set filesize for $rpmname in xml file !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

############################################################
# Collecting all rpmUniqueName in xml file.
############################################################

sub collect_uniquenames_in_xmlfile
{
    my ($xmlfile) = @_;

    my @rpmuniquenames = ();

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        my $oneline = ${$xmlfile}[$i];

        if ( $oneline =~ /^\s*rpmUniqueName\s*\=\s*\"(.*)\"\s*$/ )
        {
            my $rpmuniquename = $1;
            push(@rpmuniquenames, $rpmuniquename)
        }
    }

    return \@rpmuniquenames;
}

############################################################
# Searching for the corresponding rpm, that fits to
# the unique rpm name.
# Simple mechanism: The name of the rpm starts with the
# unique rpm name followed by a "-".
############################################################

sub find_rpmname_to_uniquename
{
    my ($uniquename, $listofpackages) = @_;

    my @all_correct_rpms = ();
    my $infoline = "";

    # special handling for java RPMs, which have a very strange naming schema
    my $localuniquename = $uniquename;
    if ( $uniquename =~ /^\s*jre\-/ ) { $localuniquename = "jre"; }

    for ( my $i = 0; $i <= $#{$listofpackages}; $i++ )
    {
        my $completerpmname = ${$listofpackages}[$i];
        my $rpmname = $completerpmname;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$rpmname);

        if ( $rpmname =~ /^\s*\Q$localuniquename\E\-\d/ ) { push(@all_correct_rpms, $rpmname); }
    }

    # @all_correct_rpms has to contain exactly one value

    if ( $#all_correct_rpms > 0 )
    {
        my $number = $#all_correct_rpms + 1;
        $infoline = "There are $number RPMs for the unique name \"$uniquename\" :\n";
        push( @installer::globals::logfileinfo, $infoline);
        my $allrpmstring = "";
        for ( my $i = 0; $i <= $#all_correct_rpms; $i++ ) { $allrpmstring = $allrpmstring . $all_correct_rpms[$i] . "\n"; }
        push( @installer::globals::logfileinfo, $allrpmstring);
        installer::exiter::exit_program("ERROR: Found $number RPMs that start with unique name \"$uniquename\". Only one allowed!", "find_rpmname_to_uniquename");
    }

    if ( $#all_correct_rpms < 0 )
    {
        $infoline = "There is no rpm for the unique name \"$uniquename\"\n";
        push( @installer::globals::logfileinfo, $infoline);
        installer::exiter::exit_program("ERROR: There is no RPM that start with unique name \"$uniquename\"!", "find_rpmname_to_uniquename");
    }

    if ( $#all_correct_rpms == 0 )
    {
        $infoline = "Found one rpm for the unique name \"$uniquename\" : $all_correct_rpms[0]\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $all_correct_rpms[0];
}

#######################################################
# Including the complete RPM name into the xml file
#######################################################

sub set_rpmname_into_xmlfile
{
    my ($rpmname, $uniquename, $xmlfile) = @_;

    my $foundrpm = 0;
    my $rpmnameset = 0;

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        my $oneline = ${$xmlfile}[$i];

        if ( $oneline =~ /^\s*rpmUniqueName\s*\=\s*\"\Q$uniquename\E\"\s*$/ )
        {
            $foundrpm = 1;

            my $number = $i;
            $number++;

            while ( ! ( ${$xmlfile}[$number] =~ /\/\>\s*$/ ))
            {
                if ( ${$xmlfile}[$number] =~ /RPMFILENAMEPLACEHOLDER/ )
                {
                    ${$xmlfile}[$number] =~ s/RPMFILENAMEPLACEHOLDER/$rpmname/;
                    $rpmnameset = 1;
                    $infoline = "Setting RPM name for $uniquename : $rpmname\n";
                    push( @installer::globals::logfileinfo, $infoline);
                    last;
                }

                $number++;
            }

            last;
        }
    }

    if ( ! $foundrpm )
    {
        $infoline = "ERROR: Did not find $rpmname in xml file !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    if ( ! $rpmnameset )
    {
        $infoline = "ERROR: Did not set rpm name for $uniquename in xml file !\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

}

############################################################
# Including the rpm path dynamically into the xml file.
# This is introduced, because system integration has
# variable PackageVersion and PackageRevision in xml file.
############################################################

sub put_rpmpath_into_xmlfile
{
    my ($xmlfile, $listofpackages) = @_;

    my $infoline = "";

    my $alluniquenames = collect_uniquenames_in_xmlfile($xmlfile);

    my $number = $#{$listofpackages} + 1;
    $infoline = "Number of packages in installation set: $number\n";
    push( @installer::globals::logfileinfo, $infoline);
    $number = $#{$alluniquenames} + 1;
    $infoline = "Number of unique RPM names in xml file: $number\n";
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "\nPackages in installation set:\n";
    push( @installer::globals::logfileinfo, $infoline);
    for ( my $i = 0; $i <= $#{$listofpackages}; $i++ )
    {
        $infoline = "${$listofpackages}[$i]\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    $infoline = "\nUnique RPM names in xml file:\n";
    push( @installer::globals::logfileinfo, $infoline);
    for ( my $i = 0; $i <= $#{$alluniquenames}; $i++ )
    {
        $infoline = "${$alluniquenames}[$i]\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    if ( $#{$alluniquenames} != $#{$listofpackages} ) { installer::exiter::exit_program("ERROR: xml file contains $#{$alluniquenames} unique names, but there are $#{$listofpackages} packages in installation set!", "put_rpmpath_into_xmlfile"); }

    for ( my $i = 0; $i <= $#{$alluniquenames}; $i++ )
    {
        my $uniquename = ${$alluniquenames}[$i];
        my $rpmname = find_rpmname_to_uniquename($uniquename, $listofpackages);
        set_rpmname_into_xmlfile($rpmname, $uniquename, $xmlfile);
    }
}

#######################################################
# Including the file size of the rpms into the
# xml file
#######################################################

sub put_filesize_into_xmlfile
{
    my ($xmlfile, $listofpackages) = @_;

    my $infoline = "";

    for ( my $i = 0; $i <= $#{$listofpackages}; $i++ )
    {
        my $completerpmname = ${$listofpackages}[$i];
        my $rpmname = $completerpmname;
        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$rpmname);

        if ( ! $installer::globals::rpmquerycommand ) { installer::exiter::exit_program("ERROR: rpm not found for querying packages!", "put_filesize_into_xmlfile"); }
        my $systemcall = "$installer::globals::rpmquerycommand -qp --queryformat \"\[\%\{FILESIZES\}\\n\]\" $completerpmname 2\>\&1 |";
        my $rpmout = make_systemcall($systemcall, 0);
        my $filesize = do_sum($rpmout);

        $infoline = "Filesize $rpmname : $filesize\n";
        push( @installer::globals::logfileinfo, $infoline);

        set_filesize_in_xmlfile($filesize, $rpmname, $xmlfile);
    }
}

#######################################################
# Creating the java installer class file dynamically
#######################################################

sub create_java_installer
{
    my ( $installdir, $newdir, $languagestringref, $languagesarrayref, $allvariableshashref, $includepatharrayref, $modulesarrayref ) = @_;

    installer::logger::include_header_into_logfile("Creating Java installer:");

    my $infoline = "";

    # collecting all packages or rpms located in the installation directory
    my $listofpackages = get_all_packages_in_installdir($installdir, $newdir);

    # creating the directory
    my $javadir = installer::systemactions::create_directories("javainstaller", $languagestringref);
    $javadir =~ s/\/\s*$//;

    # copying the content from directory install_sdk into the java directory

    my $projectroot = "";
    if ( $ENV{'PRJ'} ) { $projectroot = $ENV{'PRJ'}; }
    else { installer::exiter::exit_program("ERROR: Environment variable PRJ not set", "create_java_installer"); }

    $projectroot =~ s/\/\s*$//;
    my $sourcedir = "$projectroot/inc_global/unix/install_sdk";
    installer::systemactions::copy_complete_directory_without_cvs($sourcedir, $javadir);

    # determining the java template file

    my $templatefilename = $javadir . $installer::globals::separator . "locale/resources/MyResources_template.java";

    # Saving the content of the template file. It is used in the xml files

    my $javatemplateorigfile = installer::files::read_file($templatefilename);

    # determining the ulf language file

    my $ulffilename = "installsdk.jlf";
    $ulffilename = $installer::globals::javalanguagepath . $installer::globals::separator . $ulffilename;
    my $ulffile = installer::files::read_file($ulffilename);

    $infoline = "\nReading ulf file: $ulffilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    $infoline = "Translating the Java template file\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$languagesarrayref}; $i++ )
    {
        my $onelanguage = ${$languagesarrayref}[$i];

        # replacing all strings in the Java file with content of ulf files

        my $templatefile = installer::files::read_file($templatefilename);

        set_component_name_and_description($templatefile, $modulesarrayref, $onelanguage);
        translate_javafile($templatefile, $ulffile, $onelanguage);

        # adding the license file into the Java file

        my $licensefilesource = get_licensefilesource($onelanguage, $includepatharrayref);
        my $licensefile = installer::files::read_file($licensefilesource);
        add_license_file_into_javafile($templatefile, $licensefile, $includepatharrayref, $javadir, $onelanguage);

        # setting productname and productversion

        set_productname_and_productversion($templatefile, $allvariableshashref);

        # setting the class name in the java file ( "MyResources_TEMPLATE" -> "MyResources_en" )

        $onelanguage =~ s/en-US/en/;    # java file name and class name contain only "_en"
        $onelanguage =~ s/\-/\_/;       # "pt-BR" -> "pt_BR"
        my $classfilename = "MyResources_" . $onelanguage;
        set_classfilename($templatefile, $classfilename, "MyResources_TEMPLATE");

        # saving the new file

        my $newfilename = $templatefilename;
        $newfilename =~ s/_template\.java\s*$/_$onelanguage\.java/;

        installer::files::save_file($newfilename, $templatefile);

        $infoline = "Saving Java file: $newfilename\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    # renaming one language java file to "MyResources.java"

    my $baselanguage = installer::languages::get_default_language($languagesarrayref);
    $baselanguage =~ s/\-/\_/;      # "pt-BR" -> "pt_BR"
    $baselanguage =~ s/en_US/en/;   # java file name and class name contain only "_en"
    my $baselanguagefilename = $javadir . $installer::globals::separator . "locale/resources/MyResources_" . $baselanguage . "\.java";
    my $basedestfilename = $javadir . $installer::globals::separator . "locale/resources/MyResources.java";
    installer::systemactions::copy_one_file($baselanguagefilename, $basedestfilename);

    # setting the class file name also for the base class

    my $basetemplatefile = installer::files::read_file($basedestfilename);
    my $oldclassfilename = $baselanguagefilename;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$oldclassfilename);
    $oldclassfilename =~ s/\.java//;
    my $newclassfilename = $basedestfilename;
    installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$newclassfilename);
    $newclassfilename =~ s/\.java//;

    set_classfilename($basetemplatefile, $newclassfilename, $oldclassfilename);

    installer::files::save_file($basedestfilename, $basetemplatefile);

    $infoline = "Created base Java file: $basedestfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    # deleting the template file

    unlink($templatefilename);

    $infoline = "Deleted template Java resource file: $templatefilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    # changing into Java directory

    my $from = cwd();

    chdir($javadir);

    $infoline = "Changing into directory: $javadir\n";
    push( @installer::globals::logfileinfo, $infoline);

    # preparing the xml file

    my $xmlfilename = "";
    my $subdir = "";

    if ( $installer::globals::issolarispkgbuild )
    {
        $xmlfilename = "pkgUnit.xml";
    }
    elsif ( $installer::globals::isrpmbuild )
    {
        $xmlfilename = "rpmUnit.xml";
    }
    else
    {
        installer::exiter::exit_program("ERROR: No platform for Install SDK", "create_java_installer");
    }

    # reading, editing and saving the xmlfile

    my $xmlfile = installer::files::read_file($xmlfilename);
    prepare_language_pack_in_xmlfile($xmlfile, $languagesarrayref);
    my $xmlfilename2 = $xmlfilename . ".test2";
    installer::files::save_file($xmlfilename2, $xmlfile);
    remove_empty_packages_in_xmlfile($xmlfile);
    my $xmlfilename3 = $xmlfilename . ".test3";
    installer::files::save_file($xmlfilename3, $xmlfile);
    substitute_variables($xmlfile, $allvariableshashref);
    if (( $installer::globals::isrpmbuild ) && ( $#installer::globals::linkrpms > -1 )) { prepare_linkrpm_in_xmlfile($xmlfile,\@installer::globals::linkrpms); }
    if ( $installer::globals::issolarisx86build || $installer::globals::islinuxbuild ) { remove_w4w_from_xmlfile($xmlfile); }
    remove_module_if_not_defined($xmlfile, $modulesarrayref, "gid_Module_Optional_Onlineupdate");
    replace_component_names($xmlfile, $templatefilename, $modulesarrayref, $javatemplateorigfile, $ulffile);
    my $xmlfilename4 = $xmlfilename . ".test4";
    installer::files::save_file($xmlfilename4, $xmlfile);
    if ( $installer::globals::isrpmbuild ) { put_rpmpath_into_xmlfile($xmlfile, $listofpackages); }
    if ( $installer::globals::isrpmbuild ) { put_filesize_into_xmlfile($xmlfile, $listofpackages); }
    installer::files::save_file($xmlfilename, $xmlfile);
    $infoline = "Saving xml file: $xmlfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    # Setting the classpath and starting compiler

    set_classpath_for_install_sdk($javadir);

    # creating class files:
    # language class file, dialog class files, installer class file

    my $jdkpath = "";
    if ( $ENV{'JDKPATH'} ) { $jdkpath = $ENV{'JDKPATH'}; }

    my $javac = "javac";
    if ( $jdkpath ) { $javac = $jdkpath . $installer::globals::separator . $javac; }

    my $systemcall = "$javac locale\/resources\/\*\.java 2\>\&1 |";
    make_systemcall($systemcall, 1);

    $systemcall = "$javac com\/sun\/staroffice\/install\/\*\.java 2\>\&1 |";
    make_systemcall($systemcall, 1);

    # making subdirectory creating empty packages
    create_empty_packages($xmlfile);

    # Copy "jresetup" from solver locally to include it into the classfile
    # Copy "jresetup" from solver to installdir

    my $setupname = "jresetup";
    my $newname = "setup";
    copy_setup_locally($includepatharrayref, $setupname, $newname);

    my $java = "java";
    if ( $jdkpath ) { $java = $jdkpath . $installer::globals::separator . $java; }

    $systemcall = "$java com.sun.setup.builder.InstallBuilder $xmlfilename -novalidate 2\>\&1 |";
    make_systemcall($systemcall, 1);

    # copying the newly created classfile into the installation set

    my $archivefilename = get_archivefilename($xmlfile);
    $archivefilename = $archivefilename . ".class";

    if ( ! -f $archivefilename ) { installer::exiter::exit_program("ERROR: Could not find Java class file $archivefilename!", "create_java_installer"); }

    installer::systemactions::copy_one_file($archivefilename, $installdir);

    # Adding the loader into the installation set. The name of the loader is setup.
    put_loader_into_installset($installdir, $newname);

    chdir($from);

    $infoline = "Changing into directory: $from\n";
    push( @installer::globals::logfileinfo, $infoline);
}

1;
