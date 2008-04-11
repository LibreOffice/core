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
# $Revision: 1.22 $
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

PRJ=..$/..$/..
PRJNAME=extensions
TARGET=so_activex

use_shl_versions=

# --- Settings ----------------------------------
.IF "$(GUI)" == "WNT" && "$(DISABLE_ACTIVEX)"==""
PROF_EDITION=TRUE
.ENDIF

.INCLUDE : settings.mk

.IF "$(GUI)" == "WNT" && "$(DISABLE_ACTIVEX)"==""

VERSIONOBJ=
LIBTARGET=NO
USE_DEFFILE=YES

INCPRE+=$(foreach,i,$(ATL_INCLUDE) -I$(i)) \
    -I$(MISC) \

# --- Files -------------------------------------


.IF "$(PRODUCT)"!=""
RC+=-DPRODUCT
.ENDIF
.IF "$(USE_STLP_DEBUG)"!=""
CDEFS+=-D_DEBUG
.ENDIF # "$(USE_STLP_DEBUG)"!=""

RCFILES=\
        $(TARGET).rc
RCDEPN=$(MISC)$/envsettings.h

SLOFILES=\
    $(SLO)$/so_activex.obj \
    $(SLO)$/SOActiveX.obj \
    $(SLO)$/SOComWindowPeer.obj \
    $(SLO)$/SODispatchInterceptor.obj \
    $(SLO)$/SOActionsApproval.obj \
    $(SLO)$/StdAfx2.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    $(UUIDLIB) \
    $(ADVAPI32LIB) \
    $(OLE32LIB) \
    $(OLEAUT32LIB) \
    $(GDI32LIB) \
    $(URLMONLIB) \
    $(SHLWAPILIB)

.IF "$(COM)"!="GCC"
.IF "$(CCNUMVER)" > "001300000000"
.IF "$(USE_STLP_DEBUG)" != ""
    SHL1STDLIBS+= $(ATL_LIB)$/atlsd.lib
.ELSE
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF
.ENDIF


#    $(KERNEL32LIB) \
#    rpcndr.lib \
#    rpcns4.lib \
#    rpcrt4.lib

#$(KERNEL32LIB) rpcndr.lib rpcns4.lib rpcrt4.lib

SHL1OBJS=$(SLOFILES)

SHL1LIBS=
SHL1DEF=$(TARGET).def
SHL1RES=$(RES)$/$(TARGET).res

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(MISC)$/envsettings.h : makefile.mk
    -$(RM) $@
# it looks wrong; but rc likes it that way...
.IF "$(USE_SHELL)"!="4nt"
    echo "#define MISC .\..\$(INPATH)\misc" > $@
.ELSE			# "$(USE_SHELL)"!="4nt"
    echo #define MISC .\..\$(INPATH)\misc > $@
.ENDIF			# "$(USE_SHELL)"!="4nt"
