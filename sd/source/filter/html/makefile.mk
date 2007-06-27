#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: hr $ $Date: 2007-06-27 15:39:39 $
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

PRJ=..$/..$/..
PRJNAME=sd
TARGET=html
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE;

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

INCPRE += ..$/..$/ui$/inc

# --- Files --------------------------------------------------------

SLOFILES =	$(SLO)$/HtmlOptionsDialog.obj\
            $(SLO)$/buttonset.obj \
            $(SLO)$/sdhtmlfilter.obj	\
            $(SLO)$/htmlex.obj			\
            $(SLO)$/htmlattr.obj		\
            $(SLO)$/pubdlg.obj			

SRS1NAME=$(TARGET)
SRC1FILES =	pubdlg.src					

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1OBJFILES = \
            $(SLO)$/HtmlOptionsDialog.obj\
            $(SLO)$/sdhtmlfilter.obj	\
            $(SLO)$/buttonset.obj \
            $(SLO)$/htmlex.obj

LIB2TARGET= $(SLB)$/$(TARGET)_ui.lib
LIB2OBJFILES = \
            $(SLO)$/htmlattr.obj \
            $(SLO)$/buttonset.obj \
            $(SLO)$/pubdlg.obj

# --- Targets --------------------------------------------------------------

.INCLUDE : target.mk
