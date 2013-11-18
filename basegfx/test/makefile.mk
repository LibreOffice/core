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



PRJ=..

PRJNAME=basegfx
TARGET=tests

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(WITH_CPPUNIT)" != "YES"

@all:
    @echo "cppunit disabled. nothing do do."

.ELSE

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# --- Common ----------------------------------------------------------

SHL1OBJS=  \
    $(SLO)$/basegfx1d.obj      \
    $(SLO)$/basegfx2d.obj      \
    $(SLO)$/basegfx3d.obj      \
    $(SLO)$/boxclipper.obj     \
    $(SLO)$/basegfxtools.obj   \
    $(SLO)$/clipstate.obj      \
    $(SLO)$/genericclipper.obj \
    $(SLO)$/testtools.obj	

SHL1TARGET= basegfx_tests
SHL1STDLIBS= \
                $(BASEGFXLIB) \
                $(SALLIB)        \
                $(CPPUHELPERLIB) \
                $(CPPULIB)       \
                $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map
SHL1RPATH = NONE

# END ------------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.ENDIF # "$(WITH_CPPUNIT)" != "YES"

.INCLUDE : target.mk
.INCLUDE : _cppunit.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF
