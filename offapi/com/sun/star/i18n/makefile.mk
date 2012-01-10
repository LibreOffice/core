#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
