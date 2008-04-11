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
# $Revision: 1.19 $
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
STDLIBS+=$(SOLARLIBDIR)$/npwin.obj
SHL1STDLIBS+= $(SHELL32LIB) $(SOLARLIBDIR)$/npwin.obj

.ENDIF

OBJFILES=\
        $(OBJ)$/so_closelistener.obj\
        $(OBJ)$/so_instance.obj\
        $(OBJ)$/so_env.obj\
        $(OBJ)$/so_main.obj

# --- Plugin executable -------------------------
.IF "$(OS)"=="SOLARIS"
LINKFLAGSAPP!:=$(LINKFLAGSAPP:s/-z defs/-z nodefs/)
.ENDIF          # "$(OS)"=="SOLARIS"
APP1TARGET=nsplugin$(EXEPOSTFIX)
APP1OBJS=$(OBJFILES)

APP1STDLIBS=\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UNOTOOLSLIB)		\
            $(COMPHELPERLIB)	\
            $(VOSLIB)			\
            $(SALLIB)			\
            $(VCLLIB)			\
            $(SVLLIB)			\
            $(TOOLSLIB)

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="LINUX" || "$(OS)"=="FREEBSD" || "$(OS)"=="MACOSX"
.ELSE
APP1STDLIBS+= -ldl -lnsl -lnls -lsocket
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
DEPOBJFILES=$(OBJ)$/nsp_windows.obj
APP1OBJS+=$(OBJ)$/nsp_windows.obj
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

