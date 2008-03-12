#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 07:36:33 $
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

PRJNAME=fpicker
TARGET=fpicker
TARGET1=fps
TARGET2=fop
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- fps dynlib ----------------------------------------------

COMMON_LIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(COMPHELPERLIB)\
            $(SALLIB)\
            $(VCLLIB)\
            $(TOOLSLIB)

.IF "$(GUI)"=="WNT"

SHL1TARGET=$(TARGET1)
SHL1STDLIBS=		$(COMMON_LIBS) \
            $(UWINAPILIB) \
            $(ADVAPI32LIB) \
            $(SHELL32LIB)\
            $(OLE32LIB)\
            $(GDI32LIB)\
            $(OLEAUT32LIB)\
            $(COMDLG32LIB)\
            $(KERNEL32LIB)\
            $(UUIDLIB)

SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/fps.lib\
         $(SLB)$/utils.lib
SHL1RES=$(RES)$/$(TARGET1).res
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF # "$(GUI)"=="WNT"

# --- fop dynlib --------------------------------------------------

.IF "$(GUI)"=="WNT"
SHL2NOCHECK=TRUE
SHL2TARGET=$(TARGET2)

SHL2STDLIBS=		$(COMMON_LIBS) \
            $(UWINAPILIB) \
            $(ADVAPI32LIB) \
            $(OLE32LIB)\
            $(GDI32LIB)\
            $(SHELL32LIB)\
            $(OLEAUT32LIB)

SHL2DEPN=
SHL2IMPLIB=i$(SHL2TARGET)
SHL2LIBS=$(SLB)$/fop.lib\
         $(SLB)$/utils.lib
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

.ENDIF          # "$(GUI)"=="WNT"


.INCLUDE :  target.mk

