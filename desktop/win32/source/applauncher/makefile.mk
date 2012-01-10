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
UWINAPILIB=

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

CDEFS+=-DUNICODE


OBJFILES= \
    $(OBJ)$/launcher.obj \
    $(OBJ)$/swriter.obj \
    $(OBJ)$/scalc.obj \
    $(OBJ)$/sdraw.obj \
    $(OBJ)$/simpress.obj \
    $(OBJ)$/sbase.obj \
    $(OBJ)$/smath.obj \
    $(OBJ)$/sweb.obj

# SO launcher
.IF "$(LINK_SO)"=="TRUE"
.IF "$(BUILD_SPECIAL)"!=""
APP1DEPN= $(APP1RES) verinfo.rc
APP1TARGET=so$/swriter
APP1NOSAL=TRUE
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1ICON=$(SOLARRESDIR)$/icons/so9_writer_app.ico
APP1OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/swriter.obj
APP1STDLIBS = $(SHELL32LIB)
APP1VERINFO=verinfo.rc
APP1PRODUCTDEF+=-DRES_APP_NAME=swriter

APP2TARGET=so$/scalc
APP2NOSAL=TRUE
APP2LINKRES=$(MISC)$/$(TARGET)2.res
APP2ICON=$(SOLARRESDIR)$/icons/so9_calc_app.ico
APP2OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/scalc.obj
APP2STDLIBS = $(SHELL32LIB)
APP2DEPN=verinfo.rc
APP2VERINFO=verinfo.rc
APP2PRODUCTDEF+=-DRES_APP_NAME=scalc

APP3TARGET=so$/sdraw
APP3NOSAL=TRUE
APP3LINKRES=$(MISC)$/$(TARGET)3.res
APP3ICON=$(SOLARRESDIR)$/icons/so9_draw_app.ico
APP3OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sdraw.obj
APP3STDLIBS = $(SHELL32LIB)
APP3DEPN=verinfo.rc
APP3VERINFO=verinfo.rc
APP3PRODUCTDEF+=-DRES_APP_NAME=sdraw

APP4TARGET=so$/simpress
APP4NOSAL=TRUE
APP4LINKRES=$(MISC)$/$(TARGET)4.res
APP4ICON=$(SOLARRESDIR)$/icons/so9_impress_app.ico
APP4OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/simpress.obj
APP4STDLIBS = $(SHELL32LIB)
APP4DEPN=verinfo.rc
APP4VERINFO=verinfo.rc
APP4PRODUCTDEF+=-DRES_APP_NAME=simpress

APP5TARGET=so$/sbase
APP5NOSAL=TRUE
APP5LINKRES=$(MISC)$/$(TARGET)5.res
APP5ICON=$(SOLARRESDIR)$/icons/so9_base_app.ico
APP5OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sbase.obj
APP5STDLIBS = $(SHELL32LIB)
APP5DEPN=verinfo.rc
APP5VERINFO=verinfo.rc
APP5PRODUCTDEF+=-DRES_APP_NAME=sbase

APP6TARGET=so$/smath
APP6NOSAL=TRUE
APP6LINKRES=$(MISC)$/$(TARGET)6.res
APP6ICON=$(SOLARRESDIR)$/icons/so9_math_app.ico
APP6OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/smath.obj
APP6STDLIBS = $(SHELL32LIB)
APP6DEPN=verinfo.rc
APP6VERINFO=verinfo.rc
APP6PRODUCTDEF+=-DRES_APP_NAME=smath

APP7TARGET=so$/sweb
APP7NOSAL=TRUE
APP7LINKRES=$(MISC)$/$(TARGET)7.res
APP7ICON=$(SOLARRESDIR)$/icons/so9_writer_app.ico
APP7OBJS = \
        $(OBJ)$/launcher.obj\
        $(OBJ)$/sweb.obj
APP7STDLIBS = $(SHELL32LIB)
APP7DEPN=verinfo.rc
APP7VERINFO=verinfo.rc
APP7PRODUCTDEF+=-DRES_APP_NAME=sweb


.ENDIF # "$(BUILD_SPECIAL)"!=""
.ENDIF # "$(LINK_SO)"=="TRUE"

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

