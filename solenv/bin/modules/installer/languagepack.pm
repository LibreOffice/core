#*************************************************************************
#
#   $RCSfile: languagepack.pm,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: obo $ $Date: 2004-10-18 13:52:41 $
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

package installer::languagepack;

use installer::existence;
use installer::files;
use installer::logger;
use installer::pathanalyzer;
use installer::scriptitems;
use installer::systemactions;
use installer::worker;

####################################################
# Selecting all files with the correct language
####################################################

sub select_language_items
{
    my ( $itemsref, $languagesarrayref, $itemname ) = @_;

    installer::logger::include_header_into_logfile("Selecting languages for language pack. Item: $itemname");

    my @itemsarray = ();

    for ( my $i = 0; $i <= $#{$itemsref}; $i++ )
    {
        my $oneitem = ${$itemsref}[$i];

        my $ismultilingual = $oneitem->{'ismultilingual'};

        if (!($ismultilingual))
        {
            next;   # single language files are not included into language pack
        }

        my $specificlanguage = "";
        if ( $oneitem->{'specificlanguage'} ) { $specificlanguage = $oneitem->{'specificlanguage'}; }

        for ( my $j = 0; $j <= $#{$languagesarrayref}; $j++ )   # iterating over all languages
        {
            my $onelanguage = ${$languagesarrayref}[$j];

            if ( $specificlanguage eq $onelanguage )
            {
                $oneitem->{'modules'} = "gid_Module_Root";   # all files in a language pack are root files

                # preparing different modules for Windows Installer language packs
                if ( $installer::globals::iswindowsbuild ) { $oneitem->{'modules'} = "gid_Module_Langpack_" . $specificlanguage; }

                if (! installer::existence::exists_in_array($oneitem->{'modules'}, \@installer::globals::languagepackfeature))  # DDD
                {
                    push(@installer::globals::languagepackfeature, $oneitem->{'modules'});  # Collecting all language pack feature # DDD
                }

                push(@itemsarray, $oneitem);
            }
        }
    }

    return \@itemsarray;
}

sub replace_languagestring_variable
{
    my ($onepackageref, $languagestringref) = @_;

    my $key;

    foreach $key (keys %{$onepackageref})
    {
        my $value = $onepackageref->{$key};
        $value =~ s/\%LANGUAGESTRING/$$languagestringref/g;
        $onepackageref->{$key} = $value;
    }
}

#########################################################
# Including the license text into the script template
#########################################################

sub put_license_file_into_script
{
    my ($scriptfile, $licensefile) = @_;

    my $infoline = "Adding licensefile into language pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    my $includestring = "";

    for ( my $i = 0; $i <= $#{$licensefile}; $i++ )
    {
        $includestring = $includestring . ${$licensefile}[$i];
    }

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/LICENSEFILEPLACEHOLDER/$includestring/;
    }
}

#########################################################
# Creating a tar.gz file from a Solaris package
#########################################################

sub create_tar_gz_file
{
    my ($installdir, $packagename) = @_;

    my $targzname = $packagename . ".tar.gz";
    $systemcall = "cd $installdir; tar -cf - $packagename | gzip > $targzname";
    print "... $systemcall ...\n";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    return $targzname;
}

#########################################################
# Determining the name of the package file or the rpm
# in the installation directory. For language packs
# there is only one file in this directory
#########################################################

sub determine_packagename
{
    my ( $installdir ) = @_;

    my $packagename = "";

    if ( $installer::globals::islinuxrpmbuild )
    {
        # determining the rpm file in directory $installdir

        my $fileextension = "rpm";
        my $rpmfiles = installer::systemactions::find_file_with_file_extension($fileextension, $installdir);
        $packagename = ${$rpmfiles}[0]; # only one rpm in language pack
        if ( $packagename eq "" ) { installer::exiter::exit_program("ERROR: Could not find package in directory $installdir!", "determine_packagename"); }

    }

    if ( $installer::globals::issolarisbuild )
    {
        # determining the rpm file in directory $installdir
        my $alldirs = installer::systemactions::get_all_directories($installdir);
        $packagename = ${$alldirs}[0]; # only one Solaris package in language pack
        if ( $packagename eq "" ) { installer::exiter::exit_program("ERROR: Could not find package in directory $installdir!", "determine_packagename"); }

        installer::pathanalyzer::make_absolute_filename_to_relative_filename(\$packagename);
        $packagename = create_tar_gz_file($installdir, $packagename);   # only a file (not a directory) can be included into the shell script
        installer::systemactions::remove_complete_directory(${$alldirs}[0], 1)
    }

    my $infoline = "Found package in installation directory $installdir : $packagename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $packagename;
}

#########################################################
# Including the name of the package file or the rpm
# into the script template
#########################################################

sub put_packagename_into_script
{
    my ($scriptfile, $packagename) = @_;

    my $localpackagename = $packagename;
    $localpackagename =~ s/\.tar\.gz//; # making "OOOopenoffice-it-ea.tar.gz" to "OOOopenoffice-it-ea"
    my $infoline = "Adding packagename $localpackagename into language pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/PACKAGENAMEPLACEHOLDER/$localpackagename/;
    }
}

##################################################################
# Including the lowercase product name into the script template
##################################################################

sub put_productname_into_script
{
    my ($scriptfile, $variableshashref) = @_;

    my $productname = $variableshashref->{'PRODUCTNAME'};
    $productname = lc($productname);

    my $infoline = "Adding productname $productname into language pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/PRODUCTNAMEPLACEHOLDER/$productname/;
    }
}

#########################################################
# Including the linenumber into the script template
#########################################################

sub put_linenumber_into_script
{
    my ( $scriptfile, $licensefile ) = @_;

    my $linenumber =  $#{$scriptfile} + $#{$licensefile} + 3;   # also adding the content of the license file!

    my $infoline = "Adding linenumber $linenumber into language pack script\n";
    push( @installer::globals::logfileinfo, $infoline);

    for ( my $i = 0; $i <= $#{$scriptfile}; $i++ )
    {
        ${$scriptfile}[$i] =~ s/LINENUMBERPLACEHOLDER/$linenumber/;
    }
}

#########################################################
# Determining the name of the new scriptfile
#########################################################

sub determine_scriptfile_name
{
    my ( $packagename ) = @_;

    my $scriptfilename = $packagename;

    if ( $installer::globals::islinuxrpmbuild ) { $scriptfilename =~ s/\.rpm\s*$/\.sh/; }

    if ( $installer::globals::issolarisbuild ) { $scriptfilename =~ s/\.tar\.gz\s*$/\.sh/; }

    my $infoline = "Setting language pack script file name to $scriptfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $scriptfilename;
}

#########################################################
# Saving the script file in the installation directory
#########################################################

sub save_script_file
{
    my ($installdir, $newscriptfilename, $scriptfile) = @_;

    $newscriptfilename = $installdir . $installer::globals::separator . $newscriptfilename;
    installer::files::save_file($newscriptfilename, $scriptfile);

    my $infoline = "Saving script file $newscriptfilename\n";
    push( @installer::globals::logfileinfo, $infoline);

    return $newscriptfilename;
}

#########################################################
# Including the binary package into the script
#########################################################

sub include_package_into_script
{
    my ( $scriptfilename, $installdir, $packagename ) = @_;

    my $longpackagename = $installdir . $installer::globals::separator . $packagename;
    my $systemcall = "cat $longpackagename >>$scriptfilename";

    my $returnvalue = system($systemcall);

    my $infoline = "Systemcall: $systemcall\n";
    push( @installer::globals::logfileinfo, $infoline);

    if ($returnvalue)
    {
        $infoline = "ERROR: Could not execute \"$systemcall\"!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }
    else
    {
        $infoline = "Success: Executed \"$systemcall\" successfully!\n";
        push( @installer::globals::logfileinfo, $infoline);
    }

    my $localcall = "chmod 775 $scriptfilename \>\/dev\/null 2\>\&1";
    system($localcall);

}

#########################################################
# Removing the binary package
#########################################################

sub remove_package
{
    my ( $installdir, $packagename ) = @_;

    my $longpackagename = $installdir . $installer::globals::separator . $packagename;
    unlink $longpackagename;

    my $infoline = "Removing package: $longpackagename \n";
    push( @installer::globals::logfileinfo, $infoline);
}

####################################################
# Unix language packs, that are not part of
# multilingual installation sets, need a
# shell script installer
####################################################

sub build_installer_for_languagepack
{
    my ($installdir, $allvariableshashref, $includepatharrayref, $languagesarrayref) = @_;

    print "... creating shell script installer ...\n";

    installer::logger::include_header_into_logfile("Creating shell script installer:");

    # find and read setup script template

    my $scriptfilename = "langpackscript.sh";
    my $scriptref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$scriptfilename, $includepatharrayref, 0);
    if ($$scriptref eq "") { installer::exiter::exit_program("ERROR: Could not find script file $scriptfilename!", "build_installer_for_languagepack"); }
    my $scriptfile = installer::files::read_file($$scriptref);

    my $infoline = "Found  script file $scriptfilename: $$scriptref \n";
    push( @installer::globals::logfileinfo, $infoline);

    # find and read english license file
    my $licenselanguage = "en-US";                  # always english !
    my $licensefilename = "LICENSE_" . $licenselanguage;
    my $licenseincludepatharrayref = installer::worker::get_language_specific_include_pathes($includepatharrayref, $licenselanguage);

    my $licenseref = installer::scriptitems::get_sourcepath_from_filename_and_includepath(\$licensefilename, $licenseincludepatharrayref, 0);
    if ($$licenseref eq "") { installer::exiter::exit_program("ERROR: Could not find License file $licensefilename!", "build_installer_for_languagepack"); }
    my $licensefile = installer::files::read_file($$licenseref);

    $infoline = "Found licensefile $licensefilename: $$licenseref \n";
    push( @installer::globals::logfileinfo, $infoline);

    # add license text into script template
    put_license_file_into_script($scriptfile, $licensefile);

    # add rpm or package file name into script template
    my $packagename = determine_packagename($installdir);
    put_packagename_into_script($scriptfile, $packagename);

    # add product name into script template
    put_productname_into_script($scriptfile, $allvariableshashref);

    # replace linenumber in script template
    put_linenumber_into_script($scriptfile, $licensefile);

    # saving the script file
    my $newscriptfilename = determine_scriptfile_name($packagename);
    $newscriptfilename = save_script_file($installdir, $newscriptfilename, $scriptfile);

    # include rpm or package into script
    include_package_into_script($newscriptfilename, $installdir, $packagename);

    # remove rpm or package
    remove_package($installdir, $packagename);

}

1;