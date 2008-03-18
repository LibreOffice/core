#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 12:23:00 $
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

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for GUIBASE aqua."

.ELSE

PRJ=..
PRJNAME=padmin
TARGET=padmin
TARGETTYPE=GUI
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES=padialog.src rtsetup.src

RESLIB1NAME=spa
RESLIB1IMAGES=$(PRJ)$/source
RESLIB1SRSFILES= $(SRS)$/padmin.srs
RESLIB1DEPN=\
    padialog.src	\
    padialog.hrc	\
    rtsetup.src		\
    rtsetup.hrc

.IF "$(GUI)" == "UNX"

SLOFILES=\
    $(SLO)$/padialog.obj		\
    $(SLO)$/cmddlg.obj			\
    $(SLO)$/progress.obj		\
    $(SLO)$/newppdlg.obj		\
    $(SLO)$/prtsetup.obj		\
    $(SLO)$/prgsbar.obj			\
    $(SLO)$/fontentry.obj		\
    $(SLO)$/helper.obj			\
    $(SLO)$/adddlg.obj			\
    $(SLO)$/titlectrl.obj


OBJFILES=\
    $(OBJ)/pamain.obj

SHL1TARGET= spa$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(VCLLIB)						\
    -lpsp$(DLLPOSTFIX)		\
    $(UNOTOOLSLIB)					\
    $(TOOLSLIB)						\
    $(COMPHELPERLIB)				\
    $(CPPULIB)						\
    $(SALLIB)

APP1TARGET=spadmin.bin
APP1DEPN+=$(SHL1TARGETN)

APP1OBJS=\
    $(OBJ)$/desktopcontext.obj  \
    $(OBJ)/pamain.obj

APP1STDLIBS=	\
    -l$(SHL1TARGET)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(UCBHELPERLIB)		\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)			\
    -lXext -lX11

UNIXTEXT = $(MISC)$/spadmin.sh

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # GUIBASE==aqua

