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
PRJNAME=filter
TARGET=fl_javafilter_binaries

.INCLUDE: settings.mk

all: \
    $(BIN)$/docbooktosoffheadings.xsl   \
    $(BIN)$/sofftodocbookheadings.xsl   \
    $(BIN)$/DocBookTemplate.stw
    


$(OUT)$/bin$/docbooktosoffheadings.xsl: docbooktosoffheadings.xsl
         $(COPY) docbooktosoffheadings.xsl $(OUT)$/bin$/docbooktosoffheadings.xsl

$(OUT)$/bin$/sofftodocbookheadings.xsl: sofftodocbookheadings.xsl
         $(COPY) sofftodocbookheadings.xsl $(OUT)$/bin$/sofftodocbookheadings.xsl

$(OUT)$/bin$/DocBookTemplate.stw: DocBookTemplate.stw
         $(COPY) DocBookTemplate.stw $(OUT)$/bin$/DocBookTemplate.stw

.INCLUDE: target.mk

