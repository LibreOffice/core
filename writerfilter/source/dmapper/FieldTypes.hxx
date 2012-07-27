/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_FIELD_TYPES_HXX
#define INCLUDED_FIELD_TYPES_HXX

namespace writerfilter {
namespace dmapper {

enum FieldId
{
    /* ADDRESSBLOCK \d \* MERGEFORMAT -> Addressblock completely unsupported*/
    FIELD_ADDRESSBLOCK
    /* ADVANCE \d downvalue \l leftvalue \r rightvalue \u upvalue \x xvalue \y yvalue -> unsupported*/
    ,FIELD_ADVANCE
    /* ASK bookmarkname "hint" \d defaultanswer \o \* MERGEFORMAT ->
     the hint is not always quoted, inputfield with default answer, prompt before merge (\o)
     */
    ,FIELD_ASK
    /* AUTONUM \* Numberingswitch ->
     mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTONUM
    /* AUTONUMLGL \* Numberingswitch ->
     mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTONUMLGL
    /* AUTONUMOUT \* Numberingswitch ->
        mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTONUMOUT
    /* AUTHOR NewAuthor \* defaultswitch \* MERGEFORMAT ->
        mapped to sequence field "AutoNr"
     */
    ,FIELD_AUTHOR
    /* COMMENTS "comment" \* MERGEFORMAT ->
        Docinfo-Comments
     */
    ,FIELD_COMMENTS
    /* CREATEDATE \h \* MERGEFORMAT ->
     docinfo-created-date
     */
    ,FIELD_CREATEDATE
    /* DATE \@ "number format" \s \* MERGEFORMAT ->
        ww8filterimprovement: multiple languages now supported
     */
    ,FIELD_DATE
    /* DOCPROPERTY propertyname \* MERGEFORMAT ->
        ww8filterimprovement: some fields imported as functionally equivalent fields if possible,
        the others imported as UserField
     */
    ,FIELD_DOCPROPERTY
    /* DOCVARIABLE Name \* MERGEFORMAT ->
     ww8filterimprovement: now imported as user fields
     */
    ,FIELD_DOCVARIABLE
    /* EDITTIME \# "displayformat" \* Numberingswitch \* MERGEFORMAT ->
        DocInfo-Modified-Date
        ww8filterimprovement: multiple languages now supported
     */
    ,FIELD_EDITTIME
    ,FIELD_EQ
    /* FILLIN "text to fill in" \d defaultanswer \o \* MERGEFORMAT ->
        Function-InputField
     */
    ,FIELD_FILLIN
    /* FILENAME \p \* * MERGEFORMAT ->
        file name (\p with path)
     */
    ,FIELD_FILENAME
    /* FILESIZE \* NumberingType \* MERGEFORMAT ->
     not imported in old ww8 filter, see lcl_ParseNumberingType
     todo find alternative field
     */
    ,FIELD_FILESIZE
    /* =formula \# "number format"
     todo find alternative field
     */
    ,FIELD_FORMULA
    /* FORMCHECKBOX */
    ,FIELD_FORMCHECKBOX
    /* FORMDROPDOWN */
    ,FIELD_FORMDROPDOWN
    /* FORMTEXT */
    ,FIELD_FORMTEXT
    /* GOTOBUTTON text \* MERGEFORMAT ->
        not imported in old ww8 filter
        todo find alternative field
     */
    ,FIELD_GOTOBUTTON
    /* HYPERLINK "link" \* MERGEFORMAT ->
        not imported in old ww8 filter
        ww8filterimprovement: now imported as hyperlink
     */
    ,FIELD_HYPERLINK
    /* IF condition "then text" "else text" ->
        not imported in old ww8 filter
        ww8filterimprovement: now imported
        todo: condition, if text, else text still missing
     */
    ,FIELD_IF
    /* INFO NameOfInfo \* MERGEFORMAT -> old
        todo: filter imports wrong?
     */
    ,FIELD_INFO
    /* INCLUDEPICTURE path \* MERGEFORMAT->
     old filter imports an embedded picture
     todo: not yet supported
     */
    ,FIELD_INCLUDEPICTURE
    /* KEYWORDS keyword \* defaultswitch \* Numberingswitch \* MERGEFORMAT ->
     DocInfo Keywords
     */
    ,FIELD_KEYWORDS
    /* LASTSAVEDBY \* MERGEFORMAT ->
     DocInfo-Modified-Author
     */
    ,FIELD_LASTSAVEDBY
    /* MACROBUTTON MacroName quick help text ->
     Macro field
     */
    ,FIELD_MACROBUTTON
    /* MERGEFIELD ColumName \b prefix \f suffix \* MERGEFORMAT ->
        ww8filterimprovement: column-only API now upporterd
     */
    ,FIELD_MERGEFIELD
    /* MERGEREC \* MERGEFORMAT ->
     RecordNumber field, maybe without db name
     todo: currently unchecked
     */
    ,FIELD_MERGEREC
    /* MERGESEQ \* MERGEFORMAT ->
     not imported in old ww8 filter
     ww8filterimprovement: now imported
     todo: currently unchecked
     */
    ,FIELD_MERGESEQ
    /* NEXT text ->
     Next record
     todo: currently unchecked
     */
    ,FIELD_NEXT
    /* NEXTIF condition
     todo: condition not imported
     */
    ,FIELD_NEXTIF
    /* PAGE \* Numberingswitch \* MERGEFORMAT ->
     see lcl_ParseNumberingType
     */
    ,FIELD_PAGE
    ,FIELD_PAGEREF
    /* REF targetbkm \f \* MERGEFORMAT ->
        imports a ShowVariable (bookmarkname)?
        \h hyerlink to paragraph
        \p relative to para above/below
        \f refenence number
        \d separator number separator
        \n paragraph number
        \r paragraph number in relative context
        \t suppres non delimiters
        \w paragraph number in full context
        \* Upper/Lower...
     */
    ,FIELD_REF          //
    /* REVNUM \* Numberingswitch \* MERGEFORMAT ->
     DocInfo-revision number
     */
    ,FIELD_REVNUM
    /* SAVEDATE \@ "NumberFormat"\* MERGEFORMAT ->
     DocInfo-modified-date
     */
    ,FIELD_SAVEDATE
    /* SECTION \* NumberFormat \* MERGEFORMAT ->
        not imported in old ww8 filter see lcl_ParseNumberingType
        todo: find alternative
     */
    ,FIELD_SECTION
    /* SECTIONPAGES \* NumberFormat \* MERGEFORMAT ->
     not imported in old ww8 filter see lcl_ParseNumberingType
        todo: find alternative
     */
    ,FIELD_SECTIONPAGES
    /* SEQ sequencename \h \c \n \r \s \* MERGEFORMAT ->
     number range name:sequencename value:sequencename+1
     todo: only partially implemented, switches unsupported
     */
    ,FIELD_SEQ
    /* SET bookmarkname newtext \* MERGEFORMAT ->
     SetVariable bookmarkname = newtext
     todo: not implemented yet
     */
    ,FIELD_SET
    /* SKIPIF condition \* MERGEFORMAT ->
     ??
     todo: not implemented yet
     */
    ,FIELD_SKIPIF
    /* STYLEREF stylename \* MERGEFORMAT ->
     not imported in old ww8 filter
     todo: add an equivalent field type
     */
    ,FIELD_STYLEREF
    /* SUBJECT subject \* Defaultswitch \* MERGEFORMAT ->
     DocInfo - subject
     */
    ,FIELD_SUBJECT
    /* SYMBOL symbolnumber \* MERGEFORMAT ->
     inserts a special char (symbolnumber)
     todo: find alternative
     */
    ,FIELD_SYMBOL
    /* TEMPLATE \* Defaultswitch \* MERGEFORMAT
     TemplateName field
     */
    ,FIELD_TEMPLATE
    /* TIME \@ "number format" \* MERGEFORMAT
     ww8filterimprovement: multiple languages now supported
     */
    ,FIELD_TIME
    /* TITLE \* Defaultswitch \* MERGEFORMAT ->
     DocInfo-title
     */
    ,FIELD_TITLE
    /* USERINITIALS newinitials \* MERGEFORMAT ->
     ExtendedUser field (SHORTCUT)
     */
    ,FIELD_USERINITIALS
    /* USERADDRESS \* MERGEFORMAT ->
     not imported in old ww8 filter
        todo: find alternative
     */
    ,FIELD_USERADDRESS
    /* USERNAME newusername \* MERGEFORMAT ->
     not imported in old ww8 filter
     todo: import as extended user field(s)
     */
    ,FIELD_USERNAME
    /*
    TOC options:
    \a Builds a table of figures but does not include the captions's label and number
    \b Uses a bookmark to specify area of document from which to build table of contents
    \c Builds a table of figures of the given label
    \d Defines the separator between sequence and page numbers
    \f Builds a table of contents using TC entries instead of outline levels
    \h Hyperlinks the entries and page numbers within the table of contents
    \l Defines the TC entries field level used to build a table of contents
    \n Builds a table of contents or a range of entries, sucah as “1-9”, in a table of contents without page numbers
    \o  Builds a table of contents by using outline levels instead of TC entries
    \p Defines the separator between the table entry and its page number
    \s  Builds a table of contents by using a sequence type
    \t  Builds a table of contents by using style names other than the standard outline styles
    \u  Builds a table of contents by using the applied paragraph outline level
    \w Preserve tab characters within table entries
    \x Preserve newline characters within table entries
    \z Hides page numbers within the table of contens when shown in Web Layout View
     */
    ,FIELD_TOC
    /*
     TOC entry: “text”
        \f TC entry in doc with multiple tables
        \l Outline Level
        \n Suppress page numbers
     example: TOC "EntryText \f \l 2 \n
     */
    ,FIELD_TC
    /* document statistic - number of characters
     */
    ,FIELD_NUMCHARS
    /* document statistic - number of words
     */
    ,FIELD_NUMWORDS
    /* document statistic - number of pages
     */
    ,FIELD_NUMPAGES
};

}}
#endif // INCLUDED_FIELD_TYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
