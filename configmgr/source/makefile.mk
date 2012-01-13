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



PRJ = ..
PRJNAME = configmgr
TARGET = configmgr

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

DLLPRE =

SLOFILES = \
    $(SLO)/access.obj \
    $(SLO)/broadcaster.obj \
    $(SLO)/childaccess.obj \
    $(SLO)/components.obj \
    $(SLO)/configurationprovider.obj \
    $(SLO)/configurationregistry.obj \
    $(SLO)/data.obj \
    $(SLO)/defaultprovider.obj \
    $(SLO)/groupnode.obj \
    $(SLO)/localizedpropertynode.obj \
    $(SLO)/localizedvaluenode.obj \
    $(SLO)/lock.obj \
    $(SLO)/modifications.obj \
    $(SLO)/node.obj \
    $(SLO)/nodemap.obj \
    $(SLO)/parsemanager.obj \
    $(SLO)/partial.obj \
    $(SLO)/propertynode.obj \
    $(SLO)/rootaccess.obj \
    $(SLO)/services.obj \
    $(SLO)/setnode.obj \
    $(SLO)/type.obj \
    $(SLO)/update.obj \
    $(SLO)/valueparser.obj \
    $(SLO)/writemodfile.obj \
    $(SLO)/xcdparser.obj \
    $(SLO)/xcsparser.obj \
    $(SLO)/xcuparser.obj \
    $(SLO)/xmldata.obj

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALHELPERLIB) \
    $(SALLIB) \
    $(XMLREADERLIB)
SHL1TARGET = configmgr.uno
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk

ALLTAR : $(MISC)/configmgr.component

$(MISC)/configmgr.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        configmgr.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt configmgr.component
