#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: vg $ $Date: 2008-03-19 10:54:10 $
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
TARGET=gtka11y
ENABLE_EXCEPTIONS=TRUE

# workaround for makedepend hang
MKDEPENDSOLVER=
NO_DEFAULT_STL=YES

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

.IF "$(ENABLE_GTK)" != ""

PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE : pkg_config.mk

CFLAGS+=-DVERSION=$(EMQ)"$(UPD)$(LAST_MINOR)$(EMQ)"

ATKVERSION:=$(shell @$(PKG_CONFIG) --modversion atk | $(AWK) -v num=true -f $(SOLARENV)$/bin$/getcompver.awk)

.IF "$(ATKVERSION)" >= "000100070000"
CFLAGS+=-DHAS_ATKRECTANGLE
.ENDIF

SLOFILES=\
    $(SLO)$/atkaction.obj \
    $(SLO)$/atkbridge.obj \
    $(SLO)$/atkcomponent.obj \
    $(SLO)$/atkeditabletext.obj \
    $(SLO)$/atkfactory.obj \
    $(SLO)$/atkhypertext.obj \
    $(SLO)$/atkimage.obj \
    $(SLO)$/atklistener.obj \
    $(SLO)$/atkregistry.obj \
    $(SLO)$/atkselection.obj \
    $(SLO)$/atktable.obj \
    $(SLO)$/atktext.obj \
    $(SLO)$/atktextattributes.obj \
    $(SLO)$/atkutil.obj \
    $(SLO)$/atkvalue.obj \
    $(SLO)$/atkwindow.obj \
    $(SLO)$/atkwrapper.obj

.ELSE # "$(ENABLE_GTK)" != ""

dummy:
    @echo GTK disabled - nothing to build
.ENDIF
.ENDIF		# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

