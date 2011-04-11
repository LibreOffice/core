#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2008 by Sun Microsystems, Inc.
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
#*************************************************************************

PRJ=.

PRJNAME=nlpsolver
TARGET=nlpsolver

.IF "$(SOLAR_JAVA)"!=""
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  antsettings.mk

# --- Files --------------------------------------------------------

TARFILE_MD5=90401bca927835b6fbae4a707ed187c8
TARFILE_NAME=nlpsolver-0.9
TARFILE_ROOTDIR=nlpsolver-0.9

PATCH_FILES=nlpsolver-0.9.patch

ANT_FLAGS+=-Dplatforms.JDK_1.5.home=$(JAVA_HOME) -Doffice.program.dir=$(SOLARBINDIR)

CONFIGURE_ACTION=cp -rv --preserve=timestamps ../../../../locale src/

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) $(ANT_FLAGS) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj uno-package
.ELSE
BUILD_ACTION=$(ANT) $(ANT_FLAGS) -Dbuild.label="build-$(RSCREVISION)" -Dplatforms.JDK_1.5.home=$(JAVA_HOME) -Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER) uno-package
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ELSE
all:
        @echo java disabled
.ENDIF
