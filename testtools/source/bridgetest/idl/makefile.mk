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



PRJ := ..$/..$/..
PRJNAME := testtools
TARGET := bridgetest_idl

.INCLUDE: settings.mk
.INCLUDE: target.mk
.IF "$(L10N_framework)"==""
ALLTAR: $(MISC)$/$(TARGET).cppumaker.done $(MISC)$/$(TARGET).javamaker.done

$(MISC)$/$(TARGET).cppumaker.done: $(BIN)$/bridgetest.rdb
    $(CPPUMAKER) -O$(INCCOM) -BUCR $< -X$(SOLARBINDIR)/types.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).javamaker.done: $(BIN)$/bridgetest.rdb
    $(JAVAMAKER) -O$(CLASSDIR) -BUCR -nD -X$(SOLARBINDIR)/types.rdb $<
    $(TOUCH) $@

$(BIN)$/bridgetest.rdb: bridgetest.idl
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<
    - $(RM) $@
    $(REGMERGE) $@ /UCR $(MISC)$/$(TARGET)$/bridgetest.urd

.IF "$(GUI)" == "WNT"
.IF "$(COM)" != "GCC"

CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF

ALLTAR: $(MISC)$/$(TARGET).cppumaker.done \
    $(MISC)$/$(TARGET).javamaker.done \
    $(BIN)$/cli_types_bridgetest.dll 

$(BIN)$/cli_types_bridgetest.dll: $(BIN)$/bridgetest.rdb
    $(CLIMAKER) $(CLIMAKERFLAGS) --out $@ -r $(SOLARBINDIR)$/cli_uretypes.dll \
        -X $(SOLARBINDIR)$/types.rdb $< 
    $(TOUCH) $@

.ENDIF
.ENDIF # GUI, WNT
.ENDIF # L10N_framework

