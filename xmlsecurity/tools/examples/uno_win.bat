set WS_PATH=e:/mybuild/SRC680
set SO_CLASSPATHPATH=d:/2000Soft/OOo1.1/program/classes

@echo off
echo =============================================
echo step - 1 start OpenOffice
echo =============================================
@echo on

@echo off
echo please start up OpenOffice with "-accept=socket,host=0,port=2002;urp;"
pause Press Enter when finished...
@echo on

@echo off
echo =============================================
echo step - 2 run the testtool class
echo =============================================
@echo on

cp %WS_PATH%/xmlsecurity/tools/uno/current.gif ./.
java -classpath %WS_PATH%/xmlsecurity/wntmsci8.pro/class;%SO_CLASSPATHPATH%/rt.jar;%SO_CLASSPATHPATH%/java_uno.jar;%SO_CLASSPATHPATH%/sandbox.jar;%SO_CLASSPATHPATH%/jurt.jar;%SO_CLASSPATHPATH%/unoil.jar;%SO_CLASSPATHPATH%/ridl.jar;%SO_CLASSPATHPATH%/juh.jar com.sun.star.xml.security.uno.TestTool %WS_PATH%/xmlsecurity/tools/cryptoken/jks/testToken.jks %WS_PATH%/xmlsecurity/tools/cryptoken/nss
rm ./current.gif

