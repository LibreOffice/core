#!/bin/bash
#*************************************************************************
#*
#*  OpenOffice.org - a multi-platform office productivity suite
#*
#*  $RCSfile: run_tests.sh,v $
#*
#*  $Revision: 1.2 $
#*
#*  last change: $Author: andreschnabel $ $Date: 2008/09/05 16:56:19 $
#*
#*  The Contents of this file are made available subject to
#*  the terms of GNU Lesser General Public License Version 2.1.
#*
#*
#*    GNU Lesser General Public License Version 2.1
#*    =============================================
#*    Copyright 2005 by Sun Microsystems, Inc.
#*    901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*    This library is free software; you can redistribute it and/or
#*    modify it under the terms of the GNU Lesser General Public
#*    License version 2.1, as published by the Free Software Foundation.
#*
#*    This library is distributed in the hope that it will be useful,
#*    but WITHOUT ANY WARRANTY; without even the implied warranty of
#*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*    Lesser General Public License for more details.
#*
#*    You should have received a copy of the GNU Lesser General Public
#*    License along with this library; if not, write to the Free Software
#*    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*    MA  02111-1307  USA
#*
#########################################################################
#
# Owner : andreschnabel@openoffice.org
#
# short description : run several testscripts on unix
#                     list of scripts to run is read from stdin
#                     this script has been derived from former ooo_releasetests.sh
#
#########################################################################


# set location of testscripts (the 'qatesttool' directory)
# if this is empty, we try to parse $HOME/.testtoolrc
sLocation=""

# set location of TestTool
# (full path including executable 'testtool')
# if this is empty, we try to parse $HOME/.testtoolrc and use OOoProgramdir
# on Mac this is VCLTestTool.app/Contents/MacOS/testtool.bin
#sTestTool="/Users/sca/qa/testtool/VCLTestTool.app/Contents/MacOS/testtool.bin"
sTestTool=""

#------------------------------------------------------------------------
#--- internally used functions ---
#------------------------------------------------------------------------
function GetValueFromSection ()
# call with NameOfValue Section file
{
   $AWK -v  sVarName="$1" -v sSectionName="$2" \
      'BEGIN { bInSection = 0}
      {  if ( index ($0 ,"[" sSectionName "]") > 0 )
            bInSection = 1
         else if ( (bInSection == 1) && (substr ($0,1,1) == "[") )
            bInSection = 0
         if ( (bInSection == 1) && (index ($0 , sVarName "=") == 1)) {
             sub(/\r$/,"")
            print substr ($0,index ($0 ,"=") + 1)
            }
      }' < "$3"
}

#------------------------------------------------------------------------
#--- the main script starts here ---
#------------------------------------------------------------------------

#platform specific settings
case `uname -s` in
    Darwin)
        testtoolrc="$HOME/Library/Application Support/.testtoolrc"
        KILLOFFICEALL="/usr/bin/killall -9 soffice.bin"
    AWK=awk
        ;;
  SunOS)
        testtoolrc="$HOME/.testtoolrc"
        KILLOFFICEALL="pkill -9 soffice.bin"
    AWK=nawk
        ;;
    *)
        testtoolrc="$HOME/.testtoolrc"
        KILLOFFICEALL="pkill -9 soffice.bin"
    AWK=awk
        ;;
esac

# if sLocation is not set manuall try to get the location form testtoolrc
if [ -z "$sLocation" ]
then
   # first read the profile
   sProfile=`GetValueFromSection CurrentProfile Misc "$testtoolrc"`
   # then read the BaseDir for the profile
   sLocation=`GetValueFromSection BaseDir "$sProfile" "$testtoolrc"`
   sLocation="$sLocation/"
fi

# set location of close-office file
# (see cvs)
sExitOfficeBas="${sLocation}global/tools/closeoffice.bas"
sResetOfficeBas="${sLocation}global/tools/resetoffice.bas"

# if sTestTool is not set manuall try to get the location form testtoolrc
if [ -z "$sTestTool" ]
then
   sTestTool=`GetValueFromSection Current OOoProgramDir "$testtoolrc"`
   sTestTool="$sTestTool/basis-link/program/testtool.bin"
fi



echo "****************************************************"
echo "************ STARTING ************"
echo "****************************************************"

# check if there is a virtual display available

echo "DISPLAY is set to: " $DISPLAY
echo "My name is: " $USER

# test if location exists
if [ -d "$sLocation" ]
then
    echo "using scripts from $sLocation"
else
    echo "test scripts not found at $sLocation"
    echo "Please set sLocation in this script"
    exit 1
fi

# test if testtool.bin exists
if [ -f "$sTestTool" ]
then
    echo "using testtool from $sTestTool"
else
    echo "testtool not found at $sTestTool"
    echo "Please set sTestTool in this script"
    exit 1
fi

i=0

while read x ;
do
    echo "Running soffices' processes: "
    # kill office, if exists
    `$KILLOFFICEALL`

    echo "****************************************************"
    x=`echo "$x"|sed s/[[:blank:]]*$//` #cut all trailing blanks
    sTest="$sLocation$x"
    if [ -f "$sTest" ]; then

        # two pass logic - first pass is the real test, second pass is reset office, third is closeoffice
            for z in "1" "2";
        do
            echo "running <$sTest>"
            "$sTestTool" -run "$sTest" &
            sleep 5
            echo " "

            ######### save the PID from the last Background job
            testtoolpid=$!
            echo "PID of Testtool: " $testtoolpid

            if [ `ps -A | grep -v "grep" | grep $testtoolpid | wc -l` -gt 0 ] ; then
                echo " Successfully started"
            else
                echo " There might be something wrong with starting the Testtool!"
            fi

            ######### wait until Testtool has finished & closed
            while [ `ps -A | grep -v "grep" | grep $testtoolpid | wc -l` -gt 0 ] ;
            do
                sleep 5
                i=$((i+5))
            done

            ####### for the second run use the office reset script!
                case $z in
                        "1")    sTest="$sResetOfficeBas";;
                    esac
        done
    else
        echo "Error: file <$sTest> not found"
    fi
done

echo "Duration:" $((i/60)) "min" $((i%60)) "sec "

echo "****************************************************"
echo "************ FINISHED ************"
echo "****************************************************"
