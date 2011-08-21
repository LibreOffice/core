#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
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

PRJ=.

PRJNAME=languagetool
TARGET=languagetool

.IF "$(SOLAR_JAVA)"!=""
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  antsettings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=JLanguageTool-1.0.0
TARFILE_MD5=47e1edaa44269bc537ae8cabebb0f638
TARFILE_ROOTDIR=JLanguageTool-1.0.0
PATCH_FILES=JLanguageTool-1.0.0.patch

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -v -Dsolver.ooo.dir=$(SOLARVER)$/$(INPATH)$/bin -Dbuild.compiler=gcj dist
.ELSE
BUILD_ACTION=$(ANT) -v -Dsolver.ooo.dir=$(SOLARVER)$/$(INPATH)$/bin -Dant.build.javac.target=$(JAVA_TARGET_VER) dist
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ELSE
all:
        @echo java disabled
.ENDIF
