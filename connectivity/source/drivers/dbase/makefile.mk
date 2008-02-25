#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.27 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:05:43 $
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
TARGET=dbase

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/DCode.obj				\
    $(SLO)$/DResultSet.obj			\
    $(SLO)$/DStatement.obj			\
    $(SLO)$/DPreparedStatement.obj	\
    $(SLO)$/dindexnode.obj			\
    $(SLO)$/DIndexIter.obj			\
    $(SLO)$/DDatabaseMetaData.obj	\
    $(SLO)$/DCatalog.obj			\
    $(SLO)$/DColumns.obj			\
    $(SLO)$/DIndexColumns.obj		\
    $(SLO)$/DIndex.obj				\
    $(SLO)$/DIndexes.obj			\
    $(SLO)$/DTable.obj				\
    $(SLO)$/DTables.obj				\
    $(SLO)$/DConnection.obj			\
    $(SLO)$/Dservices.obj			\
    $(SLO)$/DDriver.obj

EXCEPTIONSFILES=\
    $(SLO)$/DCode.obj				\
    $(SLO)$/DResultSet.obj			\
    $(SLO)$/DStatement.obj			\
    $(SLO)$/DPreparedStatement.obj	\
    $(SLO)$/dindexnode.obj			\
    $(SLO)$/DIndexIter.obj			\
    $(SLO)$/DDatabaseMetaData.obj	\
    $(SLO)$/DCatalog.obj			\
    $(SLO)$/DColumns.obj			\
    $(SLO)$/DIndexColumns.obj		\
    $(SLO)$/DIndex.obj				\
    $(SLO)$/DIndexes.obj			\
    $(SLO)$/DTables.obj				\
    $(SLO)$/DConnection.obj			\
    $(SLO)$/Dservices.obj			\
    $(SLO)$/DDriver.obj

# [kh] ppc linux gcc compiler problem
.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCPOWERPC"
EXCEPTIONSNOOPTFILES= \
    $(SLO)$/DTable.obj
.ELSE
EXCEPTIONSFILES +=\
            $(SLO)$/DTable.obj
.ENDIF


SHL1VERSIONMAP=$(TARGET).map

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(SVLLIB)					\
    $(TOOLSLIB)					\
    $(SVTOOLLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(UNOTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile.lib
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(DBASE_TARGET)


SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : target.mk


