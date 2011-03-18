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
