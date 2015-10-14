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


.IF "$(ENABLE_NSS_MODULE)" == "NO"
nothing .PHONY:
.ELSE
.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE 

PRJ = ../..
PRJNAME = xmlsecurity
TARGET = qa_certext

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
    @echo unit tests are disabled. Nothing to do.

.ELSE

APP1OBJS = $(SLO)/SanCertExt.obj
APP1RPATH = NONE
APP1STDLIBS = $(GTESTLIB)     \
              $(SALLIB)         \
              $(NEON3RDLIB)     \
              $(CPPULIB)        \
              $(XMLOFFLIB)      \
              $(CPPUHELPERLIB)	\
              $(SVLLIB)			\
              $(TOOLSLIB)	    \
              $(COMPHELPERLIB) \
              $(TESTLIB)

APP1TARGET = qa_CertExt
APP1TEST = enabled

.INCLUDE: target.mk
.INCLUDE: installationtest.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
.ENDIF # "$(OOO_SUBSEQUENT_TESTS)" == ""
.ENDIF # "$(ENABLE_NSS_MODULE)" == "NO"
