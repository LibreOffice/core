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
# $Revision: 1.15 $
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

PRJNAME=embeddedobj
TARGET=emboleobj


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(DISABLE_ATL)"==""

LIBTARGET=NO
USE_DEFFILE=YES
INCPRE+=$(ATL_INCLUDE)
.IF "$(MFC_INCLUDE)"!=""
INCPRE+=$(MFC_INCLUDE)
.ENDIF

# --- Files --------------------------------------------------------

SHL1TARGET= $(TARGET)

SHL1IMPLIB= i$(TARGET)

SLOFILES =  \
        $(SLO)$/closepreventer.obj\
        $(SLO)$/oleregister.obj\
        $(SLO)$/xolefactory.obj\
        $(SLO)$/olepersist.obj\
        $(SLO)$/oleembed.obj\
        $(SLO)$/olevisual.obj\
        $(SLO)$/olemisc.obj\
        $(SLO)$/ownview.obj

EXCEPTIONSFILES= \
        $(SLO)$/closepreventer.obj\
        $(SLO)$/oleregister.obj\
        $(SLO)$/xolefactory.obj\
        $(SLO)$/olepersist.obj\
        $(SLO)$/oleembed.obj\
        $(SLO)$/olevisual.obj\
        $(SLO)$/olemisc.obj\
        $(SLO)$/ownview.obj

.IF "$(GUI)"=="WNT"

SLOFILES += \
        $(SLO)$/graphconvert.obj\
        $(SLO)$/olecomponent.obj\
        $(SLO)$/olewrapclient.obj\
        $(SLO)$/xdialogcreator.obj\
        $(SLO)$/advisesink.obj

EXCEPTIONSFILES += \
        $(SLO)$/graphconvert.obj\
        $(SLO)$/olecomponent.obj\
        $(SLO)$/olewrapclient.obj\
        $(SLO)$/xdialogcreator.obj\
        $(SLO)$/advisesink.obj

.ENDIF

SHL1OBJS= $(SLOFILES)

SHL1STDLIBS=\
    $(SALLIB)\
    $(VOSLIB)\
    $(CPPULIB)\
    $(CPPUHELPERLIB)\
    $(COMPHELPERLIB)\
    $(TOOLSLIB)\
    $(SVTOOLLIB)

.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
EMBOBJLIB=-lembobj
.ELSE
EMBOBJLIB=iembobj.lib
.ENDIF

SHL1STDLIBS+=\
    $(VCLLIB)\
    $(EMBOBJLIB)\
    $(OLE32LIB)\
    $(GDI32LIB)\
    $(UUIDLIB)\
    $(OLEAUT32LIB)

DEF1EXPORTFILE=	exports.dxp
.ELIF "$(GUI)"=="OS2"

SHL1STDLIBS+=\
    $(VCLLIB)\
    iembobj.lib

DEF1EXPORTFILE=	exports.dxp

.ELSE
SHL1STDLIBS+=\
    -lembobj
.ENDIF


SHL1DEF= $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)

.ENDIF
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

