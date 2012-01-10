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

PRJNAME=		dtrans
TARGET=			testwincb
TARGET1=		testmshl
LIBTARGET=		NO
TARGETTYPE=		CUI
USE_BOUNDCHK=
TESTCB=TRUE

.IF "$(USE_BOUNDCHK)"=="TR"
bndchk=tr
stoponerror=tr
.ENDIF

# --- Settings -----------------------------------------------------
#.INCLUDE :		$(PRJ)$/util$/makefile.pmk

.INCLUDE :  settings.mk

.IF "$(TESTCB)"=="TRUE"

CFLAGS+=-D_WIN32_DCOM -EHsc -Ob0

# --- Files --------------------------------------------------------

OBJFILES=   	$(OBJ)$/test_wincb.obj
APP1TARGET=		$(TARGET)
APP1OBJS=		$(OBJ)$/test_wincb.obj
                
                
APP1STDLIBS=	$(SALLIB) \
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(SOLARLIBDIR)$/uwinapi.lib\
                $(USER32LIB) \
                $(OLE32LIB)\
                $(COMDLG32LIB)

APP1LIBS= $(SLB)$/dtutils.lib
          

APP1NOSAL=  TRUE

.ENDIF

.IF "$(TESTCB)"==""

CFLAGS+=/D_WIN32_DCOM /EHsc /Ob0

OBJFILES=   	$(OBJ)$/testmarshal.obj
APP1TARGET=		$(TARGET1)
APP1OBJS=		$(OBJ)$/testmarshal.obj
                
APP1STDLIBS=	$(SALLIB)\
                $(USER32LIB)\
                $(OLE32LIB)\
                comsupp.lib\
                $(OLEAUT32LIB)

APP1LIBS= 

APP1NOSAL=  TRUE

.ENDIF

# --- Targets ------------------------------------------------------
.INCLUDE :		target.mk
#.INCLUDE :		$(PRJ)$/util$/target.pmk
