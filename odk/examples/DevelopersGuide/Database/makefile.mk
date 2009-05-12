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
# $Revision: 1.6 $
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
    
