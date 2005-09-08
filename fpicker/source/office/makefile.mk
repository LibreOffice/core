#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 23:33:44 $
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

PRJNAME=fpicker
TARGET=fps_office
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
GEN_HID=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# --- Files --------------------------------------------------------

SLOFILES=\
    $(SLO)$/asyncfilepicker.obj \
    $(SLO)$/commonpicker.obj \
    $(SLO)$/OfficeControlAccess.obj \
    $(SLO)$/OfficeFilePicker.obj \
    $(SLO)$/OfficeFolderPicker.obj \
    $(SLO)$/fpinteraction.obj \
    $(SLO)$/fpsmartcontent.obj \
    $(SLO)$/fps_office.obj \
    $(SLO)$/iodlg.obj \
    $(SLO)$/iodlgimp.obj

SHL1TARGET=	$(TARGET).uno
SHL1IMPLIB=	i$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1STDLIBS=\
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(VCLLIB) \
    $(SVLLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)

SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)

SRS1NAME=	$(TARGET)
SRC1FILES=	\
    OfficeFilePicker.src \
    iodlg.src

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=\
    $(SRS)$/fps_office.srs

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
