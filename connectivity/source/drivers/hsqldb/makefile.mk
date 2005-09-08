#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 06:07:25 $
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
        $(SLO)$/HViews.obj                      \
        $(SLO)$/HCatalog.obj                    \
        $(SLO)$/HColumns.obj                    \
        $(SLO)$/HUser.obj                       \
        $(SLO)$/HUsers.obj                      \
        $(SLO)$/Hservices.obj                   \
        $(SLO)$/accesslog.obj
        

.IF "$(OS)"=="MACOSX"
#SHL1VERSIONMAP=$(HSQLDB_TARGET).$(DLLPOSTFIX).map
.ELSE      
SHL1VERSIONMAP=$(HSQLDB_TARGET).map
.ENDIF

# --- Library -----------------------------------

SHL1TARGET=	$(HSQLDB_TARGET)$(HSQLDB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SALLIB)					\
    $(SALHELPERLIB)				\
    $(JVMACCESSLIB)				\
    $(DBTOOLSLIB)				\
    $(JVMFWKLIB)				\
    $(COMPHELPERLIB)


SHL1DEPN=
SHL1IMPLIB=	i$(HSQLDB_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------

.INCLUDE : target.mk


