#*************************************************************************
#
#   $RCSfile: javainstaller.pm,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2004-08-12 08:29:00 $
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

package installer::javainstaller;

use Cwd;
use installer::exiter;
use installer::files;
use installer::globals;
use installer::languages;
use installer::pathanalyzer;
use installer::systemactions;

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

    my $newstring = "";

    for ( my $i = 0; $i <= $#{$modulesarrayref}; $i++ )
    {
        my $onemodule = ${$modulesarrayref}[$i];

        my $found = 0;

        if ( $onemodule->{'gid'} eq $gid )
        {
            if ( $onemodule->{'ismultilingual'} )
            {
                if ( $onemodule->{'specificlanguage'} eq $onelanguage )
                {
                    $newstring = $onemodule->{$type};
                    $found = 1;
                }
            }
            else
            {
                $newstring = $onemodule->{$type};
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
        ${$templatefile}[$i] =~ s/\{0\}/$productname/g;
        ${$templatefile}[$i] =~ s/\{1\}/$productversion/g;
    }

    $infoline = "End of: Setting component names and description in Java template file\n\n";
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

            $infoline = "\tReplacing: OLDSTRING: $oldstring NEWSTRING $newstring\n";
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

            # if (!( $newstring eq "" )) { ${$idtfile}[$i] =~ s/$oldstring/$newstring/; }
            ${$templatefile}[$i] =~ s/$oldstring/$newstring/;   # always substitute, even if $newstring eq ""
        }
    }
}

###########################################################
# Returning the license file name for a defined language
###########################################################

sub get_licensefilesource
{
    my ($language, $filesref) = @_;

    my $licensefilename = "LICENSE_" . $language;
    my $sourcepath = "";
    my $foundlicensefile = 0;

    for ( my $i = 0; $i <= $#{$filesref}; $i++ )
    {
        my $onefile = ${$filesref}[$i];
        my $filename = $onefile->{'Name'};

        if ($filename eq $licensefilename)
        {
            $sourcepath = $onefile->{'sourcepath'};
            $foundlicensefile = 1;
            last;
        }
    }

    if ( ! $foundlicensefile ) { installer::exiter::exit_program("ERROR: Did not find file $licensefilename in file collector!", "get_licensefilesource"); }

    return $sourcepath;
}

#######################################################
# Adding the license file into the java file
# In the template java file there are two
# occurences of INSTALLSDK_GUI_LICENSE
# and INSTALLSDK_CONSOLE_LICENSE
#######################################################

sub add_license_file_into_javafile
{
    my ( $templatefile, $licensefile ) = @_;

    my $licensestring = "";

    for ( my $i = 0; $i <= $#{$licensefile}; $i++ )
    {
        my $oneline = ${$licensefile}[$i];
        $oneline =~ s/\s*$//;
        $oneline =~ s/\"/\\\"/g;    # masquerading the "
        $oneline =~ s/\'/\\\'/g;    # masquerading the '
        $licensestring = $licensestring . $oneline . "\\n";
    }

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
    my ( $systemcall ) = @_;

    my @returns = ();

    print "... $systemcall ...\n";

    open (REG, "$systemcall");
    while (<REG>) {push(@returns, $_); }
    close (REG);

    my $returnvalue = $?;   # $? contains the return value of the systemcall

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $j = 0; $j <= $#returns; $j++ ) { push( @installer::globals::logfileinfo, "$returns[$j]"); }

    if ($returnvalue)
    {
        $infoline = "ERROR: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
        $error_occured = 1;
    }
    else
    {
        $infoline = "SUCCESS: $systemcall\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
}

#######################################################
# Setting the class path for the Installer SDK
#######################################################

sub set_classpath_for_install_sdk
{
    my ( $directory ) = @_;

    my $installsdk = "";

    if ( $ENV{'ENV_ROOT'} ) { $installsdk = $ENV{'ENV_ROOT'}; }
    else { installer::exiter::exit_program("ERROR: Environment variable \"ENV_ROOT\" not set!", "set_classpath_for_install_sdk"); }

    $installsdk = $installsdk . $installer::globals::separator . "InstallSDK";

    # The variable CLASSPATH has to contain:
    # $installsdk/classes:$installsdk/classes/setupsdk.jar:
    # $installsdk/classes/parser.jar:$installsdk/classes/jaxp.jar:
    # $installsdk/classes/ldapjdk.jar:$directory

    my @additional_classpath = ();
    push(@additional_classpath, "$installsdk\/classes");
    push(@additional_classpath, "$installsdk\/classes\/setupsdk.jar");
    push(@additional_classpath, "$installsdk\/classes\/parser.jar");
    push(@additional_classpath, "$installsdk\/classes\/jaxp.jar");
    push(@additional_classpath, "$installsdk\/classes\/ldapjdk.jar");
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

###########################################################
# Removing Ada product from xml file for Solaris x86
###########################################################

sub remove_ada_from_xmlfile
{
    my ($xmlfile) = @_;

    # Component begins with "<component selected='true' name='gid_Module_Optional_Adabas' componentVersion="12">"
    # and ends with "</component>"

    for ( my $i = 0; $i <= $#{$xmlfile}; $i++ )
    {
        if ( ${$xmlfile}[$i] =~ /name\s*\=\'\s*gid_Module_Optional_Adabas/ )
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

###########################################################
# Adding the lowercase variables into the variableshashref
###########################################################

sub add_lowercasevariables_to_allvariableshashref
{
    my ($variableshashref) = @_;

    my $lcvariable = "";

    $lcvariable = lc($variableshashref->{'PRODUCTNAME'});
    $variableshashref->{'LCPRODUCTNAME'} = $lcvariable;

    if ($variableshashref->{'SHORT_PRODUCTEXTENSION'})
    {
        $lcvariable = "\-" . lc($variableshashref->{'SHORT_PRODUCTEXTENSION'}); # including the "-" !
        $variableshashref->{'LCPRODUCTEXTENSION'} = $lcvariable;
    }
    else
    {
        $variableshashref->{'LCPRODUCTEXTENSION'} = "";
    }
}

###########################################################
# Preparing the package subdirectory
###########################################################

sub create_empty_packages
{
    my ( $xmlfile ) = @_;

    if ( $installer::globals::issolarisbuild )
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

    if ( $installer::globals::islinuxbuild )
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
    my $localcall = "chmod 775 $newname \>\/dev\/null 2\>\&1";
    system($localcall);
}


#######################################################
# Copying the loader into the installation set
#######################################################

sub put_loader_into_installset
{
    my ($installdir, $filename) = @_;

    my $installname = $installdir . $installer::globals::separator . $filename;

    installer::systemactions::copy_one_file($filename, $installname);

    my $localcall = "chmod 775 $installname \>\/dev\/null 2\>\&1";
    system($localcall);
}

#######################################################
# Creating the java installer class file dynamically
#######################################################

sub create_java_installer
{
    my ( $installdir, $languagestringref, $languagesarrayref, $filesarrayref, $allvariableshashref, $includepatharrayref, $modulesarrayref ) = @_;

    installer::logger::include_header_into_logfile("Creating Java installer:");

    my $infoline = "";

    # creating the directory
    my $javadir = installer::systemactions::create_directories("javainstaller", $languagestringref);
    $javadir =~ s/\/\s*$//;
    push(@installer::globals::removedirs, $javadir);

    # copying the content from directory install_sdk into the java directory

    my $sourcedir = "../inc_global/unix/install_sdk";
    installer::systemactions::copy_complete_directory_without_cvs($sourcedir, $javadir);

    # determining the java template file

    my $templatefilename = $javadir . $installer::globals::separator . "locale/resources/MyResources_template.java";

    # determining the ulf language file

    my $ulffilename = "installsdk.ulf";
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

        my $licensefilesource = get_licensefilesource($onelanguage, $filesarrayref);
        my $licensefile = installer::files::read_file($licensefilesource);
        add_license_file_into_javafile($templatefile, $licensefile);

        # setting productname and productversion

        set_productname_and_productversion($templatefile, $allvariableshashref);

        # setting the class name in the java file ( "MyResources_TEMPLATE" -> "MyResources_en" )

        $onelanguage =~ s/en-US/en/;     # java file name and class name contain only "_en"
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
    $baselanguage =~ s/en-US/en/;    # java file name and class name contain only "_en"
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

    # Setting the classpath

    set_classpath_for_install_sdk($javadir);

    # creating class files:
    # language class file, dialog class files, installer class file

    my $from = cwd();

    chdir($javadir);

    $infoline = "Changing into directory: $javadir\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $jdkpath = "";
    if ( $ENV{'JDKPATH'} ) { $jdkpath = $ENV{'JDKPATH'}; }

    my $javac = "javac";
    if ( $jdkpath ) { $javac = $jdkpath . $installer::globals::separator . $javac; }

    my $systemcall = "$javac locale\/resources\/\*\.java 2\>\&1 |";
    make_systemcall($systemcall);

    $systemcall = "$javac com\/sun\/staroffice\/install\/\*\.java 2\>\&1 |";
    make_systemcall($systemcall);

    my $xmlfilename = "";
    my $subdir = "";

    if ( $installer::globals::issolarisbuild )
    {
        $xmlfilename = "pkgUnit.xml";
    }
    elsif ( $installer::globals::islinuxbuild )
    {
        $xmlfilename = "rpmUnit.xml";
    }
    else
    {
        installer::exiter::exit_program("ERROR: No platform for Install SDK", "create_java_installer");
    }

    # reading, editing and saving the xmlfile

    my $xmlfile = installer::files::read_file($xmlfilename);
    add_lowercasevariables_to_allvariableshashref($allvariableshashref);
    substitute_variables($xmlfile, $allvariableshashref);
    if ( $installer::globals::issolarisx86build ) { remove_ada_from_xmlfile($xmlfile); }
    installer::files::save_file($xmlfilename, $xmlfile);
    $infoline = "Saving xml file: $xmlfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

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
    make_systemcall($systemcall);

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
