#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.26 $
#
#   last change: $Author: hr $ $Date: 2003-03-25 13:52:52 $
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

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

VERINFONAME=verinfo

TARGETOBJS=   $(OBJ)$/app.obj					\
            $(OBJ)$/lockfile.obj			\
            $(OBJ)$/intro.obj				\
            $(OBJ)$/officeipcthread.obj		\
            $(OBJ)$/appinit.obj				\
            $(OBJ)$/cmdlineargs.obj			\
            $(OBJ)$/oinstanceprovider.obj	\
            $(OBJ)$/opluginframefactory.obj	\
            $(OBJ)$/appsys.obj				\
            $(OBJ)$/desktopresid.obj		\
            $(OBJ)$/dispatchwatcher.obj		\
            $(OBJ)$/ssodlg.obj				\
            $(OBJ)$/ssoinit.obj				\
            $(OBJ)$/configinit.obj				\
            $(OBJ)$/javainteractionhandler.obj	\
            $(OBJ)$/oempreload.obj			\
            $(OBJ)$/testtool.obj			\
            $(OBJ)$/checkinstall.obj		\
            $(OBJ)$/cmdlinehelp.obj


# --- Resourcen ----------------------------------------------------

.IF "$(GUI)" == "WNT"
RCFILES=verinfo.rc
.ENDIF

# --- Linken der Applikation ---------------------------------------

.IF "$(OS)" == "LINUX"
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
RESLIB1SRSFILES=	$(SRS)$/desktop.srs

APP1TARGET=so$/$(TARGET)
APP1STDLIBS=			\
    $(APP1_STDPRE)		\
    $(SVLLIB)			\
    $(SVMEMLIB)			\
    $(VCLLIB)			\
    $(APP1_STDPOST)		\
    $(UNOTOOLSLIB)		\
    $(UCBHELPERLIB)		\
    $(COMPHELPERLIB)	\
    $(SALHELPERLIB)		\
    $(SVTOOLLIB)
APP1OBJS=$(TARGETOBJS)
APP1OBJS += $(OBJ)$/copyright_ascii_sun.obj

.IF "$(GUI)" == "UNX"
.IF "$(OS)" != "MACOSX"
APP1OBJS +=	$(OBJ)$/icon_resource_sun.obj
.ENDIF
.ENDIF

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "LINUX"
APP1STDLIBS+= -lXext -lSM -lICE
.ENDIF
.ENDIF

APP1DEPN= $(APP1RES) verinfo.rc
APP1DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP1RES=    $(RES)$/desktop.res
APP1ICON=$(SOLARRESDIR)$/icons/001_star_main.ico
APP1VERINFO=verinfo.rc
APP1LINKRES=$(MISC)$/$(TARGET).res

# create a manifest file with the same name as the
#office executable file soffice.exe.manifest
#$(BIN)$/$(TARGET).exe.manifest: template.manifest
#+$(COPY) $< $@ 
   
.ENDIF

.IF "$(GUI)" == "WNT"

APP2DEPN= $(APP2RES) verinfo.rc
APP2NOSAL=TRUE
APP2TARGET=sowrapper
APP2STDLIBS+=shell32.lib
APP2OBJS=       $(OBJ)$/sowrapper.obj \
                $(OBJ)$/wrapperw.obj \
                $(OBJ)$/wrappera.obj
APP2RES=    $(RES)$/$(APP2TARGET).res
APP2ICON=$(SOLARRESDIR)$/icons/001_star_main.ico
APP2VERINFO=verinfo.rc
APP2LINKRES=$(MISC)$/$(APP2TARGET).res


APP3DEPN= $(APP3RES) verinfo.rc
APP3NOSAL=TRUE
APP3TARGET=solocal
APP3STDLIBS+=shell32.lib
APP3OBJS= \
                $(OBJ)$/sowrapper.obj \
                $(OBJ)$/lwrapw.obj \
                $(OBJ)$/lwrapa.obj
APP3RES=    $(RES)$/$(APP3TARGET).res
APP3ICON=$(SOLARRESDIR)$/icons/001_star_main.ico
APP3VERINFO=verinfo.rc
APP3LINKRES=$(MISC)$/$(APP3TARGET).res

APP4DEPN= $(APP4RES) verinfo.rc
APP4NOSAL=TRUE
APP4TARGET=soremote
APP4STDLIBS+=shell32.lib
APP4OBJS= \
                $(OBJ)$/sowrapper.obj \
                $(OBJ)$/rwrapw.obj \
                $(OBJ)$/rwrapa.obj

APP4RES=    $(RES)$/$(APP4TARGET).res
APP4ICON=$(SOLARRESDIR)$/icons/001_star_main.ico
APP4VERINFO=verinfo.rc
APP4LINKRES=$(MISC)$/$(APP4TARGET).res
.ENDIF # "$(GUI)" == "WNT"


APP5TARGET=soffice
APP5STDLIBS=			\
    $(APP1_STDPRE)		\
    $(SVLLIB)			\
    $(SVMEMLIB)			\
    $(VCLLIB)			\
    $(APP1_STDPOST)		\
    $(UNOTOOLSLIB)		\
    $(UCBHELPERLIB)		\
    $(COMPHELPERLIB)	\
    $(SALHELPERLIB)		\
    $(SVTOOLLIB)
APP5OBJS=$(TARGETOBJS)
APP5OBJS += $(OBJ)$/copyright_ascii_ooo.obj

.IF "$(GUI)" == "UNX"
.IF "$(OS)" != "MACOSX"
APP5OBJS +=	$(OBJ)$/icon_resource_ooo.obj
.ENDIF
.ENDIF

.IF "$(OS)" == "LINUX"
APP5STDLIBS+= -lXext -lSM -lICE
.ENDIF

APP5DEPN= $(APP1RES) verinfo.rc
APP5DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP5RES=    $(RES)$/desktop.res
APP5ICON=$(SOLARRESDIR)$/icons/001_star_butterfly.ico
APP5VERINFO=verinfo.rc
APP5LINKRES=$(MISC)$/ooffice.res
.ENDIF



all: $(BIN)$/so ALLTAR

.IF "$(GUI)" == "WNT"

ALLTAR: $(BIN)$/$(TARGET).exe.manifest

.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(GUI)" == "WNT"

# create a manifest file with the same name as the
#office executable file soffice.exe.manifest
$(BIN)$/$(TARGET).exe.manifest: template.manifest
   +$(COPY) $< $@ 
   
$(MISCX)$/$(APP1TARGET).def : makefile.mk
    echo  NAME			soffice								>$@
    echo  DESCRIPTION   'StarDesktop Version 5'           >>$@
    echo  DATA			READ WRITE NONSHARED		   >>$@
.ENDIF

$(BIN)$/so: makefile.mk
    @echo APP5 : $(APP5TARGET)
    @+-mkdir $(BIN)$/so
