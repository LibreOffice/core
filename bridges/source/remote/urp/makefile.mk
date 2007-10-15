#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: vg $ $Date: 2007-10-15 11:59:32 $
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

PRJNAME=bridges
TARGET=urp_uno
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

SLOFILES= 	\
        $(SLO)$/urp_environment.obj \
        $(SLO)$/urp_marshal.obj \
        $(SLO)$/urp_unmarshal.obj \
        $(SLO)$/urp_dispatch.obj \
        $(SLO)$/urp_job.obj \
        $(SLO)$/urp_reader.obj \
        $(SLO)$/urp_writer.obj \
        $(SLO)$/urp_log.obj \
        $(SLO)$/urp_bridgeimpl.obj \
        $(SLO)$/urp_propertyobject.obj \
        $(SLO)$/urp_threadid.obj

.IF "$(COM)"=="GCC"
NOOPTFILES= \
        $(SLO)$/urp_reader.obj
.ENDIF			# "$(COM)"=="GCC"

SHL1TARGET= $(TARGET)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=..$/..$/bridge_exports.map
SHL1RPATH=URELIB

SHL1STDLIBS=\
        $(SALLIB)\
        $(CPPULIB)

SHL1LIBS=\
            $(SLB)$/$(TARGET).lib \
            $(SLB)$/bridges_remote_static.lib

DEF1NAME=   $(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

