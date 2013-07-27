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

PRJNAME=shell
TARGET=testsyssh
TARGET1=testsmplmail
TARGET2=testprx
TARGET4=testfopen
LIBTARGET=NO
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CFLAGS+=/GX

OBJFILES1=$(OBJ)$/TestSysShExec.obj
OBJFILES2=$(OBJ)$/TestSmplMail.obj
OBJFILES4=$(OBJ)$/Testfopen.obj

OBJFILES=$(OBJFILES1)\
         $(OBJFILES2)

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES1)

APP1STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB) 	 		\
                $(USER32LIB)

APP1DEF=		$(MISC)$/$(APP1TARGET).def


# --- TestSmplMail ---

APP2TARGET=$(TARGET1)
APP2OBJS=$(OBJFILES2)

APP2STDLIBS+=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SALLIB) 	 		\
                $(USER32LIB)

APP2DEF=		$(MISC)$/$(APP2TARGET).def

# --- Testfopen ---

APP4TARGET=$(TARGET4)
APP4OBJS=$(OBJFILES4)

APP4STDLIBS+=	$(SALLIB)

APP4DEF=		$(MISC)$/$(APP4TARGET).def

# --- Targets ------------------------------------------------------

.INCLUDE :		target.mk


