#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.12 $
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
PRJNAME=scriptinginstaller
TARGET=scriptinginstaller
no_common_build_zip=true

.INCLUDE : settings.mk

.IF "$(SOLAR_JAVA)"!=""
ZIP1FLAGS=-r
ZIP1TARGET=ScriptsJava
ZIP1DIR=$(OUT)$/class$/examples
ZIP1LIST=java
.ENDIF

ZIP2FLAGS=-r
ZIP2TARGET=ScriptsBeanShell
ZIP2LIST=beanshell/HelloWorld beanshell/Highlight beanshell/WordCount beanshell/Capitalise beanshell/MemoryUsage 

ZIP3FLAGS=-r
ZIP3TARGET=ScriptsJavaScript
ZIP3LIST=javascript

ZIP4FLAGS=-r
ZIP4TARGET=ScriptsPython
ZIP4LIST=python/HelloWorld.py python/Capitalise.py python/pythonSamples

ZIP5FLAGS=-r
ZIP5TARGET=scriptbindinglib
ZIP5DIR=$(PRJ)$/workben$/bindings
ZIP5LIST=*.xlb Highlight.xdl

.INCLUDE : target.mk
