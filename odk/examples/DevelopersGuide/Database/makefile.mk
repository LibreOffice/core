#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: kz $ $Date: 2003-08-27 16:44:00 $
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
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

#----------------------------------------------------
# this makefile is only used for copying the example 
# files into the SDK
#----------------------------------------------------

DATABASE_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/CodeSamples.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/OpenQuery.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/RowSet.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/RowSetEventListener.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/Sales.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/SalesMan.java \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/sdbcx.java

DATABASEDRIVERSKELETON_FILES=\
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/exports.dxp \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/How_to_write_my_own_driver.txt \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/Makefile \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/OSubComponent.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/OTypeInfo.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/propertyids.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/propertyids.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SConnection.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SConnection.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SDatabaseMetaData.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SDatabaseMetaData.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SDriver.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SDriver.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/skeleton.xml \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SPreparedStatement.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SPreparedStatement.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SResultSet.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SResultSet.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SResultSetMetaData.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SResultSetMetaData.hxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SServices.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SStatement.cxx \
    $(DESTDIRDEVGUIDEEXAMPLES)$/Database$/DriverSkeleton$/SStatement.hxx

DIR_FILE_LIST= \
    $(DATABASE_FILES) \
    $(DATABASEDRIVERSKELETON_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_database_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_database.txt

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
    $(DIR_FILE_LIST) \
    $(DIR_FILE_FLAG)

#--------------------------------------------------
# use global rules
#--------------------------------------------------   
.INCLUDE: $(PRJ)$/util$/odk_rules.pmk
    
