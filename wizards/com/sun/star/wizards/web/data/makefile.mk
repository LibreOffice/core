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

PRJ		= ..$/..$/..$/..$/..$/..
PRJNAME = wizards
TARGET  = web_data
PACKAGE = com$/sun$/star$/wizards$/web$/data

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

JARFILES= unoil.jar jurt.jar ridl.jar juh.jar jut.jar java_uno.jar java_uno_accessbridge commonwizards.jar 

.IF "$(SYSTEM_SAXON)" == "YES"
EXTRAJARFILES = $(SAXON_JAR)
.ELSE
JARFILES += saxon9.jar
.ENDIF

# --- Files --------------------------------------------------------

JAVAFILES=			\
    CGArgument.java \
    CGContent.java \
    CGDesign.java \
    CGDocument.java \
    CGExporter.java \
    CGFilter.java \
    CGGeneralInfo.java \
    CGIconSet.java \
    CGImage.java \
    CGLayout.java \
    CGPublish.java \
    CGSession.java \
    CGSessionName.java \
    CGSettings.java \
    CGStyle.java \
    ConfigSetItem.java \
    TypeDetection.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
