#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: vg $ $Date: 2003-06-02 07:59:21 $
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
TARGET=evoab

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(GUI)"=="UNX"
# --- Files -------------------------------------

EXCEPTIONSFILES=\
    $(SLO)$/LResultSet.obj			\
    $(SLO)$/LStatement.obj			\
    $(SLO)$/LPreparedStatement.obj	\
    $(SLO)$/LFolderList.obj			\
    $(SLO)$/LConfigAccess.obj		\
    $(SLO)$/LColumnAlias.obj		\
    $(SLO)$/LTable.obj				\
    $(SLO)$/LDatabaseMetaData.obj	\
    $(SLO)$/LCatalog.obj			\
    $(SLO)$/LColumns.obj			\
    $(SLO)$/LTables.obj				\
    $(SLO)$/LConnection.obj			\
    $(SLO)$/LServices.obj			\
    $(SLO)$/LDriver.obj

SLOFILES=\
    $(EXCEPTIONSFILES)				\
    $(SLO)$/LNoException.obj		\
    $(SLO)$/LDebug.obj


SHL1VERSIONMAP=$(TARGET).map


# --- Library -----------------------------------
#SHL1TARGET=$(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1TARGET=	$(EVOAB_TARGET)$(EVOAB_MAJOR)
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
    $(SVTLIB)					\
    $(UNOTOOLSLIB)				\
    $(UCBHELPERLIB)				\
    $(SALLIB)					\
    $(DBTOOLSLIB)				\
    $(DBFILELIB)				\
    $(COMPHELPERLIB)

.IF "$(DBFILELIB)" == ""
SHL1STDLIBS+= ifile$(UPD).lib
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)

.ENDIF
# --- Targets ----------------------------------

.INCLUDE : target.mk


