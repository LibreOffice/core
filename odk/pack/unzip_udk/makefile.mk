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
PRJNAME=odk
TARGET=unzip_udk

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

all: ..$/misc$/deltree.txt

..$/misc$/deltree.txt .SETDIR=$(OUT)$/bin : $(PRJ)$/pack$/unzip_udk$/deltree.txt
    @@-$(MY_DELETE_RECURSIVE) $(PRODUCT_NAME)
    @@-rm -f $(PRODUCT_NAME).zip
    @@-$(MY_DELETE_RECURSIVE) $(PRODUCT_NAME2)
    @@-rm -f odkidl.zip
    @@-rm -f odk_ooidl.zip
    @echo "" > ..$/misc$/deltree.txt

