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
# $Revision: 1.34 $
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

PRJNAME=toolkit
TARGET=awt

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"=="aqua"
OBJCXXFLAGS=-x objective-c++ -fobjc-exceptions
CFLAGSCXX+=$(OBJCXXFLAGS)
.ENDIF  # "$(GUIBASE)"=="aqua"

SLOFILES=   \
            $(SLO)$/vclxaccessiblecomponent.obj         \
            $(SLO)$/vclxbitmap.obj 						\
            $(SLO)$/vclxcontainer.obj 					\
            $(SLO)$/vclxdevice.obj 						\
            $(SLO)$/vclxfont.obj 						\
            $(SLO)$/vclxgraphics.obj 					\
            $(SLO)$/vclxmenu.obj 						\
            $(SLO)$/vclxpointer.obj 					\
            $(SLO)$/vclxprinter.obj 					\
            $(SLO)$/vclxregion.obj 						\
            $(SLO)$/vclxsystemdependentwindow.obj		\
            $(SLO)$/vclxtoolkit.obj 					\
            $(SLO)$/vclxtopwindow.obj 					\
            $(SLO)$/vclxwindow.obj 						\
            $(SLO)$/vclxwindow1.obj 					\
            $(SLO)$/vclxwindows.obj                     \
            $(SLO)$/vclxspinbutton.obj                  \
            $(SLO)$/xsimpleanimation.obj                \
            $(SLO)$/xthrobber.obj						\
            $(SLO)$/asynccallback.obj

SRS1NAME=$(TARGET)
SRC1FILES=\
            xthrobber.src

.IF "$(ENABLE_LAYOUT)" == "TRUE"

SECOND_BUILD=layout
layout_SLOFILES=\
    $(SLO)/vclxbutton.obj\
    $(SLO)/vclxdialog.obj\
    $(SLO)/vclxfixedline.obj\
    $(SLO)/vclxproxy.obj\
    $(SLO)/vclxscroller.obj\
    $(SLO)/vclxsplitter.obj\
    $(SLO)/vclxtabcontrol.obj

.ENDIF # ENABLE_LAYOUT == TRUE

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

