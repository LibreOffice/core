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

PRJ=.

PRJNAME=ooo_rhino
TARGET=ooo_rhino

.IF "$(SOLAR_JAVA)"!=""
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  antsettings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=rhino1_5R5
TARFILE_MD5=798b2ffdc8bcfe7bca2cf92b62caf685
TARFILE_ROOTDIR=rhino1_5R5

ADDITIONAL_FILES= \
    toolsrc/org/mozilla/javascript/tools/debugger/OfficeScriptInfo.java

PATCH_FILES=rhino1_5R5.patch \
    rhino1_5R5-find_swing.patch \
    rhino1_5R5-updateToolTip.patch

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj jar
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" jar
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ELSE
all:
        @echo java disabled
.ENDIF
