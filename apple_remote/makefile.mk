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



PRJ=.

PRJNAME=external
TARGET=AppleRemote

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="aqua"

SHL1STDLIBS+= \
    -framework Cocoa -framework Carbon -framework IOKit

LIB1FILES+= \
        $(SLB)$/AppleRemote.lib

SLOFILES=	\
        $(SLO)$/AppleRemote.obj 		\
        $(SLO)$/RemoteControl.obj		\
        $(SLO)$/RemoteControlContainer.obj	\
        $(SLO)$/GlobalKeyboardDevice.obj	\
        $(SLO)$/HIDRemoteControlDevice.obj	\
        $(SLO)$/MultiClickRemoteBehavior.obj	\
        $(SLO)$/RemoteMainController.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1OBJS= $(SLOFILES)

OUT2INC = \
        $(BUILDDIR)$/AppleRemote.h		\
        $(BUILDDIR)$/RemoteControl.h		\
        $(BUILDDIR)$/RemoteControlContainer.h	\
        $(BUILDDIR)$/GlobalKeyboardDevice.h	\
        $(BUILDDIR)$/HIDRemoteControlDevice.h	\
        $(BUILDDIR)$/MultiClickRemoteBehavior.h	\
        $(BUILDDIR)$/RemoteMainController.h


.ENDIF		# "$(GUIBASE)"!="aqua"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
