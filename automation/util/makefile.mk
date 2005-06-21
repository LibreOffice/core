#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: rt $ $Date: 2005-06-21 19:08:49 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2002
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2002 by Sun Microsystems, Inc.
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
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc..
#
#   Copyright: 2002 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

PRJNAME=AUTOMATION
TARGET=automation
TARGETTYPE=GUI

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=                 \
    $(SLB)$/server.lib     \
    $(SLB)$/simplecm.lib   \
    $(SLB)$/communi.lib    \

# --- sts library ---------------------------------------------------

SHL1TARGET= sts$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= ists

SHL1STDLIBS= \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)       \
            $(SVLIB) \
            $(BASICLIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(COMPHELPERLIB) \
            $(SOTLIB) \


.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(LIBPRE) advapi32.lib	\
        $(LIBPRE) gdi32.lib
.ENDIF

SHL1DEF=        $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=       $(SLB)$/$(TARGET).lib \

DEF1NAME        =$(SHL1TARGET)
DEF1DEPN        =       \
    $(MISC)$/$(SHL1TARGET).flt

DEFLIB1NAME     =$(TARGET)
DEF1DES         =TestToolServer

# --- simple communication library (no vcl) ---------------------------------------------------

SHL2TARGET= simplecm$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= isimplecm

SHL2STDLIBS= \
            $(TOOLSLIB) \
            $(VOSLIB) \
            $(SALLIB) \

#			$(CPPULIB)

#			$(COMPHELPERLIB)             $(SOTLIB) 			$(CPPUHELPERLIB) 


SHL2DEF=        $(MISC)$/$(SHL2TARGET).def
SHL2LIBS=       $(SLB)$/simplecm.lib 

DEF2NAME        =$(SHL2TARGET)
DEF2DEPN        =       \
    $(MISC)$/$(SHL2TARGET).flt

DEFLIB2NAME     =simplecm
DEF2DES         =SimpleCommunication

# --- communication library ---------------------------------------------------

SHL3TARGET= communi$(UPD)$(DLLPOSTFIX)
SHL3IMPLIB= icommuni

SHL3STDLIBS= \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)       \
            $(SVLIB) \
            $(BASICLIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(CPPUHELPERLIB) \
            $(CPPULIB) \
            $(COMPHELPERLIB) \
            $(SOTLIB) \
            $(VOSLIB) \
            $(SIMPLECMLIB) \


.IF "$(GUI)"=="WNT"
SHL3STDLIBS+= \
        $(LIBPRE) advapi32.lib	\
        $(LIBPRE) gdi32.lib
.ENDIF

SHL3DEF=        $(MISC)$/$(SHL3TARGET).def
SHL3LIBS=       $(SLB)$/communi.lib 

DEF3NAME        =$(SHL3TARGET)
DEF3DEPN        =       \
    $(MISC)$/$(SHL3TARGET).flt

DEFLIB3NAME     =communi
DEF3DES         =Communication

# --- TESTTOOL IDE ------------------------------------------------------

APP1TARGET=testtool

.IF "$(GUI)" == "UNX"
APP1STDLIBS= -lapp -lsample
.ELSE
APP1STDLIBS= \
        app.lib \
        sample.lib
.ENDIF
    
APP1STDLIBS+= \
            $(CPPUHELPERLIB) \
            $(TOOLSLIB) \
            $(UNOTOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB) \
            $(SVLIB) \
            $(SALLIB) \
            $(SJLIB) \
            $(VOSLIB) \
            $(SO2LIB) \
            $(UCBHELPERLIB) \
            $(COMPHELPERLIB) \
            $(SOTLIB) \
            $(BASICLIB) \
            $(SIMPLECMLIB) \
            $(COMMUNILIB) \

.IF "$(GUI)"=="UNX"
APP1STDLIBS+= \
            $(VOSLIB) $(SALLIB) $(BASICLIB)
APP1STDLIBS+=$(CPPULIB)
.ENDIF
.IF "$(GUI)"=="WNT" || "$(COM)"=="GCC"
APP1STDLIBS+=$(CPPULIB)
.ENDIF

.IF "$(GUI)"=="WNT"
APP1STDLIBS += \
        shell32.lib \
        advapi32.lib
.ENDIF # "$(GUI)"=="WNT"



.IF "$(OS)" == "SOLARIS"
APP1STDLIBS+= -lXm
.ENDIF

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "LINUX"
APP1STDLIBS+= -lXext -lX11 -lSM -lICE
.ENDIF
.IF "$(OS)"=="MACOSX"
APP1STDLIBS+= -lapp -lsample
.ENDIF
.ENDIF



APP1LIBS=\
        $(LIBPRE) $(LB)$/testtool.lib

APP1DEPN=\
        $(LB)$/testtool.lib

APP1OBJS=       $(OBJ)$/testbasi.obj \
                $(OBJ)$/cmdbasestream.obj \
                $(OBJ)$/svcommstream.obj

# --- TESTTOOL MINIAPP ------------------------------------------------------
# ressources are linked here

RESLIB1NAME=tma
RESLIB1SRSFILES= \
                $(SRS)$/miniapp.srs

APP3TARGET=miniapp
APP3STDLIBS= \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
             $(UCBHELPERLIB) \
            $(COMPHELPERLIB) \
            $(AUTOMATIONLIB) \
            $(SALLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLIB) \
            $(SJLIB) \
            $(SO2LIB)
.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
APP3STDLIBS+= \
            $(VOSLIB) $(SALLIB) $(LIBSTLPORT)
.ELSE
APP3STDLIBS+= \
            $(VOSLIB) $(SALLIB)
.ENDIF
.ENDIF

APP3LIBS= \
        $(LIBPRE) $(LB)$/miniapp.lib

APP3DEPN=\
        $(SHL1TARGETN) \
        $(LB)$/miniapp.lib

.IF "$(GUI)" != "UNX"
#		win16 braucht ein appobj
APP3OBJS=		$(OBJ)$/testapp.obj
.ENDIF

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk

# --- Basic-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@

$(MISC)$/$(SHL2TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@

$(MISC)$/$(SHL3TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@


# ------------------------------------------------------------------------

