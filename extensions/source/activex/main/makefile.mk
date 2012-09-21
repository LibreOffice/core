#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.IF "$(HAVE_ATLTHUNK)" == "YES"
    SHL1STDLIBS+= $(ATL_LIB)$/atlthunk.lib
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

SHL1STDLIBS_X64+= $(ATL_LIB)$/amd64$/atls.lib

.ENDIF # "$(BUILD_X64)"!=""

# --- Targets ----------------------------------

.INCLUDE : set_wntx64.mk
VERSIONOBJ_X64=
.INCLUDE : target.mk
.INCLUDE : tg_wntx64.mk

$(MISC)$/envsettings.h : makefile.mk
    -$(RM) $@
# it looks wrong; but rc likes it that way...
    echo "#define MISC .\..\$(INPATH)\misc" > $@
