#*************************************************************************
#
#   $RCSfile: parameter.pm,v $
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
-simple: Path to do a simple install to
-c: Compiler, e.g. wntmsci8, unxlngi5, unxsols4, ... (optional)
-u: Path, in which zipfiles are unpacked (optional)
-msitemplate: Source of the msi file templates (Windows compiler only)
-msilanguage: Source of the msi file templates (Windows compiler only)
-msichild: Source of the child products (Windows only)
-javafiles: Source of the Java installer files  (opt., non-Windows only)
-javalanguage: Source of the Java language files (opt., non-Windows only)
-buildid: Current BuildID (optional)
-pro: Product version
-format: Package format
-debian: Create Debian packages for Linux
-dontunzip: do not unzip all files with flag ARCHIVE
-dontcallepm : do not call epm to create install sets (opt., non-Windows only)
-ispatchedepm : Usage of a patched (non-standard) epm (opt., non-Windows only)
-packagelist : file, containing a list of module gids (opt., non-Windows only)
-addpackagelist : additional packlist, only multilingual unix
-addsystemintegration : adding system integration packages (opt., Unix only)
-copyproject : is set for projects that are only used for copying (optional)
-languagepack : do create a languagepack, no product pack (optional)
-patch : do create a patch (optional)
-patchinc: Source for the patch include files (Solaris only)
-log : Logging all available information (optional)
-debug : Collecting debug information

Examples for Windows:

perl make_epmlist.pl -f zip.lst -s setup.inf -p OfficeFAT -l 01
                     -u /export/unpack -buildid 8712
                     -msitemplate /export/msi_files
                     -msilanguage /export/msi_languages

Examples for Non-Windows:

perl make_epmlist.pl -f zip.lst -s setup.inf -p OfficeFAT -l 01 -format rpm
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
        elsif ($param eq "-format") { $installer::globals::packageformat = shift(@ARGV); }
        elsif ($param eq "-log") { $installer::globals::globallogging = 1; }
        elsif ($param eq "-quiet") { $installer::globals::quiet = 1; }
        elsif ($param eq "-debug") { $installer::globals::debug = 1; }
        elsif ($param eq "-tab") { $installer::globals::tab = 1; }
        elsif ($param eq "-u") { $installer::globals::unpackpath = shift(@ARGV); }
        elsif ($param eq "-i") { $installer::globals::rootpath = shift(@ARGV); }
        elsif ($param eq "-dontcallepm") { $installer::globals::call_epm = 0; }
        elsif ($param eq "-msitemplate") { $installer::globals::idttemplatepath = shift(@ARGV); }
        elsif ($param eq "-msilanguage") { $installer::globals::idtlanguagepath = shift(@ARGV); }
        elsif ($param eq "-patchinc") { $installer::globals::patchincludepath = shift(@ARGV); }
        elsif ($param eq "-addjavainstaller") { $installer::globals::addjavainstaller = 1; }
        elsif ($param eq "-javalanguage") { $installer::globals::javalanguagepath = shift(@ARGV); }
        elsif ($param eq "-buildid") { $installer::globals::buildid = shift(@ARGV); }
        elsif ($param eq "-packagelist") { $installer::globals::packagelist = shift(@ARGV); }
        elsif ($param eq "-addpackagelist") { $installer::globals::addpackagelist = shift(@ARGV); }
        elsif ($param eq "-copyproject") { $installer::globals::is_copy_only_project = 1; }
        elsif ($param eq "-languagepack") { $installer::globals::languagepack = 1; }
        elsif ($param eq "-patch") { $installer::globals::patch = 1; }
        elsif ($param eq "-addchildprojects") { $installer::globals::addchildprojects = 1; }
        elsif ($param eq "-addsystemintegration") { $installer::globals::addsystemintegration = 1; }
        elsif ($param eq "-destdir")    # new parameter for simple installer
        {
            $installer::globals::rootpath ne "" && die "must set destdir before -i or -simple";
            $installer::globals::destdir = shift @ARGV;
        }
        elsif ($param eq "-simple")     # new parameter for simple installer
        {
            $installer::globals::simple = 1;
            $installer::globals::call_epm = 0;
            my $path = shift(@ARGV);
            $path =~ s/^$installer::globals::destdir//;
            $installer::globals::rootpath = $path;
        }
        else
        {
            installer::logger::print_error( "unknown parameter: $param" );
            usage();
            exit(-1);
        }
    }

    # Usage of simple installer (not for Windows):
    # $PERL -w $SOLARENV/bin/make_installer.pl \
    # -f openoffice.lst -l en-US -p OpenOffice \
    # -packagelist ../inc_openoffice/unix/packagelist.txt \
    # -buildid $BUILD -rpm \
    # -destdir /tmp/nurk -simple $INSTALL_PATH
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
        installer::logger::print_error( "Product name not set!" );
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
        if ( $^O =~ /cygwin/i )
        {
            if ( $$pathref !~ /^\s*\// && $$pathref !~ /^\s*\w\:/ ) # not an absolute POSIX or DOS path
            {
                $$pathref = cwd() . $installer::globals::separator . $$pathref;
            }
            my $p = $$pathref;
            chomp( $p );
            my $q = '';
            # Avoid the $(LANG) problem.
            if ($p =~ /(\A.*)(\$\(.*\Z)/) {
                $p = $1;
                $q = $2;
            }
            $p =~ s/\\/\\\\/g;
            chomp( $p = qx{cygpath -w "$p"} );
            $$pathref = $p.$q;
            # Use windows paths, but with '/'s.
            $$pathref =~ s/\\/\//g;
        }
        else
        {
            if (!($$pathref =~ /^\s*\w\:/)) # this is a relative windows path (no dos drive)
            {
                $$pathref = cwd() . $installer::globals::separator . $$pathref;

                $$pathref =~ s/\//\\/g;
            }
        }
    }
    $$pathref =~ s/[\/\\]\s*$//;    # removing ending slashes
}

##################################################
# Setting some global parameters
# This has to be expanded with furher platforms
##################################################

sub setglobalvariables
{
    if ( $installer::globals::debug ) { installer::logger::debuginfo("installer::parameter::setglobalvariables"); }

    if ( $installer::globals::compiler =~ /wntmsci/ ) { $installer::globals::iswindowsbuild = 1; }

    if ( $installer::globals::compiler =~ /unxso[lg][si]/ )
    {
        $installer::globals::issolarisbuild = 1;
        if ( $installer::globals::packageformat eq "pkg" )
        {
            $installer::globals::issolarispkgbuild = 1;
            $installer::globals::epmoutpath = "packages";
        }
    }

    if ( $installer::globals::compiler =~ /unxso[lg]s/ ) { $installer::globals::issolarissparcbuild = 1; }

    if ( $installer::globals::compiler =~ /unxso[lg]i/ ) { $installer::globals::issolarisx86build = 1; }

    if (( $installer::globals::compiler =~ /unx/ ) && ( $installer::globals::addpackagelist )) { $installer::globals::is_unix_multi = 1; }

    if ( $installer::globals::compiler =~ /unxlngi/ || $installer::globals::compiler =~ /unxlngx/ )
    {
        $installer::globals::islinuxbuild = 1;
        if ( $installer::globals::packageformat eq "rpm" )
        {
            $installer::globals::islinuxrpmbuild = 1;
            $installer::globals::epmoutpath = "RPMS";
        }
    }

    # Defaulting to native package format for epm

    if ( ! $installer::globals::packageformat ) { $installer::globals::packageformat = "native"; }

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

    if (!($installer::globals::unpackpath eq ""))
    {
        make_path_absolute(\$installer::globals::unpackpath);
    }

    if (! -d $installer::globals::unpackpath )  # create unpackpath
    {
        installer::systemactions::create_directory($installer::globals::unpackpath);
    }

    # setting and creating the temppath

    if (( $ENV{'TMP'} ) || ( $ENV{'TEMP'} ))
    {
        if ( $ENV{'TMP'} ) { $installer::globals::temppath = $ENV{'TMP'}; }
        elsif ( $ENV{'TEMP'} )  { $installer::globals::temppath = $ENV{'TEMP'}; }
        $installer::globals::temppath =~ s/\Q$installer::globals::separator\E\s*$//;    # removing ending slashes and backslashes
        $installer::globals::temppath = $installer::globals::temppath . $installer::globals::separator . "instsetunpack";
        $installer::globals::temppath = installer::systemactions::create_pid_directory($installer::globals::temppath);
        push(@installer::globals::removedirs, $installer::globals::temppath);
        $installer::globals::temppath = $installer::globals::temppath . $installer::globals::separator . $installer::globals::compiler . $installer::globals::productextension;
        installer::systemactions::create_directory($installer::globals::temppath);
        $installer::globals::temppathdefined = 1;
    }
    else
    {
        $installer::globals::temppathdefined = 0;
    }

}

############################################
# Setting child product names. This has to
# be removed after removal of old setup.
# SCP_TODO
############################################

sub set_childproductnames
{
    if ( $installer::globals::islinuxrpmbuild )
    {
        $installer::globals::adafilename = "adabas-13.01.00-1.i586.rpm";
        $installer::globals::javafilename = "jre-1_5_0_03-linux-i586.rpm";
        $installer::globals::jdsexcludefilename = "jds_excludefiles_linux.txt";
    }

    if ( $installer::globals::issolarissparcbuild )
    {
        $installer::globals::adafilename = "SUNWadabas.tar.gz";
        $installer::globals::javafilename = "SUNWj5rt_1_5_0_03_sparc.tar.gz";
        $installer::globals::javafilename2 = "SUNWj5cfg_1_5_0_03_sparc.tar.gz";
        $installer::globals::javafilename3 = "SUNWj5man_1_5_0_03_sparc.tar.gz";
        $installer::globals::jdsexcludefilename = "jds_excludefiles_solaris_sparc.txt";
        @installer::globals::requiredpackages = ("SUNWfreetype2.tar.gz");
    }

    if ( $installer::globals::issolarisx86build )
    {
        $installer::globals::adafilename = "";
        $installer::globals::javafilename = "SUNWj5rt_1_5_0_03_x86.tar.gz";
        $installer::globals::javafilename2 = "SUNWj5cfg_1_5_0_03_x86.tar.gz";
        $installer::globals::javafilename3 = "SUNWj5man_1_5_0_03_x86.tar.gz";
        $installer::globals::jdsexcludefilename = "jds_excludefiles_solaris_intel.txt";
        @installer::globals::requiredpackages = ("SUNWfreetype2.tar.gz");
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
            installer::logger::print_error( "Package list file not set (-packagelist)!\n;
                                             This package list is required for non-Windows builds!" );
            usage();
            exit(-1);
        }

        # Testing existence of packagelist file. Only needed for non-Windows compilers.

        if (!($installer::globals::iswindowsbuild))
        {
            installer::files::check_file($installer::globals::packagelist);

            # If an additional packagelist is defined (Unix, multilingual installation set), it has to exist.

            if ( $installer::globals::addpackagelist )
            {
                installer::files::check_file($installer::globals::packagelist);
            }
        }

        ##############################################################################################
        # idt template path. Only required for Windows build ($installer::globals::compiler =~ /wntmsci/)
        # for the creation of the msi database.
        ##############################################################################################

        if (($installer::globals::idttemplatepath eq "") && ($installer::globals::iswindowsbuild))
        {
            installer::logger::print_error( "idt template path not set (-msitemplate)!" );
            usage();
            exit(-1);
        }

        ##############################################################################################
        # idt language path. Only required for Windows build ($installer::globals::compiler =~ /wntmsci/)
        # for the creation of the msi database.
        ##############################################################################################

        if (($installer::globals::idtlanguagepath eq "") && ($installer::globals::iswindowsbuild))
        {
            installer::logger::print_error( "idt language path not set (-msilanguage)!" );
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

        # In the msi template directory a files "codes.txt" has to exist, in which the ProductCode
        # and the UpgradeCode for the product are defined.

        if ($installer::globals::iswindowsbuild)
        {
            $installer::globals::codefilename = $installer::globals::idttemplatepath  . $installer::globals::separator . $installer::globals::codefilename;
            installer::files::check_file($installer::globals::codefilename);
            $installer::globals::componentfilename = $installer::globals::idttemplatepath  . $installer::globals::separator . $installer::globals::componentfilename;
            installer::files::check_file($installer::globals::componentfilename);
        }

    }

    #######################################
    # Patch currently only available
    # for Solaris packages and Linux
    #######################################

    if (( $installer::globals::patch ) && ( ! $installer::globals::issolarispkgbuild ) && ( ! $installer::globals::islinuxrpmbuild ) && ( ! $installer::globals::iswindowsbuild ))
    {
        installer::logger::print_error( "Sorry, Patch flag currently only available for Solaris pkg, Linux RPM and Windows builds!" );
        usage();
        exit(-1);
    }

    if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild ) && ( ! $installer::globals::patchincludepath ))
    {
        installer::logger::print_error( "Solaris patch requires parameter -patchinc !" );
        usage();
        exit(-1);
    }

    if (( $installer::globals::patch ) && ( $installer::globals::issolarispkgbuild ) && ( $installer::globals::patchincludepath ))
    {
        make_path_absolute(\$installer::globals::patchincludepath);
        $installer::globals::patchincludepath = installer::converter::make_path_conform($installer::globals::patchincludepath);
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
    push(@output, "Package format: $installer::globals::packageformat\n");
    if (!($installer::globals::packagelist eq ""))  { push(@output, "Package list file: $installer::globals::packagelist\n"); }
    if ((!($installer::globals::packagelist eq "")) && ($installer::globals::iswindowsbuild)) { push(@output, "Package list file will be ignored for Windows!\n"); }
    if (!($installer::globals::addpackagelist eq ""))   { push(@output, "Addon-Package list file: $installer::globals::addpackagelist\n"); }
    if ((!($installer::globals::addpackagelist eq "")) && ($installer::globals::iswindowsbuild)) { push(@output, "Addon-Package list file will be ignored for Windows!\n"); }
    if (!($installer::globals::idttemplatepath eq ""))  { push(@output, "msi templatepath: $installer::globals::idttemplatepath\n"); }
    if ((!($installer::globals::idttemplatepath eq "")) && (!($installer::globals::iswindowsbuild))) { push(@output, "msi template path will be ignored for non Windows builds!\n"); }
    if (!($installer::globals::idtlanguagepath eq ""))  { push(@output, "msi languagepath: $installer::globals::idtlanguagepath\n"); }
    if ((!($installer::globals::idtlanguagepath eq "")) && (!($installer::globals::iswindowsbuild))) { push(@output, "msi language path will be ignored for non Windows builds!\n"); }
    if ((!($installer::globals::iswindowsbuild)) && ( $installer::globals::call_epm )) { push(@output, "Calling epm\n"); }
    if ((!($installer::globals::iswindowsbuild)) && (!($installer::globals::call_epm))) { push(@output, "Not calling epm\n"); }
    if ( $installer::globals::addjavainstaller ) { push(@output, "Adding Java installer\n"); }
    if (!($installer::globals::javalanguagepath eq "")) { push(@output, "Java language path: $installer::globals::javalanguagepath\n"); }
    if ((!($installer::globals::javalanguagepath eq "")) && ($installer::globals::iswindowsbuild)) { push(@output, "Java language path will be ignored for Windows builds!\n"); }
    if ( $installer::globals::patchincludepath ) { push(@output, "Patch include path: $installer::globals::patchincludepath\n"); }
    if (($installer::globals::iswindowsbuild) && ($installer::globals::addchildprojects )) { push(@output, "Adding child projects into installation set\n"); }
    if ( $installer::globals::globallogging ) { push(@output, "Complete logging activated\n"); }
    if ( $installer::globals::addsystemintegration ) { push(@output, "Adding system integration packages\n"); }
    if ( $installer::globals::debug ) { push(@output, "Debug is activated\n"); }
    if ( $installer::globals::tab ) { push(@output, "TAB version\n"); }
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
    if ( $installer::globals::patch ) { push(@output, "Creating patch!\n"); }
    push(@output, "########################################################\n");

    # output into shell and into logfile

    for ( my $i = 0; $i <= $#output; $i++ )
    {
        installer::logger::print_message( $output[$i] );
        push(@installer::globals::globallogfileinfo, $output[$i]);
    }
}

1;
