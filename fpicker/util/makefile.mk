#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hr $ $Date: 2004-12-17 11:51:20 $
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

PRJ=..

PRJNAME=fpicker
TARGET=fpicker
TARGET1=fps
TARGET2=fop
USE_LDUMP2=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_GTK)" == "TRUE"
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
.ENDIF         # "$(ENABLE_GTK)" == "TRUE"

# --- fps dynlib ----------------------------------------------

.IF "$(GUI)"=="WNT" || "$(GUIBASE)" == "unx"

COMMON_LIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(SALLIB)\
            $(VCLLIB)\
            $(TOOLSLIB)

.IF "$(GUI)"=="WNT"
SHL1TARGET=$(TARGET1)
SHL1STDLIBS=		$(COMMON_LIBS) \
            uwinapi.lib \
            advapi32.lib \
            shell32.lib\
            ole32.lib\
            gdi32.lib\
            oleaut32.lib\
            comdlg32.lib\
            kernel32.lib\
            oleaut32.lib
SHL2STDLIBS=		$(COMMON_LIBS) \
            uwinapi.lib \
            advapi32.lib \
            ole32.lib\
            gdi32.lib\
            shell32.lib\
            comsupp.lib\
            oleaut32.lib

SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(SLB)$/fps.lib\
         $(SLB)$/utils.lib
SHL1RES=$(RES)$/$(TARGET1).res
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
.ELSE
.IF "$(ENABLE_GTK)" == "TRUE"
SHL1NOCHECK=TRUE
SHL1TARGET=fps_gnome
SHL1LIBS=$(SLB)$/fps_gnome.lib
SHL1STDLIBS= $(COMMON_LIBS) $(PKGCONFIG_LIBS)

.IF "$(OS)"=="SOLARIS"
LINKFLAGS!:=$(LINKFLAGSAPP:s/-z defs/-z nodefs/)
.ENDIF          # "$(OS)"=="SOLARIS"

SHL2STDLIBS= $(SHL1STDLIBS)
DEF1NAME=$(SHL1TARGET)
.ENDIF         # "$(ENABLE_GTK)" == "TRUE"
.ENDIF         # ELSE "$(GUI)"=="WNT"

SHL1DEPN=
SHL1OBJS=$(SLOFILES)

DEF1EXPORTFILE=	exports.dxp

# --- fop dynlib --------------------------------------------------

SHL2NOCHECK=TRUE
SHL2TARGET=$(TARGET2)

SHL2DEPN=
SHL2IMPLIB=$(SHL1IMPLIB)
SHL2LIBS=$(SHL1LIBS)
SHL2OBJS=$(SLOFILES)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=	exports.dxp

# "$(GUI)"=="WNT" || "$(GUIBASE)"=="unx"
.ELSE
dummy:
    @echo "Nothing to build for OS $(OS)"
.ENDIF


.INCLUDE :  target.mk

