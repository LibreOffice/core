/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

#ifndef _XMLOFF_XMLNMSPE_HXX
#include <bf_xmloff/xmlnmspe.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <bf_xmloff/xmltoken.hxx>
#endif

#include <map>
#include <set>
namespace com { namespace sun { namespace star { 
    namespace util { struct DateTime; }
    namespace util { struct Date; }
    namespace text { class XTextField; } 
    namespace text { class XText; } 
    namespace beans { class XPropertySet; } 
    namespace frame { class XModel; } 
    namespace uno { template<typename A> class Sequence; }
} } }
namespace rtl { class OUString; }
namespace binfilter {

class SvXMLExport;
class SvXMLNumFmtExport;
struct XMLPropertyState;


/// field IDs,
//   including translation between UNO speak and XML speak if appropriate

enum FieldIdEnum {
    FIELD_ID_SENDER,		// sender == extended user
    FIELD_ID_AUTHOR,
    FIELD_ID_DATE,			// current date
    FIELD_ID_TIME,			// current time (+date)
    FIELD_ID_PAGENUMBER,	// page number
    FIELD_ID_PAGESTRING,	// page contination string (page number string)
    FIELD_ID_REFPAGE_SET,	// set reference page
    FIELD_ID_REFPAGE_GET,	// get reference page number

    FIELD_ID_PLACEHOLDER,	// placeholder field == jump edit field

    FIELD_ID_VARIABLE_DECL,	// field type for set variable
    FIELD_ID_VARIABLE_GET,	// get variable == get expression
    FIELD_ID_VARIABLE_SET,	// set variable == set expression
    FIELD_ID_VARIABLE_INPUT,	// input field (variable)
    FIELD_ID_USER_DECL,		// field type for user field
    FIELD_ID_USER_GET,		// user field
    FIELD_ID_USER_INPUT,	// input field (user field)
    FIELD_ID_TEXT_INPUT,	// input field (text)
    FIELD_ID_EXPRESSION,	// expression field = formula field
    FIELD_ID_SEQUENCE_DECL,	// field type for sequence
    FIELD_ID_SEQUENCE,		// sequence field

    FIELD_ID_DATABASE_NEXT,		// select next row
    FIELD_ID_DATABASE_SELECT,	// select row # (NumSet)
    FIELD_ID_DATABASE_DISPLAY,	// display data (form letter field)
    FIELD_ID_DATABASE_NAME,		// display current db name (database name)
    FIELD_ID_DATABASE_NUMBER,	// display row # (SetNumber)

    FIELD_ID_DOCINFO_CREATION_AUTHOR,	// docinfo fields
    FIELD_ID_DOCINFO_CREATION_TIME,
    FIELD_ID_DOCINFO_CREATION_DATE,
    FIELD_ID_DOCINFO_DESCRIPTION,
    FIELD_ID_DOCINFO_INFORMATION0,	// ATTENTION: INFO-fields must be numbered
    FIELD_ID_DOCINFO_INFORMATION1,	// 			  consecutively (to find out
    FIELD_ID_DOCINFO_INFORMATION2,	//			  number)
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

    FIELD_ID_CONDITIONAL_TEXT,			// conditionally choose between 2 texts
    FIELD_ID_HIDDEN_TEXT,				// conditionally hide a text
    FIELD_ID_HIDDEN_PARAGRAPH,			// conditionally hide a paragraph

    FIELD_ID_TEMPLATE_NAME,		// display name of template
    FIELD_ID_CHAPTER,			// display name/number of current chapter
    FIELD_ID_FILE_NAME,			// display name of current file

    FIELD_ID_COUNT_PARAGRAPHS,		// statistics fields: - paragraphs
    FIELD_ID_COUNT_WORDS,			//					  - words
    FIELD_ID_COUNT_CHARACTERS,		//					  - chars
    FIELD_ID_COUNT_PAGES,			//					  - pages
    FIELD_ID_COUNT_TABLES,			//					  - tables
    FIELD_ID_COUNT_GRAPHICS,		//					  - graphics
    FIELD_ID_COUNT_OBJECTS,			//					  - objects

    FIELD_ID_MACRO,					// macro fields
    FIELD_ID_REF_REFERENCE,			// get reference field (reference)
    FIELD_ID_REF_SEQUENCE,			// get reference field (sequence)
    FIELD_ID_REF_BOOKMARK,			// get reference field (bookmark)
    FIELD_ID_REF_FOOTNOTE,			// get reference field (footnote)
    FIELD_ID_REF_ENDNOTE,			// get reference field (endnote)
    FIELD_ID_DDE,					// DDE field

    FIELD_ID_BIBLIOGRAPHY,			// bibliography index entry

    FIELD_ID_SHEET_NAME,			// name of current (spread-)sheet
    FIELD_ID_URL,					// URL field (only Calc, Draw, Impress)

    FIELD_ID_SCRIPT,				// script fields (for HTML pages, mostly)
    FIELD_ID_ANNOTATION,			// annotation (notice) field

    FIELD_ID_COMBINED_CHARACTERS,	// combined characters (asian typography)

    FIELD_ID_MEASURE,				// for measure shapes

    FIELD_ID_TABLE_FORMULA,         // DEPRECATED: table formulas (Writer 2.0)
    FIELD_ID_DROP_DOWN,             // DEPRECATED: dropdown fields (WW8)

    FIELD_ID_UNKNOWN		// invalid or unknown field type!
};

class XMLTextFieldExport
{
    SvXMLExport& rExport;

    /// store used text field master names (NULL means: don't collect)
    ::std::map<
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >,
            ::std::set< ::rtl::OUString > > * 
        pUsedMasters;

public:

    XMLTextFieldExport( SvXMLExport& rExp, 
                        /// XMLPropertyState for the combined characters field
                        XMLPropertyState* pCombinedCharState = NULL );
    virtual ~XMLTextFieldExport();

    /// Export this field and the surrounding span element with the formatting.
    /// To be called for every field in the document body.
    void ExportField(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::text::XTextField > & rTextField );

    /// collect styles (character styles, data styles, ...) for this field
    /// (if appropriate).
    /// Also collect used field masters (if pUsedMasters is set)
    /// to be called for every field during style export.
    void ExportFieldAutoStyle(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::text::XTextField > & rTextField );

    /// export field declarations.
    /// to be called once at beginning of document body.
    void ExportFieldDeclarations();

    /// export field declarations for fields used in the the particular XText.
    /// (Requires that a list of used field declarations has previously been
    ///  built-up in ExportFieldAutoStyle() )
    void ExportFieldDeclarations(
        const ::com::sun::star::uno::Reference < 
                ::com::sun::star::text::XText > & rText);

    /// export all field declarations, or only those that have been used?
    /// Calling this method will reset the list of used field declataions.
    void SetExportOnlyUsedFieldDeclarations(
        sal_Bool bExportOnlyUsed = sal_True);

    // determine element or attribute names
    // (public, because they may be useful in related XML export classes)
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapPlaceholderType(sal_uInt16 nType);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapTemplateDisplayFormat(sal_Int16 nType);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapChapterDisplayFormat(sal_Int16 nType);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapFilenameDisplayFormat(sal_Int16 nType);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapDocInfoFieldName(enum FieldIdEnum nToken);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapReferenceSource(sal_Int16 nType);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapReferenceType(sal_Int16 nType);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapCountFieldName(sal_Int16 nToken);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapBibliographyFieldName(::rtl::OUString sName);
    static enum ::binfilter::xmloff::token::XMLTokenEnum MapMeasureKind(sal_Int16 nKind);
    enum ::binfilter::xmloff::token::XMLTokenEnum MapPageNumberName(const ::com::sun::star::uno::Reference<
                      ::com::sun::star::beans::XPropertySet> & xPropSet,
                      sal_Int32& nOffset);	/// also adjust page offset
    enum ::binfilter::xmloff::token::XMLTokenEnum MapAuthorFieldName(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::beans::XPropertySet > & xPropSet);
    enum ::binfilter::xmloff::token::XMLTokenEnum MapSenderFieldName(const ::com::sun::star::uno::Reference <
                      ::com::sun::star::beans::XPropertySet > & xPropSet);

protected:

    SvXMLExport& GetExport() { return rExport; }

    /// export a field after <text:span> is already written
    void ExportFieldHelper(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::text::XTextField> & rTextField,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet> & rPropSet,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet> & rRangePropSet,
        enum FieldIdEnum nToken);

    /// export an empty element
    void ExportElement(enum ::binfilter::xmloff::token::XMLTokenEnum eElement,	/// element token
                       sal_Bool bAddSpace = sal_False); /// add blanks around
                                                        /// element?

    /// export an element with string content
    void ExportElement(enum ::binfilter::xmloff::token::XMLTokenEnum eElement,	/// element token
                       const ::rtl::OUString& sContent,	/// element content
                       sal_Bool bAddSpace = sal_False); /// add blanks around
                                                        /// element?

    /// export a macro (as used in the macro field)
    void ExportMacro( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::beans::XPropertySet> & rPropSet,
                      const ::rtl::OUString& rContent);

    /// export a boolean attribute
    void ProcessBoolean(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXmlName,	/// attribute token (namespace text) 
        sal_Bool bBool,		/// attribute value
        sal_Bool bDefault);	/// attribute default; omit, if attribute differs

    /// export an integer attribute
    void ProcessInteger(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXmlName,	/// attribute token (namespace text)
        sal_Int32 nNum);			/// attribute value

    /// export an integer attribute, omit if default
    void ProcessInteger(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXmlName,	/// attribute token (namespace text)
        sal_Int32 nNum,				/// attribute value
        sal_Int32 nDefault);		/// default value

    /// export a string attribute
    void ProcessString(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXmlName,		/// attribute token (namespace text)
        const ::rtl::OUString& sValue,	/// attribute value
        sal_Bool bOmitEmpty = sal_False, /// omit attribute, if value is empty
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export a string attribute, omit if default
    void ProcessString(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXmlName,		/// attribute token (namespace text)
        const ::rtl::OUString& sValue,	/// attribute value
        const ::rtl::OUString& sDefault, /// default value; omit if equal
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export a string attribute
    void ProcessString(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXmlName,		/// attribute token (namespace text)
        enum ::binfilter::xmloff::token::XMLTokenEnum eValue,			/// attribute token
        sal_Bool bOmitEmpty = sal_False, /// omit attribute, if value is empty
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export a string attribute, omit if default
    void ProcessString(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXmlName,		/// attribute token (namespace text)
        enum ::binfilter::xmloff::token::XMLTokenEnum eValue,			/// attribute value token
        enum ::binfilter::xmloff::token::XMLTokenEnum eDefault,		/// default value token
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export a string as a sequence of paragraphs
    void ProcessParagraphSequence(
        /// string containing the paragraphs
        const ::rtl::OUString& sParagraphSequence);

    /// export a numbering format (numeric, roman, alphabetic, etc.)
    void ProcessNumberingType(
        sal_Int16 nNumberingType);		/// numbering type key

    /// export display attribute (value, formula, none)
    void ProcessDisplay(sal_Bool bIsVisible,	/// is visible?
                        sal_Bool bIsCommand,	/// is show command/show name?
                        sal_Bool bDefault = sal_True);	/// omit, if default

    /// export all data-style related attributes
    void ProcessValueAndType(
        sal_Bool bIsString,		/// do we process a string or a number?
        sal_Int32 nFormatKey,	/// format key for NumberFormatter; possibly -1
        const ::rtl::OUString& sContent, /// string content; possibly invalid
        const ::rtl::OUString& sDefault, /// default string
        double fValue,			/// float content; possibly invalid
        sal_Bool bExportValue,	/// export value attribute?
        sal_Bool bExportValueType,	/// export value-type attribute?
        sal_Bool bExportStyle,	/// export style-sttribute?
        sal_Bool bForceSystemLanguage, /// no style language export
        sal_Bool bTimeStyle = sal_False); /// exporting a time style?

    /// export times, dates and durations according to ISO 8601
    void ProcessDateTime(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXMLName, 	/// attribute token
        double dValue, 				/// date/time value
        sal_Bool bIsDate, 			/// export as date (rather than date/time)?
        sal_Bool bIsDuration = sal_False,			/// export as duration
        sal_Bool bOmitDurationIfZero = sal_True,	/// omit zero-length durat.
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export a date, time, or duration
    void ProcessDateTime(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXMLName, 	/// attribute token
        sal_Int32 nMinutes,				/// date/time value in minutes
        sal_Bool bIsDate, 			/// export as date?
        sal_Bool bIsDuration,		/// export as duration?
        sal_Bool bOmitDurationIfZero,	/// omit zero-length durations
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export times, dates and durations according to ISO 8601
    void ProcessDateTime(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXMLName, 	/// attribute token
        const ::com::sun::star::util::DateTime& rTime,		/// date/time value
        sal_Bool bIsDate,			/// export as date (rather than date/time)?
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export date according to ISO 8601
    void ProcessDate(
        enum ::binfilter::xmloff::token::XMLTokenEnum eXMLName, 	/// attribute token
        const ::com::sun::star::util::Date& rTime,	/// date value
        sal_uInt16 nPrefix = XML_NAMESPACE_TEXT);	/// attribute name prefix

    /// export all attributes for bibliography data fields
    void ProcessBibliographyData(
        const ::com::sun::star::uno::Reference < 
            ::com::sun::star::beans::XPropertySet > & rPropertySet);

    /// export CommandTypeAttribute
    void ProcessCommandType(
        sal_Int32 nCommandType);		/// ::com::sun::star::sdb::CommandType

    void ProcessStringSequence(
        const ::com::sun::star::uno::Sequence<rtl::OUString>& rSequence,
        const ::rtl::OUString sSelected );

    void ProcessStringSequence(
        const ::com::sun::star::uno::Sequence<rtl::OUString>& rSequence,
        sal_Int32 nSelected );

    /// for XDependentTextFields, get PropertySet of FieldMaster
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet >
    GetMasterPropertySet(const ::com::sun::star::uno::Reference <
                         ::com::sun::star::text::XTextField > & rTextField);

    /// get PropertySet of (any) DependentTextField for this FieldMaster
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
    sal_Bool IsStringField(sal_uInt16 nFieldType,	/// field ID
                           const ::com::sun::star::uno::Reference <
                           ::com::sun::star::beans::XPropertySet > & xPropSet);

    /// explode a field master name into field type and field name
    sal_Bool ExplodeFieldMasterName(
        const ::rtl::OUString& sMasterName,	/// name as returned by SO API
        ::rtl::OUString& sFieldType,		/// out: field type
        ::rtl::OUString& sVarName);			/// out: variable name

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
    const ::rtl::OUString sPropertyFullName;
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
    const ::rtl::OUString sPropertyFields;
    const ::rtl::OUString sPropertyScriptType;
    const ::rtl::OUString sPropertyURLContent;
    const ::rtl::OUString sPropertyAuthor;
    const ::rtl::OUString sPropertyDate;
    const ::rtl::OUString sPropertyMeasureKind;
    const ::rtl::OUString sPropertyInstanceName;
    const ::rtl::OUString sPropertyIsHidden;
    const ::rtl::OUString sPropertyIsConditionTrue;
    const ::rtl::OUString sPropertyDataCommandType;
    const ::rtl::OUString sPropertyIsFixedLanguage;
    const ::rtl::OUString sPropertyCharStyleNames;
    const ::rtl::OUString sPropertyItems;
    const ::rtl::OUString sPropertySelectedItem;

    const ::rtl::OUString sEmpty;

    XMLPropertyState* pCombinedCharactersPropertyState;

};

}//end of namespace binfilter
#endif
