#**************************************************************************
#
#     $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/extensions/source/activex/main/makefile.mk,v 1.3 2003-05-22 09:25:18 vg Exp $
#
# =========================================================================
#
#     $Date: 2003-05-22 09:25:18 $
#     $Author: vg $
#     $Revision: 1.3 $
#
# =========================================================================
#
#     Created: 1999/08/23
#     Creator: obr
#
#     Copyright (c) 1999 StarOffice Software Entwicklungs GmbH
#
#**************************************************************************

PRJ=..$/..$/..
PRJNAME=extensions
TARGET=so_activex

use_shl_versions=

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(GUI)" == "WNT"

VERSIONOBJ=
LIBTARGET=NO
USE_DEFFILE=YES

INCPRE+=$(ATL_INCLUDE) \
    -I$(MISC) \

# --- Files -------------------------------------


.IF "$(PRODUCT)"!=""
RC+=-DPRODUCT
.ENDIF

RCFILES=\
        $(TARGET).rc 
RCDEPN=$(MISC)$/envsettings.h

SLOFILES=\
    $(SLO)$/so_activex.obj \
    $(SLO)$/SOActiveX.obj \
    $(SLO)$/SOComWindowPeer.obj \
    $(SLO)$/SODispatchInterceptor.obj \
    $(SLO)$/StdAfx2.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    uuid.lib \
    advapi32.lib \
    ole32.lib \
    oleaut32.lib \
    gdi32.lib \
    urlmon.lib \
    Shlwapi.lib

.IF "$(COMEX)"=="8"
    SHL1STDLIBS+= $(COMPATH)$/atlmfc$/lib$/atls.lib
.ENDIF


#    kernel32.lib \
#    rpcndr.lib \
#    rpcns4.lib \
#    rpcrt4.lib

#kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib 

SHL1OBJS=$(SLOFILES)

SHL1LIBS=
SHL1DEF=$(TARGET).def
SHL1RES=$(RES)$/$(TARGET).res

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

$(MISC)$/envsettings.h : makefile.mk
    +-$(RM) $@
# it looks wrong; but rc likes it that way...
.IF "$(USE_SHELL)"!="4nt"
    +echo "#define MISC .\..\$(INPATH)\misc" > $@
.ELSE			# "$(USE_SHELL)"!="4nt"
    +echo #define MISC .\..\$(INPATH)\misc > $@
.ENDIF			# "$(USE_SHELL)"!="4nt"

