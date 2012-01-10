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
TARGET=applauncher
LIBTARGET=NO
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES= \
    $(OBJ)$/launcher.obj \
    $(OBJ)$/swriter.obj \
    $(OBJ)$/scalc.obj \
    $(OBJ)$/sdraw.obj \
    $(OBJ)$/simpress.obj \
    $(OBJ)$/sbase.obj \
    $(OBJ)$/smath.obj \
    $(OBJ)$/officeloader.obj \
    $(OBJ)$/os2quickstart.obj

APP1TARGET=swriter
APP1NOSAL=TRUE
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1ICON=$(SOLARRESDIR)$/icons$/ooo-writer-app.ico
APP1OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj


APP2TARGET=scalc
APP2NOSAL=TRUE
APP2LINKRES=$(MISC)$/$(TARGET)2.res
APP2ICON=$(SOLARRESDIR)$/icons$/ooo-calc-app.ico
APP2OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj

APP3TARGET=sdraw
APP3NOSAL=TRUE
APP3LINKRES=$(MISC)$/$(TARGET)3.res
APP3ICON=$(SOLARRESDIR)$/icons$/ooo-draw-app.ico
APP3OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj

APP4TARGET=simpress
APP4NOSAL=TRUE
APP4LINKRES=$(MISC)$/$(TARGET)4.res
APP4ICON=$(SOLARRESDIR)$/icons$/ooo-impress-app.ico
APP4OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj

APP5TARGET=sbase
APP5NOSAL=TRUE
APP5LINKRES=$(MISC)$/$(TARGET)5.res
APP5ICON=$(SOLARRESDIR)$/icons$/ooo-base-app.ico
APP5OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sbase.obj

APP6TARGET=smath
APP6NOSAL=TRUE
APP6LINKRES=$(MISC)$/$(TARGET)6.res
APP6ICON=$(SOLARRESDIR)$/icons$/ooo-math-app.ico
APP6OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj

APP7TARGET=officeloader
APP7NOSAL=TRUE
APP7LINKRES=$(MISC)$/$(TARGET)7.res
APP7ICON=$(SOLARRESDIR)$/icons$/ooo-main-app.ico
APP7OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/officeloader.obj

APP8TARGET=os2quickstart
APP8NOSAL=TRUE
APP8LINKRES=$(MISC)$/$(TARGET)8.res
APP8ICON=$(SOLARRESDIR)$/icons$/ooo-main-app.ico
APP8OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/os2quickstart.obj

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

