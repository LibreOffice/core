#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: ganaya $ $Date: 2001-02-13 21:18:03 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=dbase

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

# --- Types -------------------------------------


UNOUCRDEP=$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=$(SOLARBINDIR)$/applicat.rdb

UNOUCROUT=$(OUT)$/inc
INCPRE+=$(UNOUCROUT)

# --- Types -------------------------------------

UNOTYPES+=	\
        com.sun.star.sdbcx.XRowLocate					\
        com.sun.star.sdbcx.CompareBookmark				\
        com.sun.star.sdbcx.XDeleteRows

# --- Files -------------------------------------

SLOFILES=\
    $(SLO)$/DResultSet.obj			\
    $(SLO)$/DStatement.obj			\
    $(SLO)$/DPreparedStatement.obj	\
    $(SLO)$/dindexnode.obj			\
    $(SLO)$/DIndexPage.obj			\
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

# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"=="MACOSX" 
SLOFILES+=$(SLO)$/staticmbdbase.obj
SLOFILES+=$(SLO)$/staticmbodbc.obj
.ENDIF

.IF "$(OS)"=="MACOSX"
SHL1VERSIONMAP=$(TARGET).$(DLLPOSTFIX).map
.ELSE      
SHL1VERSIONMAP=$(TARGET).map
.ENDIF
# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SVLLIB)					\
    $(SVLIB)					\
    $(TOOLSLIB)					\
    $(SVTOOLLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(UNOTOOLSLIB)				\
    $(COMPHELPERLIB)

.IF "$(COMPHELPERLIB)" == ""
SHL1STDLIBS+= icomphelp2.lib
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(DBASE_TARGET)
SHL1LIBS=	$(SLB)$/file.lib

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : target.mk


