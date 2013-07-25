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
# configure.pl - a perl script to set a minimal environment for the SDK.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#

use IO::File;
use File::Basename;

$main::hostname= $ARGV[0];
$main::sdkpath= $ARGV[1];
$main::OO_SDK_NAME= $ARGV[2];

$main::OO_MAJORVERSION=$main::OO_SDK_NAME;
$main::OO_MINORVERSION=$main::OO_SDK_NAME;
$main::OO_MAJORVERSION =~ s#[^\d]+(\d).(\d).+#$1#go;
$main::OO_MINORVERSION =~ s#[^\d]+(\d).(\d).+#$2#go;

$main::OO_SDK_CONFIG_HOME= "$ENV{HOME}/$main::OO_SDK_NAME";

$main::operatingSystem = `$main::sdkpath/config.guess | cut -d"-" -f3,4`;
chomp ($main::operatingSystem);

$main::OO_SDK_HOME = $main::sdkpath;
#$main::OO_SDK_HOME = "";
$main::OO_SDK_HOME_SUGGESTION = $main::sdkpath;

$main::OFFICE_OR_URE = "Office";
$main::OFFICE_OR_URE_SUGGESTION = "Office";

$main::OFFICE_HOME = "";

$main::OO_SDK_URE_HOME = `cd $main::sdkpath/../ure-link && pwd`;
chomp($main::OO_SDK_URE_HOME);

$main::OO_SDK_MAKE_HOME = "";
$main::makeName = "make";
if ( $main::operatingSystem =~ m/solaris/ ||
     $main::operatingSystem =~ m/freebsd/ )
{
    $main::makeName = "gmake";
}
$main::OO_SDK_MAKE_HOME_SUGGESTION = searchprog($main::makeName);
$main::makeVersion = "3.79.1";
$main::correctVersion = 0;

$main::OO_SDK_ZIP_HOME = "";
$main::OO_SDK_ZIP_HOME_SUGGESTION = searchprog("zip");
$main::zipVersion = "2.3";

$main::OO_SDK_CAT_HOME = "";
$main::OO_SDK_CAT_HOME_SUGGESTION = searchprog("cat");
# TODO cat version
# $main::catVersion = "";

$main::OO_SDK_SED_HOME = "";
$main::OO_SDK_SED_HOME_SUGGESTION = searchprog("sed");
# TODO sed version
# $main::sedVersion = "";

$main::OO_SDK_CPP_HOME = "";
$main::cppName = "gcc";
$main::cppVersion = "4.0.1";
if ( $main::operatingSystem =~ m/solaris/ )
{
    $main::cppName = "CC";
    $main::cppVersion = "5.2";
}
$main::OO_SDK_CC_55_OR_HIGHER = "";
$main::OO_SDK_CPP_HOME_SUGGESTION = searchprog($main::cppName);

$main::OO_SDK_JAVA_HOME = "";
$main::OO_SDK_JAVA_BIN_DIR = "bin";
if ( $main::operatingSystem =~ m/darwin/ )
{
    $main::OO_SDK_JAVA_BIN_DIR="Commands";
}
$main::OO_SDK_JAVA_HOME_SUGGESTION = searchprog("javac");
$main::javaVersion = "1.5.0_01";

$main::SDK_AUTO_DEPLOYMENT = "";
$main::SDK_AUTO_DEPLOYMENT_SUGGESTION = "YES";

$main::OO_SDK_OUTPUT_DIR_SUGGESTION = "$ENV{HOME}";
$main::OO_SDK_OUTPUT_DIR = "";
$main::skipOutputDir = 0;

$main::return = 0;

if ( $main::OFFICE_OR_URE eq "Office" )
{
    if ( $main::operatingSystem =~ m/darwin/ )
    {
# used for a SDK as part of the office installation
#       $main::OFFICE_HOME = `cd $main::sdkpath/../../.. && pwd`;
#       chomp($main::OFFICE_HOME);
#       print " Used Office = $main::OFFICE_HOME\n";
        print " Used SDK = $main::OO_SDK_HOME\n\n";

        $main::OFFICE_HOME_SUGGESTION = searchMacOffice();
        while ( (! -d "$main::OFFICE_HOME" ) ||
                ((-d "$main::OFFICE_HOME") && (! -d "$main::OFFICE_HOME/Contents/MacOS")) )
        {
            print " Enter the Office installation directory [$main::OFFICE_HOME_SUGGESTION]: ";
            $main::OFFICE_HOME = readStdIn();
            chop($main::OFFICE_HOME);
            if ( $main::OFFICE_HOME eq "" )
            {
                $main::OFFICE_HOME = $main::OFFICE_HOME_SUGGESTION;
            }

            if ( ! -d "$main::OFFICE_HOME" )
            {
                $main::OFFICE_HOME = "";
                print " Error: An office installation is required, please specify the path to a valid installation.\n";
            }

            # check more details
            if ( -d "$main::OFFICE_HOME/Contents/ure-link" ) {
                $main::OO_SDK_URE_HOME = "$main::OFFICE_HOME/Contents/ure-link";
            } else {
                $main::OFFICE_HOME = "";
                $main::OO_SDK_URE_HOME = "";
                print " Error: no URE found in office installation, please specify the path to a valid installation.\n";
            }
        }
    } else
    {
        $main::OFFICE_HOME_SUGGESTION = searchoffice();

        if ( $main::OFFICE_HOME_SUGGESTION eq "" ) {
            # prepare Office path
            $main::OFFICE_HOME_SUGGESTION = searchprog("soffice");
        }

        if ( ! $main::OFFICE_HOME_SUGGESTION eq "" )
        {
            my $tmpOffice = readlink "$main::OFFICE_HOME_SUGGESTION/soffice";

            if ( $tmpOffice eq "" )
            {
                $tmpOffice = "$main::OFFICE_HOME_SUGGESTION/soffice";
            }

            my $offset = rindex($tmpOffice, "/program/soffice");
            if ( $offset != -1 )
            {
                $main::OFFICE_HOME_SUGGESTION = substr($tmpOffice, 0, $offset);
            } else
            {
                $offset = rindex($tmpOffice, "/soffice");
                if ( $offset != -1 )
                {
                    $main::OFFICE_HOME_SUGGESTION = substr($tmpOffice, 0, $offset);
                } else
                {
                    $main::OFFICE_HOME_SUGGESTION = "";
                }
            }
        }

        while ( (! -d "$main::OFFICE_HOME" ) ||
                ((-d "$main::OFFICE_HOME") && (! -d "$main::OFFICE_HOME/program")) )
        {
            print " Enter the Office installation directory [$main::OFFICE_HOME_SUGGESTION]: ";
            $main::OFFICE_HOME = readStdIn();
            chop($main::OFFICE_HOME);
            if ( $main::OFFICE_HOME eq "" )
            {
                $main::OFFICE_HOME = $main::OFFICE_HOME_SUGGESTION;
            }

            if ( ! -d "$main::OFFICE_HOME" )
            {
                $main::OFFICE_HOME = "";
                print " Error: An office installation is required, please specify the path to a valid installation.\n";
            } else
            {
                # special work for a network installation, no prgram directory but a link to the soffice binary
                if ( (! -d "$main::OFFICE_HOME/program") && (-e "$main::OFFICE_HOME/soffice") )
                {
                    my $soserver = `ls -l $OFFICE_HOME_SUGGESTION/soffice | sed -n 's/.* -> //p'`;
                    $soserver= substr($soserver, 0, rindex($soserver, "program") - 1);

                    if ( ! -d $soserver )
                    {
                        $main::OFFICE_HOME = "";
                        print " Error: An office installation is required, please specify the path to a valid installation.\n";
                    } else
                    {
                        $main::OFFICE_HOME = $soserver;
                    }
                }
            }
        }
    }
}
else
{
    # prepare URE path
    $main::OO_SDK_URE_HOME_SUGGESTION = "/opt/openoffice.org/ure";
    $main::OO_SDK_URE_HOME_SUGGESTION = "" unless
        -e "$main::OO_SDK_URE_HOME_SUGGESTION/bin/uno";
    for (;;)
    {
        print " Enter the URE installation directory",
            " [$main::OO_SDK_URE_HOME_SUGGESTION]: ";
        $main::OO_SDK_URE_HOME = readStdIn();
        chop $main::OO_SDK_URE_HOME;
        $main::OO_SDK_URE_HOME = $main::OO_SDK_URE_HOME_SUGGESTION if
            $main::OO_SDK_URE_HOME eq "" &&
            $main::OO_SDK_URE_HOME_SUGGESTION ne "";
        last if -e "$main::OO_SDK_URE_HOME/bin/uno";
        print " Error: A valid URE installation is required.\n";
    }
}

# prepare GNU make path
while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_MAKE_HOME" ) ||
         ((-d "$main::OO_SDK_MAKE_HOME") && (! -e "$main::OO_SDK_MAKE_HOME/$main::makeName"))) )
{
    print " Enter GNU make ($main::makeVersion or higher) tools directory [$main::OO_SDK_MAKE_HOME_SUGGESTION]: ";
    $main::OO_SDK_MAKE_HOME = readStdIn();
    chop($main::OO_SDK_MAKE_HOME);
    if ( $main::OO_SDK_MAKE_HOME eq "" )
    {
        $main::OO_SDK_MAKE_HOME = $main::OO_SDK_MAKE_HOME_SUGGESTION;
    }
    if ( (! -d "$main::OO_SDK_MAKE_HOME") ||
         ((-d "$main::OO_SDK_MAKE_HOME") && (! -e "$main::OO_SDK_MAKE_HOME/$main::makeName")) )
    {
        $main::OO_SDK_MAKE_HOME = "";
        print " Error: GNU make is required, please specify a GNU make tools directory.\n";
    } else
    {
        #check version
        my $testVersion = `$OO_SDK_MAKE_HOME/$main::makeName --version`;
        if ( $testVersion eq "")
        {
            print " Set the environment variable OO_SDK_MAKE_HOME to your GNU build tools directory.\n";
            print " GNU make version $main::makeVersion can be obtained at ftp://ftp.gnu.org/gnu/make/\n";
        } else
        {
            if ($testVersion =~ m#((\d+\.)+\d+)# )
            {
                $testVersion = $1;
            }
            $main::correctVersion = testVersion($main::makeVersion, $testVersion, "$main::OO_SDK_MAKE_HOME/$main::makeName", 1);
            if ( !$main::correctVersion )
            {
                print " The '$main::makeName' command found at '$main::OO_SDK_MAKE_HOME' has a wrong version\n";
                $main::OO_SDK_MAKE_HOME = "";
            }
        }
    }
}

# prepare zip path
$main::correctVersion = 0;
while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_ZIP_HOME" ) ||
         ((-d "$main::OO_SDK_ZIP_HOME") && (! -e "$main::OO_SDK_ZIP_HOME/zip"))) )
{
    print " Enter zip ($main::zipVersion or higher) tool directory [$main::OO_SDK_ZIP_HOME_SUGGESTION]: ";
    $main::OO_SDK_ZIP_HOME = readStdIn();
    chop($main::OO_SDK_ZIP_HOME);
    if ( $main::OO_SDK_ZIP_HOME eq "" )
    {
        $main::OO_SDK_ZIP_HOME = $main::OO_SDK_ZIP_HOME_SUGGESTION;
    }
    if ( (! -d "$main::OO_SDK_ZIP_HOME") ||
         ((-d "$main::OO_SDK_ZIP_HOME") && (! -e "$main::OO_SDK_ZIP_HOME/zip")) )
    {
        $main::OO_SDK_ZIP_HOME = "";
        print " Error: zip tool is required, please specify a zip tool directory.\n";
    } else
    {
        #check version
        my $testVersion = `$OO_SDK_ZIP_HOME/zip -h 2>&1 | egrep Zip | head -n 1`;
        $testVersion =~ s#Zip ([\d.]+) .*#$1#go;
        if ( $testVersion eq "")
        {
            print " Set the environment variable OO_SDK_ZIP_HOME to your zip tool directory.\n";
            print " zip version $main::zipVersion can be obtained at ftp://www.info-zip.org/\n";
        } else
        {
            if ($testVersion =~ m#((\d+\.)+\d+)# )
            {
                $testVersion = $1;
            }
            $main::correctVersion = testVersion($main::zipVersion, $testVersion, "$main::OO_SDK_MAKE_HOME/zip", 1);
            if ( !$main::correctVersion )
            {
                print " The 'zip' command found at '$main::OO_SDK_ZIP_HOME' has a wrong version\n";
                $main::OO_SDK_ZIP_HOME = "";
            }
        }
    }
}

# prepare cat path
$main::correctVersion = 0;
while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_CAT_HOME" ) ||
         ((-d "$main::OO_SDK_CAT_HOME") && (! -e "$main::OO_SDK_CAT_HOME/cat"))) )
{
    print " Enter cat tool directory [$main::OO_SDK_CAT_HOME_SUGGESTION]: ";
    $main::OO_SDK_CAT_HOME = readStdIn();
    chop($main::OO_SDK_CAT_HOME);
    if ( $main::OO_SDK_CAT_HOME eq "" )
    {
        $main::OO_SDK_CAT_HOME = $main::OO_SDK_CAT_HOME_SUGGESTION;
    }
    if ( (! -d "$main::OO_SDK_CAT_HOME") ||
         ((-d "$main::OO_SDK_CAT_HOME") && (! -e "$main::OO_SDK_CAT_HOME/cat")) )
    {
        $main::OO_SDK_CAT_HOME = "";
        print " Error: cat tool is required, please specify a cat tool directory.\n";
    }
    # else ...
    # TODO check version
    # NOTE: only Linux cat understands --version
}

# prepare sed path
$main::correctVersion = 0;
while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_SED_HOME" ) ||
         ((-d "$main::OO_SDK_SED_HOME") && (! -e "$main::OO_SDK_SED_HOME/sed"))) )
{
    print " Enter sed tool directory [$main::OO_SDK_SED_HOME_SUGGESTION]: ";
    $main::OO_SDK_SED_HOME = readStdIn();
    chop($main::OO_SDK_SED_HOME);
    if ( $main::OO_SDK_SED_HOME eq "" )
    {
        $main::OO_SDK_SED_HOME = $main::OO_SDK_SED_HOME_SUGGESTION;
    }
    if ( (! -d "$main::OO_SDK_SED_HOME") ||
         ((-d "$main::OO_SDK_SED_HOME") && (! -e "$main::OO_SDK_SED_HOME/sed")) )
    {
        $main::OO_SDK_SED_HOME = "";
        print " Error: sed tool is required, please specify a sed tool directory.\n";
    }
    # else ...
    # TODO check version
    # NOTE: only Linux sed understands --version
}

# prepare C++ compiler path
$main::correctVersion = 0;
while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_CPP_HOME" ) ||
         ((-d "$main::OO_SDK_CPP_HOME") && (! -e "$main::OO_SDK_CPP_HOME/$main::cpp"))) )
{
    print " C++ compilers where for example a language binding exist:\n";
    print "  - Solaris, Sun WorkShop 6 update 1 C++ 5.2 2000/09/11 or higher\n";
    print "  - Linux, GNU C++ compiler, gcc version 4.0.1 or higher\n";
    print "  - MacOS, GNU C++ compiler, gcc version 4.0.1 or higher\n";
    print " Enter the directory of the C++ compiler, the directory\n";
    print " where the compiler is located (optional) [$main::OO_SDK_CPP_HOME_SUGGESTION]: ";

    $main::OO_SDK_CPP_HOME = readStdIn();
    chop($main::OO_SDK_CPP_HOME);
    if ( $main::OO_SDK_CPP_HOME eq "" )
    {
        $main::OO_SDK_CPP_HOME = $main::OO_SDK_CPP_HOME_SUGGESTION;
    }

    if ( ! $main::OO_SDK_CPP_HOME eq "" )
    {
        if ( (! -d "$main::OO_SDK_CPP_HOME") ||
             ((-d "$main::OO_SDK_CPP_HOME") && (! -e "$main::OO_SDK_CPP_HOME/$main::cppName")) )
        {
            print " Error: Could not find directory '$main::OO_SDK_CPP_HOME'.\n";
            if ( skipChoice("C++ compiler") == 1 )
            {
                $main::correctVersion = 1;
            }
            $main::OO_SDK_CPP_HOME = "";
        } else
        {
            #check version
            if ( $main::cppName eq "gcc" )
            {
                my $testVersion = `$OO_SDK_CPP_HOME/$main::cppName -dumpversion`;
                if ( $testVersion eq "")
                {
                    print " The '$main::cppName' command found at $main::OO_SDK_CPP_HOME/$main::cppName is not a ";
                    print " GNU compiler.\nSet the environment variable OO_SDK_CPP_HOME to your GNU build tools ";
                    print " directory.\nA GNU compiler version $main::cppVersion can be obtained at ";
                    print " ftp://ftp.gnu.org/gnu/gcc/\n";
                } else
                {
                    $main::correctVersion = testVersion($main::cppVersion, $testVersion, "$main::OO_SDK_CPP_HOME/$main::cppName", 1);
                    if ( !$main::correctVersion )
                    {
                        print " The '$main::cppName' command found at '$main::OO_SDK_CPP_HOME' has a wrong version\n";
                        if ( skipChoice("C++ compiler") == 1 )
                        {
                            $main::correctVersion = 1;
                        }

                        $main::OO_SDK_CPP_HOME = "";
                    }
                }
            } else
            {
                # for Solaris we have to check the version too
                open(FILE, "$OO_SDK_CPP_HOME/$main::cppName -V 2>&1 |");
                my @lines = <FILE>;
                my $testVersion = $lines[0];
                if ( $testVersion eq "")
                {
                    print " The '$main::cppName' command found at $main::OO_SDK_CPP_HOME/$main::cppName is not a ";
                    print " Solaris C++ compiler.\nSet the environment variable OO_SDK_CPP_HOME to your Solaris C++ compiler directory.\n";
                } else
                {
                    if ($testVersion =~ m#((\d+\.)+\d+)# )
                    {
                        $testVersion = $1;
                    }
                    $main::correctVersion = testVersion($main::cppVersion, $testVersion, "$main::OO_SDK_CPP_HOME/$main::cppName", 1);
                    if ( !$main::correctVersion )
                    {
                        print " The '$main::cppName' command found at '$main::OO_SDK_CPP_HOME' has a wrong version\n";
                        if ( skipChoice("C++ compiler") == 1 )
                        {
                            $main::correctVersion = 1;
                        }

                        $main::OO_SDK_CPP_HOME = "";
                    } else {
                        $main::correctVersion = testVersion("5.5", $testVersion, "$main::OO_SDK_CPP_HOME/$main::cppName", 2);
                        if ( $main::correctVersion ) {
                            $main::OO_SDK_CC_55_OR_HIGHER = $testVersion;
                        }
                    }
                }
            }
        }
    } else
    {
        # the C++ compiler is optional
        $main::correctVersion = 1;
    }
}


# prepare Java path
$main::correctVersion = 0;

# prepare Java suggestion (cut bin or Commands directory to be in the root of the Java SDK)
$main::offset = rindex($main::OO_SDK_JAVA_HOME_SUGGESTION, "/$main::OO_SDK_JAVA_BIN_DIR");
if ( $main::offset != -1 )
{
    $main::OO_SDK_JAVA_HOME_SUGGESTION = substr($main::OO_SDK_JAVA_HOME_SUGGESTION, 0, $main::offset);
}

while ( (!$main::correctVersion) &&
        ((! -d "$main::OO_SDK_JAVA_HOME" ) ||
         ((-d "$main::OO_SDK_JAVA_HOME") && (! -e "$main::OO_SDK_JAVA_HOME/$main::OO_SDK_JAVA_BIN_DIR/javac"))) )
{
    print " Enter Java SDK (1.5, recommendation is 1.6 or higher) installation directory  (optional) [$main::OO_SDK_JAVA_HOME_SUGGESTION]: ";
    $main::OO_SDK_JAVA_HOME = readStdIn();
    chop($main::OO_SDK_JAVA_HOME);
    if ( $main::OO_SDK_JAVA_HOME eq "" )
    {
        $main::OO_SDK_JAVA_HOME = $main::OO_SDK_JAVA_HOME_SUGGESTION;
    }
    if ( ! $main::OO_SDK_JAVA_HOME eq "" )
    {
        if ( (! -d "$main::OO_SDK_JAVA_HOME") ||
             ((-d "$main::OO_SDK_JAVA_HOME") && (! -e "$main::OO_SDK_JAVA_HOME/$main::OO_SDK_JAVA_BIN_DIR/javac")) )
        {
            print " Error: Could not find directory '$main::OO_SDK_JAVA_HOME' or '$main::OO_SDK_JAVA_HOME/$main::OO_SDK_JAVA_BIN_DIR/javac'.\n";
            if ( skipChoice("JAVA SDK") == 1 )
            {
                $main::correctVersion = 1;
            }
            $main::OO_SDK_JAVA_HOME = "";
        } else
        {
            #check version
            my $testVersion = `$main::OO_SDK_JAVA_HOME/$main::OO_SDK_JAVA_BIN_DIR/java -version 2>&1 | egrep "java version" | head -n 1 | sed -e 's#.*version "##' | sed -e 's#".*##'`;
            $testVersion =~ s#([^\n]+)\n#$1#go;

            $main::correctVersion = testVersion($main::javaVersion, $testVersion, "$main::OO_SDK_JAVA_HOME/$main::OO_SDK_JAVA_BIN_DIR/java", 1);
            if ( !$main::correctVersion )
            {
                if ( skipChoice("JAVA SDK") == 1 )
                {
                    $main::correctVersion = 1;
                }
                $main::OO_SDK_JAVA_HOME = "";
            }
        }
    }else
    {
        # the Java SDK is optional
        $main::correctVersion = 1;
    }
}


# prepare output directory (optional)
while ( (!$main::skipOutputDir) &&
        (! -d "$main::OO_SDK_OUTPUT_DIR") )
{
    print " Default output directory is in your HOME directory.\n";
    print " Enter an existent directory if you prefer a different output directory (optional) [$main::OO_SDK_OUTPUT_DIR_SUGGESTION]: ";

    $main::OO_SDK_OUTPUT_DIR = readStdIn();

    chop($main::OO_SDK_OUTPUT_DIR);
    if ( $main::OO_SDK_OUTPUT_DIR eq "" )
    {
        $main::OO_SDK_OUTPUT_DIR = $main::OO_SDK_OUTPUT_DIR_SUGGESTION;
    }
    if ( ! $main::OO_SDK_OUTPUT_DIR eq "" )
    {
        if ( ! -d "$main::OO_SDK_OUTPUT_DIR" )
        {
            print " Error: Could not find directory '$main::OO_SDK_OUTPUT_DIR'.\n";
            if ( skipChoice("optional output directory") == 1 )
            {
                $main::skipOutputDir = 1;
            }
            $main::OO_SDK_OUTPUT_DIR = "";
        }
    } else
    {
        # the output directory is optional
        $main::skipOutputDir = 1;
    }
}

# prepare auto deployment
if ( $main::OFFICE_OR_URE eq "Office" )
{
    while ( $main::SDK_AUTO_DEPLOYMENT eq "" ||
         ((! $main::SDK_AUTO_DEPLOYMENT eq "YES") &&
          (! $main::SDK_AUTO_DEPLOYMENT eq "NO")) )
    {
        print " Automatic deployment of UNO components (YES/NO) [$main::SDK_AUTO_DEPLOYMENT_SUGGESTION]: ";
        $main::SDK_AUTO_DEPLOYMENT = uc <STDIN>;
        chop($main::SDK_AUTO_DEPLOYMENT);
        if ( $main::SDK_AUTO_DEPLOYMENT eq "" )
        {
            $main::SDK_AUTO_DEPLOYMENT = "YES";
        }
    }
}
else
{
    $main::SDK_AUTO_DEPLOYMENT = "NO";
}

prepareScriptFile("setsdkenv_unix.sh.in", "setsdkenv_unix.sh");
chmod 0644, "$main::OO_SDK_CONFIG_HOME/$main::hostname/setsdkenv_unix.sh";

print "\n";
print " ************************************************************************\n";
print " * ... your SDK environment has been prepared.\n";
print " * For each time you want to use this configured SDK environment, you\n";
print " * have to run the \"setsdkenv_unix\" script file!\n";
print " * Alternatively can you source one of the scripts\n";
print " *   \"$main::OO_SDK_CONFIG_HOME/$main::hostname/setsdkenv_unix.sh\"\n";
print " * to get an environment without starting a new shell.\n";
print " ************************************************************************\n\n";

exit $return;

sub skipChoice
{
    my $msg = shift;
    my $skip = "";
    while ( !( $skip eq "yes" || $skip eq "no") )
    {
        print " Do you want to skip the choice of the '$msg' (YES/NO): [YES] ";
        $skip = lc <STDIN>;
        chop($skip);
        if ( $skip eq "" ) { $skip = "yes"; } # default
        if ( $skip eq "yes" )
        {
            return 1;
        }
    }
    return 0;
}

sub resolveLink
{
    my $base= shift;
    my $link= shift;

    my $resolvedpath = "$base/$link";
    my $linktarget =  readlink "$resolvedpath";
    my $resolvedlink = "";

    while ( $linktarget ne "") {

    if ( $linktarget =~ m/^\/.*/ )
    {
        $resolvedpath = "$linktarget";
    } else {
        $resolvedpath = `cd $base/$linktarget; pwd`;
        chop $resolvedpath;
    }
    $base = dirname("$resolvedpath");

    $linktarget = readlink "$resolvedpath";
    }

    $resolvedlink = `cd $resolvedpath; pwd`;
    chop $resolvedlink;
    return $resolvedlink;
}

sub searchprog
{
    my $_search= shift;
    my $tmpPath = `echo "\$PATH"`;
    my @pathList = split(":" , $tmpPath);
    my $progDir = "";

    if ( $_search eq "javac" )
    {
        if ( $main::operatingSystem =~ m/darwin/ ) {
            $progDir = resolveLink("/System/Library/Frameworks/JavaVM.Framework/Versions", "CurrentJDK");

            if ( -e "$progDir/$main::OO_SDK_JAVA_BIN_DIR/javac" )
            {
                return "$progDir/$main::OO_SDK_JAVA_BIN_DIR";
            }
        }

        if ( $main::operatingSystem =~ m/solaris/ ) {
            $progDir = resolveLink("/usr/jdk", "latest");
            if ( -e "$progDir/$main::OO_SDK_JAVA_BIN_DIR/javac" )
            {
                return "$progDir/$main::OO_SDK_JAVA_BIN_DIR";
            }
        }
    }

    if ( $_search eq "gmake" && $main::operatingSystem =~ m/solaris/ ) {
        if ( -e "/usr/sfw/bin/gmake" )
        {
            return "/usr/sfw/bin";
        }
    }

    foreach $i (@pathList)
    {
        chomp ($i);

        if ( -e "$i/$_search" )
        {

            if ( index($i, "/") == 0 )
            {
                # # absolute path; leave unchanged
                $progDir = $i;
            } else
            {
                $progDir = `cd "$i"; pwd`;
            }
            return $progDir
        }
    }
    return $progDir
}

sub searchMacOffice
{
    if (-d "/Applications/LibreOffice.app" ) {
        return "/Applications/LibreOffice.app"
    }
    if (-d "/Applications/OpenOffice.org.app" ) {
        return "/Applications/OpenOffice.org.app"
    }
    if (-d "/Applications/Oracle Open Office.app" ) {
        return "/Applications/Oracle Open Office.app";
    }
    if (-d "/Applications/StarOffice.app" ) {
        return "/Applications/StarOffice.app";
    }
    if (-d "/Applications/StarSuite.app" ) {
        return "/Applications/StarSuite.app";
    }

    return "";
}

sub searchoffice
{
    my $offset = rindex($main::sdkpath, "/libreoffice");
    my $tmpOffice = substr($main::sdkpath, 0, $offset);
    my $officepath = "$tmpOffice/libreoffice";

#   if ( $main::OO_MINORVERSION > 0) {
#       $officepath = "$officepath$main::OO_MINORVERSION";
#   }

    # search corresponding office for this SDK
    if (-d $officepath && -e "$officepath/program/soffice") {
        return $officepath;
    }
    # fallback
    my $tmpversion = $main::OO_MAJORVERSION;
#   if ( $main::OO_MINORVERSION > 0) {
#       $tmpversion = "$tmpversion.$main::OO_MINORVERSION";
#   }

    $officepath = "$tmpOffice/oracle_open_office$tmpversion";
    if (-d $officepath && -e "$officepath/program/soffice") {
        return $officepath;
    }

    # Before trying versioned directories, check if the sdk is directly under
    # the office path.
    $officepath = $main::sdkpath . "/..";
    if (-d $officepath && -e "$officepath/program/soffice") {
        return $officepath;
    }

    my $tmpversion = $main::OO_MAJORVERSION + 6;
    if ( $main::OO_MINORVERSION > 0) {
        $tmpversion = "$tmpversion.$main::OO_MINORVERSION";
    }

    $officepath = "$tmpOffice/staroffice$tmpversion";
    if (-d $officepath && -e "$officepath/program/soffice") {
        return $officepath;
    }
    $officepath = "$tmpOffice/StarOffice$tmpversion";
    if (-d $officepath && -e "$officepath/program/soffice") {
        return $officepath;
    }
    $officepath = "$tmpOffice/starsuite$tmpversion";
    if (-d $officepath && -e "$officepath/program/soffice") {
        return $officepath;
    }
    $officepath = "$tmpOffice/StarSuite$tmpversion";
    if (-d $officepath && -e "$officepath/program/soffice") {
        return $officepath;
    }
    $officepath = "";

    # search other potential matching office versions
    my $path = "/opt/";
    my $entry = "";
    my $version = "0";
    for $entry (glob($path.'*')) {
        ## if the file is a directory
        if( -d $entry) {

            if ($entry =~ m#(.+(o|O)ffice(\.org){0,1}(\d([\d\.]){0,2}))# ||
                $entry =~ m#(.+(s|S)uite(.*)(\d([\d\.]){0,2}))# )
            {
                if ($4 > $version) {
                    $version = $4;
                    $officepath = $entry;
                }
            }
        }
    }
    return $officepath;
}



sub testVersion
{
    my $tmpMustBeVersion = shift;
    my $tmpTestVersion = shift;
    my $toolName = shift;
    # 1=check + message 2=check only
    my $checkOnly = shift;
    my @mustBeVersion = split(/\.|_|-/,$tmpMustBeVersion);
    my @testVersion = split(/\.|_|-/,$tmpTestVersion);
    my $length = $#mustBeVersion;

    if ($#testVersion < $#mustBeVersion) {
        $length = $#testVersion;
    }

    for ($i=0; $i <= $length; $i++ )
    {
        if ( $testVersion[$i] > $mustBeVersion[$i] )
        {
            return 1; # 1 indicates a correct version
        }

        if ( $testVersion[$i] < $mustBeVersion[$i] )
        {
            if ( $#checkOnly == 1 ) {
                print " The command '$toolName' has the version $tmpTestVersion.\n";
                print " The SDK requires at least the version $tmpMustBeVersion.\n";
            }
            return 0;
        }
    }

    return 1; # 1 indicates a correct version
}

sub readStdIn
{
    my $tmpstdin = <STDIN>;
    if ( index($tmpstdin, "\$") != -1)
    {
        return `echo $tmpstdin`;
    }

    return $tmpstdin;
}

sub prepareScriptFile()
{
    my $inputFile = shift;
    my $outputFile = shift;

    if ( ! -d "$main::OO_SDK_CONFIG_HOME/$main::hostname" )
    {
        system("mkdir -p $main::OO_SDK_CONFIG_HOME/$main::hostname");
    }

    open ( FILEIN, "$main::sdkpath/$inputFile" ) || die "\nERROR: could not open '$main::sdkpath/$inputFile' for reading";
    open ( FILEOUT, ">$main::OO_SDK_CONFIG_HOME/$main::hostname/$outputFile" ) || die "\nERROR: could not open '$main::OO_SDK_CONFIG_HOME/$main::hostname/$outputFile' for writing";

    while ( <FILEIN> )
    {
        $_ =~ s#\@OO_SDK_NAME\@#$main::OO_SDK_NAME#go;
        $_ =~ s#\@OO_SDK_HOME\@#$main::OO_SDK_HOME#go;
        $_ =~ s#\@OFFICE_HOME\@#$main::OFFICE_HOME#go;
        $_ =~ s#\@OO_SDK_URE_HOME\@#$main::OO_SDK_URE_HOME#go;
        $_ =~ s#\@OO_SDK_MAKE_HOME\@#$main::OO_SDK_MAKE_HOME#go;
        $_ =~ s#\@OO_SDK_ZIP_HOME\@#$main::OO_SDK_ZIP_HOME#go;
        $_ =~ s#\@OO_SDK_CAT_HOME\@#$main::OO_SDK_CAT_HOME#go;
        $_ =~ s#\@OO_SDK_SED_HOME\@#$main::OO_SDK_SED_HOME#go;
        $_ =~ s#\@OO_SDK_CPP_HOME\@#$main::OO_SDK_CPP_HOME#go;
        $_ =~ s#\@OO_SDK_CC_55_OR_HIGHER\@#$main::OO_SDK_CC_55_OR_HIGHER#go;
        $_ =~ s#\@OO_SDK_JAVA_HOME\@#$main::OO_SDK_JAVA_HOME#go;
        $_ =~ s#\@SDK_AUTO_DEPLOYMENT\@#$main::SDK_AUTO_DEPLOYMENT#go;
        $_ =~ s#\@OO_SDK_OUTPUT_DIR\@#$main::OO_SDK_OUTPUT_DIR#go;

        print FILEOUT $_;
    }

    close FILEIN;
    close FILEOUT;
}
