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

PRJNAME=fpicker
TARGET=fps_gnome.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE=

# ------------------------------------------------------------------

.IF "$(ENABLE_GTK)" != "TRUE"

dummy:
    @echo "Nothing to build. GUIBASE == $(GUIBASE), WITH_WIDGETSET == $(WITH_WIDGETSET)"

.ELSE # we build for GNOME

PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE : pkg_config.mk

# check gtk version
GTK_TWO_FOUR:=$(shell @$(PKG_CONFIG) --exists 'gtk+-2.0 >= 2.4.0' && echo ok)
.IF "$(GTK_TWO_FOUR)" != "ok"

dummy:
    @echo "Cannot build gtk filepicker because" 
    @$(PKG_CONFIG) --print-errors --exists 'gtk+-2.0 >= 2.4.0'

.ELSE

CFLAGS+= $(WIDGETSET_CFLAGS)
CFLAGS+= $(PKGCONFIG_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES =\
        $(SLO)$/SalGtkPicker.obj				\
        $(SLO)$/SalGtkFilePicker.obj			\
        $(SLO)$/SalGtkFolderPicker.obj			\
        $(SLO)$/resourceprovider.obj			\
        $(SLO)$/FPentry.obj

SHL1NOCHECK=TRUE
SHL1TARGET=	$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1STDLIBS=\
    $(VCLLIB) \
    $(TOOLSLIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(PKGCONFIG_LIBS)

.IF "$(OS)"=="SOLARIS"
LINKFLAGSDEFS=
.ENDIF # "$(OS)"=="SOLARIS"

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
DEF1NAME=$(SHL1TARGET)

.ENDIF # GTK_TWO_FOUR
.ENDIF # "$(GUIBASE)" != "unx" || "$(WITH_WIDGETSET)" != "gnome"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/fps_gnome.component

$(MISC)/fps_gnome.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fps_gnome.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fps_gnome.component
