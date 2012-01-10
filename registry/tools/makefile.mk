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



PRJ=..

PRJNAME=registry
TARGET=regmerge
TARGETTYPE=CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

.INCLUDE :  ..$/version.mk

# --- Files --------------------------------------------------------
CDEFS += -DDLL_VERSION=$(EMQ)"$(DLLPOSTFIX)$(EMQ)"

APP1TARGET= $(TARGET)
APP1OBJS=   $(OBJ)$/regmerge.obj $(OBJ)/fileurl.obj $(OBJ)/options.obj
APP1RPATH=  UREBIN

APP1STDLIBS=\
            $(SALLIB) \
            $(REGLIB)

APP2TARGET= regview
APP2OBJS=   $(OBJ)$/regview.obj $(OBJ)/fileurl.obj
APP2RPATH=  UREBIN

APP2STDLIBS=\
            $(SALLIB) \
            $(REGLIB)

APP3TARGET= regcompare
APP3OBJS=   $(OBJ)$/regcompare.obj $(OBJ)/fileurl.obj $(OBJ)/options.obj
APP3RPATH=  SDK

APP3STDLIBS=\
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(REGLIB)

APP4TARGET= checksingleton
APP4OBJS=   $(OBJ)$/checksingleton.obj $(OBJ)/fileurl.obj $(OBJ)/options.obj

APP4STDLIBS=\
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(REGLIB)

APP5TARGET= rdbedit
APP5OBJS=   $(OBJ)$/rdbedit.obj

APP5STDLIBS=\
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(REGLIB)

OBJFILES = $(APP1OBJS) $(APP2OBJS) $(APP3OBJS) $(APP4OBJS) $(APP5OBJS)

.INCLUDE :  target.mk
