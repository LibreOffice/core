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



PRJ=..$/..
PRJINC=..$/..$/inc
PRJNAME=unotools
TARGET=i18n

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

SLOFILES=	\
    $(SLO)$/charclass.obj	\
    $(SLO)$/calendarwrapper.obj	\
    $(SLO)$/collatorwrapper.obj	\
    $(SLO)$/intlwrapper.obj \
    $(SLO)$/localedatawrapper.obj	\
    $(SLO)$/nativenumberwrapper.obj	\
    $(SLO)$/numberformatcodewrapper.obj \
    $(SLO)$/readwritemutexguard.obj \
    $(SLO)$/transliterationwrapper.obj \
    $(SLO)$/textsearch.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

