#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..

PRJNAME=desktop
TARGET=soffice
TARGETTYPE=GUI
LIBTARGET=NO
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

UWINAPILIB =

VERINFONAME=verinfo

# --- Resourcen ----------------------------------------------------

.IF "$(GUI)" == "WNT"
RCFILES=verinfo.rc
.ENDIF
.IF "$(GUI)" == "OS2"
RCFILES=ooverinfo2.rc
.ENDIF

# --- Linken der Applikation ---------------------------------------

.IF "$(OS)" == "MACOSX"
LINKFLAGSAPPGUI!:=	$(LINKFLAGSAPPGUI:s/-bind_at_load//)
.ENDIF # MACOSX

RESLIB1NAME=		dkt
RESLIB1IMAGES=		$(PRJ)$/res
RESLIB1SRSFILES=	$(SRS)$/desktop.srs

.IF "$(L10N_framework)"==""
.IF "$(LINK_SO)"=="TRUE"
.IF "$(GUI)" != "OS2"
APP1TARGET=so$/$(TARGET)
APP1NOSAL=TRUE
APP1RPATH=BRAND
APP1OBJS=$(OBJ)$/copyright_ascii_sun.obj $(OBJ)$/main.obj
APP1STDLIBS = $(SALLIB) $(SOFFICELIB)
APP1DEPN= $(APP1RES) verinfo.rc

.IF "$(GUI)" == "WNT"
APP1RES=    $(RES)$/desktop.res
APP1ICON=$(SOLARRESDIR)$/icons/so9_main_app.ico
APP1VERINFO=verinfo.rc
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1STACK=10000000

.ENDIF # WNT

.ENDIF # "$(GUI)" != "OS2"

.ENDIF # "$(LINK_SO)"=="TRUE"

APP5TARGET=soffice
APP5NOSAL=TRUE
APP5RPATH=BRAND
APP5OBJS=$(OBJ)$/copyright_ascii_ooo.obj $(OBJ)$/main.obj
APP5STDLIBS = $(SALLIB) $(SOFFICELIB)
.IF "$(OS)" == "LINUX"
APP5STDLIBS+= -lXext -lX11
.ENDIF # LINUX

APP5DEPN= $(APP1TARGETN) $(APP5RES) ooverinfo.rc
APP5DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP5RES=    $(RES)$/oodesktop.res
APP5ICON=$(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP5VERINFO=ooverinfo.rc
APP5LINKRES=$(MISC)$/ooffice5.res
APP5STACK=10000000
.ENDIF # WNT

.IF "$(GUI)" == "OS2"
APP5DEF= # automatic
APP5RES=    $(RES)$/oodesktop.res
APP5ICON=$(SOLARRESDIR)$/icons/ooo-main-app.ico
APP5VERINFO=ooverinfo2.rc
APP5LINKRES=$(MISC)$/ooffice.res
.ENDIF # OS2

.IF "$(GUI)" == "WNT"
.IF "$(LINK_SO)"=="TRUE"
APP6TARGET=so$/officeloader
APP6RES=$(RES)$/soloader.res
APP6NOSAL=TRUE
APP6DEPN= $(APP1TARGETN) $(APP6RES) verinfo.rc
APP6VERINFO=verinfo.rc
APP6LINKRES=$(MISC)$/soffice6.res
APP6ICON=$(SOLARRESDIR)$/icons/so9_main_app.ico
APP6OBJS = \
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/officeloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB6=$(ADVAPI32LIB) $(SHELL32LIB) $(SHLWAPILIB)
.ENDIF # "$(LINK_SO)"=="TRUE"

APP7TARGET=officeloader
APP7RES=$(RES)$/ooloader.res
APP7NOSAL=TRUE
APP7DEPN= $(APP1TARGETN) $(APP7RES) ooverinfo.rc
APP7VERINFO=ooverinfo.rc
APP7LINKRES=$(MISC)$/ooffice7.res
APP7ICON=$(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP7OBJS = \
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/officeloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB7=$(ADVAPI32LIB) $(SHELL32LIB) $(SHLWAPILIB)
.ELIF "$(OS)" == "MACOSX"
APP6TARGET=officeloader
APP6NOSAL=TRUE
APP6RPATH=BRAND
APP6OBJS=$(OBJ)$/copyright_ascii_ooo.obj $(OBJ)$/officeloader.obj
APP6STDLIBS = $(SALLIB)
APP5DEPN= $(APP1TARGETN) $(APP5RES) ooverinfo.rc
APP5DEF=    $(MISCX)$/$(TARGET).def
.ENDIF # WNT

.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(L10N_framework)"==""

.IF "$(APP1TARGETN)"!=""
$(APP1TARGETN) :  $(MISC)$/binso_created.flg
.ENDIF			# "$(APP1TARGETN)"!=""

.IF "$(APP5TARGETN)"!=""
$(APP5TARGETN) :  $(MISC)$/binso_created.flg
.ENDIF			# "$(APP6TARGETN)"!=""

.IF "$(APP6TARGETN)"!=""
$(APP6TARGETN) :  $(MISC)$/binso_created.flg
.ENDIF			# "$(APP6TARGETN)"!=""

.IF "$(GUI)" == "WNT"
ALLTAR: $(MISC)$/$(TARGET).exe.manifest
ALLTAR: $(MISC)$/$(TARGET).bin.manifest
ALLTAR: $(BIN)$/$(TARGET).bin
.IF "$(LINK_SO)"=="TRUE"
ALLTAR: $(BIN)$/so$/$(TARGET).bin
.ENDIF # "$(LINK_SO)"=="TRUE"
.ENDIF # WNT

.IF "$(GUI)" == "OS2"
ALLTAR: $(BIN)$/$(TARGET).bin
.ENDIF # OS2

$(BIN)$/soffice_oo$(EXECPOST) : $(APP5TARGETN)
    $(COPY) $< $@

.IF "$(GUI)" != "OS2"
.IF "$(LINK_SO)"=="TRUE"
$(BIN)$/so$/soffice_so$(EXECPOST) : $(APP1TARGETN)
    $(COPY) $< $@

ALLTAR : $(BIN)$/so$/soffice_so$(EXECPOST)
.ENDIF # "$(LINK_SO)"=="TRUE"
ALLTAR : $(BIN)$/soffice_oo$(EXECPOST)
.ENDIF

.IF "$(OS)" == "MACOSX"
.IF "$(LINK_SO)"=="TRUE"
$(BIN)$/so$/soffice_mac$(EXECPOST) : $(APP1TARGETN)
    $(COPY) $< $@
    
ALLTAR : $(BIN)$/so$/soffice_mac$(EXECPOST)
.ENDIF # "$(LINK_SO)"=="TRUE"

$(BIN)$/soffice_mac$(EXECPOST) : $(APP5TARGETN)
    $(COPY) $< $@

ALLTAR : $(BIN)$/soffice_mac$(EXECPOST)

.ENDIF # "$(OS)" == "MACOSX"

.IF "$(GUI)" == "WNT"

# create a manifest file with the same name as the
# office executable file soffice.exe.manifest
.IF "$(CCNUMVER)" <= "001399999999"
$(MISC)$/$(TARGET).exe.manifest: template.manifest
   $(COPY) $< $@
.ELSE
$(MISC)$/$(TARGET).exe.template.manifest: template.manifest
   $(COPY) $< $@

$(MISC)$/$(TARGET).exe.linker.manifest: $(BIN)$/$(TARGET)$(EXECPOST)
   mt.exe -inputresource:$(BIN)$/$(TARGET)$(EXECPOST) -out:$@

$(MISC)$/$(TARGET).exe.manifest: $(MISC)$/$(TARGET).exe.template.manifest $(MISC)$/$(TARGET).exe.linker.manifest
   mt.exe -manifest $(MISC)$/$(TARGET).exe.linker.manifest $(MISC)$/$(TARGET).exe.template.manifest -out:$@
.ENDIF

# create a manifest file with the same name as the
# office executable file soffice.bin.manifest
.IF "$(CCNUMVER)" <= "001399999999"
$(MISC)$/$(TARGET).bin.manifest: template.manifest
   $(COPY) $< $@
.ELSE
$(MISC)$/$(TARGET).bin.manifest: $(MISC)$/$(TARGET).exe.manifest
   $(COPY) $(MISC)$/$(TARGET).exe.manifest $@
.ENDIF

$(BIN)$/$(TARGET).bin: $(BIN)$/$(TARGET)$(EXECPOST)
   $(COPY) $< $@

$(BIN)$/so$/$(TARGET).bin: $(BIN)$/so$/$(TARGET)$(EXECPOST)
   $(COPY) $< $@

.ENDIF # WNT

.IF "$(GUI)" == "OS2"
$(BIN)$/$(TARGET).bin: $(BIN)$/$(TARGET)$(EXECPOST)
   $(COPY) $< $@
.ENDIF # OS2

$(MISC)$/binso_created.flg :
    @@-$(MKDIRHIER) $(BIN)$/so && $(TOUCH) $@

.ENDIF
