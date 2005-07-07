#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.57 $
#
#   last change: $Author: obo $ $Date: 2005-07-07 13:21:12 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

PRJNAME=desktop
TARGET=soffice
TARGETTYPE=GUI
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

VERINFONAME=verinfo

TARGETOBJS=	\
            $(OBJ)$/main.obj				\
            $(OBJ)$/app.obj					\
            $(OBJ)$/lockfile.obj			\
            $(OBJ)$/lockfile2.obj			\
            $(OBJ)$/intro.obj				\
            $(OBJ)$/officeipcthread.obj		\
            $(OBJ)$/appinit.obj				\
            $(OBJ)$/cmdlineargs.obj			\
            $(OBJ)$/oinstanceprovider.obj	\
            $(OBJ)$/opluginframefactory.obj	\
            $(OBJ)$/appsys.obj				\
            $(OBJ)$/desktopresid.obj		\
            $(OBJ)$/dispatchwatcher.obj		\
            $(OBJ)$/configinit.obj				\
            $(OBJ)$/checkinstall.obj		\
            $(OBJ)$/cmdlinehelp.obj         \
            $(OBJ)$/langselect.obj          \
            $(OBJ)$/userinstall.obj         \
            $(OBJ)$/desktopcontext.obj      
            


# --- Resourcen ----------------------------------------------------

.IF "$(GUI)" == "WNT"
RCFILES=verinfo.rc
.ENDIF

# --- Linken der Applikation ---------------------------------------

.IF "$(OS)" == "MACOSX"
LINKFLAGSAPPGUI!:=	$(LINKFLAGSAPPGUI:s/-bind_at_load//)
.ENDIF # MACOSX

.IF "$(OS)" == "LINUX" || "$(OS)" == "FREEBSD" || "$(OS)" == "NETBSD"
# #74158# linux needs sal/vos/tools at end of link list, solaris needs it first,
# winXX is handled like solaris for now
APP1_STDPRE=
APP1_STDPOST=$(CPPULIB) $(CPPUHELPERLIB) $(UNOLIB) $(TOOLSLIB) \
    $(VOSLIB) $(SALLIB) 
.ELSE
APP1_STDPRE=$(SALLIB) $(VOSLIB) $(TOOLSLIB) $(UNOLIB) $(CPPULIB) \
    $(CPPUHELPERLIB)
APP1_STDPOST=
.ENDIF

RESLIB1NAME=		dkt
RESLIB1IMAGES=		$(PRJ)$/res
RESLIB1SRSFILES=	$(SRS)$/desktop.srs \
                    $(SRS)$/wizard.srs

APP1TARGET=so$/$(TARGET)
APP1NOSAL=TRUE
APP1STDLIBS=			\
    $(VCLLIB)			\
    $(SVLLIB)           \
    $(SVTOOLLIB)            \
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)		\
    $(VOSLIB)			\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(TKLIB)            \
    $(SALLIB) \
    $(SVXLIB) 




#	$(APP1_STDPRE)		\
#	$(SVLLIB)			\
#	$(SVMEMLIB)			\
#	$(VCLLIB)			\
#   $(APP1_STDPOST)		\
#	$(UNOTOOLSLIB)		\
#	$(UCBHELPERLIB)		\
#	$(COMPHELPERLIB)	\
#	$(SALHELPERLIB)		\

APP1OBJS=$(TARGETOBJS)
APP1OBJS += $(OBJ)$/copyright_ascii_sun.obj

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "LINUX" || "$(OS)" == "FREEBSD"
APP1STDLIBS+= -lXext -lSM -lICE
.ENDIF
.ENDIF

APP1DEPN= $(APP1RES) verinfo.rc
APP1DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP1RES=    $(RES)$/desktop.res
APP1ICON=$(SOLARRESDIR)$/icons/so8-main-app.ico
APP1VERINFO=verinfo.rc
APP1LINKRES=$(MISC)$/$(TARGET).res
APP1STACK=10000000

# create a manifest file with the same name as the
#office executable file soffice.exe.manifest
#$(BIN)$/$(TARGET).exe.manifest: template.manifest
#+$(COPY) $< $@

.ENDIF # WNT

APP5TARGET=soffice
APP5NOSAL=TRUE
APP5STDLIBS=			\
    $(VCLLIB)			\
    $(SVLLIB)			\
    $(SVTOOLLIB)        \
    $(UNOTOOLSLIB)      \
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)		\
    $(VOSLIB)			\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)			\
    $(TKLIB)     



APP5OBJS=$(TARGETOBJS)
APP5OBJS += $(OBJ)$/copyright_ascii_ooo.obj

.IF "$(OS)" == "LINUX"
APP5STDLIBS+= -lXext -lSM -lICE
.ENDIF # LINUX

APP5DEPN= $(APP1TARGETN) $(APP5RES) ooverinfo.rc
APP5DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP5RES=    $(RES)$/oodesktop.res
APP5ICON=$(SOLARRESDIR)$/icons/ooo-main-app.ico
APP5VERINFO=ooverinfo.rc
APP5LINKRES=$(MISC)$/ooffice.res
APP5STACK=10000000
.ENDIF # WNT

.IF "$(GUI)" == "WNT"
APP6TARGET=so$/officeloader
APP6RES=$(RES)$/soloader.res
APP6NOSAL=TRUE
APP6DEPN= $(APP1TARGETN) $(APP6RES) verinfo.rc
APP6VERINFO=verinfo.rc
APP6LINKRES=$(MISC)$/soffice.res
APP6ICON=$(SOLARRESDIR)$/icons/so8-main-app.ico
APP6OBJS = \
        $(OBJ)$/officeloader.obj

APP7TARGET=officeloader
APP7RES=$(RES)$/ooloader.res
APP7NOSAL=TRUE
APP7DEPN= $(APP1TARGETN) $(APP7RES) ooverinfo.rc
APP7VERINFO=ooverinfo.rc
APP7LINKRES=$(MISC)$/ooffice.res
APP7ICON=$(SOLARRESDIR)$/icons/ooo-main-app.ico
APP7OBJS = \
        $(OBJ)$/officeloader.obj
.ENDIF # WNT


all: $(BIN)$/so ALLTAR

.IF "$(GUI)" == "WNT"

ALLTAR: $(BIN)$/$(TARGET).exe.manifest

.ENDIF # WNT

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(APP1TARGETN)"!=""
$(APP1TARGETN) : $(BIN)$/so
.ENDIF			# "$(APP1TARGETN)"!=""

.IF "$(GUI)" == "WNT"
ALLTAR: $(BIN)$/$(TARGET).exe.manifest
ALLTAR: $(BIN)$/$(TARGET).bin
ALLTAR: $(BIN)$/so$/$(TARGET).bin
.ENDIF # WNT

$(BIN)$/soffice_oo$(EXECPOST) : $(APP5TARGETN)
    +$(COPY) $< $@

$(BIN)$/so$/soffice_so$(EXECPOST) : $(APP1TARGETN)
    +$(COPY) $< $@

ALLTAR : $(BIN)$/so$/soffice_so$(EXECPOST) $(BIN)$/soffice_oo$(EXECPOST)


.IF "$(GUI)" == "WNT"

# create a manifest file with the same name as the
# office executable file soffice.exe.manifest
$(BIN)$/$(TARGET).exe.manifest: template.manifest
   +$(COPY) $< $@

$(BIN)$/$(TARGET).bin: $(BIN)$/$(TARGET)$(EXECPOST)
   +$(COPY) $< $@
   
$(BIN)$/so$/$(TARGET).bin: $(BIN)$/so$/$(TARGET)$(EXECPOST)
   +$(COPY) $< $@

$(MISCX)$/$(APP1TARGET).def : makefile.mk
    echo  NAME			soffice								>$@
    echo  DESCRIPTION   'StarDesktop Version 5'           >>$@
    echo  DATA			READ WRITE NONSHARED		   >>$@

.ENDIF # WNT

$(BIN)$/so: makefile.mk
    @echo APP5 : $(APP5TARGET)
    @+-mkdir $(BIN)$/so >& $(NULLDEV)
