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
# $Revision: 1.16 $
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

PRJ=..

PRJNAME=dtrans
TARGET=dtrans
TARGET1=mcnttype
TARGET2=ftransl
TARGET3=sysdtrans
TARGET4=dnd

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk


# --- mcnttype dynlib ----------------------------------------------

SHL1TARGET=$(TARGET1)

SHL1LIBS=$(SLB)$/mcnttype.lib

SHL1STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) 

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(UWINAPILIB)
.ENDIF



SHL1IMPLIB=i$(SHL1TARGET) 

#--- comment -----------------

SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp
SHL1VERSIONMAP=exports.map


.IF "$(GUI)"=="WNT"

# --- ftransl dll ---

SHL2TARGET=$(TARGET2)

SHL2LIBS=$(SLB)$/ftransl.lib\
         $(SLB)$/dtutils.lib

SHL2STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)\
        $(UWINAPILIB)\
        $(ADVAPI32LIB)\
        $(OLE32LIB)\
        $(GDI32LIB)

SHL2IMPLIB=i$(SHL2TARGET) 

SHL2DEF=		$(MISC)$/$(SHL2TARGET).def
DEF2NAME=		$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp


# --- sysdtrans dll ---

SHL3TARGET=$(TARGET3)

SHL3LIBS=$(SLB)$/sysdtrans.lib\
         $(SLB)$/dtutils.lib\
         $(SLB)$/dtobjfact.lib\
         $(SLB)$/mtaolecb.lib

.IF "$(COM)" == "GCC"
SHL3OBJS=$(SLO)$/XNotifyingDataObject.obj
.ENDIF

SHL3STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB)\
        $(UWINAPILIB)\
        $(ADVAPI32LIB)\
        $(OLE32LIB)\
        $(OLEAUT32LIB)\
        $(GDI32LIB)\
        $(SHELL32LIB)\
        $(UUIDLIB)
        
SHL3IMPLIB=i$(SHL3TARGET) 

SHL3DEF=		$(MISC)$/$(SHL3TARGET).def
DEF3NAME=		$(SHL3TARGET)
DEF3EXPORTFILE=	exports.dxp


# --- dnd dll ---

SHL4TARGET=$(TARGET4)

SHL4LIBS=	\
            $(SLB)$/dnd.lib\
            $(SLB)$/dtobjfact.lib\
            $(SLB)$/dtutils.lib

SHL4STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) \
        $(UWINAPILIB)\
        $(ADVAPI32LIB)\
        $(OLE32LIB)\
        $(OLEAUT32LIB)\
        $(GDI32LIB)\
        $(SHELL32LIB)\
        $(UUIDLIB)

SHL4DEPN=
SHL4IMPLIB=i$(SHL4TARGET) 

SHL4DEF=		$(MISC)$/$(SHL4TARGET).def

DEF4NAME=		$(SHL4TARGET)
DEF4EXPORTFILE=	exports.dxp

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

# --- sysdtrans dll ---

SHL3TARGET=$(TARGET3)

SHL3LIBS=$(SLB)$/sysdtrans.lib

SHL3STDLIBS= \
        $(SALLIB)	\
        $(CPPULIB) 	\
        $(CPPUHELPERLIB) \
        UClip.lib
        
SHL3IMPLIB=i$(SHL3TARGET) 

SHL3DEF=		$(MISC)$/$(SHL3TARGET).def
DEF3NAME=		$(SHL3TARGET)
DEF3EXPORTFILE=	exports.dxp

.ENDIF			# "$(GUI)"=="OS2"

.INCLUDE :  target.mk

