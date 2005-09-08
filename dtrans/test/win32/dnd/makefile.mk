#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.12 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 18:31:53 $
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

PRJ=..$/..$/..$/

PRJNAME=dtrans
TARGET=dndTest
TARGETTYPE=CUI
LIBTARGET=NO

#USE_DEFFILE=	TRUE
NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE
BOOTSTRAP_SERVICE=FALSE

# --- Settings ---

.INCLUDE : svpre.mk
.INCLUDE : settings.mk
.INCLUDE : sv.mk

# --- Files ---

# CFLAGS+=-GR -DUNICODE -D_UNICODE
CFLAGS+= -D_WIN32_DCOM

.IF "$(BOOTSTRAP_SERVICE)" == "TRUE"
UNOUCROUT=	$(OUT)$/inc$/comprehensive
INCPRE+=	$(OUT)$/inc$/comprehensive
CPPUMAKERFLAGS += -C
.ELSE
UNOUCROUT=	$(OUT)$/inc
INCPRE+=	$(OUT)$/inc -I$(ATL_INCLUDE)
.ENDIF

UNOTYPES= com.sun.star.lang.XInitialization \
          com.sun.star.lang.XComponent	\
          com.sun.star.datatransfer.dnd.XDropTarget \
          com.sun.star.datatransfer.dnd.XDropTargetListener \
          com.sun.star.datatransfer.dnd.DNDConstants		\
          com.sun.star.datatransfer.dnd.XDragSource



.IF "$(depend)" != ""

.ENDIF # depend

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)
APP1OBJS=	$(OBJ)$/dndTest.obj	\
        $(OBJ)$/atlwindow.obj \
        $(OBJ)$/targetlistener.obj \
        $(OBJ)$/sourcelistener.obj \
        $(OBJ)$/dataobject.obj

LIBCIMT=msvcrtd.lib


APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)		\
    uwinapi.lib \
    user32.lib		\
    ole32.lib		\
    comsupp.lib		\
    oleaut32.lib	\
    gdi32.lib		\
    uuid.lib

APP1LIBS=	\
            $(SLB)$/dtobjfact.lib	\
            $(SLB)$/dtutils.lib

#			$(SOLARLIBDIR)$/imtaolecb.lib\



.IF "$(GUI)"=="WNT"
APP1STDLIBS += $(LIBCIMT)
APP2STDLIBS += $(LIBCIMT)
.ENDIF

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

