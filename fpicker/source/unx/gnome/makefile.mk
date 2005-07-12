#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2005-07-12 12:00:51 $
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

PRJNAME=fpicker
TARGET=fps_gnome.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# ------------------------------------------------------------------

.IF "$(ENABLE_GTK)" != "TRUE"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE), WITH_WIDGETSET == $(WITH_WIDGETSET)"

.ELSE # we build for GNOME

PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE : pkg_config.mk

# check gtk version
GTK_TWO_FOUR:=$(shell +-$(PKGCONFIG) --exists 'gtk+-2.0 >= 2.4.0' && echo ok)
.IF "$(GTK_TWO_FOUR)" != "ok"

dummy:
    @echo "Cannot build gtk filepicker because" 
    @+-$(PKGCONFIG) --print-errors --exists 'gtk+-2.0 >= 2.4.0'

.ELSE

CFLAGS+= $(WIDGETSET_CFLAGS)
CFLAGS+= $(PKGCONFIG_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/SalGtkPicker.obj				\
        $(SLO)$/SalGtkFilePicker.obj			\
        $(SLO)$/SalGtkFolderPicker.obj			\
        $(SLO)$/resourceprovider.obj			\
        $(SLO)$/FPentry.obj

SHL1NOCHECK=TRUE
SHL1TARGET=	$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1STDLIBS=\
    $(VCLLIB) \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(PKGCONFIG_LIBS)

.IF "$(OS)"=="SOLARIS"
LINKFLAGS!:=$(LINKFLAGSAPP:s/-z defs/-z nodefs/)
.ENDIF          # "$(OS)"=="SOLARIS"

SHL1VERSIONMAP=exports.map
DEF1NAME=$(SHL1TARGET)

.ENDIF # GTK_TWO_FOUR
.ENDIF # "$(GUIBASE)" != "unx" || "$(WITH_WIDGETSET)" != "gnome"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
