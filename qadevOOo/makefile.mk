#***************************************************************************
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
#***************************************************************************


PRJNAME = OOoRunner
TARGET=qadevOOo
PRJ=.

# ------------------------------------------------
# NEVER REMOVE THIS!
# The OOoRunner.jar and OOoRunnerLight.jar should build with debug information all the time, also in the .pro builds.
# If you have any questions about this, ask the qadevOOo Maintainer.
debug=true
# ------------------------------------------------

.INCLUDE : ant.mk
TST:
    echo $(SOLAR_JAVA)

.IF "$(SOLAR_JAVA)"=="TRUE"	
.IF "$(ANT_HOME)"!="NO_ANT_HOME"
ANT_FLAGS+=-Dbuild.source=$(JAVA_SOURCE_VER)
.IF "$(L10N_framework)"==""
ALLTAR: ANTBUILD
.ENDIF
.ENDIF
.ENDIF 
