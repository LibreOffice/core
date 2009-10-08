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
# $Revision: 1.1.2.1 $
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

PRJNAME=saxon
TARGET=saxon

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
.IF "$(SYSTEM_SAXON)" == "YES"
all:
        @echo "An already available installation of saxon should exist on your system."
    @echo "Therefore the version provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------

TARFILE_NAME=source-9.0.0.7-bj
ADDITIONAL_FILES=build.xml
TARFILE_ROOTDIR=src

PATCH_FILE_NAME=saxon-9.0.0.7-bj.patch

BUILD_ACTION=$(ANT) $(ANT_FLAGS) -Dsolarbindir=$(SOLARBINDIR) jar-bj 

OUT2CLASS= saxon-build$/9.0.0.7$/bj$/saxon9.jar

.ELSE			# $(SOLAR_JAVA)!= ""
nojava:
    @echo "Not building $(PRJNAME) because Java is disabled"
.ENDIF			# $(SOLAR_JAVA)!= ""
# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF

