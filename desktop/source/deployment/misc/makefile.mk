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



PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = deployment_misc
USE_DEFFILE = TRUE
ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN=TRUE

.IF "$(GUI)"=="OS2"
TARGET = deplmisc
.ENDIF

.INCLUDE : settings.mk

# Reduction of exported symbols:
CDEFS += -DDESKTOP_DEPLOYMENTMISC_DLLIMPLEMENTATION

SRS1NAME = $(TARGET)
SRC1FILES = \
    dp_misc.src

.IF "$(GUI)"=="OS2"
SHL1TARGET = $(TARGET)
.ELSE
SHL1TARGET = deploymentmisc$(DLLPOSTFIX)
.ENDIF
SHL1OBJS = \
        $(SLO)$/dp_misc.obj \
        $(SLO)$/dp_resource.obj \
        $(SLO)$/dp_identifier.obj \
        $(SLO)$/dp_interact.obj \
        $(SLO)$/dp_ucb.obj \
        $(SLO)$/dp_version.obj \
        $(SLO)$/dp_descriptioninfoset.obj \
        $(SLO)$/dp_dependencies.obj \
        $(SLO)$/dp_platform.obj \
        $(SLO)$/dp_update.obj
        
SHL1STDLIBS = \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(XMLSCRIPTLIB) \
    $(COMPHELPERLIB)
.IF "$(GUI)"=="OS2"
SHL1IMPLIB = ideploymentmisc$(DLLPOSTFIX)
LIB1TARGET = $(SLB)$/_deplmisc.lib
LIB1OBJFILES = $(SHL1OBJS)
DEFLIB1NAME = _deplmisc
.ELSE
SHL1IMPLIB = i$(SHL1TARGET)
.ENDIF
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE : ..$/target.pmk
.INCLUDE : target.mk

