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

PRJNAME=dtrans
TARGET=dnd
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
USE_BOUNDCHK=

.IF "$(USE_BOUNDCHK)"=="TR"
bndchk=tr
stoponerror=tr
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

#CFLAGS+=-GR -DUNICODE -D_UNICODE

#DBG_CONSOLE_OUT: when IDropTarget or IDropSource are called we write to a console
#DBG_CLIPBOARD_DATA: To provide DoDragDrop with an data object we call OleGetClipboard
#CFLAGS+= -DDBG_CONSOLE_OUT -DDBG_CLIPBOARD_DATA


SLOFILES=	$(SLO)$/dndentry.obj	\
            $(SLO)$/target.obj		\
            $(SLO)$/idroptarget.obj	\
            $(SLO)$/source.obj		\
            $(SLO)$/globals.obj		\
            $(SLO)$/targetdropcontext.obj	\
            $(SLO)$/targetdragcontext.obj	\
            $(SLO)$/sourcecontext.obj


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
