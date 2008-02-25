#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:29:12 $
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

PRJ=..$/..

PRJNAME=dtrans
TARGET=dtransX11
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for Mac OS X"
 
.ELSE		# "$(GUIBASE)"=="aqua"

.IF "$(COM)$(CPU)" == "C50I" || "$(COM)$(CPU)" == "C52I"
NOOPTFILES=\
    $(SLO)$/X11_selection.obj
.ENDIF

SLOFILES=\
    $(SLO)$/X11_dndcontext.obj		\
    $(SLO)$/X11_transferable.obj	\
    $(SLO)$/X11_clipboard.obj		\
    $(SLO)$/X11_selection.obj		\
    $(SLO)$/X11_droptarget.obj		\
    $(SLO)$/X11_service.obj			\
    $(SLO)$/bmp.obj					\
    $(SLO)$/config.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS= $(LIBSTLPORT) $(CPPUHELPERLIB)
.ELSE
SHL1STDLIBS= $(CPPUHELPERLIB)
.ENDIF

SHL1STDLIBS+= \
        $(UNOTOOLSLIB)	\
        $(CPPULIB) 	\
        $(SALLIB)	\
        -lX11

SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET) 
SHL1OBJS=		$(SLOFILES)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF		# "$(OS)"=="MACOSX"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
