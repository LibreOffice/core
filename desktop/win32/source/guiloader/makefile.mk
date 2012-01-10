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

PRJNAME=desktop
TARGET=guiloader
LIBTARGET=NO
TARGETTYPE=GUI
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

APP1TARGET=guiloader
APP1NOSAL=TRUE
APP1ICON=$(SOLARRESDIR)$/icons/ooo-main-app.ico
APP1OBJS=\
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/genericloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB1=$(SHLWAPILIB)

.IF "$(LINK_SO)"=="TRUE"
APP2TARGET=so$/guiloader
APP2NOSAL=TRUE
APP2ICON=$(SOLARRESDIR)$/icons/so9_main_app.ico
APP2OBJS=\
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/genericloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB2=$(SHLWAPILIB)
.ENDIF # "$(LINK_SO)"=="TRUE"

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

