#*************************************************************************
#*
#*  $RCSfile: makefile.mk,v $
#*
#*  $Revision: 1.8 $
#*
#*  last change: $Author: bustamam $ $Date: 2002-03-26 13:40:06 $
#*
#*  The Contents of this file are made available subject to the terms of
#*  either of the following licenses
#*
#*         - GNU Lesser General Public License Version 2.1
#*         - Sun Industry Standards Source License Version 1.1
#*
#*  Sun Microsystems Inc., October, 2000
#*
#*  GNU Lesser General Public License Version 2.1
#*  =============================================
#*  Copyright 2000 by Sun Microsystems, Inc.
#*  901 San Antonio Road, Palo Alto, CA 94303, USA
#*
#*  This library is free software; you can redistribute it and/or
#*  modify it under the terms of the GNU Lesser General Public
#*  License version 2.1, as published by the Free Software Foundation.
#*
#*  This library is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#*  Lesser General Public License for more details.
#*
#*  You should have received a copy of the GNU Lesser General Public
#*  License along with this library; if not, write to the Free Software
#*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#*  MA  02111-1307  USA
#*
#*
#*  Sun Industry Standards Source License Version 1.1
#*  =================================================
#*  The contents of this file are subject to the Sun Industry Standards
#*  Source License Version 1.1 (the "License"); You may not use this file
#*  except in compliance with the License. You may obtain a copy of the
#*  License at http://www.openoffice.org/license.html.
#*
#*  Software provided under this License is provided on an "AS IS" basis,
#*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#*  See the License for the specific provisions governing your rights and
#*  obligations concerning the Software.
#*
#*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#*
#*  Copyright: 2000 by Sun Microsystems, Inc.
#*
#*  All Rights Reserved.
#*
#*  Contributor(s): _______________________________________
#*
#*
#************************************************************************

PRJ=..

PRJNAME=i18npool
TARGET=i18npool
VERSION=$(UPD)
USE_LDUMP2=TRUE

# Make symbol renaming match library name for Mac OS X
.IF "$(OS)"=="MACOSX"
SYMBOLPREFIX=i18n$(UPD)$(DLLPOSTFIX)
.ENDIF

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Allgemein ----------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/defaultnumberingprovider.lib	\
        $(SLB)$/registerservices.lib		\
        $(SLB)$/numberformatcode.lib		\
        $(SLB)$/locale.lib			\
        $(SLB)$/indexentry.lib			\
        $(SLB)$/calendar.lib		\
        $(SLB)$/breakiterator.lib \
        $(SLB)$/transliterationImpl.lib \
        $(SLB)$/characterclassification.lib \
        $(SLB)$/collator.lib \
        $(SLB)$/inputchecker.lib
LIB1DEPN=	$(MISC)$/$(LIB1TARGET).flt

SHL1TARGET= $(TARGET)$(VERSION)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)
.IF "$(OS)"=="WNT"
ICUI18NLIB= icuin.lib icuuc.lib
.ELSE
ICUI18NLIB= -licui18n -licuuc -licudata
.IF "$(OS)"=="SOLARIS"
ICUI18NLIB+= -lCstd
.ENDIF
.ENDIF

SHL1STDLIBS=\
        $(TOOLSLIB)				\
        $(CPPULIB)				\
        $(COMPHELPERLIB)			\
        $(CPPUHELPERLIB)			\
        $(SALLIB)				\
        $(ICUI18NLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	$(TARGET).dxp

I18NINC=	$(PRJ)$/inc$/transliterationImpl.hxx \
        $(PRJ)$/inc$/transliteration_body.hxx \
        $(PRJ)$/inc$/transliteration_caseignore.hxx \
        $(PRJ)$/inc$/transliteration_commonclass.hxx \
        $(PRJ)$/inc$/x_rtl_ustring.h \
        $(PRJ)$/inc$/unicode.hxx

# --- Targets ------------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Provider>> $@

$(MISC)$/$(LIB1TARGET).flt: $(I18NINC)
    +$(COPY) $< $(INCCOM)
