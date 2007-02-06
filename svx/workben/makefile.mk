#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: vg $ $Date: 2007-02-06 13:44:00 $
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

PRJNAME=SVX
TARGET=edittest
TARGETTYPE=GUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

CXXFILES=	edittest.cxx

OBJFILES=	$(OBJ)$/edittest.obj

APP1TARGET= $(TARGET)
APP1OBJS=	$(OBJFILES)
APP1LIBS=	$(LB)/svx.lib $(LB)/dl.lib
APP1STDLIBS=$(TOOLSLIB) 		\
            $(OSLLIB)			\
            $(ONELIB)			\
            $(VOSLIB)			\
            $(SO2LIB)			\
            $(SVTOOLLIB)		\
            $(SVLLIB)			\
            $(COMPHELPERLIB)		\
            $(CPPULIB)		\
            $(CPPUHELPERLIB)		\
            $(SALLIB)		\
            $(GOODIESLIB)		\
            $(SFXLIB)			\
            $(SOTLIB)			\
            $(DLLIB)			\
            $(SVLIB)

.IF "$(GUI)"!="UNX"
APP1DEF=	$(MISC)$/$(TARGET).def
.ENDIF


.IF "$(COM)"=="GCC"
ADDOPTFILES=$(OBJ)$/edittest.obj
add_cflagscxx="-frtti -fexceptions"
.ENDIF


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


# ------------------------------------------------------------------
# MAC
# ------------------------------------------------------------------

.IF "$(GUI)" == "MAC"

$(MISC)$/$(TARGET).def: makefile
    echo Kein Def-File fuer Applikationen auf Mac
.ENDIF


# ------------------------------------------------------------------
# Windows
# ------------------------------------------------------------------

.IF "$(GUI)" == "WIN"

$(MISC)$/$(TARGET).def: makefile
    echo  NAME			$(TARGET)							>$@
    echo  DESCRIPTION	'StarView - Testprogramm'          >>$@
    echo  EXETYPE		WINDOWS 						   >>$@
    echo  STUB			'winSTUB.EXE'                      >>$@
    echo  PROTMODE										   >>$@
    echo  CODE			PRELOAD MOVEABLE DISCARDABLE	   >>$@
    echo  DATA			PRELOAD MOVEABLE MULTIPLE		   >>$@
    echo  HEAPSIZE		8192							   >>$@
    echo  STACKSIZE 	32768							   >>$@

.ENDIF

# ------------------------------------------------------------------
# OS2
# ------------------------------------------------------------------

.IF "$(GUI)" == "OS2"

$(MISC)$/$(TARGET).def: makefile
    echo  NAME			$(TARGET) WINDOWAPI 				>$@
    echo  DESCRIPTION	'StarView - Testprogramm'          >>$@
.IF "$(COM)" != "BLC"
    echo  STUB			'os2STUB.EXE'                      >>$@
.ENDIF
.IF "$(COM)"!="MTW"
    echo  EXETYPE		OS2 							   >>$@
.ENDIF
    echo  PROTMODE										   >>$@
    echo  CODE			LOADONCALL						   >>$@
    echo  DATA			PRELOAD MULTIPLE				   >>$@
    echo  HEAPSIZE		16384							   >>$@
    echo  STACKSIZE 	32768							   >>$@

.ENDIF
