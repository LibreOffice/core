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

PRJNAME=	ucbhelper
TARGET=		provider
AUTOSEG=	TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

SLOFILES=\
        $(SLO)$/contentidentifier.obj  \
        $(SLO)$/providerhelper.obj     \
        $(SLO)$/contenthelper.obj      \
        $(SLO)$/contentinfo.obj        \
        $(SLO)$/propertyvalueset.obj   \
        $(SLO)$/registerucb.obj        \
        $(SLO)$/resultsetmetadata.obj  \
        $(SLO)$/resultset.obj          \
        $(SLO)$/resultsethelper.obj    \
        $(SLO)$/commandenvironmentproxy.obj     \
        $(SLO)$/interactionrequest.obj          \
        $(SLO)$/simpleinteractionrequest.obj    \
        $(SLO)$/simpleauthenticationrequest.obj \
        $(SLO)$/simplenameclashresolverequest.obj \
        $(SLO)$/simpleioerrorrequest.obj        \
        $(SLO)$/cancelcommandexecution.obj \
        $(SLO)$/handleinteractionrequest.obj \
        $(SLO)$/simplecertificatevalidationrequest.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

