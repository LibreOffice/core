#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.21 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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

