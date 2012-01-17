package cwstestresulthelper;

#**************************************************************
#
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#
#**************************************************************



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
