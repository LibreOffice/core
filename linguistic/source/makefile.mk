#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: hr $ $Date: 2000-11-21 15:34:46 $
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

PRJ = ..

PRJNAME	= linguistic
TARGET	= oolng
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=	$(SOLARBINDIR)$/applicat.rdb
UNOUCRRDB=	$(SOLARBINDIR)$/applicat.rdb

UNOTYPES=\
    com.sun.star.linguistic2.DictionaryEvent\
    com.sun.star.linguistic2.DictionaryEventFlags\
    com.sun.star.linguistic2.DictionaryListEvent\
    com.sun.star.linguistic2.DictionaryListEventFlags\
    com.sun.star.linguistic2.DictionaryType\
    com.sun.star.linguistic2.LinguServiceEventFlags\
    com.sun.star.linguistic2.SpellFailure\
    com.sun.star.linguistic2.XDictionary\
    com.sun.star.linguistic2.XDictionary1\
    com.sun.star.linguistic2.XDictionaryEntry\
    com.sun.star.linguistic2.XDictionaryEventListener\
    com.sun.star.linguistic2.XDictionaryList\
    com.sun.star.linguistic2.XDictionaryListEventListener\
    com.sun.star.linguistic2.XHyphenatedWord\
    com.sun.star.linguistic2.XHyphenator\
    com.sun.star.linguistic2.XLinguServiceEventBroadcaster\
    com.sun.star.linguistic2.XLinguServiceEventListener\
    com.sun.star.linguistic2.XLinguServiceManager\
    com.sun.star.linguistic2.XMeaning\
    com.sun.star.linguistic2.XOtherLingu\
    com.sun.star.linguistic2.XPossibleHyphens\
    com.sun.star.linguistic2.XSearchableDictionaryList\
    com.sun.star.linguistic2.XSpellAlternatives\
    com.sun.star.linguistic2.XSpellChecker\
    com.sun.star.linguistic2.XSpellChecker1\
    com.sun.star.linguistic2.XSupportedLanguages\
    com.sun.star.linguistic2.XSupportedLocales\
    com.sun.star.linguistic2.XThesaurus


.IF "$(header)" == ""

EXCEPTIONSFILES=\
        $(SLO)$/dicimp.obj\
        $(SLO)$/dlistimp.obj\
        $(SLO)$/hyphdsp.obj\
        $(SLO)$/lngopt.obj\
        $(SLO)$/lngreg.obj\
        $(SLO)$/lngsvcmgr.obj\
        $(SLO)$/misc.obj\
        $(SLO)$/spelldsp.obj\
        $(SLO)$/thesdsp.obj

LNGOBJS = \
        $(SLO)$/dicimp.obj\
        $(SLO)$/dlistimp.obj\
        $(SLO)$/hyphdsp.obj\
        $(SLO)$/hyphdta.obj\
        $(SLO)$/iprcache.obj\
        $(SLO)$/lngopt.obj\
        $(SLO)$/lngsvcmgr.obj\
        $(SLO)$/misc.obj\
        $(SLO)$/spelldsp.obj\
        $(SLO)$/spelldta.obj\
        $(SLO)$/thesdsp.obj
#		$(SLO)$/thesdta.obj

REGOBJS = \
        $(SLO)$/lngreg.obj

SLOFILES = \
        $(LNGOBJS) $(REGOBJ)

TARGET1 = lng
TARGET2 = lngreg


LIB1TARGET		= $(SLB)$/$(TARGET1).lib
LIB1ARCHIV		= $(LB)$/lib$(TARGET1).a
LIB1OBJFILES	= $(LNGOBJS)

LIB2TARGET		= $(SLB)$/$(TARGET2).lib
LIB2OBJFILES    = $(REGOBJS)


SHL1TARGET= $(TARGET1)$(UPD)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(UNOLIB)		\
        $(VOSLIB)		\
        $(TOOLSLIB)		\
        $(SVTOOLLIB)	\
        $(SVLLIB)		\
        $(VCLLIB)		\
        $(SFXLIB)		\
        $(SALLIB)		\
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)

        
.IF "$(GUI)" == "UNX"
SHL1STDLIBS += $(LNGLIB)
.ENDIF
#

# build DLL
.IF "$(GUI)" == "WNT"
SHL1LIBS=		$(SLB)$/$(TARGET1).lib $(SLB)$/$(TARGET2).lib 
.ELSE
SHL1LIBS=		$(SLB)$/$(TARGET2).lib
.ENDIF
SHL1DEPN=		$(SLB)$/$(TARGET1).lib $(SLB)$/$(TARGET2).lib  
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP= $(TARGET1).map

# build DEF file
DEF1NAME	=$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =$(TARGET)
DEF1DES     =Linguistic main DLL

.ENDIF

# --- Targets ------------------------------------------------------


.INCLUDE : target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo lcl > $@

