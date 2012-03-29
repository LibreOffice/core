# *************************************************************
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
# *************************************************************
PRJ=..
PRJNAME=external
TARGET=gcc3_specific

# ------------------------------------------------------------------
.INCLUDE: settings.mk
# ------------------------------------------------------------------


.IF "$(BUILD_SPECIAL)"==""

.IF "$(GUI)" == "WNT"

.IF "$(COM)" == "GCC"

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
MINGWGCCDLL=$(BIN)$/$(MINGW_GCCDLL)
.ENDIF

.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
MINGWGXXDLL=$(BIN)$/$(MINGW_GXXDLL)
.ENDIF

all : $(BIN)$/mingwm10.dll $(MINGWGCCDLL) $(MINGWGXXDLL)

$(BIN)$/mingwm10.dll :
    $(COPY) -p $(COMPATH)$/bin$/mingwm10.dll $(BIN)$/

.IF "$(MINGW_SHARED_GCCLIB)" == "YES"
$(MINGWGCCDLL) :
    $(COPY) -p $(COMPATH)$/bin$/$(MINGW_GCCDLL) $(BIN)$/
.ENDIF

.IF "$(MINGW_SHARED_GXXLIB)" == "YES"
$(MINGWGXXDLL) :
    $(COPY) -p $(COMPATH)$/bin$/$(MINGW_GXXDLL) $(BIN)$/
.ENDIF

.ENDIF

.ELSE

.IF "$(SYSTEM_STDLIBS)" != "YES" && "$(COMID)"=="gcc3"

.IF "$(OS)"!="MACOSX"
.IF "$(OS)"!="OS2"

.EXPORT : CC

all .SEQUENTIAL : $(LB)$/libstdc++.so.$(SHORTSTDCPP3) $(LB)$/libgcc_s.so.$(SHORTSTDC3)


$(LB)$/libstdc++.so.$(SHORTSTDCPP3) :
    $(GCCINSTLIB) libstdc++.so.$(SHORTSTDCPP3) $(LB)

$(LB)$/libgcc_s.so.$(SHORTSTDC3) :
    $(GCCINSTLIB) libgcc_s.so.$(SHORTSTDC3) $(LB)

.ENDIF
.ENDIF 
.ENDIF 

.ENDIF
.ENDIF			# "$(BUILD_SPECIAL)"==""


.INCLUDE: target.mk

