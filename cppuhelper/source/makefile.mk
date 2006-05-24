#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.42 $
#
#   last change: $Author: vg $ $Date: 2006-05-24 13:29:58 $
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
PRJ=..

PRJNAME=cppuhelper
TARGET=cppuhelper

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(OS)" != "WNT"
UNIXVERSIONNAMES=UDK
.ENDIF # WNT

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

INCPRE+=$(OUT)$/inc$/private

.IF "$(debug)" != ""
# msvc++: no inlining for debugging
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

SLOFILES= \
        $(SLO)$/typeprovider.obj 	\
        $(SLO)$/exc_thrower.obj 	\
        $(SLO)$/servicefactory.obj 	\
        $(SLO)$/bootstrap.obj 		\
        $(SLO)$/implbase.obj 		\
        $(SLO)$/implbase_ex.obj 	\
        $(SLO)$/propshlp.obj 		\
        $(SLO)$/weak.obj		\
        $(SLO)$/interfacecontainer.obj	\
        $(SLO)$/stdidlclass.obj 	\
        $(SLO)$/factory.obj		\
        $(SLO)$/component_context.obj	\
        $(SLO)$/component.obj		\
        $(SLO)$/shlib.obj		\
        $(SLO)$/tdmgr.obj		\
        $(SLO)$/implementationentry.obj	\
        $(SLO)$/access_control.obj	\
        $(SLO)$/macro_expander.obj \
            $(SLO)$/unourl.obj \
        $(SLO)$/propertysetmixin.obj

.IF "$(GUI)" == "WNT"
SHL1TARGET=$(TARGET)$(UDK_MAJOR)$(COMID)
.ELSE
SHL1TARGET=uno_$(TARGET)$(COMID)
.ENDIF

SHL1STDLIBS= \
        $(SALLIB)		\
        $(SALHELPERLIB)	\
        $(CPPULIB)

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
SHL1OBJS = $(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

.IF "$(COMNAME)"=="msci"
SHL1VERSIONMAP=msvc_win32_intel.map
.ELIF "$(COMNAME)"=="sunpro5"
SHL1VERSIONMAP=cc5_solaris_sparc.map
.ELIF "$(OS)$(CPU)$(COMNAME)"=="LINUXIgcc3"
SHL1VERSIONMAP=gcc3_linux_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)"=="FREEBSDIgcc3"
SHL1VERSIONMAP=gcc3_linux_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)"=="LINUXSgcc3"
SHL1VERSIONMAP=gcc3_linux_intel.map
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(diag)"!=""
CFLAGS += -DDIAG=$(diag)
.ENDIF

.INCLUDE :	target.mk
