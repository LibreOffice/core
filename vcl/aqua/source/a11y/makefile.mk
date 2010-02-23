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

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=sala11y
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="aqua"

SLOFILES=	\
        $(SLO)$/aqua11ywrapper.obj \
        $(SLO)$/aqua11yfactory.obj \
        $(SLO)$/aqua11yfocuslistener.obj \
        $(SLO)$/aqua11yfocustracker.obj \
        $(SLO)$/aqua11ylistener.obj \
        $(SLO)$/aqua11yrolehelper.obj \
        $(SLO)$/aqua11yactionwrapper.obj \
        $(SLO)$/aqua11ycomponentwrapper.obj \
        $(SLO)$/aqua11yselectionwrapper.obj \
        $(SLO)$/aqua11ytablewrapper.obj \
        $(SLO)$/aqua11ytextattributeswrapper.obj \
        $(SLO)$/aqua11ytextwrapper.obj \
        $(SLO)$/aqua11yutil.obj \
        $(SLO)$/aqua11yvaluewrapper.obj \
        $(SLO)$/aqua11ywrapperbutton.obj \
        $(SLO)$/aqua11ywrappercheckbox.obj \
        $(SLO)$/aqua11ywrappercombobox.obj \
        $(SLO)$/aqua11ywrappergroup.obj \
        $(SLO)$/aqua11ywrapperlist.obj \
        $(SLO)$/aqua11ywrapperradiobutton.obj \
        $(SLO)$/aqua11ywrapperradiogroup.obj \
        $(SLO)$/aqua11ywrapperrow.obj \
        $(SLO)$/aqua11ywrapperscrollarea.obj \
        $(SLO)$/aqua11ywrapperscrollbar.obj \
        $(SLO)$/aqua11ywrappersplitter.obj \
        $(SLO)$/aqua11ywrapperstatictext.obj \
        $(SLO)$/aqua11ywrappertabgroup.obj \
        $(SLO)$/aqua11ywrappertextarea.obj \
        $(SLO)$/aqua11ywrappertoolbar.obj \
        $(SLO)$/documentfocuslistener.obj

.ENDIF		# "$(GUIBASE)"!="aqua"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk
