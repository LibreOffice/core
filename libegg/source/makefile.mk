#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:59:28 $
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


PRJ=..
PRJNAME=libegg
TARGET=eggtray
TARGETTYPE=GUI

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE
C_RESTRICTIONFLAGS*=-xc99=no_lib

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(ENABLE_GTK)"!=""

PKGCONFIG_MODULES=gtk+-2.0 gdk-2.0
.INCLUDE: pkg_config.mk
CFLAGS+=$(PKGCONFIG_CFLAGS)

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/eggtrayicon.obj

SHL1STDLIBS+=$(PKGCONFIG_LIBS)
.IF "$(PKGCONFIG_ROOT)"!=""
SHL1SONAME+=-z nodefs
SHL1NOCHECK=TRUE
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""


SHL1TARGET= 	eggtray$(DLLPOSTFIX)
SHL1LIBS=       $(SLB)$/eggtray.lib

.ENDIF #	"$(ENABLE_GTK)"!=""

# --- Targets ----------------------------------

.INCLUDE : target.mk
