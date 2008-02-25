#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:06:12 $
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
TARGET=evoab2
ENABLE_EXCEPTIONS=TRUE

.IF "$(ENABLE_EVOAB2)"!="TRUE"
dummy:
    @echo "Evolution 2.x Addressbook build disabled"
.ELSE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/ 
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

CFLAGS+=$(GOBJECT_CFLAGS)

# --- Files -------------------------------------

EXCEPTIONSFILES=\
    $(SLO)$/NDriver.obj \
    $(SLO)$/NTable.obj \
    $(SLO)$/NColumns.obj \
    $(SLO)$/NTables.obj \
    $(SLO)$/NCatalog.obj \
    $(SLO)$/NConnection.obj \
    $(SLO)$/NDatabaseMetaData.obj \
    $(SLO)$/NStatement.obj \
    $(SLO)$/NPreparedStatement.obj \
    $(SLO)$/NServices.obj \
    $(SLO)$/NResultSet.obj  \
    $(SLO)$/NResultSetMetaData.obj \
    $(SLO)$/EApi.obj 

SLOFILES=\
    $(EXCEPTIONSFILES)				\
    $(SLO)$/NDebug.obj


SHL1VERSIONMAP=$(TARGET).map


# --- Library -----------------------------------
#SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1TARGET=	$(EVOAB_TARGET)2
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(VCLLIB)					\
    $(TOOLSLIB)					\
    $(SVTOOLLIB)				\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile.lib
.ENDIF
SHL1STDLIBS+=$(GOBJECT_LIBS)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)

.ENDIF
# --- Targets ----------------------------------

.INCLUDE : target.mk
