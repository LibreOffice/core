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



# Copy *.py files into output tree and call a script once to
# force python to create the *.pyc files.

PRJ=..
TARGET = l10ntools_dummy_pyc

.INCLUDE: settings.mk

.IF "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) $(SOLARBINDIR)/python
.ELSE                   # "$(SYSTEM_PYTHON)"!="YES"
PYTHON=$(AUGMENT_LIBRARY_PATH) $(WRAPCMD) python
.ENDIF                  # "$(SYSTEM_PYTHON)"!="YES"

PYFILES = $(BIN)$/const.py \
          $(BIN)$/l10ntool.py \
          $(BIN)$/pseudo.py \
          $(BIN)$/sdf.py \
          $(BIN)$/xhtex.py \
          $(BIN)$/xtxex.py   

.INCLUDE: target.mk

.IGNORE : create_pyc 
ALLTAR : create_pyc 
create_pyc : $(PYFILES)
.IF "$(GUI)"=="OS2"
    @$(PYTHON) $(BIN)/xtxex.py > /dev/null
.ELSE
    @$(PYTHON) $(BIN)/xtxex.py >& /dev/null
.ENDIF

$(BIN)$/%.py : tool/%.py
    @$(COPY) $< $@


