#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.23 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:51:39 $
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

PRJNAME=basic
TARGET=sb
#basic.hid generieren
GEN_HID=TRUE

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/sb.lib
LIB1FILES=		\
    $(SLB)$/basicmgr.lib \
    $(SLB)$/classes.lib \
    $(SLB)$/comp.lib \
    $(SLB)$/uno.lib \
    $(SLB)$/runtime.lib \
    $(SLB)$/sbx.lib

SHL1TARGET= sb$(DLLPOSTFIX)
SHL1IMPLIB= basic

SHL1STDLIBS= \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)	\
            $(VCLLIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(COMPHELPERLIB) \
            $(UNOTOOLSLIB) \
            $(SOTLIB) \
            $(VOSLIB) \
            $(XMLSCRIPTLIB)

.IF "$(SOLAR_JAVA)" != "TRUE"
SHL1STDLIBS+=$(SJLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=	\
    $(UWINAPILIB)	\
    $(OLEAUT32LIB)
.ENDIF # WNT

.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
SHL1OBJS=	\
    $(SLO)$/sb.obj
.ENDIF
.ENDIF

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/sb.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=	\
    $(MISC)$/$(SHL1TARGET).flt

DEFLIB1NAME	=sb
DEF1DES		=StarBasic


RES1TARGET=$(PRJNAME)
SRS1FILES= \
        $(SRS)$/classes.srs \
        $(SRS)$/sbx.srs

# --- TESTTOOL IDE ------------------------------------------------------
# die ressourcen werden hier gelinkt

RESLIB1NAME=stt
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= \
        $(SRS)$/app.srs
        
RESLIB2NAME=sb
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES= \
        $(SRS)$/classes.srs \
        $(SRS)$/sbx.srs
########## remove sbx.srx (and this line) when issue i53795 is fixed

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk
 
#-------------------------------------------------------------------------
#								Windows NT
#-------------------------------------------------------------------------
#
#					default targets aus target.mk
#

# --- Basic-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo SvRTL >> $@
    @echo SbRtl_ >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@

$(SRS)$/basic.srs:
    $(TYPE) $(SRS)$/classes.srs + $(SRS)$/runtime.srs + $(SRS)$/sbx.srs > $@


