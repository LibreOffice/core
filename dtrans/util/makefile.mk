#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: hr $ $Date: 2007-11-02 13:15:55 $
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

