#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 14:08:28 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = unopkg
TARGETTYPE = GUI
ENABLE_EXCEPTIONS = TRUE
NO_BSYMBOLIC = TRUE
LIBTARGET=NO

PRJINC += ..$/..$/deployment
.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF


SHARED_OBJS = \
        $(OBJ)$/unopkg_misc.obj \
        $(OBJ)$/unopkg_cmdenv.obj

#################################################################

MYAPP1OBJS = \
        $(SHARED_OBJS) \
        $(OBJ)$/unopkg_app.obj
APP1NOSAL = TRUE        
APP1OBJS = \
        $(MYAPP1OBJS) \
        $(OBJ)$/lockfile.obj


APP1STDLIBS = \
        $(SALLIB) \
        $(CPPULIB) \
        $(CPPUHELPERLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(VCLLIB) \
        $(DEPLOYMENTMISCLIB)

APP1TARGET = so$/unopkg

.IF "$(GUI)" == "WNT"
APP1ICON=$(SOLARRESDIR)$/icons/so8-main-app.ico
APP1LINKRES=$(MISC)$/$(TARGET)1.res
.ENDIF # WNT
#################################################################
APP2NOSAL = TRUE     
APP2OBJS = $(APP1OBJS)
APP2STDLIBS = $(APP1STDLIBS)
APP2TARGET = unopkg


.IF "$(GUI)" == "WNT"
APP2ICON=$(SOLARRESDIR)$/icons/ooo-main-app.ico
APP2LINKRES=$(MISC)$/$(TARGET)2.res
.ENDIF # WNT

DEPOBJFILES = $(MYAPP1OBJS) \
        $(OBJ)$/lockfile.obj


.IF "$(debug)" != ""

# disable MSVC inlining for debugging
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF

.ENDIF

.INCLUDE : target.mk

$(APP1TARGETN) : $(PRJ)$/$(BIN)$/so

$(PRJ)$/$(BIN)$/so:
    @@+-$(MKDIRHIER) $(BIN)$/so

