#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:05:38 $
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
    
