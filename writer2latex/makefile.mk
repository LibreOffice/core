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
# $Revision: 1.3 $
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

PRJNAME=writer2latex
TARGET=so_writer2latex

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

.IF "$(WITH_WRITER2LATEX)" == "NO"
@all:
    echo "building without writer2latex"
.ELSE
.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=writer2latex0502

TARFILE_ROOTDIR=writer2latex05

PATCH_FILES=writer2latex05.patch

CONVERTFILES=build.xml
JARFILES 		= ridl.jar unoil.jar jurt.jar juh.jar
BUILD_ACTION=$(ANT) $(ANT_FLAGS) -DSOLVER=$(SOLARVER)/$(INPATH)  -Dsolarbindir=$(SOLARBINDIR) oxt

.ENDIF # $(SOLAR_JAVA)!= ""
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF

