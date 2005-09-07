#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 19:55:11 $
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
PRJNAME=shell
TARGET=misc

LIBTARGET=NO
TARGETTYPE=CUI
NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

SCRIPTFILES = \
    $(BIN)$/senddoc \
    $(BIN)$/open-url \
    $(BIN)$/cde-open-url \
    $(BIN)$/gnome-open-url \
    $(BIN)$/kde-open-url

AWKFILES = \
    $(BIN)$/uri-encode

OBJFILES = \
    $(OBJ)$/gnome-open-url.obj

APP1TARGET = gnome-open-url.bin
APP1OBJS = \
    $(OBJ)$/gnome-open-url.obj
APP1LIBS =
.IF "$(OS)"!="FREEBSD"
APP1STDLIBS=-ldl
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

ALLTAR : $(SCRIPTFILES) $(AWKFILES)

$(SCRIPTFILES) : $$(@:f:+".sh")
    +@tr -d "\015" < $(@:f:+".sh") > $@

$(AWKFILES) : $$(@:f:+".awk")
    +@tr -d "\015" < $(@:f:+".awk") > $@

