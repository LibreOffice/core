#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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
    
