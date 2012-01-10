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

# for dummy
TARGET = dynload

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.IF "$(BUILD_FOR_CLI)" != ""

ALLTAR : \
    $(OUT)$/bin$/dynload.exe

CSFILES = \
    dynload.cs				

$(OUT)$/bin$/dynload.exe : $(CSFILES) 
        $(CSC) $(CSCFLAGS) \
        -target:exe \
        -out:$@ \
        -reference:$(OUT)$/bin$/cli_ure.dll \
        -reference:$(OUT)$/bin$/cli_types.dll \
        -reference:$(OUT)$/bin$/cli_cppuhelper.dll \
        -reference:System.dll \
        $(CSFILES) 

#$(OUT)$/bin$/dynload.ini : $(OUT)$/bin$/types.rdb
#	$(GNUCOPY) -p dynload.ini $(OUT)$/bin

#$(OUT)$/bin$/types.rdb $(OUT)$/bin$/services.rdb 

#$(BIN)$/types.rdb : $(SOLARBINDIR)$/types.rdb
#	$(GNUCOPY) -p $? $@



.ENDIF
