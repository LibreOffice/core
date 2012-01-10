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

PRJNAME=bridges
TARGET=java_uno
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building jni-uno bridge because Java is disabled"
.ENDIF

# --- Files --------------------------------------------------------

.IF "$(GUI)$(COM)" == "WNTGCC"
.IF "$(EXCEPTIONS)" == "sjlj"
CFLAGS += -DBROKEN_ALLOCA
.ENDIF
.ENDIF

SLOFILES= \
    $(SLO)$/jni_info.obj		\
    $(SLO)$/jni_data.obj		\
    $(SLO)$/jni_uno2java.obj	\
    $(SLO)$/jni_java2uno.obj	\
    $(SLO)$/jni_bridge.obj \
    $(SLO)$/nativethreadpool.obj 

SHL1TARGET=$(TARGET)

SHL1STDLIBS= \
    $(JVMACCESSLIB)			\
    $(CPPULIB)			\
    $(SALLIB) \
    $(SALHELPERLIB)

.IF "$(GUI)" != "OS2"
SHL1VERSIONMAP=$(TARGET).map
.ENDIF

SHL1CREATEJNILIB=TRUE
SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
SHL1RPATH=URELIB

.IF "$(debug)" != ""
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

