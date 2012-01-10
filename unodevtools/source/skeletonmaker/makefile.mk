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
PRJNAME := unodevtools

TARGET := uno-skeletonmaker
TARGETTYPE := CUI
LIBTARGET := NO

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk
.INCLUDE : $(PRJ)$/unodevtools.pmk

APP1TARGET = $(TARGET)
APP1RPATH=SDK

APP1OBJS = $(OBJ)$/skeletonmaker.obj \
    $(OBJ)$/skeletoncommon.obj \
    $(OBJ)$/javatypemaker.obj \
    $(OBJ)$/cpptypemaker.obj \
    $(OBJ)$/javacompskeleton.obj \
    $(OBJ)$/cppcompskeleton.obj

APP1DEPN= $(OUT)$/lib$/$(UNODEVTOOLSLIBDEPN) $(SOLARLIBDIR)$/$(CODEMAKERLIBDEPN) \
    $(SOLARLIBDIR)$/$(COMMONCPPLIBDEPN) $(SOLARLIBDIR)$/$(COMMONJAVALIBDEPN)
APP1STDLIBS = $(REGLIB) $(SALLIB) $(SALHELPERLIB) $(CPPULIB) $(CPPUHELPERLIB) \
    $(UNODEVTOOLSLIBST) $(CODEMAKERLIBST) $(COMMONCPPLIBST) $(COMMONJAVALIBST)

OBJFILES = $(APP1OBJS)

.INCLUDE: target.mk
