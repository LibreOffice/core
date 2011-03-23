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
TARGET=libfonts

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk
.INCLUDE : $(PRJ)$/version.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------
.IF "$(L10N_framework)"==""
TARFILE_NAME=$(TARGET)-$(LIBFONTS_VERSION)
TARFILE_IS_FLAT=true
TARFILE_MD5=3bdf40c0d199af31923e900d082ca2dd
PATCH_FILES=$(PACKAGE_DIR)$/$(TARGET).patch
CONVERTFILES=common_build.xml

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dlib="../../../class" -Dbuild.label="build-$(RSCREVISION)" -Dantcontrib.available="true" -Dbuild.id="10682" -Dproject.revision="$(LIBFONTS_VERSION)" -Dbuild.compiler=gcj -f $(ANT_BUILDFILE) jar
.ELSE
BUILD_ACTION=$(ANT) -Dlib="../../../class" -Dbuild.label="build-$(RSCREVISION)" -Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER) -f $(ANT_BUILDFILE) jar
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""
.ENDIF
# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.IF "$(L10N_framework)"==""
.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk

ALLTAR : $(CLASSDIR)$/$(TARGET)-$(LIBFONTS_VERSION).jar 

$(PACKAGE_DIR)$/$(TARGET).patch : 
    @-$(MKDIRHIER) $(PACKAGE_DIR)$(fake_root_dir)
    ( $(TYPE:s/+//) $(PRJ)$/patches$/common_build.patch | $(SED) 's/libloader-1.1.3/$(TARGET)-$(LIBFONTS_VERSION)/g' > $(PACKAGE_DIR)$/$(TARGET).patch )
    $(COMMAND_ECHO)$(TOUCH) $(PACKAGE_DIR)$/so_converted_$(TARGET).dummy
    
$(CLASSDIR)$/$(TARGET)-$(LIBFONTS_VERSION).jar : $(PACKAGE_DIR)$/$(INSTALL_FLAG_FILE)
    $(COPY) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/dist$/$(TARGET)-$(LIBFONTS_VERSION).jar $(CLASSDIR)$/$(TARGET)-$(LIBFONTS_VERSION).jar

.ENDIF
.ENDIF

