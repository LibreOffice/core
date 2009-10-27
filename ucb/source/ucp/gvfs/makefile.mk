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

PRJ=..$/..$/..
PRJNAME=ucb
# Version
UCPGVFS_MAJOR=1
TARGET=ucpgvfs
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

UNIXTEXT=$(MISC)/$(TARGET)-ucd.txt

.IF "$(ENABLE_GNOMEVFS)"!=""
COMPILER_WARN_ALL=TRUE
PKGCONFIG_MODULES=gnome-vfs-2.0
.INCLUDE: pkg_config.mk

.IF "$(OS)" == "SOLARIS"
LINKFLAGS+=-z nodefs
.ENDIF          # "$(OS)" == "SOLARIS"

.IF "$(OS)" == "LINUX"
# hack for faked SO environment
CFLAGS+=-gdwarf-2
PKGCONFIG_LIBS!:=-Wl,--export-dynamic $(PKGCONFIG_LIBS:s/ -llinc//:s/ -lbonobo-activation//:s/ -lgconf-2//:s/ -lORBit-2//:s/ -lIDL-2//:s/ -lgmodule-2.0//:s/ -lgobject-2.0//:s/ -lgthread-2.0//)
.ENDIF          # "$(OS)" == "LINUX"

# no "lib" prefix
DLLPRE =

SLOFILES=\
    $(SLO)$/content.obj	\
    $(SLO)$/directory.obj	\
    $(SLO)$/stream.obj	\
    $(SLO)$/provider.obj

SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET)$(UCPGVFS_MAJOR).uno
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1STDLIBS=\
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB)  \
    $(SALHELPERLIB)  \
    $(UCBHELPERLIB)

SHL1STDLIBS+=$(PKGCONFIG_LIBS)

SHL1VERSIONMAP= exports.map

.ENDIF          # "$(ENABLE_GNOMEVFS)"!=""
.ENDIF # L10N_framework

.INCLUDE: target.mk

