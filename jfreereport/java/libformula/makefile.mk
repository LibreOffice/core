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
# $Revision: 1.6 $
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
TARGET=libformula
VERSION=-0.1.14

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# override buildfile
ANT_BUILDFILE=build.xml

.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=$(TARGET)

TARFILE_ROOTDIR=$(TARGET)

PATCH_FILE_NAME=$(PRJ)$/patches$/$(TARGET).patch

CONVERTFILES=build.xml\
                build.properties \
                source/org/jfree/formula/function/text/MidFunctionDescription.java \
                source/org/jfree/formula/function/AbstractFunctionDescription.java \
                source/org/jfree/formula/function/datetime/Hour-Function.properties \
                source/org/jfree/formula/function/information/IsBlank-Function.properties \
                source/org/jfree/formula/function/information/IsErr-Function.properties \
                source/org/jfree/formula/function/information/IsError-Function.properties \
                source/org/jfree/formula/function/information/IsEven-Function.properties \
                source/org/jfree/formula/function/information/IsLogical-Function.properties \
                source/org/jfree/formula/function/information/IsNa-Function.properties \
                source/org/jfree/formula/function/information/IsNonText-Function.properties \
                source/org/jfree/formula/function/information/IsNumber-Function.properties \
                source/org/jfree/formula/function/information/IsOdd-Function.properties \
                source/org/jfree/formula/function/information/IsText-Function.properties \
                source/org/jfree/formula/function/logical/If-Function.properties \
                source/org/jfree/formula/function/logical/Not-Function.properties \
                source/org/jfree/formula/function/logical/Or-Function.properties \
                source/org/jfree/formula/function/math/Even-Function.properties \
                source/org/jfree/formula/function/math/ModFunctionDescription.java \
                source/org/jfree/formula/function/text/Trim-Function.properties \
                source/org/jfree/formula/parser/FormulaParser.java

ADDITIONAL_FILES=source/org/jfree/formula/function/information/IsRef-Function.properties
OUT2CLASS=$(TARGET)$(VERSION).jar

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dlibdir="../../../class" -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -f $(ANT_BUILDFILE) compile
.ELSE
BUILD_ACTION=$(ANT) -Dlibdir="../../../class" -Dbuild.label="build-$(RSCREVISION)" -f $(ANT_BUILDFILE) compile
.ENDIF


.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF
# $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/source$/org$/jfree$/formula$/function$/information$/IsRef-Function.properties : 
#	@@-$(MKDIRHIER) $(@:d)
#    $(MV) $(PACKAGE_DIR)$/$(TARFILE_ROOTDIR)$/source$/org$/jfree$/formula$/function$/information$/isRef-Function.properties $@	

