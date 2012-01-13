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

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
SHL1STDLIBS = $(CPPUNITLIB)     \
              $(SALLIB)         \
              $(NEON3RDLIB)     \
              $(CPPULIB)        \
              $(XMLOFFLIB)      \
              $(CPPUHELPERLIB)	\
              $(SVLLIB)			\
              $(TOOLSLIB)	    \
              $(COMPHELPERLIB) \
              $(TESTLIB)

SHL1TARGET = qa_CertExt
SHL1VERSIONMAP = $(PRJ)/qa/certext/export.map
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SLO)/SanCertExt.obj

.INCLUDE: target.mk
.INCLUDE: installationtest.mk

ALLTAR : cpptest

cpptest : $(SHL1TARGETN)

CPPTEST_LIBRARY = $(SHL1TARGETN)

.END
.END
