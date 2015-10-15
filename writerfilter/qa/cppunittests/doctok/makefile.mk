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

PRJNAME=writerfilter
TARGET=test-doctok

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE

# BEGIN ----------------------------------------------------------------
# auto generated Target:doctok by codegen.pl
APP1OBJS=  \
       $(SLO)$/testdoctok.obj

APP1TARGET= testdoctok

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC" || "$(GUI)"=="OS2"
RESOURCEMODELLIB=-lresourcemodel
.ELIF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
RESOURCEMODELLIB=-lresourcemodel
.ELSE
RESOURCEMODELLIB=$(LB)$/iresourcemodel.lib
.ENDIF
.ENDIF

APP1STDLIBS=\
    $(SALLIB) \
    $(UCBHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(RESOURCEMODELLIB) \
    $(GTESTLIB)

.IF "$(GUI)"=="WNT"
APP1STDLIBS+=   $(LB)$/doctok.lib
.ELIF "$(GUI)"=="UNX" && "$(GUIBASE)"!="aqua"
APP1STDLIBS+=$(LB)$/libdoctok.so
.ELIF "$(GUI)"=="UNX" && "$(GUIBASE)"=="aqua"
APP1STDLIBS+=$(LB)$/libdoctok.dylib
.ENDIF

APP1LIBS=\
    $(SLB)$/doctok.lib
APP1DEPS= \
    $(APP1LIBS)
APP1RPATH = NONE
APP1TEST = enabled
# auto generated Target:doctok
# END ------------------------------------------------------------------

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
