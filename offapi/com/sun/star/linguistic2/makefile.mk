#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.16 $
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


PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=csslinguistic2
PACKAGE=com$/sun$/star$/linguistic2

# --- Settings -----------------------------------------------------
.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    ConversionDictionary.idl \
    ConversionDictionaryList.idl \
    ConversionDictionaryType.idl \
    ConversionDirection.idl \
    ConversionPropertyType.idl \
    Dictionary.idl\
    DictionaryEvent.idl\
    DictionaryEventFlags.idl\
    DictionaryList.idl\
    DictionaryListEvent.idl\
    DictionaryListEventFlags.idl\
    DictionaryType.idl\
    HangulHanjaConversionDictionary.idl \
    Hyphenator.idl\
    LinguProperties.idl\
    LinguServiceEvent.idl\
    LinguServiceEventFlags.idl\
    LinguServiceManager.idl\
    SpellChecker.idl\
    SpellFailure.idl\
    Thesaurus.idl\
    XAvailableLocales.idl\
    XConversionDictionary.idl \
    XConversionDictionaryList.idl \
    XConversionPropertyType.idl \
    XDictionary.idl\
    XDictionary1.idl\
    XDictionaryEntry.idl\
    XDictionaryEventListener.idl\
    XDictionaryList.idl\
    XDictionaryListEventListener.idl\
    XHyphenatedWord.idl\
    XHyphenator.idl\
    XLanguageGuessing.idl\
    XLinguServiceEventBroadcaster.idl\
    XLinguServiceEventListener.idl\
    XLinguServiceManager.idl\
    XMeaning.idl\
    XPossibleHyphens.idl\
    XSearchableDictionary.idl\
    XSearchableDictionaryList.idl\
    XSpellAlternatives.idl\
    XSetSpellAlternatives.idl\
    XSpellChecker.idl\
    XSpellChecker1.idl\
    XSupportedLanguages.idl\
    XSupportedLocales.idl\
    XThesaurus.idl\
    SingleProofreadingError.idl\
    ProofreadingResult.idl\
    Proofreader.idl\
    XProofreader.idl\
    XProofreadingIterator.idl\
    ProofreadingIterator.idl


# ------------------------------------------------------------------

.INCLUDE :	target.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk
