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

/** @#file
 *  XML export of all text fields
 */

#ifndef INCLUDED_XMLOFF_INC_TXTFLDE_HXX
#define INCLUDED_XMLOFF_INC_TXTFLDE_HXX

#include <com/sun/star/uno/Reference.h>
#include <xmloff/xmlnamespace.hxx>

#include <rtl/ustring.hxx>
#include <xmloff/xmltoken.hxx>

#include <map>
#include <set>
#include <memory>
#include <string_view>

class SvXMLExport;
struct XMLPropertyState;

namespace com::sun::star {
    namespace util { struct DateTime; }
    namespace util { struct Date; }
    namespace text { class XTextField; }
    namespace text { class XText; }
    namespace beans { class XPropertySet; }
    namespace beans { class XPropertySetInfo; }
    namespace frame { class XModel; }
    namespace uno { template<typename A> class Sequence; }
}


/// field IDs,
//   including translation between UNO speak and XML speak if appropriate

enum FieldIdEnum {
    FIELD_ID_SENDER,        // sender == extended user
    FIELD_ID_AUTHOR,
    FIELD_ID_DATE,          // current date
    FIELD_ID_TIME,          // current time (+date)
    FIELD_ID_PAGENAME,      // page/slide name
    FIELD_ID_PAGENUMBER,    // page number
    FIELD_ID_PAGESTRING,    // page continuation string (page number string)
    FIELD_ID_REFPAGE_SET,   // set reference page
    FIELD_ID_REFPAGE_GET,   // get reference page number

    FIELD_ID_PLACEHOLDER,   // placeholder field == jump edit field

    FIELD_ID_VARIABLE_GET,  // get variable == get expression
    FIELD_ID_VARIABLE_SET,  // set variable == set expression
    FIELD_ID_VARIABLE_INPUT,    // input field (variable)
    FIELD_ID_USER_GET,      // user field
    FIELD_ID_USER_INPUT,    // input field (user field)
    FIELD_ID_TEXT_INPUT,    // input field (text)
    FIELD_ID_EXPRESSION,    // expression field = formula field
    FIELD_ID_SEQUENCE,      // sequence field

    FIELD_ID_DATABASE_NEXT,     // select next row
    FIELD_ID_DATABASE_SELECT,   // select row # (NumSet)
    FIELD_ID_DATABASE_DISPLAY,  // display data (form letter field)
    FIELD_ID_DATABASE_NAME,     // display current db name (database name)
    FIELD_ID_DATABASE_NUMBER,   // display row # (SetNumber)

    FIELD_ID_DOCINFO_CREATION_AUTHOR,   // docinfo fields
    FIELD_ID_DOCINFO_CREATION_TIME,
    FIELD_ID_DOCINFO_CREATION_DATE,
    FIELD_ID_DOCINFO_DESCRIPTION,
    FIELD_ID_DOCINFO_CUSTOM,
    FIELD_ID_DOCINFO_PRINT_TIME,
    FIELD_ID_DOCINFO_PRINT_DATE,
    FIELD_ID_DOCINFO_PRINT_AUTHOR,
    FIELD_ID_DOCINFO_TITLE,
    FIELD_ID_DOCINFO_SUBJECT,
    FIELD_ID_DOCINFO_KEYWORDS,
    FIELD_ID_DOCINFO_REVISION,
    FIELD_ID_DOCINFO_EDIT_DURATION,
    FIELD_ID_DOCINFO_SAVE_TIME,
    FIELD_ID_DOCINFO_SAVE_DATE,
    FIELD_ID_DOCINFO_SAVE_AUTHOR,

    FIELD_ID_CONDITIONAL_TEXT,          // conditionally choose between 2 texts
    FIELD_ID_HIDDEN_TEXT,               // conditionally hide a text
    FIELD_ID_HIDDEN_PARAGRAPH,          // conditionally hide a paragraph

    FIELD_ID_TEMPLATE_NAME,     // display name of template
    FIELD_ID_CHAPTER,           // display name/number of current chapter
    FIELD_ID_FILE_NAME,         // display name of current file

    FIELD_ID_COUNT_PARAGRAPHS,      // statistics fields: - paragraphs
    FIELD_ID_COUNT_WORDS,           //                    - words
    FIELD_ID_COUNT_CHARACTERS,      //                    - chars
    FIELD_ID_COUNT_PAGES,           //                    - pages
    FIELD_ID_COUNT_TABLES,          //                    - tables
    FIELD_ID_COUNT_GRAPHICS,        //                    - graphics
    FIELD_ID_COUNT_OBJECTS,         //                    - objects

    FIELD_ID_MACRO,                 // macro fields
    FIELD_ID_REF_REFERENCE,         // get reference field (reference)
    FIELD_ID_REF_SEQUENCE,          // get reference field (sequence)
    FIELD_ID_REF_BOOKMARK,          // get reference field (bookmark)
    FIELD_ID_REF_FOOTNOTE,          // get reference field (footnote)
    FIELD_ID_REF_ENDNOTE,           // get reference field (endnote)
    FIELD_ID_DDE,                   // DDE field

    FIELD_ID_BIBLIOGRAPHY,          // bibliography index entry

    FIELD_ID_SHEET_NAME,            // name of current (spread-)sheet
    FIELD_ID_URL,                   // URL field (only Calc, Draw, Impress)

    FIELD_ID_SCRIPT,                // script fields (for HTML pages, mostly)
    FIELD_ID_ANNOTATION,            // annotation (notice) field

    FIELD_ID_COMBINED_CHARACTERS,   // combined characters (asian typography)

    FIELD_ID_META,                  // text:meta-field (RDF metadata)

    FIELD_ID_MEASURE,               // for measure shapes

    FIELD_ID_TABLE_FORMULA,         // DEPRECATED: table formulas (Writer 2.0)
    FIELD_ID_DROP_DOWN,             // DEPRECATED: dropdown fields (WW8)

    FIELD_ID_DRAW_HEADER,
    FIELD_ID_DRAW_FOOTER,
    FIELD_ID_DRAW_DATE_TIME,

    FIELD_ID_UNKNOWN        // invalid or unknown field type!
};


class XMLTextFieldExport final
{
    SvXMLExport& rExport;

    /// store used text field master names (NULL means: don't collect)
    std::unique_ptr< ::std::map<
            css::uno::Reference< css::text::XText >,
            ::std::set< OUString > > >
        pUsedMasters;

public:

    XMLTextFieldExport( SvXMLExport& rExp,
                        /// XMLPropertyState for the combined characters field
                        std::unique_ptr<XMLPropertyState> pCombinedCharState );
    ~XMLTextFieldExport();

    /// Export this field and the surrounding span element with the formatting.
    /// To be called for every field in the document body.
    void ExportField(const css::uno::Reference < css::text::XTextField > & rTextField,
                     bool bProgress, bool & rPrevCharIsSpace);

    /// collect styles (character styles, data styles, ...) for this field
    /// (if appropriate).
    /// Also collect used field masters (if pUsedMasters is set)
    /// to be called for every field during style export.
    void ExportFieldAutoStyle(const css::uno::Reference < css::text::XTextField > & rTextField,
                 const bool bProgress, const bool bRecursive );

    /// export field declarations.
    /// to be called once at beginning of document body.
    void ExportFieldDeclarations();

    /// export field declarations for fields used in the particular XText.
    /// (Requires that a list of used field declarations has previously been
    ///  built-up in ExportFieldAutoStyle() )
    void ExportFieldDeclarations(
        const css::uno::Reference < css::text::XText > & rText);

    /// export all field declarations, or only those that have been used?
    /// Calling this method will reset the list of used field declarations.
    void SetExportOnlyUsedFieldDeclarations(
        bool bExportOnlyUsed = true);

    // determine element or attribute names
    // (public, because they may be useful in related XML export classes)
    static enum ::xmloff::token::XMLTokenEnum MapPlaceholderType(sal_uInt16 nType);
    static enum ::xmloff::token::XMLTokenEnum MapTemplateDisplayFormat(sal_Int16 nType);
    static enum ::xmloff::token::XMLTokenEnum MapChapterDisplayFormat(sal_Int16 nType);
    static enum ::xmloff::token::XMLTokenEnum MapFilenameDisplayFormat(sal_Int16 nType);
    static enum ::xmloff::token::XMLTokenEnum MapDocInfoFieldName(enum FieldIdEnum nToken);
    static enum ::xmloff::token::XMLTokenEnum MapReferenceSource(sal_Int16 nType);
    static enum ::xmloff::token::XMLTokenEnum MapReferenceType(sal_Int16 nType);
    static enum ::xmloff::token::XMLTokenEnum MapCountFieldName(FieldIdEnum nToken);
    static enum ::xmloff::token::XMLTokenEnum MapBibliographyFieldName(std::u16string_view sName);
    static enum ::xmloff::token::XMLTokenEnum MapMeasureKind(sal_Int16 nKind);
    static enum ::xmloff::token::XMLTokenEnum MapPageNumberName(const css::uno::Reference< css::beans::XPropertySet> & xPropSet,
                      sal_Int32& nOffset);  /// also adjust page offset
    static enum ::xmloff::token::XMLTokenEnum MapAuthorFieldName(const css::uno::Reference< css::beans::XPropertySet > & xPropSet);
    static enum ::xmloff::token::XMLTokenEnum MapSenderFieldName(const css::uno::Reference< css::beans::XPropertySet > & xPropSet);

private:

    SvXMLExport& GetExport() { return rExport; }

    /// export a field after <text:span> is already written
    void ExportFieldHelper(
        const css::uno::Reference< css::text::XTextField> & rTextField,
        const css::uno::Reference< css::beans::XPropertySet> & rPropSet,
        const css::uno::Reference< css::beans::XPropertySet> & rRangePropSet,
        enum FieldIdEnum nToken,
        bool bProgress,
        bool & rPrevCharIsSpace);

    /// export an empty element
    void ExportElement(enum ::xmloff::token::XMLTokenEnum eElement, /// element token
                       bool bAddSpace = false); /// add blanks around
                                                        /// element?

    /// export an element with string content
    void ExportElement(enum ::xmloff::token::XMLTokenEnum eElement, /// element token
                       const OUString& sContent); /// element content

    /// export a macro (as used in the macro field)
    void ExportMacro( const css::uno::Reference< css::beans::XPropertySet> & rPropSet,
                      const OUString& rContent);

    /// export text:meta-field (RDF metadata)
    void ExportMetaField( const css::uno::Reference< css::beans::XPropertySet> & i_xMeta,
                          bool i_bAutoStyles, bool i_bProgress,
                          bool & rPrevCharIsSpace);

    /// export a boolean attribute
    void ProcessBoolean(
        enum ::xmloff::token::XMLTokenEnum eXmlName,    /// attribute token (namespace text)
        bool bBool,     /// attribute value
        bool bDefault); /// attribute default; omit, if attribute differs

    /// export an integer attribute
    void ProcessInteger(
        enum ::xmloff::token::XMLTokenEnum eXmlName,    /// attribute token (namespace text)
        sal_Int32 nNum);            /// attribute value

    /// export an integer attribute, omit if default
    void ProcessIntegerDef(
        enum ::xmloff::token::XMLTokenEnum eXmlName,    /// attribute token (namespace text)
        sal_Int32 nNum,             /// attribute value
        sal_Int32 nDefault);        /// default value

    /// export a string attribute
    void ProcessString(
        enum ::xmloff::token::XMLTokenEnum eXmlName,        /// attribute token (namespace text)
        const OUString& sValue,  /// attribute value
        bool bOmitEmpty = false, /// omit attribute, if value is empty
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);   /// attribute name prefix

    /// export a string attribute that gets a QName value
    void ProcessString(
        enum ::xmloff::token::XMLTokenEnum eXmlName,        /// attribute token (namespace text)
        sal_uInt16 nValuePrefix,
        const OUString& sValue);  /// attribute value


    /// export a string attribute, omit if default
    void ProcessString(
        enum ::xmloff::token::XMLTokenEnum eXmlName,        /// attribute token (namespace text)
        const OUString& sValue,  /// attribute value
        std::u16string_view sDefault); /// default value; omit if equal

    /// export a string attribute, omit if default
    void ProcessString(
        enum ::xmloff::token::XMLTokenEnum eXmlName,        /// attribute token (namespace text)
        sal_uInt16 nValuePrefix,
        const OUString& sValue,  /// attribute value
        std::u16string_view sDefault); /// default value; omit if equal

    /// export a string attribute
    void ProcessString(
        enum ::xmloff::token::XMLTokenEnum eXmlName,        /// attribute token (namespace text)
        enum ::xmloff::token::XMLTokenEnum eValue,          /// attribute token
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);   /// attribute name prefix

    /// export a string attribute, omit if default
    void ProcessString(
        enum ::xmloff::token::XMLTokenEnum eXmlName,        /// attribute token (namespace text)
        enum ::xmloff::token::XMLTokenEnum eValue,          /// attribute value token
        enum ::xmloff::token::XMLTokenEnum eDefault);       /// default value token

    /// export a string as a sequence of paragraphs
    void ProcessParagraphSequence(
        /// string containing the paragraphs
        std::u16string_view sParagraphSequence);

    /// export a numbering format (numeric, roman, alphabetic, etc.)
    void ProcessNumberingType(
        sal_Int16 nNumberingType);      /// numbering type key

    /// export display attribute (value, formula, none)
    void ProcessDisplay(bool bIsVisible,    /// is visible?
                        bool bIsCommand);    /// is show command/show name?

    /// export all data-style related attributes
    void ProcessValueAndType(
        bool bIsString,     /// do we process a string or a number?
        sal_Int32 nFormatKey,   /// format key for NumberFormatter; possibly -1
        const OUString& sContent, /// string content; possibly invalid
        std::u16string_view sDefault, /// default string
        double fValue,          /// float content; possibly invalid
        bool bExportValue,  /// export value attribute?
        bool bExportValueType,  /// export value-type attribute?
        bool bExportStyle,  /// export style-attribute?
        bool bForceSystemLanguage, /// no style language export
        bool bTimeStyle = false); /// exporting a time style?

    /// export times, dates and durations according to ISO 8601
    void ProcessDateTime(
        enum ::xmloff::token::XMLTokenEnum eXMLName,    /// attribute token
        double dValue,              /// date/time value
        bool bIsDate,           /// export as date (rather than date/time)?
        bool bIsDuration,           /// export as duration
        bool bOmitDurationIfZero = true,    /// omit zero-length durat.
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);   /// attribute name prefix

    /// export a date, time, or duration
    void ProcessDateTime(
        enum ::xmloff::token::XMLTokenEnum eXMLName,    /// attribute token
        sal_Int32 nMinutes,             /// date/time value in minutes
        bool bIsDate,           /// export as date?
        bool bIsDuration);       /// export as duration?

    /// export times, dates and durations according to ISO 8601
    void ProcessDateTime(
        enum ::xmloff::token::XMLTokenEnum eXMLName,    /// attribute token
        const css::util::DateTime& rTime);      /// date/time value

    /// export time or dateTime
    void ProcessTimeOrDateTime(
        enum ::xmloff::token::XMLTokenEnum eXMLName,    /// attribute token
        const css::util::DateTime& rTime);  /// date/time value

    /// export all attributes for bibliography data fields
    void ProcessBibliographyData(
        const css::uno::Reference <
            css::beans::XPropertySet > & rPropertySet);

    /// export CommandTypeAttribute
    void ProcessCommandType(
        sal_Int32 nCommandType);        /// css::sdb::CommandType

    void ProcessStringSequence(
        const css::uno::Sequence<OUString>& rSequence,
        const OUString& sSelected );

    void ProcessStringSequence(
        const css::uno::Sequence<OUString>& rSequence,
        sal_Int32 nSelected );

    /// export attributes that describe a data source
    void ExportDataBaseElement(
        enum ::xmloff::token::XMLTokenEnum eElement,
        const OUString& sContent,
        const css::uno::Reference < css::beans::XPropertySet > & rPropertySet,
        const css::uno::Reference < css::beans::XPropertySetInfo > & rPropertySetInfo );

    /// for XDependentTextFields, get PropertySet of FieldMaster
    static css::uno::Reference < css::beans::XPropertySet >
    GetMasterPropertySet(const css::uno::Reference < css::text::XTextField > & rTextField);

    /// get PropertySet of (any) DependentTextField for this FieldMaster
    static bool GetDependentFieldPropertySet(
        const css::uno::Reference< css::beans::XPropertySet> & xmaster,
        css::uno::Reference< css::beans::XPropertySet> & xField);


    /// get field ID from XTextField (and it's Property-Set)
    static enum FieldIdEnum GetFieldID(const css::uno::Reference < css::text::XTextField > & rTextField,
                          const css::uno::Reference < css::beans::XPropertySet > & xPropSet);

    /// get field ID from XTextField service name (and it's PropertySet)
    static enum FieldIdEnum MapFieldName(std::u16string_view sFieldName,
                            const css::uno::Reference < css::beans::XPropertySet> & xPropSet);

    /// determine, whether field has string or numeric content
    static bool IsStringField(FieldIdEnum nFieldType,  /// field ID
                           const css::uno::Reference < css::beans::XPropertySet > & xPropSet);


    /// explode a field master name into field type and field name
    static void ExplodeFieldMasterName(
        const OUString& sMasterName, /// name as returned by SO API
        OUString& sFieldType,        /// out: field type
        OUString& sVarName);         /// out: variable name

    /// make reference name for a foot- or endnote
    static OUString MakeFootnoteRefName(sal_Int16 nSeqNo);

    /// make reference name for a sequence field
    static OUString MakeSequenceRefName(sal_Int16 nSeqNo,
                                              std::u16string_view rSeqName);

    std::unique_ptr<XMLPropertyState> pCombinedCharactersPropertyState;

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
