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

PRJNAME=so_lucene
TARGET=so_lucene


# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :	antsettings.mk
# --- Files --------------------------------------------------------

.IF "$(SOLAR_JAVA)" != ""

LUCENE_MAJOR=2
LUCENE_MINOR=3
LUCENE_MICRO=2

LUCENE_NAME=lucene-$(LUCENE_MAJOR).$(LUCENE_MINOR).$(LUCENE_MICRO)
# NOTE that the jar names do not contain the micro version
LUCENE_CORE_JAR=lucene-core-$(LUCENE_MAJOR).$(LUCENE_MINOR).jar
LUCENE_ANALYZERS_JAR=lucene-analyzers-$(LUCENE_MAJOR).$(LUCENE_MINOR).jar

TARFILE_NAME=$(LUCENE_NAME)
TARFILE_MD5=48d8169acc35f97e05d8dcdfd45be7f2
PATCH_FILES=lucene.patch 

.IF "$(OS)" == "WNT"
PATCH_FILES+= long_path.patch
.ENDIF


BUILD_DIR=.
BUILD_ACTION= ${ANT} -buildfile .$/contrib$/analyzers$/build.xml

OUT2BIN=.$/build$/$(LUCENE_CORE_JAR) .$/build$/contrib$/analyzers$/$(LUCENE_ANALYZERS_JAR)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk 
.ENDIF
