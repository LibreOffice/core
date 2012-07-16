rem
rem This file is part of the LibreOffice project.
rem
rem This Source Code Form is subject to the terms of the Mozilla Public
rem License, v. 2.0. If a copy of the MPL was not distributed with this
rem file, You can obtain one at http://mozilla.org/MPL/2.0/.
rem
rem This file incorporates work covered by the following license notice:
rem
rem   Licensed to the Apache Software Foundation (ASF) under one or more
rem   contributor license agreements. See the NOTICE file distributed
rem   with this work for additional information regarding copyright
rem   ownership. The ASF licenses this file to you under the Apache
rem   License, Version 2.0 (the "License"); you may not use this file
rem   except in compliance with the License. You may obtain a copy of
rem   the License at http://www.apache.org/licenses/LICENSE-2.0 .
rem

call env_win.bat

@echo off
echo =============================================
echo step - 1 create a temporary folder
echo =============================================
@echo on
%TEMP_DRIVE%
cd /
mkdir %TEMP_FOLDER%

@echo off
echo =============================================
echo step - 2 copy the xsecsim.rdb
echo =============================================
@echo on

cp %WS_PATH%/xmlsecurity/wntmsci8.pro/bin/xsecsim.rdb %TEMP_DRIVE%/%TEMP_FOLDER%/.

@echo off
echo =============================================
echo step - 3 copy services.rdb and types.rdb
echo =============================================
@echo on

cp %SO_DRIVE%/%SO_PATH%/program/services.rdb %TEMP_DRIVE%/%TEMP_FOLDER%/.
cp %SO_DRIVE%/%SO_PATH%/program/types.rdb %TEMP_DRIVE%/%TEMP_FOLDER%/.

@echo off
echo =============================================
echo step - 4 add types.rdb, xsecsim.rdb to services.rdb
echo =============================================
@echo on

cd %TEMP_FOLDER%
regmerge services.rdb / types.rdb
regmerge services.rdb / xsecsim.rdb

@echo off
echo =============================================
echo step - 5 register new component
echo =============================================
@echo on

mkdir windows.plt
cp %WS_PATH%/xmlsecurity/wntmsci8.pro/bin/xsec_sim.dll windows.plt/.
cp %WS_PATH%/xmlsecurity/wntmsci8.pro/bin/xsecsim.rdb windows.plt/.
cp %WS_PATH%/xmlsecurity/wntmsci8.pro/class/jflatfilter.jar windows.plt/.
zip xsec_sim.zip windows.plt\*.*
rm -R windows.plt

%SO_DRIVE%
cd %SO_DRIVE%/%SO_PATH%/program
pkgchk -s %TEMP_DRIVE%/%TEMP_FOLDER%/xsec_sim.zip

%TEMP_DRIVE%

@echo off
echo =============================================
echo step - 6 copy dynamic libraries
echo =============================================
@echo on

cp %SO_DRIVE%/%SO_PATH%/program/*.dll ./.

@echo off
echo =============================================
echo step - 7 copy testtool program
echo =============================================
@echo on

cp %WS_PATH%/xmlsecurity/wntmsci8.pro/bin/testtool.exe ./.

@echo off
echo =============================================
echo step - 8 start LibreOffice
echo =============================================
@echo on

@echo off
echo please start up LibreOffice with "--accept=socket,host=0,port=2002;urp;"
pause Press Enter when finished...
@echo on

@echo off
echo =============================================
echo step - 9 run the testtool program
echo =============================================
@echo on

mkdir examples
cd examples
cp %WS_PATH%/xmlsecurity/tools/examples/warmup.xml ./.
cp %WS_PATH%/xmlsecurity/tools/examples/sign-0.xml ./.
cp %WS_PATH%/xmlsecurity/tools/examples/sign-1.xml ./.
cp %WS_PATH%/xmlsecurity/tools/examples/sign-2.xml ./.
cp %WS_PATH%/xmlsecurity/tools/examples/sign-3.xml ./.
cp %WS_PATH%/xmlsecurity/tools/examples/sign-4.xml ./.
cp %WS_PATH%/xmlsecurity/tools/examples/sign-5.xml ./.
cd ..
cp %WS_PATH%/xmlsecurity/tools/examples/eval_import.txt ./.
cp %WS_PATH%/xmlsecurity/tools/examples/eval_export.txt ./.

testtool %WS_PATH%/xmlsecurity/tools/cryptoken/jks/testToken.jks %WS_PATH%/xmlsecurity/tools/cryptoken/nss %TEMP_DRIVE%/%TEMP_FOLDER%/eval_export.txt %TEMP_DRIVE%/%TEMP_FOLDER%/eval_import.txt 

@echo off
echo =============================================
echo step - 10 stop LibreOffice
echo =============================================
@echo on

@echo off
echo please stop the LibreOffice application
pause Press Enter when finished...
@echo on

@echo off
echo =============================================
echo step - 11 remove new component
echo =============================================
@echo on

rm %SO_DRIVE%/%SO_PATH%/user/uno_packages/xsec_sim.zip
%SO_DRIVE%
cd %SO_DRIVE%/%SO_PATH%/program

pkgchk -s -d xsec_sim.zip

%TEMP_DRIVE%

@echo off
echo =============================================
echo step - 12 remove the temporary folder
echo =============================================
@echo on

cd ..
rm -R %TEMP_FOLDER%
