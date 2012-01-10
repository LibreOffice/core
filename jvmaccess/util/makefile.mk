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
PRJNAME = jvmaccess
TARGET = $(PRJNAME)

ENABLE_EXCEPTIONS = TRUE

.IF "$(OS)" != "WNT" && "$(OS)" != "OS2"
UNIXVERSIONNAMES = UDK
.ENDIF # WNT

.INCLUDE: settings.mk

.IF "$(UNIXVERSIONNAMES)" == ""
SHL1TARGET = $(TARGET)$(UDK_MAJOR)$(COMID)
.ELSE # UNIXVERSIONNAMES
SHL1TARGET = $(TARGET)$(COMID)
.ENDIF # UNIXVERSIONNAMES

SHL1IMPLIB = i$(TARGET)
SHL1LIBS = $(SLB)$/$(TARGET).lib
SHL1STDLIBS = $(CPPULIB) $(SALLIB) $(SALHELPERLIB)
.IF "$(OS)" == "WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF # WNT
SHL1RPATH = URELIB

.IF "$(COMNAME)" == "msci"
SHL1VERSIONMAP = msvc_win32_intel.map
.ELIF "$(COMNAME)" == "sunpro5"
SHL1VERSIONMAP = cc5_solaris_sparc.map
.ELIF "$(GUI)$(COM)" == "WNTGCC"
SHL1VERSIONMAP = mingw.map
.ELIF "$(COMNAME)" == "gcc3"
SHL1VERSIONMAP = gcc3.map
.ENDIF

DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
