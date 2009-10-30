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
# $Revision: 1.23 $
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
UWINAPILIB=

.IF "$(BUILD_X64)"!=""
USE_DEFFILE_X64=TRUE
.ENDIF

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

.IF "$(BUILD_X64)"!=""
# -------------------- x64 -----------------------
.IF "$(USE_STLP_DEBUG)"!=""
CDEFS_X64+=-D_DEBUG
.ENDIF # "$(USE_STLP_DEBUG)"!=""

CDEFS_X64+:=$(foreach,i,$(CDEFS) $(subst,-D_X86_=1,  $i))
LIBTARGET_X64=NO
USE_DEFFILE_X64=YES
UWINAPILIB_X64=

SLOFILES_X64= \
    $(SLO_X64)$/so_activex.obj \
    $(SLO_X64)$/SOActiveX.obj \
    $(SLO_X64)$/SOComWindowPeer.obj \
    $(SLO_X64)$/SODispatchInterceptor.obj \
    $(SLO_X64)$/SOActionsApproval.obj \
    $(SLO_X64)$/StdAfx2.obj

SHL1TARGET_X64=$(TARGET)

SHL1STDLIBS_X64+=\
    $(UUIDLIB_X64) \
    $(ADVAPI32LIB_X64) \
    $(OLE32LIB_X64) \
    $(OLEAUT32LIB_X64) \
    $(GDI32LIB_X64) \
    $(URLMONLIB_X64) \
    $(SHLWAPILIB_X64) \
    $(KERNEL32LIB_X64) \
    $(USER32LIB_X64) \
    $(MSVCRT_X64) \
    $(MSVCPRT_X64) \
    $(OLDNAMESLIB_X64)

SHL1OBJS_X64=$(SLOFILES_X64)
SHL1DEF_X64=$(TARGET).def

.IF "$(USE_STLP_DEBUG)" != ""
    SHL1STDLIBS_X64+= $(ATL_LIB)$/amd64$/atlsd.lib
.ELSE
    SHL1STDLIBS_X64+= $(ATL_LIB)$/amd64$/atls.lib
.ENDIF

.ENDIF # "$(BUILD_X64)"!=""

# --- Targets ----------------------------------

.INCLUDE : set_wntx64.mk
VERSIONOBJ_X64=
.INCLUDE : target.mk
.INCLUDE : tg_wntx64.mk

$(MISC)$/envsettings.h : makefile.mk
    -$(RM) $@
# it looks wrong; but rc likes it that way...
.IF "$(USE_SHELL)"!="4nt"
    echo "#define MISC .\..\$(INPATH)\misc" > $@
.ELSE			# "$(USE_SHELL)"!="4nt"
    echo #define MISC .\..\$(INPATH)\misc > $@
.ENDIF			# "$(USE_SHELL)"!="4nt"
