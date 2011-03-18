#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ = ..

PRJNAME = linguistic
TARGET	= lex
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

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
    com.sun.star.linguistic2.XPossibleHyphens\
    com.sun.star.linguistic2.XSearchableDictionaryList\
    com.sun.star.linguistic2.XSpellAlternatives\
    com.sun.star.linguistic2.XSpellChecker\
    com.sun.star.linguistic2.XSupportedLocales\
    com.sun.star.linguistic2.XThesaurus


SLOFILES=	\
        $(SLO)$/sprophelp.obj\
        $(SLO)$/sreg.obj\
        $(SLO)$/sspellimp.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(TOOLSLIB)		\
        $(SVTOOLLIB)	\
        $(SVLLIB)		\
        $(VCLLIB)		\
        $(SFXLIB)		\
        $(SALLIB)		\
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)	\
        $(LNGLIB)

# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
.IF "$(OS)"!="MACOSX"
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
.ENDIF

# build DEF file
DEF1NAME	 =$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

