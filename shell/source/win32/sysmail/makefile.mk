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
PRJNAME=shell
TARGET=sysmail
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/sysmailentry.obj\
         $(SLO)$/sysmailmsg.obj\
         $(SLO)$/sysmailclient.obj\
         $(SLO)$/sysmailprov.obj
    
SHL1TARGET=$(TARGET).uno

SHL1STDLIBS=$(CPPULIB)\
            $(CPPUHELPERLIB)\
            $(SALLIB)\
            $(ADVAPI32LIB)

SHL1IMPLIB=i$(SHL1TARGET)

SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- mapimailer --------------------------------------------------------

TARGETTYPE=CUI

OBJFILES=   $(OBJ)$/senddoc.obj\
            $(OBJ)$/sysmapi.obj

APP1TARGET=senddoc
APP1OBJS=$(OBJFILES)
APP1STDLIBS=$(KERNEL32LIB)\
            $(SALLIB)

APP1DEF=$(MISC)$/$(APP1TARGET).def

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/sysmail.component

$(MISC)/sysmail.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sysmail.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sysmail.component
