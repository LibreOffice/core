package cwstestresulthelper;

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

use English;
use warnings;
use strict;
use Cwd;
use Cwd 'chdir';

use stringhelper;
use loghelper;
use oshelper;
use filehelper;
use CallExternals;

BEGIN {
    use Exporter   ();
    our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);

    $VERSION     = 1.00;
    # if using RCS/CVS, this may be preferred
    $VERSION = do { my @r = (q$Revision: 1.1 $ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
    @ISA         = qw(Exporter);
    @EXPORT      = qw(&cwstestresult);
    %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
    # your exported package globals go here,
    # as well as any optionally exported functions
    @EXPORT_OK   = ( ); # qw($Var1 %Hashit &func3);
}

sub cwstestresult($$$$$$)
{
    my $sStatus = shift;
    my $sDBdistinct = shift;
    my $sourceversion = shift;
    my $destinationversion = shift;

    my $sSOLARENV; # = getSolenvPath();
    my $nSOLARENV_fake = 0;
    my $sCOMMON_ENV_TOOLS;
    my $nCOMMON_ENV_TOOLS_fake = 0;

    my $MAJOR;
    my $MINOR;
    # we need an extra state in DB
    # if this state is given here, we need to add information in cws back.
    if ( ! $sSOLARENV)
    {
        my @MAJORMINOR=split('_', $sourceversion);
        if ($#MAJORMINOR < 1)
        {
            print "Failure with sourceversion '$sourceversion' not splitable.\n";
            return;
        }
        $MAJOR=$MAJORMINOR[0]; # DEV300, OOH310, ...
        $MINOR=$MAJORMINOR[1]; # m45, ...
        if (getEnvironment() eq "wntmsci")
        {
            $sSOLARENV="o:/$MAJOR/ooo.$MINOR/solenv";
            if (! -e $sSOLARENV)
            {
                # fallback to old before ause103 (treeconfig)
                $sSOLARENV="o:/$MAJOR/src.$MINOR/solenv";
            }
        }
        elsif (getEnvironment() eq "unxlngi" ||
               getEnvironment() eq "unxsoli")
        {
            $sSOLARENV="/so/ws/$MAJOR/ooo.$MINOR/solenv";
            # automount
            system("ls -al $sSOLARENV >/dev/null");
            sleep(1);
            if (! -e $sSOLARENV)
            {
                # fallback to old before ause103 (treeconfig)
                $sSOLARENV="/so/ws/$MAJOR/src.$MINOR/solenv";
            }
        }
        else
        {
            log_print("cwstestresult(): This environment is not supported.");
            return;
        }
    }
    if ( !defined($ENV{SOLARENV}) || length($ENV{SOLARENV}) == 0 )
    {
        $ENV{SOLARENV} = $sSOLARENV;
        log_print("        SOLARENV is: $ENV{SOLARENV} faked\n");
        $nSOLARENV_fake = 1;
    }
    if ( ! $sCOMMON_ENV_TOOLS)
    {
        if (isWindowsEnvironment())
        {
            $sCOMMON_ENV_TOOLS="r:/etools";
        }
        elsif (isUnixEnvironment() )
        {
            $sCOMMON_ENV_TOOLS="/so/env/etools";
            # automount
            system("ls -al $sCOMMON_ENV_TOOLS >/dev/null");
            sleep(1);
        }
        else
        {
            log_print("cwstestresult(): This environment is not supported. (variable COMMON_ENV_TOOLS not set.)");
            return;
        }
    }
    if ( !defined($ENV{COMMON_ENV_TOOLS}) || length($ENV{COMMON_ENV_TOOLS}) == 0 )
    {
        $ENV{COMMON_ENV_TOOLS} = $sCOMMON_ENV_TOOLS;
        log_print( "COMMON_ENV_TOOLS is: $ENV{COMMON_ENV_TOOLS} faked\n");
        $nCOMMON_ENV_TOOLS_fake = 1;
    }

    # if ( !defined($ENV{WORK_STAMP}) )
    # {
    #     $ENV{WORK_STAMP} = $MAJOR;
    #     log_print( "      WORK_STAMP is: $ENV{WORK_STAMP} faked\n");
    # }
    # if ( !defined($ENV{UPDMINOR}) )
    # {
    #     $ENV{UPDMINOR} = $MINOR;
    #     log_print( "        UPDMINOR is: $ENV{UPDMINOR} faked\n");
    # }

    my $nWORK_STAMP_fake = 0;
    my $nUPDMINOR_fake = 0;

    if ( !defined($ENV{WORK_STAMP}) || length($ENV{WORK_STAMP}) == 0 )
    {
        $ENV{WORK_STAMP} = $MAJOR;
        log_print("        WORK_STAMP is: $ENV{WORK_STAMP} faked\n");
        $nWORK_STAMP_fake = 1;
    }
    if ( !defined($ENV{UPDMINOR}) || length($ENV{WORK_STAMP}) == 0 )
    {
        $ENV{UPDMINOR} = $MINOR;
        log_print("        UPDMINOR is: $ENV{UPDMINOR} faked\n");
        $nUPDMINOR_fake = 1;
    }

    # my $sStatus = "ok";
    # if ($nFailure == 0)
    # {
    #     $sStatus = $sInfo;
    # }
    # elsif ($nFailure == 1)
    # {
    #     $sStatus = "failed";
    # }
    # elsif ($nFailure == 2)
    # {
    #     $sStatus = "incomplete";
    # }

    # system("cwstestresult -c mycws -n Performance -p Windows ok");
    my $sPerlProgram = appendPath($sSOLARENV, "bin/cwstestresult.pl");
    # if ( -e "cwstestresult.pl" )
    # {
    #     # use a local version instead
    #     $sPerlProgram = "cwstestresult.pl";
    # }
    # else
    # {
    #     my $currentdir =cwd();
    #     log_print( "We are in $currentdir\n");
    # }

    my $sPerlParam;
    # $sPerlParam  = " -m $MAJOR"; # master CWS
    $sPerlParam .= " -c $destinationversion"; # name of CWS
    $sPerlParam .= " -n ConvWatch"; # ConvWatch need to be capitalised for cwstestresult
    my $sCWSEnv;
    if (isWindowsEnvironment())
    {
        $sCWSEnv = "Windows";
    }
    elsif (getEnvironment() eq "unxlngi")
    {
        $sCWSEnv = "Linux";
    }
    elsif (getEnvironment() eq "unxsoli")
    {
        $sCWSEnv = "SolarisX86";
    }
    else
    {
        log_print("cwstestresult(): This environment is not supported. (getEnvironment() returns wrong value?)");
        return;
    }
    $sPerlParam .= " -p " . $sCWSEnv;
    $sPerlParam .= " -r http://so-gfxcmp-lin.germany.sun.com/gfxcmp_ui/status_new.php?distinct=$sDBdistinct";

    $sPerlParam .= " ";
    $sPerlParam .= $sStatus;


    # my $sSetcwsAndPerl = "setcws $destinationversion; " . getPerlExecutable();

    my $err = callperl(getPerlExecutable(), $sPerlProgram, $sPerlParam);
    if ($err != 0)
    {
        log_print( "Can't call cwstestresult.pl\n");
    }
    if ($nSOLARENV_fake == 1)
    {
        $ENV{SOLARENV} = "";
        undef( $ENV{SOLARENV} );
        $nSOLARENV_fake = 0;
        # if ( defined($ENV{SOLARENV}) )
        # {
        #     print "SOLARENV always defined.\n";
        # }
    }
    if ($nCOMMON_ENV_TOOLS_fake == 1)
    {
        $ENV{COMMON_ENV_TOOLS} = "";
        undef( $ENV{COMMON_ENV_TOOLS} );
        $nCOMMON_ENV_TOOLS_fake = 0;
    }

    if ( $nWORK_STAMP_fake == 1 )
    {
        # undef($ENV{WORK_STAMP});
        $ENV{WORK_STAMP} = "";
        undef($ENV{WORK_STAMP});
        $nWORK_STAMP_fake = 0;
    }
    if ( $nUPDMINOR_fake == 1 )
    {
        $ENV{UPDMINOR} = "";
        undef($ENV{UPDMINOR});
        $nUPDMINOR_fake = 0;
    }


}


1;
