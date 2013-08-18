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

#
# check - a perl script to check some files and directories if they exists
# A first simple check if the SDK was built completely.
#

$return = 0;

$SdkDir = "$ARGV[0]";
$OperatingSystem = "$ARGV[1]";
$ExePrefix = "$ARGV[2]";

sub check_file
{
    my ($file) = @_;
    return -e "$SdkDir/$file";
}

sub check_dir
{
    my ($dir) = @_;
    return -d "$SdkDir/$dir";
}

print "Check for $OperatingSystem\n";

if (-d "$SdkDir") {
    # check binaries
    print "check binaries: ";
    if (check_dir("bin")) {
	my @binarylist = ( "idlc","cppumaker","javamaker",
               "regcompare",
               "unoapploader", "uno-skeletonmaker" );
    if ($ENV{SYSTEM_UCPP} eq "") {
	push @binarylist,"ucpp";
    }

    foreach $i (@binarylist)
    {
        if (!check_file("bin/$i$ExePrefix")) {
        $return++;
        print "\nERROR: \"bin/$i$ExePrefix\" is missing\n";
        } else {
        print "+";
        }
    }

    if ($OperatingSystem eq "windows" || $OperatingSystem eq "mingw") {
        if ($OperatingSystem eq "windows" && !check_file("bin/climaker.exe")) {
        $return++;
        print "\nERROR: \"bin/climaker.exe\" is missing\n";
        } else {
        print "+";
        }
    }
    if ($OperatingSystem eq "macosx") {
        if (!check_file("bin/addsym-macosx.sh")) {
        $return++;
        print "\nERROR: \"bin/addsym-macosx.sh\" is missing\n";
        } else {
        print "+";
        }
    }
    } else {
    $return++;
    }
    print "\n";

    # packaging files
    print "check packaging files: ";
    if (check_dir("docs")) {
    my @filelist = ( "install.html",
             "sdk_styles.css","tools.html",
             "images/arrow-1.gif", "images/arrow-3.gif",
             "images/odk-footer-logo.gif",
             "images/bg_table.png","images/bg_table2.png",
             "images/bg_table3.png", "images/nav_down.png",
             "images/nav_home.png","images/nav_left.png",
             "images/nav_right.png","images/nav_up.png",
             "images/sdk_head-1.png", "images/sdk_head-2.png",
             "images/sdk_line-1.gif", "images/sdk_line-2.gif",
             "images/nada.gif",
             "images/arrow-2.gif", "images/bluball.gif",
             "images/ooo-main-app_32.png");

    foreach $i (@filelist)
    {
        if (!check_file("docs/$i")) {
        $return++;
        print "\nERROR: \"docs/$i\" is missing\n";
        } else {
        print "+";
        }
    }
    } else {
    $return++;
    }
    print "\n";

    #check configure files
    print "check config files: ";
    if ($OperatingSystem eq "windows" || $OperatingSystem eq "mingw") {
    if (!check_file("setsdkenv_windows.bat")) {
        print "\nERROR: \"setsdkenv_windows.bat\" is missing\n";
        $return++;
    }
    if (!check_file("setsdkenv_windows.template")) {
        print "\nERROR: \"setsdkenv_windows.template\" is missing\n";
        $return++;
    }
    if (!check_file("cfgWin.js")) {
        print "\nERROR: \"cfgWin.js\" is missing\n";
        $return++;
    }
    } else {
    if (!check_file("configure.pl")) {
        print "\nERROR: \"configure.pl\" is missing\n";
        $return++;
    }
    if (!check_file("config.guess")) {
        print "\nERROR: \"config.guess\" is missing\n";
        $return++;
    }
    if (!check_file("config.sub")) {
        print "\nERROR: \"config.sub\" is missing\n";
        $return++;
       }
    if (!check_file("setsdkenv_unix")) {
        print "\nERROR: \"setsdkenv_unix\" is missing\n";
        $return++;
    }
    if (!check_file("setsdkenv_unix.sh.in")) {
        print "\nERROR: \"setsdkenv_unix.sh.in\" is missing\n";
        $return++;
    }
    }
    print "\n";

    #check setting files
    print "check setting files: ";
    if (check_dir("settings")) {
    if (!check_file("settings/settings.mk")) {
        print "\nERROR: \"settings/settings.mk\" is missing\n";
        $return++;
    }
    if (!check_file("settings/std.mk")) {
        print "\nERROR: \"settings/std.mk\" is missing\n";
        $return++;
    }
    if (!check_file("settings/stdtarget.mk")) {
        print "\nERROR: \"settings/stdtarget.mk\" is missing\n";
        $return++;
    }
    } else {
    $return++;
    }
    print "\n";

    #check cpp docu, it is only a first and simple check
    # improvement required
    if ($ENV{'DOXYGEN'} ne '') {
        print "check cpp docu: ";
        if (check_dir("docs/cpp/ref")) {
            if (!check_file("docs/cpp/ref/index.html")) {
                print "\nERROR: \"docs/cpp/ref/index.html\" is missing\n";
                $return++;
            }
        } else {
            $return++;
        }
        print "\n";
    }

    #check java docu, it is only a first and simple check
    # improvement required
    my $solar_java = $ENV{"ENABLE_JAVA"};
    my $JDK = $ENV{"JDK"};
    if (defined($solar_java) && $solar_java ne "" && (!defined($JDK) || $JDK ne "gcj")) {
    print "check java docu: ";
    if (check_dir("docs/java/ref")) {
        if (!check_file("docs/java/ref/index.html")) {
        print "\nERROR: \"docs/java/ref/index.html\" is missing\n";
        $return++;
        }

        my @dir_list = ( "lib","lib/uno","lib/uno/helper","lib/uno/helper/class-use",
                 "uno","uno/class-use","comp","comp/helper",
                 "comp/helper/class-use");

        foreach $i (@dir_list)
        {
        if (!check_dir("docs/java/ref/com/sun/star/$i")) {
            $return++;
            print "\nERROR: \"docs/java/ref/com/sun/star/$i\" is missing\n";
        } else {
            print "+";
        }
        }
    } else {
        $return++;
    }
    print "\n";
    }

    #check idl docu, it is only a first and simple check
    # improvement required
    if ($ENV{'DOXYGEN'} ne '') {
        print "check idl docu: ";
        if (check_dir("docs/idl/ref")) {
        if (!check_file("docs/idl/ref/index.html")) {
            print "\nERROR: \"docs/idl/ref/index.html\" is missing\n";
            $return++;
        }
        if (!check_file("docs/idl/ref/classes.html")) {
            print "\nERROR: \"docs/idl/ref/classes.html\" is missing\n";
            $return++;
        }
        if (!check_file("docs/idl/ref/namespaces.html")) {
            print "\nERROR: \"docs/idl/ref/namespaces.html\" is missing\n";
            $return++;
        }

        my @idl_dirlist = ( "accessibility",
                    "animations",
                    "auth",
                    "awt",
                    "awt/tab",
                    "awt/tree",
                    "awt/grid",
                    "beans",
                    "bridge",
                    "bridge/oleautomation",
                    "chart",
                    "chart2",
                    "chart2/data",
                    "configuration",
                    "configuration/backend",
                    "configuration/backend/xml",
                    "configuration/bootstrap",
                    "connection",
                    "container",
                    "cui",
                    "datatransfer",
                    "datatransfer/clipboard",
                    "datatransfer/dnd",
                    "deployment",
                    "deployment/ui",
                    "document",
                    "drawing",
                    "drawing/framework",
                    "embed",
                    "form",
                    "form/binding",
                    "form/component",
                    "form/control",
                    "form/inspection",
                    "form/runtime",
                    "form/submission",
                    "form/validation",
                    "formula",
                    "frame",
                    "frame/status",
                    "gallery",
                    "geometry",
                    "graphic",
                    "i18n",
                    "image",
                    "inspection",
                    "io",
                    "java",
                    "lang",
                    "ldap",
                    "linguistic2",
                    "loader",
                    "logging",
                    "mail",
                    "media",
                    "mozilla",
                    "packages",
                    "packages/manifest",
                    "packages/zip",
                    "plugin",
                    "presentation",
                    "reflection",
                    "registry",
                    "rendering",
                    "report",
                    "report/inspection",
                    "resource",
                    "scanner",
                    "script",
                    "script/browse",
                    "script/provider",
                    "sdb",
                    "sdb/application",
                    "sdb/tools",
                    "sdbc",
                    "sdbcx",
                    "security",
                    "sheet",
                    "smarttags",
                    "style",
                    "svg",
                    "system",
                    "table",
                    "task",
                    "text",
                    "text/fieldmaster",
                    "text/textfield",
                    "text/textfield/docinfo",
                    "ucb",
                    "ui",
                    "ui/dialogs",
                    "uno",
                    "uri",
                    "util",
                    "view",
                    "xforms",
                    "xml",
                    "xml/crypto",
                    "xml/crypto/sax",
                    "xml/csax",
                    "xml/dom",
                    "xml/dom/events",
                    "xml/dom/views",
                    "xml/input",
                    "xml/sax",
                    "xml/wrapper",
                    "xml/xpath",
                    "xsd" );

        # Due to MSI limitations have to use SHORT_NAMES on windows so can't check
        if ($OperatingSystem ne "windows") {
        foreach $i (@idl_dirlist)
        {
            $i =~ s/\//_1_1/g;
            if (!check_file("docs/idl/ref/namespacecom_1_1sun_1_1star_1_1$i.html")) {
            $return++;
            print "\nERROR: \"docs/idl/ref/namespacecom_1_1sun_1_1star_1_1$i.html\" is missing\n";
            } else {
            print "+";
            }
        }
        }
        } else {
        $return++;
        }
        print "\n";
    }

} else {
    $return++;
}

if( $return != 0 )
{
    print "ERROR\n";
} else {
    print "OK\n";
}
exit $return;
