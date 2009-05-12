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
# $Revision: 1.4.2.1 $
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

PRJNAME=stax
TARGET=stax
USE_JAVAVER=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
.IF "$(JAVANUMVER:s/.//)" >= "000100060000" || "$(JDK)"=="gcj"
all:
    @echo "Your java version already contains StAX"
.ENDIF			# "$(JAVANUMVER:s/.//)" >= "000100060000" 
.IF "$(SYSTEM_SAXON)" == "YES"
all:
    @echo "An already available installation of saxon should exist on your system."
    @echo "Therefore the files provided here does not need to be built in addition."
.ENDIF

# --- Files --------------------------------------------------------
TARFILE_NAME=stax-api-1.0-2-sources
TARFILE_ROOTDIR=src

BUILD_ACTION=$(JAVAC) javax$/xml$/stream$/*.java && jar -cf jsr173_1.0_api.jar javax$/xml$/stream$/*.class javax$/xml$/stream$/events$/*.class javax$/xml$/stream$/util$/*.class

OUT2CLASS=jsr173_1.0_api.jar

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

