#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 16:43:22 $
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

PRJNAME=hwpfilter
TARGET=hwp

USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

SLOFILES =	\
        $(SLO)$/attributes.obj \
        $(SLO)$/cspline.obj \
        $(SLO)$/fontmap.obj \
        $(SLO)$/formula.obj \
        $(SLO)$/grammar.obj \
        $(SLO)$/hbox.obj \
        $(SLO)$/hcode.obj \
        $(SLO)$/hfont.obj \
        $(SLO)$/hgzip.obj \
        $(SLO)$/himgutil.obj \
        $(SLO)$/hinfo.obj \
        $(SLO)$/hiodev.obj \
        $(SLO)$/hpara.obj \
        $(SLO)$/hstream.obj \
        $(SLO)$/hstyle.obj \
        $(SLO)$/htags.obj \
        $(SLO)$/hutil.obj \
        $(SLO)$/hwpeq.obj \
        $(SLO)$/hwpfile.obj \
        $(SLO)$/hwpread.obj \
        $(SLO)$/hwpreader.obj \
        $(SLO)$/lexer.obj \
        $(SLO)$/mzstring.obj \
        $(SLO)$/solver.obj

# --- Shared-Library -----------------------------------------------

SHL1TARGET= $(TARGET)
#SHL1IMPLIB= ihwp

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(SALLIB)		\
        $(ZLIB3RDLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        ole32.lib uuid.lib
.ENDIF			# "$(GUI)"=="WNT"		


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(SLB)$/$(TARGET).lib
SHL1VERSIONMAP = hwp.map

DEF1NAME=$(SHL1TARGET)

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
