#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1.8.2 $
#
#   last change: $Author: mav $ $Date: 2008/10/30 11:59:06 $
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
PRJNAME=embedserv
TARGET=inprocserv

use_shl_versions=

# --- Settings ----------------------------------
.INCLUDE : settings.mk

.IF "$(GUI)" == "WNT"

LIBTARGET=NO
USE_DEFFILE=YES

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/dllentry.obj \
    $(SLO)$/advisesink.obj \
    $(SLO)$/inprocembobj.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
    $(UUIDLIB)\
    $(OLE32LIB)\
    $(GDI32LIB)\
    $(ADVAPI32LIB)

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(TARGET).def

DEF1NAME= $(TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

