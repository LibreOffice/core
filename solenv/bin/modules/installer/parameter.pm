#*************************************************************************
#
#   $RCSfile: parameter.pm,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2004-07-29 16:11:05 $
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

package installer::parameter;

use Cwd;
use installer::files;
use installer::globals;
use installer::logger;
use installer::remover;
use installer::systemactions;

############################################
# Parameter Operations
############################################

sub usage
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::usage"); }

    print <<Ende;
--------------------------------------------------------------------------------
$installer::globals::prog V1.0 (c) Ingo Schmidt 2003
The following parameter are needed:
-f: Path to the product list (zip.lst) (optional, defaulted to zip.lst)
-s: Path to the setup script (optional, if defined in product list)
-i: Install path of the product (/opt/openofficeorg20) (optional)
-p: Product from product list to be created (required)
-l: Language of the product (comma and hash) (optional, defined in productlist)
-b: Build, e.g. srx645 (optional)
-m: Minor, e.g. m10 (optional)
-c: Compiler, e.g. wntmsci8, unxlngi5, unxsols4, ... (optional)
-u: Path, in which zipfiles are unpacked (optional)
-msitemplate: Source of the msi file templates (Windows compiler only)
-msilanguage: Source of the msi file templates (Windows compiler only)
-msifiles: Source of the msifiles instmsia.exe, instmsiw.exe (Windows only)
-javafiles: Source of the Java installer files  (opt., non-Windows only)
-buildid: Current BuildID (optional)
-pro: Product version
-dontunzip: do not unzip all files with flag ARCHIVE
-dontcallepm : do not call epm to create install sets (opt., non-Windows only)
-ispatchedepm : Usage of a patched (non-standard) epm (opt., non-Windows only)
-packagelist : file, containing a list of module gids (opt., non-Windows only)
-copyproject : is set for projects that are only used for copying (optional)
-languagepack : do create a languagepack, no product pack (optional)
-log : Logging all available information (optional)
-debug : Collecting debug information

Examples for Windows:

perl make_epmlist.pl -f zip.lst -s setup.inf -p OfficeFAT -l 01
                     -u /export/unpack -buildid 8712
                     -msitemplate /export/msi_files
                     -msilanguage /export/msi_languages
                     -msifiles /export/msi_files

Examples for Non-Windows:

perl make_epmlist.pl -f zip.lst -s setup.inf -p OfficeFAT -l 01
                     -u /export/unpack -buildid 8712 -i /opt/openofficeorg20
                     -packagelist packagelist.txt -ispatchedepm
--------------------------------------------------------------------------------
Ende
    exit(-1);
}

#########################################
# Writing all parameter into logfile
#########################################

sub saveparameter
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::saveparameter"); }

    my $include = "";

    installer::logger::globallog("Command line arguments:");

    for ( my $i = 0; $i <= $#ARGV; $i++ )
    {
        $include = $ARGV[$i] . "\n";
        push(@installer::globals::globallogfileinfo, $include);
    }

    # also saving global settings:

    $include = "Separator: $installer::globals::separator\n";
    push(@installer::globals::globallogfileinfo, $include);

}

#####################################
# Reading parameter
#####################################

sub getparameter
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::getparameter"); }

    while ( $#ARGV >= 0 )
    {
        my $param = shift(@ARGV);

        if ($param eq "-f") { $installer::globals::ziplistname = shift(@ARGV); }
        elsif ($param eq "-s") { $installer::globals::setupscriptname = shift(@ARGV); }
        elsif ($param eq "-p") { $installer::globals::product = shift(@ARGV); }
        elsif ($param eq "-l") { $installer::globals::languagelist = shift(@ARGV); }
        elsif ($param eq "-b") { $installer::globals::build = shift(@ARGV); }
        elsif ($param eq "-m") { $installer::globals::minor = shift(@ARGV); }
        elsif ($param eq "-dontunzip") { $installer::globals::dounzip = 0; }
        elsif ($param eq "-c") { $installer::globals::compiler = shift(@ARGV); }
        elsif ($param eq "-pro") { $installer::globals::pro = 1; }
        elsif ($param eq "-log") { $installer::globals::globallogging = 1; }
        elsif ($param eq "-debug") { $installer::globals::debug = 1; }
        elsif ($param eq "-u") { $installer::globals::unpackpath = shift(@ARGV); }
        elsif ($param eq "-i") { $installer::globals::rootpath = shift(@ARGV); }
        elsif ($param eq "-dontcallepm") { $installer::globals::call_epm = 0; }
        elsif ($param eq "-msitemplate") { $installer::globals::idttemplatepath = shift(@ARGV); }
        elsif ($param eq "-msilanguage") { $installer::globals::idtlanguagepath = shift(@ARGV); }
        elsif ($param eq "-msifiles") { $installer::globals::msifilespath = shift(@ARGV); }
        elsif ($param eq "-javafiles") { $installer::globals::javafilespath = shift(@ARGV); }
        elsif ($param eq "-buildid") { $installer::globals::buildid = shift(@ARGV); }
        elsif ($param eq "-packagelist") { $installer::globals::packagelist = shift(@ARGV); }
        elsif ($param eq "-copyproject") { $installer::globals::is_copy_only_project = 1; }
        elsif ($param eq "-languagepack") { $installer::globals::languagepack = 1; }
        elsif ($param eq "-addchildprojects") { $installer::globals::addchildprojects = 1; }
        else
        {
            print("\n*************************************\n");
            print("Sorry, unknown parameter: $param");
            print("\n*************************************\n");
            usage();
            exit(-1);
        }
    }
}

############################################
# Controlling  the fundamental parameter
# (required for every process)
############################################

sub control_fundamental_parameter
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::control_fundamental_parameter"); }

    if ($installer::globals::product eq "")
    {
        print "\n******************************************\n";
        print "ERROR: Product name not set!";
        print "\n******************************************\n";
        usage();
        exit(-1);
    }
}

##########################################################
# The path parameters can be relative or absolute.
# This function creates absolute pathes.
##########################################################

sub make_path_absolute
{
    my ($pathref) = @_;

    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::make_path_absolute : $$pathref"); }

    if ( $installer::globals::isunix )
    {
        if (!($$pathref =~ /^\s*\//))   # this is a relative unix path
        {
            $$pathref = cwd() . $installer::globals::separator . $$pathref;
        }
    }

    if ( $installer::globals::iswin )
    {
        if (!($$pathref =~ /^\s*\w\:/)) # this is a relative windows path
        {
            $$pathref = cwd() . $installer::globals::separator . $$pathref;
            $$pathref =~ s/\//\\/g;
        }
    }

    $$pathref =~ s/\Q$installer::globals::separator\E\s*$//;    # removing ending slashes
}

##################################################
# Setting some global parameters
# This has to be expanded with furher platforms
##################################################

sub setglobalvariables
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::setglobalvariables"); }

    if ( $installer::globals::compiler =~ /wntmsci/ )
    {
        $installer::globals::iswindowsbuild = 1;
    }

    if ( $installer::globals::compiler =~ /unxlngi/ )
    {
        $installer::globals::islinuxbuild = 1;
        $installer::globals::packageformat = "rpm";
    }

    if (( $installer::globals::compiler =~ /unxsols/ ) || ( $installer::globals::compiler =~ /unxsoli/ ))
    {
        $installer::globals::issolarisbuild = 1;
        $installer::globals::packageformat = "pkg";
    }

    if ( $installer::globals::compiler =~ /unxsols/ ) { $installer::globals::issolarissparcbuild = 1; }

    if ( $installer::globals::compiler =~ /unxsoli/ ) { $installer::globals::issolarisx86build = 1; }

    # ToDo: Needs to be expanded for additional compiler

    # epm supports:
    # aix - AIX software distribution
    # bsd - FreeBSD, NetBSD, or OpenBSD software distribution
    # depot or swinstall - HP-UX software distribution
    # dpkg - Debian software distribution
    # inst or tardist - IRIX software distribution
    # osx - MacOS X software distribution
    # pkg - Solaris software distribution
    # rpm - RedHat software distribution
    # setld - Tru64 (setld) software distribution
    # native - "Native" software distribution for the platform

    # Defaulting to native

    if ( $installer::globals::packageformat eq "") { $installer::globals::packageformat = "native"; }

    # $installer::globals::servicesrdb_can_be_created can only be set, if regcomp (regcomp.exe) can be executed.

    if ( $installer::globals::iswin && $installer::globals::iswindowsbuild ) { $installer::globals::servicesrdb_can_be_created = 1; }
    if ( $installer::globals::islinux && $installer::globals::islinuxbuild ) { $installer::globals::servicesrdb_can_be_created = 1; }
    if ( $installer::globals::issolaris && $installer::globals::issolarisbuild ) { $installer::globals::servicesrdb_can_be_created = 1; }

    # ToDo: Needs to be expanded for additional compiler (setting $installer::globals::servicesrdb_can_be_created = 1 for all external platforms)

    if ((!($installer::globals::iswindowsbuild)) && (!($installer::globals::islinuxbuild)) && (!($installer::globals::issolarisbuild)))
    {
        $installer::globals::servicesrdb_can_be_created = 1;
    }

    # extension, if $installer::globals::pro is set
    if ($installer::globals::pro) { $installer::globals::productextension = ".pro"; }

    # no languages defined as parameter
    if ($installer::globals::languagelist eq "") { $installer::globals::languages_defined_in_productlist = 1; }

    # setting and creating the unpackpath

    if ($installer::globals::unpackpath eq "")  # unpackpath not set
    {
        $installer::globals::unpackpath = cwd();
        if ( $installer::globals::iswin ) { $installer::globals::unpackpath =~ s/\//\\/g; }
    }

    if (!($installer::globals::unpackpath eq ""))   # unpackpath set, relative or absolute?
    {
        make_path_absolute(\$installer::globals::unpackpath);
    }

    if (! -d $installer::globals::unpackpath )  # create unpackpath
    {
        installer::systemactions::create_directory($installer::globals::unpackpath);
    }

    # The following setting has to be removed, after removal of old setup and changes in scp projects
    # No binary file custom actions for Ada products and language packs
    # Typical scp definition, files with flag: BINARYTABLE or SELFREG

    if (( $installer::globals::languagepack ) || ($installer::globals::product =~ /ada/i ))
    {
        @installer::globals::binarytablefiles = ();
        @installer::globals::selfreglibraries = ();
    }

    if ($installer::globals::product =~ /OpenOffice/i )
    {
        push(@installer::globals::binarytablefiles, "gid_File_Pythonmsi_Dll");  # to be removed after scp changes, see parameter.pm
    }

}

############################################
# Controlling  the parameter that are
# required for special processes
############################################

sub control_required_parameter
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::control_required_parameter"); }

    if (!($installer::globals::is_copy_only_project))
    {

        ############################################################################
        # Non Windows: Checking $installer::globals::packagelist
        ############################################################################

        if (($installer::globals::packagelist eq "") && (!($installer::globals::iswindowsbuild)))
        {
            print "\n*********************************************************\n";
            print "ERROR: Package list file not set (-packagelist)!\n";
            print "This package list is required for non-Windows builds!\n";
            print "*********************************************************\n";
            usage();
            exit(-1);
        }

        # Testing existence of packagelist file. Only needed for non-Windows compilers.

        if (!($installer::globals::iswindowsbuild))
        {
            installer::files::check_file($installer::globals::packagelist);
        }

        ##############################################################################################
        # idt template path. Only required for Windows build ($installer::globals::compiler =~ /wntmsci/)
        # for the creation of the msi database.
        ##############################################################################################

        if (($installer::globals::idttemplatepath eq "") && ($installer::globals::iswindowsbuild))
        {
            print "\n**************************************************\n";
            print "ERROR: idt template path not set (-msitemplate)!";
            print "\n**************************************************\n";
            usage();
            exit(-1);
        }

        ##############################################################################################
        # idt language path. Only required for Windows build ($installer::globals::compiler =~ /wntmsci/)
        # for the creation of the msi database.
        ##############################################################################################

        if (($installer::globals::idtlanguagepath eq "") && ($installer::globals::iswindowsbuild))
        {
            print "\n**************************************************\n";
            print "ERROR: idt language path not set (-msilanguage)!";
            print "\n**************************************************\n";
            usage();
            exit(-1);
        }

        ##############################################################################################
        # msi files path. Only required for Windows build ($installer::globals::compiler =~ /wntmsci/)
        # for the creation of the msi installation set.
        ##############################################################################################

        if (($installer::globals::msifilespath eq "") && ($installer::globals::iswindowsbuild))
        {
            print "\n**************************************************\n";
            print "ERROR: msi files path not set (-msifiles)!";
            print "\n**************************************************\n";
            usage();
            exit(-1);
        }

        # Analyzing the idt template path

        if (!($installer::globals::idttemplatepath eq ""))  # idttemplatepath set, relative or absolute?
        {
            make_path_absolute(\$installer::globals::idttemplatepath);
        }

        installer::remover::remove_ending_pathseparator(\$installer::globals::idttemplatepath);

        # Analyzing the idt language path

        if (!($installer::globals::idtlanguagepath eq ""))  # idtlanguagepath set, relative or absolute?
        {
            make_path_absolute(\$installer::globals::idtlanguagepath);
        }

        installer::remover::remove_ending_pathseparator(\$installer::globals::idtlanguagepath);

        # Analyzing the msi files path

        if (!($installer::globals::msifilespath eq "")) # msifilespath set, relative or absolute?
        {
            make_path_absolute(\$installer::globals::msifilespath);
        }

        installer::remover::remove_ending_pathseparator(\$installer::globals::msifilespath);

        # In the msi template directory a files "codes.txt" has to exist, in which the ProductCode
        # and the UpgradeCode for the product are defined.

        if ($installer::globals::iswindowsbuild)
        {
            $installer::globals::codefilename = $installer::globals::idttemplatepath  . $installer::globals::separator . $installer::globals::codefilename;
            installer::files::check_file($installer::globals::codefilename);
        }

    }

    #######################################
    # Testing existence of files
    # also for copy-only projects
    #######################################

    if ($installer::globals::ziplistname eq "") { $installer::globals::use_default_ziplist = 1; }
    else { installer::files::check_file($installer::globals::ziplistname); }  # if the ziplist file is defined, it has to exist

    if ($installer::globals::setupscriptname eq "") { $installer::globals::setupscript_defined_in_productlist = 1; }
    else { installer::files::check_file($installer::globals::setupscriptname); } # if the setupscript file is defined, it has to exist

}

################################################
# Writing parameter to shell and into logfile
################################################

sub outputparameter
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::outputparameter"); }

    my $element;

    my @output = ();

    push(@output, "\n########################################################\n");
    push(@output, "$installer::globals::prog, version 1.0\n");
    if (!($installer::globals::use_default_ziplist))
    {
        push(@output, "Product list file: $installer::globals::ziplistname\n");
    }
    else
    {
        push(@output, "Product list file: Defaulted\n");        # to b_server/zip/zip.lst
    }
    if (!($installer::globals::setupscript_defined_in_productlist))
    {
        push(@output, "Setup script: $installer::globals::setupscriptname\n");
    }
    else
    {
        push(@output, "Taking setup script from solver\n");
    }
    push(@output, "Unpackpath: $installer::globals::unpackpath\n");
    push(@output, "Compiler: $installer::globals::compiler\n");
    push(@output, "Product: $installer::globals::product\n");
    push(@output, "BuildID: $installer::globals::buildid\n");
    push(@output, "Build: $installer::globals::build\n");
    if ( $installer::globals::minor ) { push(@output, "Minor: $installer::globals::minor\n"); }
    else  { push(@output, "No minor set\n"); }
    if ( $installer::globals::pro ) { push(@output, "Product version\n"); }
    else  { push(@output, "Non-Product version\n"); }
    if ( $installer::globals::rootpath eq "" ) { push(@output, "Using default installpath\n"); }
    else { push(@output, "Installpath: $installer::globals::rootpath\n"); }
    if (!($installer::globals::packagelist eq ""))  { push(@output, "Package list file: $installer::globals::packagelist\n"); }
    if ((!($installer::globals::packagelist eq "")) && ($installer::globals::iswindowsbuild)) { push(@output, "Package list file will be ignored for Windows!\n"); }
    if (!($installer::globals::idttemplatepath eq ""))  { push(@output, "msi templatepath: $installer::globals::idttemplatepath\n"); }
    if ((!($installer::globals::idttemplatepath eq "")) && (!($installer::globals::iswindowsbuild))) { push(@output, "msi template path will be ignored for non Windows builds!\n"); }
    if (!($installer::globals::idtlanguagepath eq ""))  { push(@output, "msi languagepath: $installer::globals::idtlanguagepath\n"); }
    if ((!($installer::globals::idtlanguagepath eq "")) && (!($installer::globals::iswindowsbuild))) { push(@output, "msi langugage path will be ignored for non Windows builds!\n"); }
    if (!($installer::globals::msifilespath eq "")) { push(@output, "msi files path: $installer::globals::msifilespath\n"); }
    if ((!($installer::globals::msifilespath eq "")) && (!($installer::globals::iswindowsbuild))) { push(@output, "msi files path will be ignored for non Windows builds!\n"); }
    if ((!($installer::globals::iswindowsbuild)) && ( $installer::globals::call_epm )) { push(@output, "Calling epm\n"); }
    if ((!($installer::globals::iswindowsbuild)) && (!($installer::globals::call_epm))) { push(@output, "Not calling epm\n"); }
    if (!($installer::globals::javafilespath eq "")) { push(@output, "Java installer files path: $installer::globals::javafilespath\n"); }
    if ((!($installer::globals::javafilespath eq "")) && ($installer::globals::iswindowsbuild)) { push(@output, "Java files path will be ignored for Windows builds!\n"); }
    if (($installer::globals::iswindowsbuild) && ($installer::globals::addchildprojects )) { push(@output, "Adding child projects into installation set\n"); }
    if ( $installer::globals::globallogging ) { push(@output, "Complete logging activated\n"); }
    if ( $installer::globals::debug ) { push(@output, "Debug is activated\n"); }
    if ( $installer::globals::dounzip ) { push(@output, "Unzip ARCHIVE files\n"); }
    else  { push(@output, "Not unzipping ARCHIVE files\n"); }
    if ( $installer::globals::servicesrdb_can_be_created ) { push(@output, "services.rdb can be created\n"); }
    else  { push(@output, "services.rdb cannot be created !\n"); }
    if (!($installer::globals::languages_defined_in_productlist))
    {
        push(@output, "Languages:\n");
        foreach $element (@installer::globals::languageproducts) { push(@output, "\t$element\n"); }
    }
    else
    {
        push(@output, "Languages defined in $installer::globals::ziplistname\n");
    }
    if ( $installer::globals::is_copy_only_project ) { push(@output, "This is a copy only project!\n"); }
    if ( $installer::globals::languagepack ) { push(@output, "Creating language pack!\n"); }
    push(@output, "########################################################\n");

    # output into shell and into logfile

    for ( my $i = 0; $i <= $#output; $i++ )
    {
        print $output[$i];
        push(@installer::globals::globallogfileinfo, $output[$i]);
    }
}

1;
