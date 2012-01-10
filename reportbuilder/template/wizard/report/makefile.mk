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



PRJ=..$/..$/..

PRJNAME=reportbuilder
TARGET=template
PACKAGE=

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Targets ------------------------------------------------------

XCUFILES=

MODULEFILES=

ODR_FILES_IN_REPORT_BUILDER_EXT=\
    default.odr

ODR_FILES_IN_TEMPLATE_EXT=\
    finance.odr \
    finance_2.odr \
    yellow.odr

ALLTAR: COPYFILES COPYFILES2

.INCLUDE :  target.mk

#-------------------------------------------------------------------------------

TARGETDIR=$(MISC)$/template$/wizard$/report

COPYFILES: $(ODR_FILES_IN_REPORT_BUILDER_EXT) $(XCU_FILES)
    @@-$(MKDIRHIER) $(TARGETDIR)
    $(COPY) $(ODR_FILES_IN_REPORT_BUILDER_EXT) $(TARGETDIR)

#-------------------------------------------------------------------------------

TARGETDIR2=$(MISC)$/templates$/template$/wizard$/report

COPYFILES2: $(ODR_FILES_IN_TEMPLATE_EXT) $(XCU_FILES)
    @@-$(MKDIRHIER) $(TARGETDIR2)
    $(COPY) $(ODR_FILES_IN_TEMPLATE_EXT) $(TARGETDIR2)

