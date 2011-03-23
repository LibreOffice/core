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

PRJ=..$/..

PRJNAME=jfreereport
TARGET=sac-1.3

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(L10N_framework)"==""

# override buildfile
ANT_BUILDFILE=build.xml

.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=sacjava-1.3
TARFILE_MD5=39bb3fcea1514f1369fcfc87542390fd

TARFILE_ROOTDIR=$(TARGET)

PATCH_FILES=$(PRJ)$/patches$/sac.patch

ADDITIONAL_FILES=build.xml

# CONVERTFILES=build.xml

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dantcontrib.available="true" -Dproject.revision="$(VERSION)" -Dbuild.compiler=gcj -f $(ANT_BUILDFILE) all
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER) -f $(ANT_BUILDFILE) all
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""
.ENDIF # L10N_framework
# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(L10N_framework)"==""
.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk

ALLTAR : $(CLASSDIR)$/sac.jar

$(CLASSDIR)$/sac.jar : $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
    $(COPY) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/build$/lib$/sac.jar $(CLASSDIR)$/sac.jar
.ENDIF
.ENDIF # L10N_framework

