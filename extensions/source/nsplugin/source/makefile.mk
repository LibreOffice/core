#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: hjs $ $Date: 2004-12-17 14:13:03 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..
PRJNAME=extensions
TARGET=npsoplugin

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GTK)"==""

dummy:
    @echo GTK disabled - nothing to build

.ELSE           # "$(ENABLE_GTK)"==""
.IF "$(OS)"=="LINUX"
INC+= -DNP_LINUX
.ENDIF
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
.ENDIF
.IF "$(GUI)"=="WNT"
INC+= -DENGLISH
.ENDIF
.ENDIF          # "$(ENABLE_GTK)"==""
# --- Files -------------------------------------

SLOFILES=       $(SLO)$/npshell.obj \
            $(SLO)$/so_env.obj

SHL1OBJS=	$(SLO)$/npshell.obj \
            $(SLO)$/so_env.obj

.IF "$(GUI)"=="UNX"
SHL1OBJS+=$(SLO)$/npunix.obj
SLOFILES+=$(SLO)$/npunix.obj
SHL1STDLIBS+= -ldl -lnsl
.ENDIF
.IF "$(GUI)"=="WNT"
SHL1OBJS+=$(SLO)$/npwin.obj
SLOFILES+=$(SLO)$/npwin.obj
SHL1STDLIBS+= shell32.lib
.ENDIF


# --- Plugin executable -------------------------
.IF "$(OS)"=="SOLARIS"
LINKFLAGSAPP!:=$(LINKFLAGSAPP:s/-z defs/-z nodefs/)
.ENDIF          # "$(OS)"=="SOLARIS"
APP1TARGET=nsplugin$(EXEPOSTFIX)
APP1OBJS=\
        $(SLO)$/so_instance.obj\
        $(SLO)$/so_env.obj\
        $(SLO)$/so_main.obj

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
APP1STDLIBS+=$(PKGCONFIG_LIBS:s/-lpangoxft-1.0//)
.IF "$(OS)"=="LINUX"
.ELSE
APP1STDLIBS+= -ldl -lnsl -lnls -lsocket
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
APP1OBJS+=$(SLO)$/nsp_windows.obj
APP1STDLIBS+= ws2_32.lib shell32.lib ole32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib oleaut32.lib uuid.lib
.ENDIF

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)
.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= ws2_32.lib shell32.lib ole32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
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
SHL2TARGET=$(TARGET)_oo
.IF "$(GUI)"=="WNT"
SHL2STDLIBS+= ws2_32.lib shell32.lib ole32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib ole32.lib oleaut32.lib uuid.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib
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

