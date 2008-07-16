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
# $Revision: 1.3 $
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

.IF "$(USE_JDK_VERSION)" == "140"
JDK_VERSION=140
JAVA_HOME=$(JDK14PATH)

XCLASSPATH:=$(JDK14PATH)$/jre/lib/rt.jar
CLASSPATH:=$(XCLASSPATH)
.ENDIF

ANT_BUILDFILE*=../../../../build.xml

# --- Settings -----------------------------------------------------

#.INCLUDE :	ant.mk
.INCLUDE :	settings.mk
.INCLUDE :	antsettings.mk
# --- Files --------------------------------------------------------

CLASSPATH!:=$(CLASSPATH)$(PATH_SEPERATOR)$(ANT_CLASSPATH)$(PATH_SEPERATOR)$(JAVA_HOME)$/lib$/tools.jar
.EXPORT : CLASSPATH

LUCENE_NAME=lucene-2.3.2

TARFILE_NAME=$(LUCENE_NAME)
PATCH_FILE_NAME=lucene.patch

ANT_FLAGS += -Dbasedir=../../../../$(INPATH)/misc/build/lucene-2.3.2

BUILD_DIR=.
BUILD_ACTION= ${ANT} ${ANT_FLAGS}

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk 

