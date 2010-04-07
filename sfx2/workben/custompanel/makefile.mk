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

PRJ=../..
PRJNAME=sfx2

TARGET=custompanel
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO
EXTENSIONNAME:=custom-tool-panel

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

#-------------------------------------------------------------------

#---- extension version
EXTENSION_VERSION_BASE=0.1
.IF ( "$(CWS_WORK_STAMP)" == "" ) || ( "$(UPDATER)" == "YES" )
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE)
.ELSE
    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE).cws.$(CWS_WORK_STAMP)
.ENDIF

#---- extension title package name
EXTENSION_TITLE=Custom Tool Panel Example
EXTENSION_ZIPNAME=$(EXTENSIONNAME)-$(EXTENSION_VERSION_BASE)-$(RTL_OS:l)-$(RTL_ARCH:l)

#--------------------------------------------------

SHL1DLLPRE=
SHL1TARGET=$(TARGET).uno
LIB1TARGET=$(SLB)/$(SHL1TARGET).lib
LIB1OBJFILES= \
        $(SLO)/ctp_factory.obj \
        $(SLO)/ctp_services.obj \
        $(SLO)/ctp_panel.obj

SHL1STDLIBS= \
        $(CPPULIB)          \
        $(SALLIB)           \
        $(SALHELPERLIB)     \
        $(CPPUHELPERLIB)

SHL1VERSIONMAP=$(SOLARSRC)/solenv/src/component.map
SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)/$(SHL1TARGET).def
SHL1RPATH=  OXT
DEF1NAME=	$(SHL1TARGET)

# create Extension -----------------------------

COMPONENT_CONFIGDEST=.

COMPONENT_XCU = \
    $(EXTENSIONDIR)/WriterWindowState.xcu \
    $(EXTENSIONDIR)/Factories.xcu

COMPONENT_LIBRARIES = \
        $(EXTENSIONDIR)$/$(SHL1TARGET)$(DLLPOST)

# ........ dependencies for packaging the extension ........
EXTENSION_PACKDEPS=makefile.mk

# --- Targets ------------------------------------------------------
.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk

