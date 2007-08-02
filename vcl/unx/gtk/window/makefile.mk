#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: hr $ $Date: 2007-08-02 15:00:33 $
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

PRJNAME=vcl
TARGET=gtkwin
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# workaround for makedepend hang
MKDEPENDSOLVER=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

.IF "$(ENABLE_GTK)" != ""

PKGCONFIG_MODULES=gtk+-2.0
.IF "$(ENABLE_DBUS)" != ""
CDEFS+=-DENABLE_DBUS
PKGCONFIG_MODULES+= dbus-glib-1
.ENDIF
.INCLUDE : pkg_config.mk

SLOFILES=\
            $(SLO)$/gtkframe.obj				\
            $(SLO)$/gtkobject.obj
EXCEPTIONSFILES=$(SLO)$/gtkframe.obj
.ELSE # "$(ENABLE_GTK)" != ""

dummy:
    @echo GTK disabled - nothing to build
.ENDIF
.ENDIF		# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk
