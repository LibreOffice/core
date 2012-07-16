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
echo step - 1 start LibreOffice
echo =============================================
@echo on

@echo off
echo please start up LibreOffice with "--accept=socket,host=0,port=2002;urp;"
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

