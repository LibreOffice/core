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



PRJ=..$/..

PRJNAME=desktop
TARGET=dkt
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : ../deployment/inc/dp_misc.mk

.IF "$(ENABLE_GNOMEVFS)"=="TRUE"
CFLAGS+=-DGNOME_VFS_ENABLED
.ENDIF

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2" || "$(GUIBASE)"=="aqua" || "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
CFLAGS+=-DENABLE_QUICKSTART_APPLET
.ENDIF

# .IF "$(OS)" == "WNT"
# .IF "$(COM)" == "GCC"
# DEPLOYMENTMISCLIB = -ldeploymentmisc$(DLLPOSTFIX)
# .ELSE
# DEPLOYMENTMISCLIB = ideploymentmisc$(DLLPOSTFIX).lib
# .ENDIF
# .ELIF "$(OS)" == "OS2"
# DEPLOYMENTMISCLIB = ideploymentmisc$(DLLPOSTFIX).lib
# .ELSE
# DEPLOYMENTMISCLIB = -ldeploymentmisc$(DLLPOSTFIX)
# .ENDIF

.IF "$(GUI)"=="WNT" || "$(GUI)"=="OS2" || "$(GUIBASE)"=="aqua" || "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
CFLAGS+=-DENABLE_QUICKSTART_APPLET
.ENDIF

SHL1TARGET = sofficeapp
SHL1OBJS = \
    $(SLO)$/app.obj \
    $(SLO)$/appfirststart.obj \
    $(SLO)$/appinit.obj \
    $(SLO)$/appsys.obj \
    $(SLO)$/checkinstall.obj \
    $(SLO)$/check_ext_deps.obj \
    $(SLO)$/cmdlineargs.obj \
    $(SLO)$/cmdlinehelp.obj \
    $(SLO)$/configinit.obj \
    $(SLO)$/desktopcontext.obj \
    $(SLO)$/desktopresid.obj \
    $(SLO)$/dispatchwatcher.obj \
    $(SLO)$/langselect.obj \
    $(SLO)$/lockfile.obj \
    $(SLO)$/lockfile2.obj \
    $(SLO)$/officeipcthread.obj \
    $(SLO)$/sofficemain.obj \
    $(SLO)$/userinstall.obj

SHL1LIBS = $(SLB)$/mig.lib

SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(DEPLOYMENTMISCLIB) \
    $(I18NISOLANGLIB) \
    $(SALLIB) \
    $(SFXLIB) \
    $(SVLLIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(VCLLIB) \
    $(VOSLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

OBJFILES = \
    $(OBJ)$/copyright_ascii_ooo.obj \
    $(OBJ)$/main.obj
.IF "$(GUI)" != "OS2"
OBJFILES += \
    $(OBJ)$/copyright_ascii_sun.obj
.ENDIF

SLOFILES = $(SHL1OBJS)

SRS1NAME=	desktop
SRC1FILES=	desktop.src

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

