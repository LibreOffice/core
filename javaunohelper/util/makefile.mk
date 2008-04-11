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
# $Revision: 1.10 $
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

PRJ		= ..
PRJNAME = juhelper
TARGET  = juh

.INCLUDE : settings.mk
.INCLUDE: settings.pmk

JARCLASSDIRS = com
JARTARGET		= $(TARGET).jar
JARCOMPRESS		= TRUE
JARCLASSPATH = $(JARFILES)
CUSTOMMANIFESTFILE = manifest

# Special work necessary for building java reference with javadoc.
# The source of puplic APIs must be delivered and used later in the
# odk module.
ZIP1TARGET=$(TARGET)_src
ZIP1FLAGS=-u -r
ZIP1DIR=$(PRJ)
ZIP1LIST=com -x "*makefile.mk"

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk


