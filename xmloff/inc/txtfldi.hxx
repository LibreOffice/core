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
 *
 *  import of all text fields
 *  (except variable related + database display field: see txtvfldi.hxx)
 */

#ifndef INCLUDED_XMLOFF_INC_TXTFLDI_HXX
#define INCLUDED_XMLOFF_INC_TXTFLDI_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/txtimp.hxx>
#include <rtl/ustrbuf.hxx>

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace text { class XTextField; }
    namespace beans { class XPropertySet; struct PropertyValue; }
} } }

class SvXMLImport;
class XMLTextImportHelper;
class SvXMLTokenMap;

enum XMLTextFieldAttrTokens
{
    XML_TOK_TEXTFIELD_FIXED = 0,
    XML_TOK_TEXTFIELD_DESCRIPTION,
    XML_TOK_TEXTFIELD_HELP,
    XML_TOK_TEXTFIELD_HINT,
    XML_TOK_TEXTFIELD_PLACEHOLDER_TYPE,
    XML_TOK_TEXTFIELD_TIME_ADJUST,
    XML_TOK_TEXTFIELD_DATE_ADJUST,
    XML_TOK_TEXTFIELD_PAGE_ADJUST,
    XML_TOK_TEXTFIELD_SELECT_PAGE,
    XML_TOK_TEXTFIELD_ACTIVE,

    XML_TOK_TEXTFIELD_NAME,
    XML_TOK_TEXTFIELD_FORMULA,
    XML_TOK_TEXTFIELD_NUM_FORMAT,
    XML_TOK_TEXTFIELD_NUM_LETTER_SYNC,
    XML_TOK_TEXTFIELD_DISPLAY_FORMULA,
    XML_TOK_TEXTFIELD_NUMBERING_LEVEL,
    XML_TOK_TEXTFIELD_NUMBERING_SEPARATOR,
    XML_TOK_TEXTFIELD_DISPLAY,
    XML_TOK_TEXTFIELD_OUTLINE_LEVEL,

    XML_TOK_TEXTFIELD_VALUE_TYPE,
    XML_TOK_TEXTFIELD_VALUE,
    XML_TOK_TEXTFIELD_STRING_VALUE,
    XML_TOK_TEXTFIELD_DATE_VALUE,
    XML_TOK_TEXTFIELD_TIME_VALUE,
    XML_TOK_TEXTFIELD_BOOL_VALUE,
    XML_TOK_TEXTFIELD_CURRENCY,
    XML_TOK_TEXTFIELD_DATA_STYLE_NAME,

    XML_TOK_TEXTFIELD_DATABASE_NAME,
    XML_TOK_TEXTFIELD_TABLE_NAME,
    XML_TOK_TEXTFIELD_COLUMN_NAME,
    XML_TOK_TEXTFIELD_ROW_NUMBER,
    XML_TOK_TEXTFIELD_CONDITION,
    XML_TOK_TEXTFIELD_STRING_VALUE_IF_TRUE,
    XML_TOK_TEXTFIELD_STRING_VALUE_IF_FALSE,
    XML_TOK_TEXTFIELD_REVISION,
    XML_TOK_TEXTFIELD_IS_HIDDEN,
    XML_TOK_TEXTFIELD_CURRENT_VALUE,

    XML_TOK_TEXTFIELD_REFERENCE_FORMAT,
    XML_TOK_TEXTFIELD_REF_NAME,
    XML_TOK_TEXTFIELD_CONNECTION_NAME,

    XML_TOK_TEXTFIELD_HREF,
    XML_TOK_TEXTFIELD_TARGET_FRAME,

    XML_TOK_TEXTFIELD_OFFICE_CREATE_DATE,
    XML_TOK_TEXTFIELD_OFFICE_AUTHOR,
    XML_TOK_TEXTFIELD_ANNOTATION,
    XML_TOK_TEXTFIELD_LANGUAGE,

    XML_TOK_TEXTFIELD_MEASURE_KIND,
    XML_TOK_TEXTFIELD_TABLE_TYPE,

    XML_TOK_TEXTFIELD_NOTE_CLASS,

    XML_TOK_TEXTFIELD_UNKNOWN
};

/// abstract class for text field import
class XMLTextFieldImportContext : public SvXMLImportContext
{
    // data members
    OUStringBuffer sContentBuffer;   /// collect character data
    OUString sContent;               /// character data after collection
    OUString sServiceName;           /// service name for text field
    XMLTextImportHelper& rTextImportHelper; /// the import helper

protected:
    OUString sServicePrefix;

    // data members for use in subclasses
    bool bValid;                 /// whether this field is valid ?

public:
    TYPEINFO_OVERRIDE();

    XMLTextFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        const sal_Char* pService,               /// name of SO API service
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& rLocalName);     /// element name w/o prefix

    virtual ~XMLTextFieldImportContext();

    /// process character data: will be collected in member sContentBuffer
    virtual void Characters( const OUString& sContent ) SAL_OVERRIDE;

    /// parses attributes and calls ProcessAttribute
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList) SAL_OVERRIDE;

    /// create XTextField and insert into document; calls PrepareTextField
    virtual void EndElement() SAL_OVERRIDE;

    /// create the appropriate field context from
    /// (for use in paragraph import)
    static XMLTextFieldImportContext* CreateTextFieldImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        sal_uInt16 nPrefix,
        const OUString& rName,
        sal_uInt16 nToken);

protected:
    /// get helper
    inline XMLTextImportHelper& GetImportHelper() { return rTextImportHelper; }

    inline OUString GetServiceName() { return sServiceName; }
    inline void SetServiceName(const OUString& sStr) { sServiceName = sStr; }

    OUString GetContent();

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) = 0;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) = 0;

    /// create field from ServiceName
    bool CreateField(::com::sun::star::uno::Reference<
                         ::com::sun::star::beans::XPropertySet> & xField,
                         const OUString& sServiceName);

    /// force an update of the field's value
    /// call update on optional XUptadeable interface; (disable Fixed property)
    static void ForceUpdate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
};

class XMLSenderFieldImportContext : public XMLTextFieldImportContext
{

    sal_Int16 nSubType;         /// API subtype for ExtUser field

    const OUString sPropertyFixed;
    const OUString sPropertyFieldSubType;
    const OUString sPropertyContent;

protected:
    // variables for access in subclass
    bool bFixed;
    sal_uInt16 nElementToken;   /// token for this element field

public:
    TYPEINFO_OVERRIDE();

    XMLSenderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName,      /// element name w/o prefix
        sal_uInt16 nToken);                     /// element token

protected:
    /// start element
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList) SAL_OVERRIDE;

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** inherit sender field because of fixed attribute in ProcessAttributes */
class XMLAuthorFieldImportContext : public XMLSenderFieldImportContext
{
    bool bAuthorFullName;
    const OUString sServiceAuthor;
    const OUString sPropertyAuthorFullName;
    const OUString sPropertyFixed;
    const OUString sPropertyContent;

public:
    TYPEINFO_OVERRIDE();

    XMLAuthorFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName,      /// element name w/o prefix
        sal_uInt16 nToken);                     /// element token

protected:
    /// start element
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

class XMLPlaceholderFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyPlaceholderType;
    const OUString sPropertyPlaceholder;
    const OUString sPropertyHint;

    OUString sDescription;

    sal_Int16 nPlaceholderType;

public:
    TYPEINFO_OVERRIDE();

    XMLPlaceholderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

class XMLTimeFieldImportContext : public XMLTextFieldImportContext
{
protected:
    const OUString sPropertyNumberFormat;
    const OUString sPropertyFixed;
    const OUString sPropertyDateTimeValue;
    const OUString sPropertyDateTime;
    const OUString sPropertyAdjust;
    const OUString sPropertyIsDate;
    const OUString sPropertyIsFixedLanguage;

    ::com::sun::star::util::DateTime aDateTimeValue;
    sal_Int32 nAdjust;
    sal_Int32 nFormatKey;
    bool bTimeOK;
    bool bFormatOK;
    bool bFixed;
    bool bIsDate;           // is this a date?
                                // (for XMLDateFieldImportContext, really)
    bool     bIsDefaultLanguage;

public:
    TYPEINFO_OVERRIDE();

    XMLTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import date fields (<text:date>);
    inherit from TimeField to reuse implementation */
class XMLDateFieldImportContext : public XMLTimeFieldImportContext
{
public:
    TYPEINFO_OVERRIDE();

    XMLDateFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;
};

/** import page continuation fields (<text:page-continuation-string>) */
class XMLPageContinuationImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertySubType;
    const OUString sPropertyUserText;
    const OUString sPropertyNumberingType;

    OUString sString;            /// continuation string
    com::sun::star::text::PageNumberType eSelectPage;   /// previous, current
                                                        /// or next page
    bool sStringOK;                 /// continuation string encountered?

public:
    TYPEINFO_OVERRIDE();

    XMLPageContinuationImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import page number fields (<text:page-number>) */
class XMLPageNumberImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertySubType;
    const OUString sPropertyNumberingType;
    const OUString sPropertyOffset;

    OUString sNumberFormat;
    OUString sNumberSync;
    sal_Int16 nPageAdjust;
    com::sun::star::text::PageNumberType eSelectPage;   /// previous, current
                                                        /// or next page
    bool sNumberFormatOK;

public:
    TYPEINFO_OVERRIDE();

    XMLPageNumberImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix


    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** superclass for database fields: handle database and table names */
class XMLDatabaseFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyDataBaseName;
    const OUString sPropertyDataBaseURL;
    const OUString sPropertyTableName;
    const OUString sPropertyDataCommandType;
    const OUString sPropertyIsVisible;

    OUString sDatabaseName;
    OUString sDatabaseURL;
    OUString sTableName;

    sal_Int32 nCommandType;
    bool bCommandTypeOK;

    bool bDisplay;
    bool bDisplayOK;
    bool bUseDisplay;

protected:
    bool bDatabaseOK;
    bool bDatabaseNameOK;
    bool bDatabaseURLOK;
    bool bTableOK;

    /// protected constructor: only for subclasses
    XMLDatabaseFieldImportContext(SvXMLImport& rImport,
                                  XMLTextImportHelper& rHlp,
                                  const sal_Char* pServiceName,
                                  sal_uInt16 nPrfx,
                                  const OUString& sLocalName,
                                  bool bUseDisplay );

public:
    TYPEINFO_OVERRIDE();

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;

    /// handle database-location children
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) SAL_OVERRIDE;
};

/** import database name fields (<text:database-name>) */
class XMLDatabaseNameImportContext : public XMLDatabaseFieldImportContext
{
public:
    TYPEINFO_OVERRIDE();

    XMLDatabaseNameImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 sal_uInt16 nPrfx,
                                 const OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;
};

/** import database next fields (<text:database-next>) */
class XMLDatabaseNextImportContext : public XMLDatabaseFieldImportContext
{
    const OUString sPropertyCondition;
    const OUString sTrue;
    OUString sCondition;
    bool bConditionOK;

protected:
    // for use in child classes
    XMLDatabaseNextImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 const sal_Char* pServiceName,
                                 sal_uInt16 nPrfx,
                                 const OUString& sLocalName);

public:
    TYPEINFO_OVERRIDE();

    XMLDatabaseNextImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 sal_uInt16 nPrfx,
                                 const OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import database select fields (<text:database-select>) */
class XMLDatabaseSelectImportContext : public XMLDatabaseNextImportContext
{
    const OUString sPropertySetNumber;
    sal_Int32 nNumber;
    bool bNumberOK;

public:
    TYPEINFO_OVERRIDE();

    XMLDatabaseSelectImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp,
                                   sal_uInt16 nPrfx,
                                   const OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import database display number fields (<text:database-row-number>) */
class XMLDatabaseNumberImportContext : public XMLDatabaseFieldImportContext
{
    const OUString sPropertyNumberingType;
    const OUString sPropertySetNumber;
    OUString sNumberFormat;
    OUString sNumberSync;
    sal_Int32 nValue;
    bool bValueOK;

public:
    TYPEINFO_OVERRIDE();

    XMLDatabaseNumberImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp,
                                   sal_uInt16 nPrfx,
                                   const OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import docinfo fields with only fixed attribute */
class XMLSimpleDocInfoImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyFixed;
    const OUString sPropertyContent;
    const OUString sPropertyAuthor;
    const OUString sPropertyCurrentPresentation;

protected:
    bool bFixed;
    bool bHasAuthor;
    bool bHasContent;

public:
    TYPEINFO_OVERRIDE();

    XMLSimpleDocInfoImportContext(SvXMLImport& rImport,
                                  XMLTextImportHelper& rHlp,
                                  sal_uInt16 nPrfx,
                                  const OUString& sLocalName,
                                  sal_uInt16 nToken,
                                  bool bContent,
                                  bool bAuthor);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;

    static const sal_Char* MapTokenToServiceName(sal_uInt16 nToken);
};

/** import docinfo fields with date or time attributes and numberformats */
class XMLDateTimeDocInfoImportContext : public XMLSimpleDocInfoImportContext
{
    const OUString sPropertyNumberFormat;
    const OUString sPropertyIsDate;
    const OUString sPropertyIsFixedLanguage;

    sal_Int32 nFormat;
    bool bFormatOK;
    bool bIsDate;
    bool bHasDateTime;
    bool     bIsDefaultLanguage;

public:
    TYPEINFO_OVERRIDE();

    XMLDateTimeDocInfoImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName,
                                    sal_uInt16 nToken);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import revision field (<text:editing-cycles>) */
class XMLRevisionDocInfoImportContext : public XMLSimpleDocInfoImportContext
{
    const OUString sPropertyRevision;

public:
    TYPEINFO_OVERRIDE();

    XMLRevisionDocInfoImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName,
                                    sal_uInt16 nToken);

protected:
    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import user docinfo field (<text:user-defined>) */
class XMLUserDocInfoImportContext : public XMLSimpleDocInfoImportContext
{
    OUString aName;
    const OUString sPropertyName;
    const OUString sPropertyNumberFormat;
    const OUString sPropertyIsFixedLanguage;
    sal_Int32   nFormat;
    bool    bFormatOK;
    bool        bIsDefaultLanguage;

public:
    TYPEINFO_OVERRIDE();

    XMLUserDocInfoImportContext(SvXMLImport& rImport,
                                XMLTextImportHelper& rHlp,
                                sal_uInt16 nPrfx,
                                const OUString& sLocalName,
                                sal_uInt16 nToken);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import hidden paragraph fields (<text:hidden-paragraph>) */
class XMLHiddenParagraphImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyCondition;
    const OUString sPropertyIsHidden;

    OUString sCondition;
    bool bIsHidden;

public:
    TYPEINFO_OVERRIDE();

    XMLHiddenParagraphImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import conditional text fields (<text:conditional-text>) */
class XMLConditionalTextImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyCondition;
    const OUString sPropertyTrueContent;
    const OUString sPropertyFalseContent;
    const OUString sPropertyIsConditionTrue;
    const OUString sPropertyCurrentPresentation;

    OUString sCondition;
    OUString sTrueContent;
    OUString sFalseContent;

    bool bConditionOK;
    bool bTrueOK;
    bool bFalseOK;
    bool bCurrentValue;

public:
    TYPEINFO_OVERRIDE();

    XMLConditionalTextImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import conditional text fields (<text:hidden-text>) */
class XMLHiddenTextImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyCondition;
    const OUString sPropertyContent;
    const OUString sPropertyIsHidden;

    OUString sCondition;
    OUString sString;

    bool bConditionOK;
    bool bStringOK;
    bool bIsHidden;

public:
    TYPEINFO_OVERRIDE();

    XMLHiddenTextImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nPrfx,
                               const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import file name fields (<text:file-name>) */
class XMLFileNameImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyFixed;
    const OUString sPropertyFileFormat;
    const OUString sPropertyCurrentPresentation;

    sal_Int16 nFormat;
    bool bFixed;

public:
    TYPEINFO_OVERRIDE();

    XMLFileNameImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp,
                             sal_uInt16 nPrfx,
                             const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import document template name fields (<text:template-name>) */
class XMLTemplateNameImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyFileFormat;

    sal_Int16 nFormat;

public:
    TYPEINFO_OVERRIDE();

    XMLTemplateNameImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 sal_uInt16 nPrfx,
                                 const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import chapter fields (<text:chapter>) */
class XMLChapterImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyChapterFormat;
    const OUString sPropertyLevel;

    sal_Int16 nFormat;
    sal_Int8 nLevel;

public:
    TYPEINFO_OVERRIDE();

    XMLChapterImportContext(SvXMLImport& rImport,
                            XMLTextImportHelper& rHlp,
                            sal_uInt16 nPrfx,
                            const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import count fields (<text:[XXX]-count>) */
class XMLCountFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyNumberingType;

    OUString sNumberFormat;
    OUString sLetterSync;

    bool bNumberFormatOK;

public:
    TYPEINFO_OVERRIDE();

    XMLCountFieldImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nPrfx,
                               const OUString& sLocalName,
                               sal_uInt16 nToken);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;

    static const sal_Char* MapTokenToServiceName(sal_uInt16 nToken);
};

/** import page variable fields (<text:get-page-variable>) */
class XMLPageVarGetFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyNumberingType;

    OUString sNumberFormat;
    OUString sLetterSync;

    bool bNumberFormatOK;

public:
    TYPEINFO_OVERRIDE();

    XMLPageVarGetFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import page variable fields (<text:get-page-variable>) */
class XMLPageVarSetFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyOn;
    const OUString sPropertyOffset;

    sal_Int16 nAdjust;
    bool bActive;

public:
    TYPEINFO_OVERRIDE();

    XMLPageVarSetFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import macro fields (<text:execute-macro>) */
class XMLMacroFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyHint;
    const OUString sPropertyMacroName;
    const OUString sPropertyScriptURL;

    OUString sDescription;
    SvXMLImportContextRef xEventContext;

    OUString sMacro; // macro for old documents (pre 638i)

    bool bDescriptionOK;

public:
    TYPEINFO_OVERRIDE();

    XMLMacroFieldImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nPrfx,
                               const OUString& sLocalName);

protected:
    /// for <office:events> children
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) SAL_OVERRIDE;

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import reference fields (<text:reference-get>) */
class XMLReferenceFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyReferenceFieldPart;
    const OUString sPropertyReferenceFieldSource;
    const OUString sPropertySourceName;
    const OUString sPropertyCurrentPresentation;

    OUString sName;
    sal_uInt16 nElementToken;
    sal_Int16 nSource;
    sal_Int16 nType;

    bool bNameOK;
    bool bTypeOK;

public:
    TYPEINFO_OVERRIDE();

    XMLReferenceFieldImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp,
                                   sal_uInt16 nToken,
                                   sal_uInt16 nPrfx,
                                   const OUString& sLocalName);

protected:
    /// start element
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList) SAL_OVERRIDE;

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import dde field declaration container (<text:dde-connection-decls>) */
class XMLDdeFieldDeclsImportContext : public SvXMLImportContext
{
    SvXMLTokenMap aTokenMap;

public:
    TYPEINFO_OVERRIDE();

    XMLDdeFieldDeclsImportContext(SvXMLImport& rImport,
                                  sal_uInt16 nPrfx,
                                  const OUString& sLocalName);

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList ) SAL_OVERRIDE;
};

/** import dde field declaration (<text:dde-connection-decl>) */
class XMLDdeFieldDeclImportContext : public SvXMLImportContext
{
    const OUString sPropertyIsAutomaticUpdate;
    const OUString sPropertyName;
    const OUString sPropertyDDECommandType;
    const OUString sPropertyDDECommandFile;
    const OUString sPropertyDDECommandElement;

    const SvXMLTokenMap& rTokenMap;

public:
    TYPEINFO_OVERRIDE();

    XMLDdeFieldDeclImportContext(SvXMLImport& rImport,
                                 sal_uInt16 nPrfx,
                                 const OUString& sLocalName,
                                 const SvXMLTokenMap& rMap);

    // create fieldmaster
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList) SAL_OVERRIDE;
};

/** import dde fields (<text:dde-connection>) */
class XMLDdeFieldImportContext : public XMLTextFieldImportContext
{
    OUString sName;
    OUString sPropertyContent;

public:
    TYPEINFO_OVERRIDE();

    XMLDdeFieldImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp,
                             sal_uInt16 nPrfx,
                             const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// create textfield, attach master, and insert into document
    virtual void EndElement() SAL_OVERRIDE;

    /// empty method
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import sheet name fields (Calc) dde fields (<text:sheet-name>) */
class XMLSheetNameImportContext : public XMLTextFieldImportContext
{

public:
    TYPEINFO_OVERRIDE();

    XMLSheetNameImportContext(SvXMLImport& rImport,
                              XMLTextImportHelper& rHlp,
                              sal_uInt16 nPrfx,
                              const OUString& sLocalName);

protected:
    /// no attributes -> empty method
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// no attributes -> empty method
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import page|slide name fields (<text:page-name>) */
class XMLPageNameFieldImportContext : public XMLTextFieldImportContext
{
public:
    TYPEINFO_OVERRIDE();

    XMLPageNameFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import hyperlinks as URL fields (Calc, Impress, Draw) (<office:a>) */
class XMLUrlFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyURL;
    const OUString sPropertyTargetFrame;
    const OUString sPropertyRepresentation;

    OUString sURL;
    OUString sFrame;
    bool bFrameOK;

public:
    TYPEINFO_OVERRIDE();

    XMLUrlFieldImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp,
                             sal_uInt16 nPrfx,
                             const OUString& sLocalName);

protected:
    /// no attributes -> empty method
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// no attributes -> empty method
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import bibliography info fields (<text:bibliography-mark>) */
class XMLBibliographyFieldImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyFields;

    ::std::vector< ::com::sun::star::beans::PropertyValue> aValues;

public:
    TYPEINFO_OVERRIDE();

    XMLBibliographyFieldImportContext(SvXMLImport& rImport,
                                      XMLTextImportHelper& rHlp,
                                      sal_uInt16 nPrfx,
                                      const OUString& sLocalName);

protected:
    /// process attributes (fill aValues)
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList) SAL_OVERRIDE;

    /// empty method; all attributes are handled in StartElement
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// convert aValues into sequence and set property
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;

    static const sal_Char* MapBibliographyFieldName(const OUString& sName);
};

/** Import an annotation field (<text:annotation>) */
class XMLAnnotationImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyAuthor;
    const OUString sPropertyInitials;
    const OUString sPropertyContent;
    const OUString sPropertyDate;
    const OUString sPropertyTextRange;
    const OUString sPropertyName;

    OUStringBuffer aAuthorBuffer;
    OUStringBuffer aInitialsBuffer;
    OUString aName;
    OUStringBuffer aTextBuffer;
    OUStringBuffer aDateBuffer;

    com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > mxField;
    com::sun::star::uno::Reference < com::sun::star::text::XTextCursor >  mxCursor;
    com::sun::star::uno::Reference < com::sun::star::text::XTextCursor >  mxOldCursor;

    sal_uInt16 m_nToken;

public:
    TYPEINFO_OVERRIDE();

    XMLAnnotationImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nToken,
                               sal_uInt16 nPrfx,
                               const OUString& sLocalName);

protected:
    /// process attributes
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// set properties
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xPropertySet) SAL_OVERRIDE;

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;
    virtual void EndElement() SAL_OVERRIDE;
};

/** Import a script field (<text:script>) */
class XMLScriptImportContext : public XMLTextFieldImportContext
{
    const OUString sPropertyScriptType;
    const OUString sPropertyURLContent;
    const OUString sPropertyContent;

    OUString sContent;
    OUString sScriptType;

    bool bContentOK;
    bool bScriptTypeOK;

public:
    TYPEINFO_OVERRIDE();

    XMLScriptImportContext(SvXMLImport& rImport,
                           XMLTextImportHelper& rHlp,
                           sal_uInt16 nPrfx,
                           const OUString& sLocalName);

protected:
    /// process attributes
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// set properties
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import measure fields (<text:measure>) */
class XMLMeasureFieldImportContext : public XMLTextFieldImportContext
{
    sal_Int16 mnKind;

public:
    TYPEINFO_OVERRIDE();

    XMLMeasureFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** dropdown field (filter legacy) */
class XMLDropDownFieldImportContext : public XMLTextFieldImportContext
{
    std::vector<OUString> aLabels;
    OUString sName;
    OUString sHelp;
    OUString sHint;
    sal_Int32 nSelected;
    bool bNameOK;
    bool bHelpOK;
    bool bHintOK;

    const OUString sPropertyItems;
    const OUString sPropertySelectedItem;
    const OUString sPropertyName;
    const OUString sPropertyHelp;
    const OUString sPropertyToolTip;

public:
    TYPEINFO_OVERRIDE();

    XMLDropDownFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName);

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const ::com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList >& xAttrList ) SAL_OVERRIDE;

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import header fields (<draw:header>) */
class XMLHeaderFieldImportContext : public XMLTextFieldImportContext
{
public:
    TYPEINFO_OVERRIDE();

    XMLHeaderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import footer fields (<draw:footer>) */
class XMLFooterFieldImportContext : public XMLTextFieldImportContext
{
public:
    TYPEINFO_OVERRIDE();

    XMLFooterFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

/** import footer fields (<draw:date-and-time>) */
class XMLDateTimeFieldImportContext : public XMLTextFieldImportContext
{
public:
    TYPEINFO_OVERRIDE();

    XMLDateTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

class XMLCustomPropertyFieldImportContext : public XMLTextFieldImportContext
{
    OUString sName;
    ::com::sun::star::uno::Any aValue;
    const OUString sPropertyName;
    const OUString sPropertyValue;

public:
    TYPEINFO_OVERRIDE();

    XMLCustomPropertyFieldImportContext (SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const OUString& sLocalName);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const OUString& sAttrValue ) SAL_OVERRIDE;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
