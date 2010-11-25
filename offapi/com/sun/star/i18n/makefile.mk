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

PRJ=..$/..$/..$/..

PRJNAME=offapi

TARGET=cssi18n
PACKAGE=com$/sun$/star$/i18n

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AmPmValue.idl \
    Boundary.idl \
    BreakIterator.idl \
    BreakType.idl \
    CTLScriptType.idl \
    Calendar.idl \
    CalendarDisplayCode.idl \
    CalendarDisplayIndex.idl \
    CalendarFieldIndex.idl \
    CalendarItem.idl \
    ChapterCollator.idl \
    CharType.idl \
    CharacterClassification.idl \
    CharacterIteratorMode.idl \
    Collator.idl \
    CollatorOptions.idl \
    Currency.idl \
    Currency2.idl \
    DirectionProperty.idl \
    ForbiddenCharacters.idl \
    FormatElement.idl \
    Implementation.idl \
    IndexEntrySupplier.idl \
    InputSequenceChecker.idl \
    InputSequenceCheckMode.idl \
    KCharacterType.idl \
    KNumberFormatType.idl \
    KNumberFormatUsage.idl \
    KParseTokens.idl \
    KParseType.idl \
    LanguageCountryInfo.idl \
    LineBreakHyphenationOptions.idl  \
    LineBreakResults.idl \
    LineBreakUserOptions.idl  \
    LocaleCalendar.idl \
    LocaleData.idl \
    LocaleDataItem.idl \
    LocaleItem.idl \
    Months.idl \
    MultipleCharsOutputException.idl \
    NativeNumberMode.idl \
    NativeNumberSupplier.idl \
    NativeNumberXmlAttributes.idl \
    NumberFormatCode.idl \
    NumberFormatIndex.idl \
    NumberFormatMapper.idl \
    ParseResult.idl \
    ScriptDirection.idl \
    ScriptType.idl \
    TextConversion.idl \
    TextConversionOption.idl \
    TextConversionResult.idl \
    TextConversionType.idl \
    Transliteration.idl \
    TransliterationModules.idl \
    TransliterationModulesNew.idl \
    TransliterationModulesExtra.idl \
    TransliterationType.idl \
    UnicodeScript.idl \
    UnicodeType.idl \
    Weekdays.idl \
    WordType.idl \
    XBreakIterator.idl \
    XCalendar.idl \
    XCharacterClassification.idl \
    XCollator.idl \
    XExtendedCalendar.idl \
    XExtendedIndexEntrySupplier.idl \
    XExtendedTransliteration.idl \
    XForbiddenCharacters.idl \
    XIndexEntrySupplier.idl \
    XInputSequenceChecker.idl \
    XExtendedInputSequenceChecker.idl \
    XLocaleData.idl \
    XLocaleData2.idl \
    XNativeNumberSupplier.idl \
    XNumberFormatCode.idl \
    XTextConversion.idl \
    XExtendedTextConversion.idl \
    XScriptTypeDetector.idl \
    XTransliteration.idl \
    reservedWords.idl \
    XOrdinalSuffix.idl \
    OrdinalSuffix.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
