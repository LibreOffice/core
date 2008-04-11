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


