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
TARGET=npsoplugin

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

INCPRE=$(SOLARINCDIR)$/npsdk

.IF "$(GUI)"=="UNX"

# not sure about -DMOZ_X11 but otheriwse some struct member don't exist...
CFLAGS+=-DMOZ_X11

.IF "$(ENABLE_GTK)"==""

dummy:
    @echo GTK disabled - nothing to build

.ELSE           # "$(ENABLE_GTK)"==""
.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD"
INC+= -DNP_LINUX
.ENDIF
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
.ENDIF          # "$(ENABLE_GTK)"==""
.ENDIF          # "$(GUI)"=="UNX"
.IF "$(GUI)"=="WNT" 
INC+= -DENGLISH
.ENDIF


# --- Files -------------------------------------

SLOFILES=       $(SLO)$/npshell.obj \
            $(SLO)$/so_env.obj

SHL1OBJS=	$(SLO)$/npshell.obj \
            $(SLO)$/so_env.obj

.IF "$(GUI)"=="UNX"
STDLIBS+=$(SOLARLIBDIR)$/npunix.o
SHL1STDLIBS+=$(SOLARLIBDIR)$/npunix.o
#.IF "$(OS)"!="FREEBSD"
#SHL1STDLIBS+= -ldl -lnsl
#.ENDIF #FREEBSD
.ENDIF #UNX
.IF "$(GUI)"=="WNT"
SHL1OBJS+=$(SOLARLIBDIR)$/pathutils-obj.obj
STDLIBS+=$(SOLARLIBDIR)$/npwin.obj
SHL1STDLIBS+= $(SHELL32LIB) $(SOLARLIBDIR)$/npwin.obj

.ENDIF

OBJFILES=\
        $(OBJ)$/so_closelistener.obj\
        $(OBJ)$/so_instance.obj\
        $(OBJ)$/so_env.obj\
        $(OBJ)$/so_main.obj

# --- Plugin executable -------------------------
APP1TARGET=nsplugin$(EXEPOSTFIX)
APP1OBJS=$(OBJFILES)

APP1STDLIBS=\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(SALLIB)

.IF "$(OS)"=="SOLARIS"
APP1STDLIBS+= -lsocket -lnsl
.ENDIF

.IF "$(GUI)"=="WNT"
DEPOBJFILES=$(OBJ)$/nsp_windows.obj
APP1OBJS+=$(OBJ)$/nsp_windows.obj $(SOLARLIBDIR)$/pathutils-obj.obj
APP1STDLIBS+= $(WS2_32LIB) $(SHELL32LIB) $(OLE32LIB) $(KERNEL32LIB) $(USER32LIB) $(GDI32LIB) $(WINSPOOLLIB) $(COMDLG32LIB) $(ADVAPI32LIB) $(OLEAAUT32LIB) $(UUIDLIB)
.ENDIF

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)_so
.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(WS2_32LIB) $(SHELL32LIB) $(OLE32LIB) $(KERNEL32LIB) $(USER32LIB) $(GDI32LIB) $(WINSPOOLLIB) $(COMDLG32LIB) $(ADVAPI32LIB) $(OLE32LIB) $(OLEAAUT32LIB) $(UUIDLIB) $(KERNEL32LIB) $(USER32LIB) $(GDI32LIB) $(WINSPOOLLIB) $(COMDLG32LIB) $(ADVAPI32LIB)
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=BRAND

# ----- get some additional keys in versioninfo ------------
SHL1ADD_VERINFO=nsplugin.rc
SHL1DEPN+=$(SHL1ADD_VERINFO)

DEF1NAME=$(SHL1TARGET)
.IF "$(GUI)"=="UNX"
DEF1EXPORTFILE=exports.dxp
.ENDIF
.IF "$(GUI)"=="WNT"
DEF1EXPORTFILE=exports_wnt.dxp
.ENDIF

SHL2OBJS=$(SHL1OBJS)
SHL2STDLIBS=$(SHL1STDLIBS)
SHL2TARGET=$(TARGET)
.IF "$(GUI)"=="WNT"
SHL2STDLIBS+= $(WS2_32LIB) $(SHELL32LIB) $(OLE32LIB) $(KERNEL32LIB) $(USER32LIB) $(GDI32LIB) $(WINSPOOLLIB) $(COMDLG32LIB) $(ADVAPI32LIB) $(OLE32LIB) $(OLEAAUT32LIB) $(UUIDLIB) $(KERNEL32LIB) $(USER32LIB) $(GDI32LIB) $(WINSPOOLLIB) $(COMDLG32LIB) $(ADVAPI32LIB)
.ENDIF

SHL2DEPN=
SHL2IMPLIB=	i$(SHL2TARGET)
SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
SHL2RPATH=BRAND

# ----- get some additional keys in versioninfo ------------
SHL2ADD_VERINFO=nsplugin_oo.rc
SHL2DEPN+=$(SHL2ADD_VERINFO)

DEF2NAME=$(SHL2TARGET)
.IF "$(GUI)"=="UNX"
DEF2EXPORTFILE=exports.dxp
.ENDIF
.IF "$(GUI)"=="WNT"
DEF2EXPORTFILE=exports_wnt.dxp
.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

