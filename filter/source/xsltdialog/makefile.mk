#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:33:59 $
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
PRJNAME=filter
TARGET=xsltdlg

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
#GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files -------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	xmlfiltersettingsdialog.src							\
            xmlfiltertabdialog.src								\
            xmlfiltertabpagebasic.src							\
            xmlfiltertabpagexslt.src							\
            xmlfiltertestdialog.src								\
            xmlfileview.src										\
            xmlfilterdialogstrings.src

SLOFILES=	$(SLO)$/typedetectionimport.obj						\
            $(SLO)$/attributelist.obj							\
            $(SLO)$/typedetectionexport.obj						\
            $(SLO)$/xmlfilterjar.obj							\
            $(SLO)$/xmlfilterdialogcomponent.obj				\
            $(SLO)$/xmlfiltersettingsdialog.obj					\
            $(SLO)$/xmlfiltertabdialog.obj						\
            $(SLO)$/xmlfiltertabpagebasic.obj					\
            $(SLO)$/xmlfiltertabpagexslt.obj					\
            $(SLO)$/xmlfiltertestdialog.obj						\
            $(SLO)$/xmlfileview.obj

# --- Library -----------------------------------

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES= $(SRS)$/$(TARGET).srs

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
    
SHL1STDLIBS=\
    $(SFX2LIB)			\
    $(GOODIESLIB)		\
    $(SVTOOLLIB)		\
    $(SVLLIB)			\
    $(TKLIB)			\
    $(CPPULIB)			\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(VOSLIB)			\
    $(SALLIB)			\
    $(TOOLSLIB)			\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(ONELIB)

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk
