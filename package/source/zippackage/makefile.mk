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
# $Revision: 1.25 $
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
PRJNAME=package
TARGET=zippackage
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(L10N_framework)"==""

# --- Files --------------------------------------------------------
# the following flag un-inlines function calls and disables optimisations
#CFLAGS+=/Ob0 /Od

SLOFILES= \
        $(SLO)$/ZipPackage.obj			\
        $(SLO)$/ZipPackageBuffer.obj	\
        $(SLO)$/ZipPackageEntry.obj		\
        $(SLO)$/ZipPackageFolder.obj	\
        $(SLO)$/ZipPackageFolderEnumeration.obj	\
        $(SLO)$/ZipPackageSink.obj		\
        $(SLO)$/ZipPackageStream.obj	\
        $(SLO)$/wrapstreamforshare.obj	\
        $(SLO)$/zipfileaccess.obj

#		$(SLO)$/InteractionRequest.obj  \
#		$(SLO)$/InteractionContinuation.obj

.ENDIF # L10N_framework

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
