#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: ihi $ $Date: 2007-11-21 15:02:46 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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


