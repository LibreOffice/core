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
PRJNAME = cli_ure

TARGET = cli_uno
NO_BSYMBOLIC = TRUE
ENABLE_EXCEPTIONS = TRUE
USE_DEFFILE = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk


.IF "$(COM)" == "MSC"
# When compiling for CLR, disable "warning C4339: use of undefined type detected
# in CLR meta-data - use of this type may lead to a runtime exception":
.IF "$(CCNUMVER)" <= "001399999999"
CFLAGSCXX += -clr -AI $(DLLDEST) -AI $(SOLARBINDIR) -wd4339
.ELSE
CFLAGSCXX += -clr:oldSyntax -AI $(DLLDEST) -AI $(SOLARBINDIR) -wd4339
.ENDIF

.IF "$(debug)" != ""
CFLAGS += -Ob0
.ENDIF



.IF "$(CCNUMVER)" <= "001399999999"
#see  Microsoft Knowledge Base Article - 814472
LINKFLAGS += -NOENTRY -NODEFAULTLIB:nochkclr.obj -INCLUDE:__DllMainCRTStartup@12
.ENDIF
# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/cli_environment.obj	\
    $(SLO)$/cli_bridge.obj		\
    $(SLO)$/cli_data.obj		\
    $(SLO)$/cli_proxy.obj		\
    $(SLO)$/cli_uno.obj

SHL1OBJS = $(SLOFILES)

SHL1TARGET = $(TARGET)

SHL1STDLIBS = \
    $(CPPULIB)			\
    $(SALLIB)			\
    mscoree.lib

.IF "$(CCNUMVER)" >= "001399999999"
SHL1STDLIBS += \
    msvcmrt.lib
.ENDIF

SHL1VERSIONMAP = bridge_exports.map

SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
