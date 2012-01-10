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
PRJNAME := registry
TARGET := test_regcompare

.INCLUDE: settings.mk
.INCLUDE: target.mk

ALLTAR: test

test .PHONY: $(MISC)$/$(TARGET)$/psa.urd $(MISC)$/$(TARGET)$/psb.urd \
        $(MISC)$/$(TARGET)$/usa.urd $(MISC)$/$(TARGET)$/usb.urd \
        $(MISC)$/$(TARGET)$/pe.urd $(MISC)$/$(TARGET)$/ue.urd \
        $(MISC)$/$(TARGET)$/other1.urd $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/psa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/psb.urd
    $(EXECTEST) -SUCCESS $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usa.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/usb.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/pe.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/ue.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other1.urd
    $(EXECTEST) -FAILURE $(BIN)/regcompare$(EXECPOST) -f -t -u \
        -r1 $(MISC)$/$(TARGET)$/usa.urd -r2 $(MISC)$/$(TARGET)$/other2.urd

$(MISC)$/$(TARGET)$/%.urd: %.idl
    $(IDLC) -O$(MISC)$/$(TARGET) -cid -we $<
