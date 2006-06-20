#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: hr $ $Date: 2006-06-20 05:43:19 $
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

PRJNAME=embedserv
TARGET=emser
LIBTARGET=NO
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)" == "WNT"

SHL1TARGET= emser$(UPD)$(DLLPOSTFIX)

SHL1IMPLIB= emserimp
SHL1OBJS= \
        $(SLO)$/register.obj \
        $(SLO)$/servprov.obj \
        $(SLO)$/docholder.obj \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj \
        $(SLO)$/ed_ioleobject.obj \
        $(SLO)$/ed_iinplace.obj \
        $(SLO)$/iipaobj.obj \
        $(SLO)$/guid.obj \
        $(SLO)$/esdll.obj \
        $(SLO)$/intercept.obj \
        $(SLO)$/syswinwrapper.obj \
        $(SLO)$/tracker.obj

SHL1STDLIBS=\
        $(SALLIB)			\
        $(CPPULIB)			\
        $(CPPUHELPERLIB)	\
        ole32.lib			\
        gdi32.lib			\
        uuid.lib				\
        oleaut32.lib

.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
SHL1STDLIBS+=\
        advapi32.lib	\
        $(ATL_LIB)$/atls.lib
.ENDIF


SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


