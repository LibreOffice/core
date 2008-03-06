#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.33 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 11:39:07 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

