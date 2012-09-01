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

package installer::parameter;

use Cwd;
use installer::exiter;
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
    print <<Ende;
--------------------------------------------------------------------------------
The following parameter are needed:
-f: Path to the product list (required)
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
-buildid: Current BuildID (optional)
-pro: Product version
-format: Package format
-debian: Create Debian packages for Linux
-dontunzip: do not unzip all files with flag ARCHIVE
-dontcallepm : do not call epm to create install sets (opt., non-Windows only)
-ispatchedepm : Usage of a patched (non-standard) epm (opt., non-Windows only)
-copyproject : is set for projects that are only used for copying (optional)
-languagepack : do create a languagepack, no product pack (optional)
-helppack : do create a helppack, no product pack (optional)
-patch : do create a patch (optional)
-patchinc: Source for the patch include files (Solaris only)
-strip: Stripping files (Unix only)
-log : Logging all available information (optional)

Examples for Windows:

perl make_epmlist.pl -f zip.lst -p OfficeFAT -l en-US
                     -u /export/unpack -buildid 8712
                     -msitemplate /export/msi_files
                     -msilanguage /export/msi_languages

Examples for Non-Windows:

perl make_epmlist.pl -f zip.lst -p OfficeFAT -l en-US -format rpm
                     -u /export/unpack -buildid 8712 -ispatchedepm
--------------------------------------------------------------------------------
Ende
    exit(-1);
}

#########################################
# Writing all parameter into logfile
#########################################

sub saveparameter
{
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
        elsif ($param eq "-quiet") { $installer::globals::quiet = 1; }
        elsif ($param eq "-verbose") { $installer::globals::quiet = 0; }
        elsif ($param eq "-u") { $installer::globals::unpackpath = shift(@ARGV); }
        elsif ($param eq "-i") { $installer::globals::rootpath = shift(@ARGV); }
        elsif ($param eq "-dontcallepm") { $installer::globals::call_epm = 0; }
        elsif ($param eq "-msitemplate") { $installer::globals::idttemplatepath = shift(@ARGV); }
        elsif ($param eq "-msilanguage") { $installer::globals::idtlanguagepath = shift(@ARGV); }
        elsif ($param eq "-patchinc") { $installer::globals::patchincludepath = shift(@ARGV); }
        elsif ($param eq "-buildid") { $installer::globals::buildid = shift(@ARGV); }
        elsif ($param eq "-copyproject") { $installer::globals::is_copy_only_project = 1; }
        elsif ($param eq "-languagepack") { $installer::globals::languagepack = 1; }
        elsif ($param eq "-helppack") { $installer::globals::helppack = 1;}
        elsif ($param eq "-patch") { $installer::globals::patch = 1; }
        elsif ($param eq "-debian") { $installer::globals::debian = 1; }
        elsif ($param eq "-strip") { $installer::globals::strip = 1; }
        elsif ($param eq "-destdir")    # new parameter for simple installer
        {
            $installer::globals::rootpath ne "" && die "must set destdir before -i or -simple";

            my $path = shift(@ARGV);
            mkdir $path;
            $installer::globals::destdir = Cwd::realpath($path);
        }
        elsif ($param eq "-simple")     # new parameter for simple installer
        {
            $installer::globals::simple = 1;
            $installer::globals::call_epm = 0;
            $installer::globals::makedownload = 0;
            my $path = shift(@ARGV);
            $path =~ s/^\Q$installer::globals::destdir\E//;
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
    # -buildid $BUILD -rpm \
    # -destdir /tmp/nurk -simple $INSTALL_PATH
}

############################################
# Controlling  the fundamental parameter
# (required for every process)
############################################

sub control_fundamental_parameter
{
    if ($installer::globals::product eq "")
    {
        installer::logger::print_error( "Product name not set!" );
        usage();
        exit(-1);
    }
}

##########################################################
# The path parameters can be relative or absolute.
# This function creates absolute paths.
##########################################################

sub make_path_absolute
{
    my ($pathref) = @_;

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
    # Setting the installertype directory corresponding to the environment variable PKGFORMAT
    # The global variable $installer::globals::packageformat can only contain one package format.
    # If PKGFORMAT cotains more than one format (for example "rpm deb") this is splitted in the
    # makefile calling the perl program.
    $installer::globals::installertypedir = $installer::globals::packageformat;

    if ( $installer::globals::compiler =~ /wnt(msc|gcc)i/ )
    {
        $installer::globals::iswindowsbuild = 1;
    }

    if ( $installer::globals::compiler =~ /unxso[lg][siux]/ )
    {
        $installer::globals::issolarisbuild = 1;
        if ( $installer::globals::packageformat eq "pkg" )
        {
            $installer::globals::issolarispkgbuild = 1;
            $installer::globals::epmoutpath = "packages";
        }
    }

    if (( $installer::globals::compiler =~ /unxmacxi/ ) || ( $installer::globals::compiler =~ /unxmacxp/ ))
    {
        $installer::globals::ismacbuild = 1;

        if ( $installer::globals::packageformat eq "dmg" )
        {
            $installer::globals::ismacdmgbuild = 1;
        }
    }

    if ( $installer::globals::compiler =~ /unxobsd/ )
    {
            $installer::globals::epmoutpath = "openbsd";
    }

    if ( $installer::globals::compiler =~ /unxfbsd/ )
    {
        $installer::globals::isfreebsdbuild = 1;

        if ( $installer::globals::packageformat eq "bsd" )
        {
            $installer::globals::epmoutpath = "freebsd";
            $installer::globals::isfreebsdpkgbuild = 1;
        }
    }

    if ( $installer::globals::compiler =~ /unxso[lg]s/ ) { $installer::globals::issolarissparcbuild = 1; }

    if ( $installer::globals::compiler =~ /unxso[lg]i/ ) { $installer::globals::issolarisx86build = 1; }

    if ($ENV{OS} eq 'AIX')
    {
        if ( $installer::globals::packageformat eq "rpm" )
        {
            $installer::globals::isrpmbuild = 1;
            $installer::globals::epmoutpath = "RPMS";
        }
        if ( $installer::globals::rpm eq "" ) { installer::exiter::exit_program("ERROR: Environment variable \"\$RPM\" has to be defined!", "setglobalvariables"); }
    }

    if ($ENV{OS} eq 'LINUX')
    {
        $installer::globals::islinuxbuild = 1;
        if ( $installer::globals::packageformat eq "rpm" )
        {
            $installer::globals::isrpmbuild = 1;
            $installer::globals::epmoutpath = "RPMS";

            if ( $installer::globals::rpm eq "" ) { installer::exiter::exit_program("ERROR: Environment variable \"\$RPM\" has to be defined!", "setglobalvariables"); }
        }

        # Creating Debian packages ?
        if (( $installer::globals::packageformat eq "deb" ) || ( $installer::globals::debian ))
        {
            $installer::globals::debian = 1;
            $installer::globals::packageformat = "deb";
            my $message = "Creating Debian packages";
            installer::logger::print_message( $message );
            push(@installer::globals::globallogfileinfo, $message);
            $installer::globals::isrpmbuild = 0;
            $installer::globals::isdebbuild = 1;
            $installer::globals::epmoutpath = "DEBS";
        }
    }

    # Defaulting to native package format for epm

    if ( ! $installer::globals::packageformat ) { $installer::globals::packageformat = "native"; }

    # extension, if $installer::globals::pro is set
    if ($installer::globals::pro) { $installer::globals::productextension = ".pro"; }

    # no languages defined as parameter
    if ($installer::globals::languagelist eq "") { $installer::globals::languages_defined_in_productlist = 1; }

    # setting and creating the unpackpath

    if ($installer::globals::unpackpath eq "")  # unpackpath not set
    {
        $installer::globals::unpackpath = cwd();
    }

    if ( $installer::globals::localunpackdir ne "" ) { $installer::globals::unpackpath = $installer::globals::localunpackdir; }

    if (!($installer::globals::unpackpath eq ""))
    {
        make_path_absolute(\$installer::globals::unpackpath);
    }

    $installer::globals::unpackpath =~ s/\Q$installer::globals::separator\E\s*$//;

    if (! -d $installer::globals::unpackpath )  # create unpackpath
    {
        installer::systemactions::create_directory($installer::globals::unpackpath);
    }

    # setting and creating the temppath

    if (( $ENV{'TMP'} ) || ( $ENV{'TEMP'} ) || ( $ENV{'TMPDIR'} ))
    {
        if ( $ENV{'TMP'} ) { $installer::globals::temppath = $ENV{'TMP'}; }
        elsif ( $ENV{'TEMP'} )  { $installer::globals::temppath = $ENV{'TEMP'}; }
        elsif ( $ENV{'TMPDIR'} )  { $installer::globals::temppath = $ENV{'TMPDIR'}; }
        $installer::globals::temppath =~ s/\Q$installer::globals::separator\E\s*$//;    # removing ending slashes and backslashes
        $installer::globals::temppath .= $installer::globals::separator . 'ooopackaging';
        installer::systemactions::create_directory_with_privileges($installer::globals::temppath, "777");
        my $dirsave = $installer::globals::temppath;

        if ( $installer::globals::compiler =~ /^unxmac/ )
        {
            chmod 0777, $installer::globals::temppath;
        }

        $installer::globals::temppath = $installer::globals::temppath . $installer::globals::separator . "i";
        $installer::globals::temppath = installer::systemactions::create_pid_directory($installer::globals::temppath);
        push(@installer::globals::removedirs, $installer::globals::temppath);

        if ( ! -d $installer::globals::temppath ) { installer::exiter::exit_program("ERROR: Failed to create directory $installer::globals::temppath ! Possible reason: Wrong privileges in directory $dirsave .", "setglobalvariables"); }

        $installer::globals::temppath = $installer::globals::temppath . $installer::globals::separator . $installer::globals::compiler . $installer::globals::productextension;
        installer::systemactions::create_directory($installer::globals::temppath);
        if ( $^O =~ /cygwin/i )
        {
            $installer::globals::cyg_temppath = $installer::globals::temppath;
            $installer::globals::cyg_temppath =~ s/\\/\\\\/g;
            chomp( $installer::globals::cyg_temppath = qx{cygpath -w "$installer::globals::cyg_temppath"} );
        }
        $installer::globals::temppathdefined = 1;
    }
    else
    {
        $installer::globals::temppathdefined = 0;
    }

    # only one cab file, if Windows msp patches shall be prepared
    if ( $installer::globals::prepare_winpatch ) { $installer::globals::number_of_cabfiles = 1; }

}

############################################
# Controlling  the parameter that are
# required for special processes
############################################

sub control_required_parameter
{
    if (!($installer::globals::is_copy_only_project))
    {
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
        # The name "codes.txt" can be overwritten in Product definition with CODEFILENAME (msiglobal.pm)

        if (( $installer::globals::iswindowsbuild ) && ( $installer::globals::packageformat ne "archive" ) && ( $installer::globals::packageformat ne "installed" ))
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

    if (( $installer::globals::patch ) && ( ! $installer::globals::issolarispkgbuild ) && ( ! $installer::globals::isrpmbuild ) && ( ! $installer::globals::isdebbuild ) && ( ! $installer::globals::iswindowsbuild ) && ( ! $installer::globals::ismacdmgbuild ))
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

    if ($installer::globals::ziplistname eq "")
    {
        installer::logger::print_error( "ERROR: Zip list file has to be defined (Parameter -f) !" );
        usage();
        exit(-1);
    }
    else
    {
        installer::files::check_file($installer::globals::ziplistname);
    }

    if ($installer::globals::setupscriptname eq "") { $installer::globals::setupscript_defined_in_productlist = 1; }
    else { installer::files::check_file($installer::globals::setupscriptname); } # if the setupscript file is defined, it has to exist

}

################################################
# Writing parameter to shell and into logfile
################################################

sub outputparameter
{
    my $element;

    my @output = ();

    push(@output, "\n########################################################\n");
    push(@output, "Product list file: $installer::globals::ziplistname\n");
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
    if (!($installer::globals::idttemplatepath eq ""))  { push(@output, "msi templatepath: $installer::globals::idttemplatepath\n"); }
    if ((!($installer::globals::idttemplatepath eq "")) && (!($installer::globals::iswindowsbuild))) { push(@output, "msi template path will be ignored for non Windows builds!\n"); }
    if (!($installer::globals::idtlanguagepath eq ""))  { push(@output, "msi languagepath: $installer::globals::idtlanguagepath\n"); }
    if ((!($installer::globals::idtlanguagepath eq "")) && (!($installer::globals::iswindowsbuild))) { push(@output, "msi language path will be ignored for non Windows builds!\n"); }
    if ((!($installer::globals::iswindowsbuild)) && ( $installer::globals::call_epm )) { push(@output, "Calling epm\n"); }
    if ((!($installer::globals::iswindowsbuild)) && (!($installer::globals::call_epm))) { push(@output, "Not calling epm\n"); }
    if ( $installer::globals::patchincludepath ) { push(@output, "Patch include path: $installer::globals::patchincludepath\n"); }
    if ( $installer::globals::strip ) { push(@output, "Stripping files\n"); }
    else { push(@output, "No file stripping\n"); }
    if ( $installer::globals::debian ) { push(@output, "Linux: Creating Debian packages\n"); }
    if ( $installer::globals::dounzip ) { push(@output, "Unzip ARCHIVE files\n"); }
    else  { push(@output, "Not unzipping ARCHIVE files\n"); }
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
    if ( $installer::globals::helppack ) { push(@output, "Creating help pack!\n"); }
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
