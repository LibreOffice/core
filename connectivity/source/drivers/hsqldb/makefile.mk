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
# $Revision: 1.14 $
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
PRJINC=..$/..
PRJNAME=connectivity
TARGET=hsqldb

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings ----------------------------------
.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jurt because Java is disabled"
.ENDIF

.IF "$(SYSTEM_HSQLDB)" == "YES"
CDEFS+=-DSYSTEM_HSQLDB -DHSQLDB_JAR=\""file://$(HSQLDB_JAR)"\"
.ENDIF
# --- Resources ---------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =	\
        hsqlui.src
        
RES1FILELIST=\
    $(SRS)$/$(TARGET).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(SOLARSRC)$/$(RSCDEFIMG)$/database
RESLIB1SRSFILES=$(RES1FILELIST)

# Note that the resource file built here is currently *not* included
# in installation sets. See hsqlui.src for an explanation

# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/HStorageMap.obj					\
        $(SLO)$/HStorageAccess.obj				\
        $(SLO)$/HDriver.obj						\
        $(SLO)$/HConnection.obj					\
        $(SLO)$/HTerminateListener.obj			\
        $(SLO)$/StorageNativeOutputStream.obj	\
        $(SLO)$/StorageNativeInputStream.obj	\
        $(SLO)$/StorageFileAccess.obj			\
        $(SLO)$/HTables.obj                     \
        $(SLO)$/HTable.obj                      \
        $(SLO)$/HView.obj                       \
        $(SLO)$/HViews.obj                      \
        $(SLO)$/HCatalog.obj                    \
        $(SLO)$/HColumns.obj                    \
        $(SLO)$/HUser.obj                       \
        $(SLO)$/HUsers.obj                      \
        $(SLO)$/Hservices.obj                   \
        $(SLO)$/HTools.obj                      \
        $(SLO)$/accesslog.obj

SHL1VERSIONMAP=$(HSQLDB_TARGET).map

# --- Library -----------------------------------

SHL1TARGET=	$(HSQLDB_TARGET)$(HSQLDB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SALLIB)					\
    $(SALHELPERLIB)				\
    $(JVMACCESSLIB)				\
    $(DBTOOLSLIB)				\
    $(JVMFWKLIB)				\
    $(COMPHELPERLIB)            \
    $(UNOTOOLSLIB)


SHL1DEPN=
SHL1IMPLIB=	i$(HSQLDB_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk


