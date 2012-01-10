#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dataaccess

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/dba.pmk

# --- Files -------------------------------------

SLOFILES=	\
        $(SLO)$/SharedConnection.obj	\
        $(SLO)$/ContentHelper.obj		\
        $(SLO)$/bookmarkcontainer.obj	\
        $(SLO)$/definitioncontainer.obj	\
        $(SLO)$/commanddefinition.obj	\
        $(SLO)$/documentcontainer.obj	\
        $(SLO)$/commandcontainer.obj	\
        $(SLO)$/documentdefinition.obj	\
        $(SLO)$/ComponentDefinition.obj	\
        $(SLO)$/databasecontext.obj		\
        $(SLO)$/connection.obj			\
        $(SLO)$/datasource.obj			\
        $(SLO)$/databaseregistrations.obj \
        $(SLO)$/intercept.obj			\
        $(SLO)$/myucp_datasupplier.obj	\
        $(SLO)$/myucp_resultset.obj		\
        $(SLO)$/databasedocument.obj	\
        $(SLO)$/dataaccessdescriptor.obj\
        $(SLO)$/ModelImpl.obj           \
        $(SLO)$/documentevents.obj      \
        $(SLO)$/documenteventexecutor.obj \
        $(SLO)$/documenteventnotifier.obj \

# --- Targets ----------------------------------

.INCLUDE : target.mk

