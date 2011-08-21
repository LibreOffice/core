#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
#***********************************************************************/

PRJ = ../..
PRJNAME = jurt
TARGET = jpipe

NO_DEFAULT_STL = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

.IF "$(OS)" == "WNT"
SHL1TARGET = jpipx
.ELSE
SHL1TARGET = jpipe
.END

SHL1CODETYPE = C
SHL1CREATEJNILIB=TRUE
SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLO)/com_sun_star_lib_connections_pipe_PipeConnection.obj
SHL1RPATH = URELIB
SHL1STDLIBS = $(SALLIB)
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.IF "$(SOLAR_JAVA)" == ""
nothing .PHONY :
.END

.INCLUDE: target.mk
