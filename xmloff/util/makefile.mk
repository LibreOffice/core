#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 13:58:44 $
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

PRJNAME=xmloff
TARGET=xo
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/xo.lib
LIB1FILES=	\
    $(SLB)$/core.lib \
    $(SLB)$/meta.lib \
    $(SLB)$/script.lib \
    $(SLB)$/style.lib \
    $(SLB)$/text.lib \
    $(SLB)$/draw.lib \
    $(SLB)$/chart.lib \
    $(SLB)$/forms.lib \
    $(SLB)$/xforms.lib \
    $(SLB)$/table.lib

# --- Shared-Library -----------------------------------------------

.IF "$(GUI)"!="UNX"
LIB4TARGET= $(LB)$/ixo.lib
LIB4FILES=	$(LB)$/_ixo.lib
.IF "$(GUI)"!="OS2"
LIB4OBJFILES=\
    $(OBJ)$/xmlkywd.obj
.ENDIF
.ENDIF

SHL1TARGET= xo$(DLLPOSTFIX)
SHL1IMPLIB= _ixo
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
        $(SVTOOLLIB)	\
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB) \
        $(VOSLIB)		\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(COMPHELPERLIB)\
        $(VCLLIB)		\
        $(TKLIB)		\
        $(SVLLIB)		\
        $(SALLIB)		\
        $(UNOTOOLSLIB)	\
        $(BASEGFXLIB)	\
        $(SALHELPERLIB)

# SCO: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "(OS)"=="SCO"
SHL1STDLIBS+=-licg617mxp
.ENDIF

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(LIB1TARGET)


SHL5STDLIBS= \
        $(TOOLSLIB)         \
        $(SALLIB)           \
        $(SALHELPERLIB)     \
        $(CPPULIB)          \
        $(CPPUHELPERLIB)    \
        $(COMPHELPERLIB)

.IF "$(GUI)"=="UNX" || "$(COM)"=="GCC"
    SHL5STDLIBS += -lxo$(DLLPOSTFIX)
.ELSE
    SHL5STDLIBS += ixo.lib
.ENDIF

# --- Def-File ---------------------------------------------------------

DEF1NAME    =$(SHL1TARGET)
DEFLIB1NAME =xo
DEF1DES     =XML Office Lib

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk
