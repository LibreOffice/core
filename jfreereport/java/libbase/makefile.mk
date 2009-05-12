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
# $Revision: 1.4 $
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

PRJNAME=jfreereport
TARGET=libbase
VERSION=-1.0.0
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=$(TARGET)
TARFILE_ROOTDIR=$(TARGET)

# PATCH_FILES=$(PRJ)$/patches$/libbase.patch
# CONVERTFILES=build.xml

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dlib="../../../class" -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -f $(ANT_BUILDFILE) jar
.ELSE
BUILD_ACTION=$(ANT) -Dlib="../../../class" -Dbuild.label="build-$(RSCREVISION)" -f $(ANT_BUILDFILE) jar
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk

ALLTAR : $(CLASSDIR)$/$(TARGET)$(VERSION).jar 

# XCLASSPATH/CLASSPATH does not work and we only can give lib once. But
# the build.xmls fortunately take *.jar out of lib so we can copy our
# commons-logging.jar here - yes, even in the system-apache commons case.
# Sucks.
$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(CLASSDIR)$/commons-logging.jar

$(CLASSDIR)$/commons-logging.jar : 
.IF "$(SYSTEM_APACHE_COMMONS)" != "YES"
    $(COPY) $(SOLARBINDIR)$/commons-logging-1.1.1.jar $(CLASSDIR)$/commons-logging.jar
.ELSE
    $(COPY) $(COMMONS_LOGGING_JAR) $(CLASSDIR)$/commons-logging.jar
.ENDIF

$(CLASSDIR)$/$(TARGET)$(VERSION).jar : $(CLASSDIR)$/commons-logging.jar $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
    $(COPY) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/build$/lib$/$(TARGET).jar $(CLASSDIR)$/$(TARGET)$(VERSION).jar
.ENDIF

