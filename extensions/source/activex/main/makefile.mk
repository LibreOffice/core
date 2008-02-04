#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.21 $
#
#   last change: $Author: ihi $ $Date: 2008-02-04 12:58:26 $
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
