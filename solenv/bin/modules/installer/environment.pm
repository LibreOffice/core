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

package installer::environment;

use strict;
use warnings;

use installer::globals;

######################################################
# Create path variables from environment variables
######################################################

sub create_pathvariables
{
    my ($environment) = @_;

    my %variables = ();

    # The following variables are needed in the path file list
    # solarenvpath, os, pmiscpath

    my $solarenvpath = "";

    if ( $ENV{'SO_PACK'} ) { $solarenvpath  = $ENV{'SO_PACK'}; }
    # overriding with STAR_INSTPATH, if set
    if ( $ENV{'STAR_INSTPATH'} ) { $solarenvpath = $ENV{'STAR_INSTPATH'}; }

    $variables{'solarenvpath'} = $solarenvpath;

    my $localpath  = $environment->{'LOCAL_OUT'};
    $variables{'localpath'} = $localpath;

    my $localcommonpath  = $environment->{'LOCAL_COMMON_OUT'};
    $variables{'localcommonpath'} = $localcommonpath;

    my $installscriptdir = $environment->{'WORKDIR'} . $installer::globals::separator . "InstallScriptTarget";
    $variables{'installscriptdir'} = $installscriptdir;

    my $extensionsdir = $environment->{'WORKDIR'} . $installer::globals::separator . "Extension";
    $variables{'extensionsdir'} = $extensionsdir;

    my $customtargetpath = $environment->{'WORKDIR'} . $installer::globals::separator . "CustomTarget";
    $variables{'customtargetpath'} = $customtargetpath;

    my $filelistpath = $environment->{'WORKDIR'};
    $variables{'filelistpath'} = $filelistpath;

    my $licensepath = $environment->{'SRCDIR'} . $installer::globals::separator . "readlicense_oo/license";
    $variables{'licensepath'} = $licensepath;

    my $packinfopath = $environment->{'SRCDIR'} . $installer::globals::separator . "setup_native/source/packinfo";
    $variables{'packinfopath'} = $packinfopath;

    return \%variables;
}

##################################################
# Replacing tilde in paths, because of
# problem with deep recursion (task 104830)
##################################################

sub check_tilde_in_directory
{
    if ( $ENV{'HOME'} )
    {
        my $home = $ENV{'HOME'};
        $home =~ s/\Q$installer::globals::separator\E\s*$//;
        $installer::globals::localinstalldir =~ s/~/$home/;
        my $infoline = "Info: Changing LOCALINSTALLDIR to $installer::globals::localinstalldir\n";
        push(@installer::globals::logfileinfo, $infoline);
    }
    else
    {
        # exit, because "~" is not allowed, if HOME is not set
        my $infoline = "ERROR: If \"~\" is used in \"LOCALINSTALLDIR\", environment variable \"HOME\" needs to be defined!\n";
        push(@installer::globals::logfileinfo, $infoline);
        die 'If "~" is used in "LOCALINSTALLDIR", environment variable "HOME" needs to be defined!';
    }
}

##################################################
# Setting some fundamental global variables.
# All these variables can be overwritten
# by parameters.
##################################################

sub set_global_environment_variables
{
    my ( $environment ) = @_;

    $installer::globals::build = $environment->{'LIBO_VERSION_MAJOR'}.$environment->{'LIBO_VERSION_MINOR'}."0";
    $installer::globals::os = $environment->{'OS'};
    $installer::globals::com = $environment->{'COM'};
    $installer::globals::cpuname = $environment->{'CPUNAME'};
    $installer::globals::platformid = $environment->{'PLATFORMID'};

    if ( $ENV{'ENABLE_DBGUTIL'} ) {} else { $installer::globals::pro = 1; }

    if ( $ENV{'VERBOSE'} && ( (lc $ENV{'VERBOSE'}) eq "false" ) ) { $installer::globals::quiet = 1; }
    if ( $ENV{'PREPARE_WINPATCH'} ) { $installer::globals::prepare_winpatch = 1; }
    if ( $ENV{'PREVIOUS_IDT_DIR'} ) { $installer::globals::previous_idt_dir = $ENV{'PREVIOUS_IDT_DIR'}; }
    if ( $ENV{'LOCALINSTALLDIR'} ) { $installer::globals::localinstalldir = $ENV{'LOCALINSTALLDIR'}; }
    if ( $ENV{'LOCALUNPACKDIR'} ) { $installer::globals::localunpackdir = $ENV{'LOCALUNPACKDIR'}; }
    if ( $ENV{'MAX_LANG_LENGTH'} ) { $installer::globals::max_lang_length = $ENV{'MAX_LANG_LENGTH'}; }

    if ( $ENV{'RPM'} ) { $installer::globals::rpm = $ENV{'RPM'}; }
    if ( $ENV{'DONTCOMPRESS'} ) { $installer::globals::solarisdontcompress = 1; }
    if ( $ENV{'IGNORE_ERROR_IN_LOGFILE'} ) { $installer::globals::ignore_error_in_logfile = 1; }
    if (( $ENV{'ENABLE_STRIP'} ) && ( $ENV{'ENABLE_STRIP'} ne '' )) { $installer::globals::strip = 1; }
    if (( $ENV{'DISABLE_STRIP'} ) && ( $ENV{'DISABLE_STRIP'} ne '' )) { $installer::globals::strip = 0; }

    if ( $installer::globals::localinstalldir ) { $installer::globals::localinstalldirset = 1; }
    # Special handling, if LOCALINSTALLDIR contains "~" in the path
    if ( $installer::globals::localinstalldir =~ /^\s*\~/ ) { check_tilde_in_directory(); }
}

1;
