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
TARGET = unopkg
.IF "$(GUI)" == "OS2"
TARGETTYPE = CUI
.ELSE
TARGETTYPE = GUI
.ENDIF
ENABLE_EXCEPTIONS = TRUE
LIBTARGET=NO

PRJINC += ..$/..$/deployment ..$/..
.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk

.IF "$(LINK_SO)"!=""
APP1TARGET = so$/unopkg
APP1OBJS = $(OBJFILES)
APP1STDLIBS = $(SALLIB) $(UNOPKGAPPLIB)
APP1DEPN = $(SHL1TARGETN)
APP1NOSAL = TRUE
APP1RPATH = BRAND
.IF "$(OS)" == "WNT"
APP1ICON = $(SOLARRESDIR)$/icons/so9_main_app.ico
APP1LINKRES = $(MISC)$/$(TARGET)1.res
.ENDIF
.ENDIF			# "$(LINK_SO)"!=""

APP2TARGET = unopkg
APP2OBJS = $(OBJFILES)
APP2STDLIBS = $(SALLIB) $(UNOPKGAPPLIB)
APP2DEPN = $(SHL1TARGETN)
APP2NOSAL = TRUE
APP2RPATH = BRAND
.IF "$(OS)" == "WNT"
APP2ICON = $(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP2LINKRES = $(MISC)$/$(TARGET)2.res
.ENDIF

SHL1TARGET = unopkgapp
SHL1OBJS = $(SLOFILES) $(SLO)$/lockfile.obj
SHL1STDLIBS = \
    $(SALLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(VCLLIB) \
    $(DEPLOYMENTMISCLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = \
    $(SLO)$/unopkg_app.obj \
    $(SLO)$/unopkg_cmdenv.obj \
    $(SLO)$/unopkg_misc.obj

OBJFILES = $(OBJ)$/unopkg_main.obj

.INCLUDE : target.mk

.IF "$(APP1TARGETN)" != "" # not set during depend=x
$(APP1TARGETN) : $(MISC)$/binso_created.flg
.ENDIF			# "$(APP1TARGETN)"!=""

$(MISC)$/binso_created.flg:
    @@-$(MKDIRHIER) $(BIN)$/so && $(TOUCH) $@
    @@-$(MKDIRHIER) $(MISC)$/so && $(TOUCH) $@

