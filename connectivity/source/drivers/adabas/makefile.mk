#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: fs $ $Date: 2000-10-05 08:35:59 $
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
TARGET=adabas

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
        com.sun.star.util.XCancellable					\
        com.sun.star.util.XNumberFormatter				\
        com.sun.star.uno.TypeClass						\
        com.sun.star.uno.XWeak							\
        com.sun.star.uno.XAggregation					\
        com.sun.star.beans.XPropertyState				\
        com.sun.star.beans.XPropertySet					\
        com.sun.star.beans.PropertyValue				\
        com.sun.star.beans.XMultiPropertySet			\
        com.sun.star.beans.XFastPropertySet				\
        com.sun.star.lang.XTypeProvider					\
        com.sun.star.lang.EventObject					\
        com.sun.star.lang.XComponent					\
        com.sun.star.lang.IllegalArgumentException		\
        com.sun.star.lang.DisposedException				\
        com.sun.star.lang.XMultiServiceFactory			\
        com.sun.star.lang.XSingleServiceFactory			\
        com.sun.star.lang.XUnoTunnel					\
        com.sun.star.registry.XRegistryKey				\
        com.sun.star.java.XJavaThreadRegister_11		\
        com.sun.star.java.XJavaVM						\
        com.sun.star.sdbc.XConnection					\
        com.sun.star.sdbc.XStatement					\
        com.sun.star.sdbc.XResultSet					\
        com.sun.star.sdbc.XResultSetMetaDataSupplier	\
        com.sun.star.sdbc.XColumnLocate					\
        com.sun.star.sdbc.XResultSetUpdate				\
        com.sun.star.sdbc.XWarningsSupplier				\
        com.sun.star.sdbc.XRowUpdate					\
        com.sun.star.sdbc.XMultipleResults				\
        com.sun.star.sdbc.XBatchExecution				\
        com.sun.star.sdbc.XPreparedBatchExecution		\
        com.sun.star.sdbc.XParameters					\
        com.sun.star.sdbc.XOutParameters				\
        com.sun.star.sdbc.DriverPropertyInfo			\
        com.sun.star.sdbc.XDriver						\
        com.sun.star.sdbc.XRow							\
        com.sun.star.sdbc.SQLWarning					\
        com.sun.star.sdbc.ColumnSearch					\
        com.sun.star.sdbc.DataType						\
        com.sun.star.sdbc.ResultSetConcurrency			\
        com.sun.star.sdbc.ResultSetType					\
        com.sun.star.sdbc.ColumnValue					\
        com.sun.star.sdbcx.KeyType						\
        com.sun.star.sdb.XColumnUpdate					\
        com.sun.star.sdb.XColumn						\
        


# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/BConnection.obj					\
        $(SLO)$/BDriver.obj						\
        $(SLO)$/BCatalog.obj					\
        $(SLO)$/BGroups.obj						\
        $(SLO)$/BGroup.obj						\
        $(SLO)$/BUser.obj						\
        $(SLO)$/BUsers.obj						\
        $(SLO)$/BKeyColumns.obj					\
        $(SLO)$/BKey.obj						\
        $(SLO)$/BKeys.obj						\
        $(SLO)$/BColumns.obj					\
        $(SLO)$/BIndex.obj						\
        $(SLO)$/BIndexColumns.obj				\
        $(SLO)$/BIndexes.obj					\
        $(SLO)$/BTable.obj						\
        $(SLO)$/BTables.obj						\
        $(SLO)$/Bservices.obj					\
        $(SLO)$/BDatabaseMetaData.obj			\
        $(SLO)$/OPreparedStatement.obj			\
        $(SLO)$/OStatement.obj					\
        $(SLO)$/OResultSetMetaData.obj			\
        $(SLO)$/OResultSet.obj					\
        $(SLO)$/OTools.obj						\
        $(SLO)$/ODatabaseMetaData.obj			\
        $(SLO)$/ODriver.obj						\
        $(SLO)$/OFunctions.obj					\
        $(SLO)$/OConnection.obj
        
# NETBSD: somewhere we have to instantiate the static data members.
# NETBSD-1.2.1 doesn't know about weak symbols so the default mechanism for GCC won't work.
# SCO and MACOSX: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="NETBSD" || "$(OS)"=="SCO" || "$(OS)$(COM)"=="OS2GCC" || "$(OS)"=="MACOSX"
SLOFILES+=$(SLO)$/staticmbadabas.obj
.ENDIF

# --- Library -----------------------------------

SHL1TARGET=	$(ADABAS_TARGET)$(ADABAS_MAJOR)
SHL1VERSIONMAP= $(TARGET).map
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)					\
    $(CPPUHELPERLIB)			\
    $(VOSLIB)					\
    $(OSLLIB)					\
    $(SALLIB)					\
    $(COMPHELPERLIB)

.IF "$(COMPHELPERLIB)" == ""
SHL1STDLIBS+= icomphelp2.lib
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/commontools.lib	\
            $(SLB)$/resource.lib	\
            $(SLB)$/sdbcx.lib		

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ----------------------------------

.INCLUDE : target.mk


