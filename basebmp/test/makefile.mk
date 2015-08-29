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

PRJNAME=basebmp
TARGET=tests

ENABLE_EXCEPTIONS=TRUE

.IF "$(ENABLE_UNIT_TESTS)" != "YES"

@all:
    @echo "unit tests are disabled. Nothing do do."

.ELSE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(debug)"!="" || "$(DEBUG)"!=""

.IF "$(COM)"=="MSC"
# disable inlining for MSVC
CFLAGS += -Ob0
.ENDIF

.IF "$(COM)"=="GCC"
# disable inlining for gcc
CFLAGS += -fno-inline
.ENDIF

.ENDIF

# SunStudio 12 (-m64 and -m32 modes): three test cases of the unit tests fail
# if compiled with default -xalias_level (and optimization level -xO3)
.IF "$(OS)"=="SOLARIS"
# For Sun Studio 8 this switch does not work: compilation fails on bitmapdevice.cxx
.IF "$(CCNUMVER)"!="00050005"
CDEFS+=-xalias_level=compatible
.ENDIF
.ENDIF

# --- Common ----------------------------------------------------------
.IF "$(L10N_framework)"==""

# BEGIN ----------------------------------------------------------------
# auto generated Target:tests by codegen.pl
APP1OBJS=  \
    $(SLO)$/basictest.obj		\
    $(SLO)$/bmpmasktest.obj		\
    $(SLO)$/bmptest.obj		    \
    $(SLO)$/cliptest.obj		\
    $(SLO)$/filltest.obj		\
    $(SLO)$/linetest.obj		\
    $(SLO)$/main.obj		\
    $(SLO)$/masktest.obj		\
    $(SLO)$/polytest.obj		\
    $(SLO)$/tools.obj
APP1TARGET= tests
APP1STDLIBS=			$(BASEBMPLIB) \
                $(SALLIB)     \
                $(GTESTLIB) \
                $(BASEGFXLIB)
APP1RPATH = NONE
APP1TEST  = enabled

.ENDIF
# END ------------------------------------------------------------------

#APP2TARGET= bmpdemo

#APP2OBJS=	\
#	$(OBJ)$/bmpdemo.obj

#APP2STDLIBS=$(TOOLSLIB) 		\
#			$(COMPHELPERLIB)	\
#			$(BASEGFXLIB)	    \
#			$(BASEBMPLIB)	    \
#			$(CPPULIB)			\
#			$(CPPUHELPERLIB)	\
#			$(UCBHELPERLIB)		\
#			$(SALLIB)			\
#			$(VCLLIB)
#
#.IF "$(GUI)"!="UNX"
#APP2DEF=	$(MISC)$/$(TARGET).def
#.ENDIF


# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

# --- Enable test execution in normal build ------------------------

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
