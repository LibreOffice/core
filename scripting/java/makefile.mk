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



PRJ=..
PRJNAME=scripting
TARGET=scriptruntimeforjava

.INCLUDE : ant.mk

.IF "$(L10N_framework)"==""
.IF "$(SOLAR_JAVA)"!=""
ALLTAR : ANTBUILD
.ENDIF
.ENDIF

ALLTAR : \
    $(MISC)/ScriptFramework.component \
    $(MISC)/ScriptProviderForBeanShell.component \
    $(MISC)/ScriptProviderForJava.component \
    $(MISC)/ScriptProviderForJavaScript.component

$(MISC)/ScriptFramework.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt ScriptFramework.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)ScriptFramework.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ScriptFramework.component

$(MISC)/ScriptProviderForBeanShell.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt \
        ScriptProviderForBeanShell.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)ScriptProviderForBeanShell.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt \
        ScriptProviderForBeanShell.component

$(MISC)/ScriptProviderForJava.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt ScriptProviderForJava.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)ScriptProviderForJava.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ScriptProviderForJava.component

$(MISC)/ScriptProviderForJavaScript.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt \
        ScriptProviderForJavaScript.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)ScriptProviderForJavaScript.jar' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt \
        ScriptProviderForJavaScript.component
