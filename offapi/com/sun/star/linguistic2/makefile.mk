#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: obo $ $Date: 2008-02-26 09:53:33 $
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
    XSearchableDictionaryList.idl\
    XSpellAlternatives.idl\
    XSpellChecker.idl\
    XSpellChecker1.idl\
    XSupportedLanguages.idl\
    XSupportedLocales.idl\
    XThesaurus.idl\
    SingleGrammarError.idl\
    GrammarCheckingResult.idl\
    XGrammarChecker.idl\
    XGrammarCheckerListener.idl\
    XGrammarCheckingIterator.idl\
    XGrammarCheckingResultBroadcaster.idl\
    XGrammarCheckingResultListener.idl\
    GrammarCheckingIterator.idl


# ------------------------------------------------------------------

.INCLUDE :	target.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk
