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

PRJNAME=tomacat
TARGET=servlet

.IF "$(SOLAR_JAVA)" != ""
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=jakarta-tomcat-5.0.30-src
TARFILE_MD5=2a177023f9ea8ec8bd00837605c5df1b

TARFILE_ROOTDIR=jakarta-tomcat-5.0.30-src

ADDITIONAL_FILES = jakarta-servletapi-5/build.xml

PATCH_FILES=tomcat.patch

BUILD_DIR=jakarta-servletapi-5

.IF "$(JAVACISGCJ)"=="yes"
BUILD_ACTION=$(ANT)
.ELSE
BUILD_ACTION=$(ANT) -Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER)
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.INCLUDE : tg_ext.mk

.ELSE
@all:
    @echo java disabled
.ENDIF
