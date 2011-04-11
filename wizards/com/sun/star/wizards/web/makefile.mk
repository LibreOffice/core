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

PRJ		= ..$/..$/..$/..$/..
PRJNAME = wizards
TARGET  = web
PACKAGE = com$/sun$/star$/wizards$/web

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

JARFILES= unoil.jar jurt.jar ridl.jar juh.jar jut.jar java_uno.jar java_uno_accessbridge commonwizards.jar 

.IF "$(SYSTEM_SAXON)" == "YES"
EXTRAJARFILES = $(SAXON_JAR)
.ELSE
JARFILES += saxon9.jar
.ENDIF

CUSTOMMANIFESTFILE= MANIFEST.MF

JARCLASSDIRS	= com$/sun$/star$/wizards$/web
JARTARGET		= web.jar
JARCLASSPATH = \
    commonwizards.jar saxon9.jar

# --- Files --------------------------------------------------------

JAVAFILES=			\
    AbstractErrorHandler.java \
    BackgroundsDialog.java \
    CallWizard.java \
    ErrorHandler.java \
    ExtensionVerifier.java \
    FTPDialog.java \
    FTPDialogResources.java \
    IconsDialog.java \
    ImageListDialog.java \
    LogTaskListener.java \
    Process.java \
    ProcessErrorHandler.java \
    ProcessErrors.java \
    ProcessStatusRenderer.java \
    StatusDialog.java \
    StylePreview.java \
    TOCPreview.java \
    WebWizard.java \
    WebWizardConst.java \
    WebWizardDialog.java \
    WebWizardDialogResources.java \
    WWD_Events.java \
    WWD_General.java \
    WWD_Startup.java \
    WWHID.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/web.component

$(MISC)/web.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        web.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_JAVA)$(JARTARGET)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt web.component
