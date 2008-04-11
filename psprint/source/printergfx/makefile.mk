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
# $Revision: 1.8 $
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

PRJ=..$/..

PRJNAME=psprint
TARGET=gfx

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=true

.INCLUDE :	settings.mk

.IF "$(ENABLE_CUPS)" != ""
CDEFS += -DENABLE_CUPS
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"=="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"=="aqua"

SLOFILES=\
    $(SLO)$/printerjob.obj		\
    $(SLO)$/text_gfx.obj		\
    $(SLO)$/psputil.obj			\
    $(SLO)$/common_gfx.obj		\
    $(SLO)$/glyphset.obj		\
    $(SLO)$/bitmap_gfx.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

