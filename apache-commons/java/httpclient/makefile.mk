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
# $Revision: 1.2 $
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

PRJ=..$/..

PRJNAME=apache-commons
TARGET=commons-httpclient

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# override buildfile
ANT_BUILDFILE=build.xml

.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=commons-httpclient-3.1-src

TARFILE_ROOTDIR=commons-httpclient-3.1

#PATCH_FILE_NAME=patches$/httpclient.patch

#CONVERTFILES=build.xml
                
OUT2CLASS=dist$/commons-httpclient.jar

COMMONS_LOGGING_JAR=..$/..$/..$/..$/$(INPATH)$/class$/commons-logging-1.1.1-SNAPSHOT.jar
COMMONS_CODEC_JAR=..$/..$/..$/..$/$/$(INPATH)$/class$/commons-codec-1.3.jar

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -Dcommons-logging.jar=$(COMMONS_LOGGING_JAR) -Dcommons-codec.jar=$(COMMONS_CODEC_JAR) -f $(ANT_BUILDFILE) dist
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)"  -Dcommons-logging.jar=$(COMMONS_LOGGING_JAR) -Dcommons-codec.jar=$(COMMONS_CODEC_JAR) -f $(ANT_BUILDFILE) dist
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF

