#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: obo $ $Date: 2005-11-16 09:59:40 $
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

PRJNAME=api

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
    XNativeNumberSupplier.idl \
    XNumberFormatCode.idl \
    XTextConversion.idl \
    XExtendedTextConversion.idl \
    XScriptTypeDetector.idl \
    XTransliteration.idl \
    reservedWords.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
