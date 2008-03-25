#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.53 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:28:31 $
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

ENABLE_EXCEPTIONS=TRUE
PRJNAME=sfx2
TARGET=sfx
#sfx.hid generieren
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=  $(SLB)$/appl.lib		\
            $(SLB)$/explorer.lib	\
            $(SLB)$/doc.lib			\
            $(SLB)$/view.lib		\
            $(SLB)$/control.lib		\
            $(SLB)$/notify.lib		\
            $(SLB)$/menu.lib		\
            $(SLB)$/inet.lib		\
            $(SLB)$/toolbox.lib		\
            $(SLB)$/statbar.lib		\
            $(SLB)$/dialog.lib		\
            $(SLB)$/bastyp.lib		\
            $(SLB)$/config.lib

HELPIDFILES=\
            ..\inc\sfx2\sfxsids.hrc	\
            ..\source\inc\helpid.hrc

.IF "$(GUI)"!="UNX"
LIB2TARGET= $(LB)$/$(TARGET).lib
LIB2FILES=  $(LB)$/isfx.lib
.ENDIF

SHL1TARGET= sfx$(DLLPOSTFIX)
SHL1IMPLIB= isfx
SHL1USE_EXPORTS=name

SHL1STDLIBS+=\
        $(FWELIB) \
        $(BASICLIB) \
        $(XMLSCRIPTLIB) \
        $(SVTOOLLIB) \
        $(TKLIB) \
        $(VCLLIB) \
        $(SVLLIB)	\
        $(SOTLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(I18NISOLANGLIB) \
        $(SAXLIB) \
        $(SYSSHELLLIB) \
        $(COMPHELPERLIB) \
        $(UCBHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(VOSLIB) \
        $(SALLIB) \
        $(SJLIB)

.IF "$(GUI)"=="WNT"

SHL1STDLIBS+=\
        $(UWINAPILIB) \
        $(ADVAPI32LIB) \
        $(SHELL32LIB) \
        $(GDI32LIB) \
        $(OLE32LIB) \
        $(UUIDLIB)
.ELSE # WNT
.IF "$(OS)" == "MACOSX"
SHL1STDLIBS+= -framework Cocoa
.ENDIF # MACOSX
.ENDIF # WNT


SHL1DEPN += $(shell @$(FIND) $(SLO) -type f -name "*.OBJ" -print)

SHL1LIBS=   $(LIB1TARGET)

#SHL1OBJS=   $(SLO)$/sfxdll.obj

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME	=$(SHL1TARGET)
DEFLIB1NAME	=sfx
DEF1DES		=Sfx

SFXSRSLIST=\
        $(SRS)$/appl.srs \
        $(SRS)$/sfx.srs \
        $(SRS)$/doc.srs \
        $(SRS)$/view.srs \
        $(SRS)$/config.srs \
        $(SRS)$/menu.srs \
        $(SRS)$/dialog.srs \
                $(SRS)$/bastyp.srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(SFXSRSLIST)

# gtk quick-starter
.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
CFLAGS+=$(PKGCONFIG_CFLAGS)

SHL3TARGET=qstart_gtk$(DLLPOSTFIX)
SHL3LIBS=$(SLB)$/quickstart.lib
SHL3DEPN=$(SHL1IMPLIBN) $(SHL1TARGETN)
# libs for gtk plugin
SHL3STDLIBS=$(SHL1STDLIBS) $(SFX2LIB) $(EGGTRAYLIB)
SHL3STDLIBS+=$(PKGCONFIG_LIBS:s/ -lpangoxft-1.0//)
# hack for faked SO environment
.IF "$(PKGCONFIG_ROOT)"!=""
SHL3SONAME+=-z nodefs
SHL3NOCHECK=TRUE
.ENDIF          # "$(PKGCONFIG_ROOT)"!=""
.ENDIF # "$(ENABLE_SYSTRAY_GTK)"=="TRUE"
.ENDIF # "$(GUI)"=="UNX"

# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk
