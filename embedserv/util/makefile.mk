#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:05:04 $
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

.IF "$(GUI)" == "WNT" && "$(DISABLE_ATL)"==""

SHL1TARGET= emser$(DLLPOSTFIX)

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
        $(OLE32LIB)			\
        $(GDI32LIB)			\
        $(UUIDLIB)				\
        $(OLEAUT32LIB)

.IF "$(CCNUMVER)" >= "001300000000" && "$(CCNUMVER)" <= "001399999999"
SHL1STDLIBS+=\
        $(ADVAPI32LIB)	\
        $(ATL_LIB)$/atls.lib
.ENDIF

.IF "$(WINDOWS_VISTA_PSDK)"!=""
SHL1STDLIBS+=\
        $(ADVAPI32LIB)	\
        $(ATL_LIB)$/atls.lib
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!=""


SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk


