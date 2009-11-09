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
# $Revision: 1.7 $
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

PRJ	= ..$/..$/..$/..$/..
PRJNAME = beans
TARGET  = com_sun_star_comp_beans
PACKAGE = com$/sun$/star$/comp$/beans

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""
.IF "$(OS)"=="MACOSX"

dummy:
    @echo "Nothing to build for OS $(OS)"

.ELSE		# "$(OS)"=="MACOSX"

JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar

# --- Sources --------------------------------------------------------

JAVAFILES=  \
    ContainerFactory.java \
    Controller.java \
    Frame.java \
    HasConnectionException.java \
    InvalidArgumentException.java \
    JavaWindowPeerFake.java \
    LocalOfficeConnection.java \
    LocalOfficeWindow.java \
    NativeConnection.java \
    NativeService.java \
    NoConnectionException.java \
    NoDocumentException.java \
    OfficeConnection.java \
    OfficeDocument.java \
    OfficeWindow.java \
    OOoBean.java \
    SystemWindowException.java \
    Wrapper.java \
    CallWatchThread.java

JAVACLASSFILES= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

.ENDIF

# --- Targets ------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :  target.mk
