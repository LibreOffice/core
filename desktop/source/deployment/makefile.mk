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



PRJ = ..$/..

PRJNAME = desktop
TARGET = deployment
ENABLE_EXCEPTIONS = TRUE
#USE_DEFFILE = TRUE
NO_BSYMBOLIC = TRUE

.IF "$(GUI)"=="OS2"
TARGET = deploy
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk

INCPRE += inc

DLLPRE =

SHL1TARGET = $(TARGET)$(DLLPOSTFIX).uno
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

SHL1LIBS = \
    $(SLB)$/deployment_manager.lib \
    $(SLB)$/deployment_registry.lib \
    $(SLB)$/deployment_registry_executable.lib \
    $(SLB)$/deployment_registry_component.lib \
    $(SLB)$/deployment_registry_configuration.lib \
    $(SLB)$/deployment_registry_package.lib \
    $(SLB)$/deployment_registry_script.lib \
    $(SLB)$/deployment_registry_sfwk.lib \
    $(SLB)$/deployment_registry_help.lib

SHL1OBJS = \
    $(SLO)$/dp_log.obj \
    $(SLO)$/dp_persmap.obj \
    $(SLO)$/dp_services.obj \
    $(SLO)$/dp_xml.obj

SHL1STDLIBS = \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(UCBHELPERLIB) \
        $(COMPHELPERLIB) \
        $(TOOLSLIB) \
        $(XMLSCRIPTLIB) \
        $(SVLLIB) \
        $(UNOTOOLSLIB) \
        $(DEPLOYMENTMISCLIB) \
    $(HELPLINKERLIB)

SHL1DEPN =
SHL1IMPLIB = i$(TARGET)
SHL1DEF = $(MISC)$/$(SHL1TARGET).def

DEF1NAME = $(SHL1TARGET)

SLOFILES = $(LIB1OBJFILES)

RESLIB1NAME = $(TARGET)

RESLIB1SRSFILES = \
        $(SRS)$/deployment_registry_configuration.srs \
        $(SRS)$/deployment_registry_component.srs \
        $(SRS)$/deployment_registry_script.srs \
        $(SRS)$/deployment_registry_sfwk.srs \
        $(SRS)$/deployment_registry_package.srs \
        $(SRS)$/deployment_registry_help.srs \
        $(SRS)$/deployment_registry.srs \
        $(SRS)$/deployment_manager.srs \
    $(SRS)$/deployment_unopkg.srs

.IF "$(GUI)"=="OS2"
RESLIB1SRSFILES += $(SRS)$/deplmisc.srs
.ELSE
RESLIB1SRSFILES += $(SRS)$/deployment_misc.srs
.ENDIF

.INCLUDE : target.mk


ALLTAR : $(MISC)/deployment.component

$(MISC)/deployment.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        deployment.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt deployment.component
