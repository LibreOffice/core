rem *************************************************************
rem  
rem  Licensed to the Apache Software Foundation (ASF) under one
rem  or more contributor license agreements.  See the NOTICE file
rem  distributed with this work for additional information
rem  regarding copyright ownership.  The ASF licenses this file
rem  to you under the Apache License, Version 2.0 (the
rem  "License"); you may not use this file except in compliance
rem  with the License.  You may obtain a copy of the License at
rem  
rem    http://www.apache.org/licenses/LICENSE-2.0
rem  
rem  Unless required by applicable law or agreed to in writing,
rem  software distributed under the License is distributed on an
rem  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
rem  KIND, either express or implied.  See the License for the
rem  specific language governing permissions and limitations
rem  under the License.
rem  
rem *************************************************************
call env_win.bat

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
java -classpath %WS_PATH%/xmlsecurity/%OUTPUTDIR%/class;%SO_CLASSPATHPATH%/rt.jar;%SO_CLASSPATHPATH%/java_uno.jar;%SO_CLASSPATHPATH%/jurt.jar;%SO_CLASSPATHPATH%/unoil.jar;%SO_CLASSPATHPATH%/ridl.jar;%SO_CLASSPATHPATH%/juh.jar com.sun.star.xml.security.uno.TestTool %WS_PATH%/xmlsecurity/tools/cryptoken/jks/testToken.jks ""
rm ./current.gif

