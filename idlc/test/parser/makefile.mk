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



PRJ := ..$/..
PRJNAME := idlc
TARGET := test_parser

.INCLUDE: settings.mk
.INCLUDE: target.mk

ALLTAR: test

test .PHONY:
    $(EXECTEST) attribute.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) constant.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) constructor.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) interfaceinheritance.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) methodoverload.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) polystruct.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) published.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) struct.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
    $(EXECTEST) typedef.tests $(BIN)/idlc$(EXECPOST) \
        -O $(MISC)/$(TARGET) -stdin
