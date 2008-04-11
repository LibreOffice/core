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
# $Revision: 1.5 $
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

PRJ=.

PRJNAME=fondu
TARGET=fondu

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=fondu_src-051010

.IF "$(OS)"!="MACOSX" || "$(GUIBASE)"=="aqua"
dummy:
    @echo "Nothing to build for OS $(OS)"
.ENDIF # "$(OS)"!="MACOSX" || "$(GUIBASE)"=="aqua"

CONFIGURE_DIR=.

#relative to CONFIGURE_DIR
CONFIGURE_ACTION=configure

CONFIGURE_FLAGS=

BUILD_DIR=$(CONFIGURE_DIR)

BUILD_ACTION=$(GNUMAKE) fondu

OUT2BIN=$(BUILD_DIR)$/fondu

# --- Targets ------------------------------------------------------

# contains some necessary variables.

.INCLUDE : set_ext.mk

.INCLUDE : target.mk

# contains the "magic" to create targets out of the set variables.

.INCLUDE : tg_ext.mk