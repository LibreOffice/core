#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.19 $
#
#   last change: $Author: vg $ $Date: 2003-04-24 10:53:43 $
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
    XLocaleData.idl \
    XNativeNumberSupplier.idl \
    XNumberFormatCode.idl \
    XTextConversion.idl \
    XScriptTypeDetector.idl \
    XTransliteration.idl \
    reservedWords.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
