/*************************************************************************
 *
 *  $RCSfile: txtflde.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-12 15:56:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/** @#file
 *  XML export of all text fields
 */

#ifndef _XMLOFF_TXTFLDE_HXX
#define _XMLOFF_TXTFLDE_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _RTL_USTRING
#include <rtl/ustring>
#endif

class SvXMLExport;
class SvXMLNumFmtExport;

namespace com { namespace sun { namespace star { namespace util {
struct DateTime;
}}}}

/// field IDs,
//   including translation between UNO speak and XML speak if appropriate

enum FieldIdEnum {
    FIELD_ID_SENDER,        // sender == extended user
    FIELD_ID_AUTHOR,
    FIELD_ID_DATE,          // current date
    FIELD_ID_TIME,          // current time (+date)
    FIELD_ID_PAGENUMBER,    // page number
    FIELD_ID_PAGESTRING,    // page contination string (page number string)
    FIELD_ID_REFPAGE_SET,   // set reference page
    FIELD_ID_REFPAGE_GET,   // get reference page number

    FIELD_ID_PLACEHOLDER,   // placeholder field == jump edit field

    FIELD_ID_VARIABLE_DECL, // field type for set variable
    FIELD_ID_VARIABLE_GET,  // get variable == get expression
    FIELD_ID_VARIABLE_SET,  // set variable == set expression
    FIELD_ID_VARIABLE_INPUT,    // input field (variable)
    FIELD_ID_USER_DECL,     // field type for user field
    FIELD_ID_USER_GET,      // user field
    FIELD_ID_USER_INPUT,    // input field (user field)
    FIELD_ID_TEXT_INPUT,    // input field (text)
    FIELD_ID_EXPRESSION,    // expression field = formula field
    FIELD_ID_SEQUENCE_DECL, // field type for sequence
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
    FIELD_ID_DOCINFO_INFORMATION0,  // ATTENTION: INFO-fields must be numbered
    FIELD_ID_DOCINFO_INFORMATION1,  //            consecutively (to find out
    FIELD_ID_DOCINFO_INFORMATION2,  //            number)
    FIELD_ID_DOCINFO_INFORMATION3,
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

    FIELD_ID_UNKNOWN        // invalid or unknown field type!
};

namespace com { namespace sun { namespace star{
    namespace text { class XTextField; }
    namespace beans { class XPropertySet; }
    namespace frame { class XModel; }
} } }

namespace rtl { class OUString; }



class XMLTextFieldExport
{
    SvXMLExport& rExport;

public:

    XMLTextFieldExport( SvXMLExport& rExp );
    virtual ~XMLTextFieldExport();

    /// export this field. Assumes styles have been exported.
    /// To be called for every field in the document body.
    void ExportField(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::text::XTextField > & rTextField );

    /// export styles for this field (if appropriate).
    /// (requires final call to SvXMLNumFmtExport)
    /// to be called for every field during style export.
    void ExportFieldAutoStyle(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::text::XTextField > & rTextField );

    /// export field declarations.
    /// to be called once at beginning of document body.
    void ExportFieldDeclarations();

    // determine element or attribute names
    // (public, because they may be useful in related XML export classes)
    static const sal_Char* MapPlaceholderType(sal_uInt16 nType);
    static const sal_Char* MapTemplateDisplayFormat(sal_Int16 nType);
    static const sal_Char* MapChapterDisplayFormat(sal_Int16 nType);
    static const sal_Char* MapFilenameDisplayFormat(sal_Int16 nType);
    static const sal_Char* MapDocInfoFieldName(enum FieldIdEnum nToken);
    static const sal_Char* MapReferenceSource(sal_Int16 nType);
    static const sal_Char* MapReferenceType(sal_Int16 nType);
    static const sal_Char* MapCountFieldName(sal_Int16 nToken);
    const sal_Char* MapPageNumberName(const ::com::sun::star::uno::Reference<
                      ::com::sun::star::beans::XPropertySet> & xPropSet,
                      sal_Int32& nOffset);  /// also adjust page offset
    const sal_Char* MapAuthorFieldName(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::beans::XPropertySet > & xPropSet);
    const sal_Char* MapSenderFieldName(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::beans::XPropertySet > & xPropSet);

protected:

    SvXMLExport& GetExport() { return rExport; }

    /// export an empty element
    void ExportElement(const sal_Char* pElementName,    /// element name
                       sal_Bool bAddSpace = sal_False); /// add blanks around
                                                        /// element?

    /// export an element with string content
    void ExportElement(const sal_Char* pElementName,    /// element name
                       const ::rtl::OUString& sContent, /// element content
                       sal_Bool bAddSpace = sal_False); /// add blanks around
                                                        /// element?



    /// export a boolean attribute
    void ProcessBoolean(
        const sal_Char* pXmlName,   /// attribute name (namespace text)
        sal_Bool bBool,     /// attribute value
        sal_Bool bDefault); /// attribute default; omit, if attribute differs

    /// export an integer attribute
    void ProcessInteger(
        const sal_Char* pXmlName,   /// attribute name (namespace text)
        sal_Int32 nNum);            /// attribute value

    /// export an integer attribute, omit if default
    void ProcessInteger(
        const sal_Char* pXmlName,   /// attribute name (namespace text)
        sal_Int32 nNum,             /// attribute value
        sal_Int32 nDefault);        /// default value

    /// export a string attribute
    void ProcessString(
        const sal_Char* pXmlName,       /// attribute name (namespace text)
        const ::rtl::OUString& sValue,  /// attribute value
        sal_Bool bOmitEmpty = sal_False); /// omit attribute, if value is empty

    /// export a string attribute, omit if default
    void ProcessString(
        const sal_Char* pXmlName,       /// attribute name (namespace text)
        const ::rtl::OUString& sValue,  /// attribute value
        const ::rtl::OUString& sDefault);/// default value; omit if equal

    /// export a string attribute
    void ProcessString(
        const sal_Char* pXmlName,       /// attribute name (namespace text)
        const sal_Char* pValue,         /// attribute value
        sal_Bool bOmitEmpty = sal_False); /// omit attribute, if value is empty

    /// export a string attribute, omit if default
    void ProcessString(
        const sal_Char* pXmlName,       /// attribute name (namespace text)
        const sal_Char* pValue,         /// attribute value
        const sal_Char* pDefault);      /// attribute default

    /// export a numbering format (numeric, roman, alphabetic, etc.)
    void ProcessNumberingType(
        sal_Int16 nNumberingType);      /// numbering type key

    /// export display attribute (value, formula, none)
    void ProcessDisplay(sal_Bool bIsVisible,    /// is visible?
                        sal_Bool bIsCommand,    /// is show command/show name?
                        sal_Bool bDefault = sal_True);  /// omit, if default

    /// export all data-style related attributes
    void ProcessValueAndType(
        sal_Bool bIsString,     /// do we process a string or a number?
        sal_Int32 nFormatKey,   /// format key for NumberFormatter; possibly -1
        const ::rtl::OUString& sContent, /// string content; possibly invalid
        const ::rtl::OUString& sDefault, /// default string
        double fValue,          /// float content; possibly invalid
        sal_Bool bExportValue,  /// export value attribute?
        sal_Bool bExportValueType,  /// export value-type attribute?
        sal_Bool bExportStyle); /// export style-sttribute?

    /// export times, dates and durations according to ISO 8601
    void ProcessDateTime(
        const sal_Char* sXMLName,   /// name of attribute
        double dValue,              /// date/time value
        sal_Bool bIsDate,           /// export as date (rather than date/time)?
        sal_Bool bIsDuration = sal_False,           /// export as duration
        sal_Bool bOmitDurationIfZero = sal_True);   /// omit zero-length durat.

    /// export a date, time, or duration
    void ProcessDateTime(
        const sal_Char* sXMLName,   /// name of attribute
        sal_Int32 nMinutes,             /// date/time value in minutes
        sal_Bool bIsDate,           /// export as date?
        sal_Bool bIsDuration,       /// export as duration?
        sal_Bool bOmitDurationIfZero);  /// omit zero-length durations

    /// export times, dates and durations according to ISO 8601
    void ProcessDateTime(
        const sal_Char* sXMLName,   /// name of attribute
        const ::com::sun::star::util::DateTime& rTime,      /// date/time value
        sal_Bool bIsDate );             /// export as date (rather than date/time)?

    /// for XDependentTextFields, get PropertySet of FieldMaster
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >
    GetMasterPropertySet(const ::com::sun::star::uno::Reference <
                         ::com::sun::star::text::XTextField > & rTextField);

//  /// get PropertySet of (any) DependentTextField for this FieldMaster
//  ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >
//  GetDependentFieldPropertySet(const ::rtl::OUString& sFieldMasterName,
//                               const ::com::sun::star::uno::Reference<
//                               ::com::sun::star::frame::XModel> & rModel);

    /// get PropertySet of (any) DependentTextField for this FieldMaster
    // (real version)
    sal_Bool GetDependentFieldPropertySet(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xmaster,
        ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xField);


    /// get field ID from XTextField (and it's Property-Set)
    enum FieldIdEnum GetFieldID(const ::com::sun::star::uno::Reference <
                          ::com::sun::star::text::XTextField > & rTextField,
                          const ::com::sun::star::uno::Reference <
                          ::com::sun::star::beans::XPropertySet > & xPropSet);

    /// get field ID from XTextField service name (and it's PropertySet)
    enum FieldIdEnum MapFieldName(const ::rtl::OUString& sFieldName,
                            const ::com::sun::star::uno::Reference <
                            ::com::sun::star::beans::XPropertySet> & xPropSet);

    /// determine, whether field has string or numeric content
    sal_Bool IsStringField(sal_uInt16 nFieldType,   /// field ID
                           const ::com::sun::star::uno::Reference <
                           ::com::sun::star::beans::XPropertySet > & xPropSet);


    /// explode a field master name into field type and field name
    sal_Bool ExplodeFieldMasterName(
        const ::rtl::OUString& sMasterName, /// name as returned by SO API
        ::rtl::OUString& sFieldType,        /// out: field type
        ::rtl::OUString& sVarName);         /// out: variable name

    /// make reference name for a foot- or endnote
    static ::rtl::OUString MakeFootnoteRefName(sal_Int16 nSeqNo);

    /// make reference name for a sequence field
    static ::rtl::OUString MakeSequenceRefName(sal_Int16 nSeqNo,
                                              const ::rtl::OUString& rSeqName);

private:
    // constants

    // service names
    const ::rtl::OUString sServicePrefix;
    const ::rtl::OUString sFieldMasterPrefix;

    // property names
    const ::rtl::OUString sPropertyContent;
    const ::rtl::OUString sPropertyIsFixed;
    const ::rtl::OUString sPropertyAuthorFullname;
    const ::rtl::OUString sPropertyFieldSubType;
    const ::rtl::OUString sPropertyHint;
    const ::rtl::OUString sPropertyPlaceholder;
    const ::rtl::OUString sPropertyPlaceholderType;
    const ::rtl::OUString sPropertyIsVisible;
    const ::rtl::OUString sPropertyIsShowFormula;
    const ::rtl::OUString sPropertyIsInput;
    const ::rtl::OUString sPropertyIsExpression;
    const ::rtl::OUString sPropertyNumberFormat;
    const ::rtl::OUString sPropertyVariableName;
    const ::rtl::OUString sPropertySubType;
    const ::rtl::OUString sPropertyName;
    const ::rtl::OUString sPropertyVariableSubType;
    const ::rtl::OUString sPropertyValue;
    const ::rtl::OUString sPropertyChapterNumberingLevel;
    const ::rtl::OUString sPropertyNumberingSeparator;
    const ::rtl::OUString sPropertyNumberingType;
    const ::rtl::OUString sPropertyDataBaseName;
    const ::rtl::OUString sPropertyDataTableName;
    const ::rtl::OUString sPropertyDateTimeValue;
    const ::rtl::OUString sPropertyDataColumnName;
    const ::rtl::OUString sPropertySetNumber;
    const ::rtl::OUString sPropertyIsDataBaseFormat;
    const ::rtl::OUString sPropertyUserText;
    const ::rtl::OUString sPropertyOffset;
    const ::rtl::OUString sPropertyCondition;
    const ::rtl::OUString sPropertyDateTime;
    const ::rtl::OUString sPropertyTrueContent;
    const ::rtl::OUString sPropertyFalseContent;
    const ::rtl::OUString sPropertyRevision;
    const ::rtl::OUString sPropertyChapterFormat;
    const ::rtl::OUString sPropertyFileFormat;
    const ::rtl::OUString sPropertyLevel;
    const ::rtl::OUString sPropertyIsDate;
    const ::rtl::OUString sPropertyAdjust;
    const ::rtl::OUString sPropertyOn;
    const ::rtl::OUString sPropertyMacro;
    const ::rtl::OUString sPropertyReferenceFieldPart;
    const ::rtl::OUString sPropertyReferenceFieldType;
    const ::rtl::OUString sPropertyReferenceFieldSource;
    const ::rtl::OUString sPropertySequenceNumber;
    const ::rtl::OUString sPropertySourceName;
    const ::rtl::OUString sPropertyIsAutomaticUpdate;
    const ::rtl::OUString sPropertyDependentTextFields;
    const ::rtl::OUString sPropertyDDECommandType;
    const ::rtl::OUString sPropertyDDECommandFile;
    const ::rtl::OUString sPropertyDDECommandElement;
    const ::rtl::OUString sPropertySequenceValue;
    const ::rtl::OUString sPropertyURL;
    const ::rtl::OUString sPropertyTargetFrame;

    const ::rtl::OUString sEmpty;
};






#endif
