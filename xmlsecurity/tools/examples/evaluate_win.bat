set SO_DRIVE=D:
set SO_PATH=2000Soft/OOo1.1
set WS_PATH=e:/mybuild/SRC680
set TEMP_DRIVE=C:
set TEMP_FOLDER=temp-folder

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
echo step - 8 start OpenOffice
echo =============================================
@echo on

@echo off
echo please start up OpenOffice with "-accept=socket,host=0,port=2002;urp;"
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
echo step - 10 stop OpenOffice
echo =============================================
@echo on

@echo off
echo please stop the OpenOffice application
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
