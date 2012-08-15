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

#ifndef _XMLOFF_TXTFLDI_HXX
#define _XMLOFF_TXTFLDI_HXX

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

namespace rtl
{
    class OUString;
}


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
    const ::rtl::OUString sIsFixed;

    // data members
    ::rtl::OUStringBuffer sContentBuffer;   /// collect character data
    ::rtl::OUString sContent;               /// character data after collection
    ::rtl::OUString sServiceName;           /// service name for text field
    XMLTextImportHelper& rTextImportHelper; /// the import helper

protected:
    ::rtl::OUString sServicePrefix;

    // data members for use in subclasses
    sal_Bool bValid;                        /// ist dieses Feld gültig?

public:

    TYPEINFO();

    XMLTextFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        const sal_Char* pService,               /// name of SO API service
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& rLocalName);     /// element name w/o prefix

    virtual ~XMLTextFieldImportContext();

    /// process character data: will be collected in member sContentBuffer
    virtual void Characters( const ::rtl::OUString& sContent );

    /// parses attributes and calls ProcessAttribute
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    /// create XTextField and insert into document; calls PrepareTextField
    virtual void EndElement();

    /// create the appropriate field context from
    /// (for use in paragraph import)
    static XMLTextFieldImportContext* CreateTextFieldImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rName,
        sal_uInt16 nToken);


protected:

    /// get helper
    inline XMLTextImportHelper& GetImportHelper() { return rTextImportHelper; }

    inline ::rtl::OUString GetServiceName() { return sServiceName; }
    inline void SetServiceName(::rtl::OUString sStr) { sServiceName = sStr; }

    ::rtl::OUString GetContent();

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue ) = 0;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet) = 0;

    /// create field from ServiceName
    sal_Bool CreateField(::com::sun::star::uno::Reference<
                         ::com::sun::star::beans::XPropertySet> & xField,
                         const ::rtl::OUString& sServiceName);

    /// force an update of the field's value
    /// call update on optional XUptadeable interface; (disable Fixed property)
    void ForceUpdate(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
};



class XMLSenderFieldImportContext : public XMLTextFieldImportContext
{

    sal_Int16 nSubType;         /// API subtype for ExtUser field

    const ::rtl::OUString sEmpty;       /// empty string
    const ::rtl::OUString sPropertyFixed;
    const ::rtl::OUString sPropertyFieldSubType;
    const ::rtl::OUString sPropertyContent;

protected:

    // variables for access in subclass
    sal_Bool bFixed;
    sal_uInt16 nElementToken;   /// token for this elment field

public:

    TYPEINFO();

    XMLSenderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName,      /// element name w/o prefix
        sal_uInt16 nToken);                     /// element token

protected:

    /// start element
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** inherit sender field because of fixed attribute in ProcessAttributes */
class XMLAuthorFieldImportContext : public XMLSenderFieldImportContext
{

    sal_Bool bAuthorFullName;
    const ::rtl::OUString sServiceAuthor;
    const ::rtl::OUString sPropertyAuthorFullName;
    const ::rtl::OUString sPropertyFixed;
    const ::rtl::OUString sPropertyContent;

public:

    TYPEINFO();

    XMLAuthorFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName,      /// element name w/o prefix
        sal_uInt16 nToken);                     /// element token

protected:

    /// start element
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



class XMLPlaceholderFieldImportContext : public XMLTextFieldImportContext
{

    const ::rtl::OUString sEmpty;       /// empty string
    const ::rtl::OUString sServiceJumpEdit;
    const ::rtl::OUString sPropertyPlaceholderType;
    const ::rtl::OUString sPropertyPlaceholder;
    const ::rtl::OUString sPropertyHint;

    ::rtl::OUString sDescription;

    sal_Int16 nPlaceholderType;

public:

    TYPEINFO();

    XMLPlaceholderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

class XMLTimeFieldImportContext : public XMLTextFieldImportContext
{

protected:
    const ::rtl::OUString sPropertyNumberFormat;
    const ::rtl::OUString sPropertyFixed;
    const ::rtl::OUString sPropertyDateTimeValue;
    const ::rtl::OUString sPropertyDateTime;
    const ::rtl::OUString sPropertyAdjust;
    const ::rtl::OUString sPropertyIsDate;
    const ::rtl::OUString sPropertyIsFixedLanguage;

    double fTimeValue;
    ::com::sun::star::util::DateTime aDateTimeValue;
    sal_Int32 nAdjust;
    sal_Int32 nFormatKey;
    sal_Bool bTimeOK;
    sal_Bool bFormatOK;
    sal_Bool bFixed;
    sal_Bool bIsDate;           // is this a date?
                                // (for XMLDateFieldImportContext, really)
    sal_Bool bIsDefaultLanguage;

public:

    TYPEINFO();

    XMLTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** import date fields (<text:date>);
    inherit from TimeField to reuse implementation */
class XMLDateFieldImportContext : public XMLTimeFieldImportContext
{

public:

    TYPEINFO();

    XMLDateFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );
};


/** import page continuation fields (<text:page-continuation-string>) */
class XMLPageContinuationImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertySubType;
    const ::rtl::OUString sPropertyUserText;
    const ::rtl::OUString sPropertyNumberingType;

    ::rtl::OUString sString;            /// continuation string
    com::sun::star::text::PageNumberType eSelectPage;   /// previous, current
                                                        /// or next page
    sal_Bool sStringOK;                 /// continuation string encountered?

public:

    TYPEINFO();

    XMLPageContinuationImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix


    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** import page number fields (<text:page-number>) */
class XMLPageNumberImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertySubType;
    const ::rtl::OUString sPropertyNumberingType;
    const ::rtl::OUString sPropertyOffset;

    ::rtl::OUString sNumberFormat;
    ::rtl::OUString sNumberSync;
    sal_Int16 nPageAdjust;
    com::sun::star::text::PageNumberType eSelectPage;   /// previous, current
                                                        /// or next page
    sal_Bool sNumberFormatOK;

public:

    TYPEINFO();

    XMLPageNumberImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix


    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** superclass for database fields: handle database and table names */
class XMLDatabaseFieldImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyDataBaseName;
    const ::rtl::OUString sPropertyDataBaseURL;
    const ::rtl::OUString sPropertyTableName;
    const ::rtl::OUString sPropertyDataCommandType;
    const ::rtl::OUString sPropertyIsVisible;

    ::rtl::OUString sDatabaseName;
    ::rtl::OUString sDatabaseURL;
    ::rtl::OUString sTableName;

    sal_Int32 nCommandType;
    sal_Bool bCommandTypeOK;

    sal_Bool bDisplay;
    bool bDisplayOK;
    bool bUseDisplay;

protected:
    sal_Bool bDatabaseOK;
    sal_Bool bDatabaseNameOK;
    sal_Bool bDatabaseURLOK;
    sal_Bool bTableOK;

    /// protected constructor: only for subclasses
    XMLDatabaseFieldImportContext(SvXMLImport& rImport,
                                  XMLTextImportHelper& rHlp,
                                  const sal_Char* pServiceName,
                                  sal_uInt16 nPrfx,
                                  const ::rtl::OUString& sLocalName,
                                  bool bUseDisplay );

public:

TYPEINFO();

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);

    /// handle database-location children
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

/** import database name fields (<text:database-name>) */
class XMLDatabaseNameImportContext : public XMLDatabaseFieldImportContext
{
public:

    TYPEINFO();

    XMLDatabaseNameImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 sal_uInt16 nPrfx,
                                 const ::rtl::OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );
};


/** import database next fields (<text:database-next>) */
class XMLDatabaseNextImportContext : public XMLDatabaseFieldImportContext
{
    const ::rtl::OUString sPropertyCondition;
    const ::rtl::OUString sTrue;
    ::rtl::OUString sCondition;
    sal_Bool bConditionOK;

protected:

    // for use in child classes
    XMLDatabaseNextImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 const sal_Char* pServiceName,
                                 sal_uInt16 nPrfx,
                                 const ::rtl::OUString& sLocalName);

public:

    TYPEINFO();

    XMLDatabaseNextImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 sal_uInt16 nPrfx,
                                 const ::rtl::OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import database select fields (<text:database-select>) */
class XMLDatabaseSelectImportContext : public XMLDatabaseNextImportContext
{
    const ::rtl::OUString sPropertySetNumber;
    sal_Int32 nNumber;
    sal_Bool bNumberOK;

public:

    TYPEINFO();

    XMLDatabaseSelectImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp,
                                   sal_uInt16 nPrfx,
                                   const ::rtl::OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** import database display number fields (<text:database-row-number>) */
class XMLDatabaseNumberImportContext : public XMLDatabaseFieldImportContext
{
    const ::rtl::OUString sPropertyNumberingType;
    const ::rtl::OUString sPropertySetNumber;
    ::rtl::OUString sNumberFormat;
    ::rtl::OUString sNumberSync;
    sal_Int32 nValue;
    sal_Bool bValueOK;

public:

    TYPEINFO();

    XMLDatabaseNumberImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp,
                                   sal_uInt16 nPrfx,
                                   const ::rtl::OUString& sLocalName);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** import docinfo fields with only fixed atribute */
class XMLSimpleDocInfoImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyFixed;
    const ::rtl::OUString sPropertyContent;
    const ::rtl::OUString sPropertyAuthor;
    const ::rtl::OUString sPropertyCurrentPresentation;

protected:
    sal_Bool bFixed;
    sal_Bool bHasAuthor;
    sal_Bool bHasContent;

public:

    TYPEINFO();

    XMLSimpleDocInfoImportContext(SvXMLImport& rImport,
                                  XMLTextImportHelper& rHlp,
                                  sal_uInt16 nPrfx,
                                  const ::rtl::OUString& sLocalName,
                                  sal_uInt16 nToken,
                                  sal_Bool bContent,
                                  sal_Bool bAuthor);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);

    static const sal_Char* MapTokenToServiceName(sal_uInt16 nToken);
};



/** import docinfo fields with date or time attributes and numberformats */
class XMLDateTimeDocInfoImportContext : public XMLSimpleDocInfoImportContext
{
    const ::rtl::OUString sPropertyNumberFormat;
    const ::rtl::OUString sPropertyIsDate;
    const ::rtl::OUString sPropertyIsFixedLanguage;

    sal_Int32 nFormat;
    sal_Bool bFormatOK;
    sal_Bool bIsDate;
    sal_Bool bHasDateTime;
    sal_Bool bIsDefaultLanguage;

public:

    TYPEINFO();

    XMLDateTimeDocInfoImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName,
                                    sal_uInt16 nToken);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import revision field (<text:editing-cycles>) */
class XMLRevisionDocInfoImportContext : public XMLSimpleDocInfoImportContext
{
    const ::rtl::OUString sPropertyRevision;

public:

    TYPEINFO();

    XMLRevisionDocInfoImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName,
                                    sal_uInt16 nToken);

protected:

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import user docinfo field (<text:user-defined>) */
class XMLUserDocInfoImportContext : public XMLSimpleDocInfoImportContext
{
    rtl::OUString aName;
    const ::rtl::OUString sPropertyName;
    const ::rtl::OUString sPropertyNumberFormat;
    const ::rtl::OUString sPropertyIsFixedLanguage;
    sal_Int32   nFormat;
    sal_Bool    bFormatOK;
    sal_Bool    bIsDefaultLanguage;

public:

    TYPEINFO();

    XMLUserDocInfoImportContext(SvXMLImport& rImport,
                                XMLTextImportHelper& rHlp,
                                sal_uInt16 nPrfx,
                                const ::rtl::OUString& sLocalName,
                                sal_uInt16 nToken);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import hidden paragraph fields (<text:hidden-paragraph>) */
class XMLHiddenParagraphImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyCondition;
    const ::rtl::OUString sPropertyIsHidden;

    ::rtl::OUString sCondition;
    sal_Bool bIsHidden;

public:

    TYPEINFO();

    XMLHiddenParagraphImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import conditional text fields (<text:conditional-text>) */
class XMLConditionalTextImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyCondition;
    const ::rtl::OUString sPropertyTrueContent;
    const ::rtl::OUString sPropertyFalseContent;
    const ::rtl::OUString sPropertyIsConditionTrue;
    const ::rtl::OUString sPropertyCurrentPresentation;

    ::rtl::OUString sCondition;
    ::rtl::OUString sTrueContent;
    ::rtl::OUString sFalseContent;

    sal_Bool bConditionOK;
    sal_Bool bTrueOK;
    sal_Bool bFalseOK;
    sal_Bool bCurrentValue;

public:

    TYPEINFO();

    XMLConditionalTextImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import conditional text fields (<text:hidden-text>) */
class XMLHiddenTextImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyCondition;
    const ::rtl::OUString sPropertyContent;
    const ::rtl::OUString sPropertyIsHidden;

    ::rtl::OUString sCondition;
    ::rtl::OUString sString;

    sal_Bool bConditionOK;
    sal_Bool bStringOK;
    sal_Bool bIsHidden;

public:

    TYPEINFO();

    XMLHiddenTextImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nPrfx,
                               const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import file name fields (<text:file-name>) */
class XMLFileNameImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyFixed;
    const ::rtl::OUString sPropertyFileFormat;
    const ::rtl::OUString sPropertyCurrentPresentation;

    sal_Int16 nFormat;
    sal_Bool bFixed;

public:

    TYPEINFO();

    XMLFileNameImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp,
                             sal_uInt16 nPrfx,
                             const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import document template name fields (<text:template-name>) */
class XMLTemplateNameImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyFileFormat;

    sal_Int16 nFormat;

public:

    TYPEINFO();

    XMLTemplateNameImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp,
                                 sal_uInt16 nPrfx,
                                 const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** import chapter fields (<text:chapter>) */
class XMLChapterImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyChapterFormat;
    const ::rtl::OUString sPropertyLevel;

    sal_Int16 nFormat;
    sal_Int8 nLevel;

public:

    TYPEINFO();

    XMLChapterImportContext(SvXMLImport& rImport,
                            XMLTextImportHelper& rHlp,
                            sal_uInt16 nPrfx,
                            const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import count fields (<text:[XXX]-count>) */
class XMLCountFieldImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyNumberingType;

    ::rtl::OUString sNumberFormat;
    ::rtl::OUString sLetterSync;

    sal_Bool bNumberFormatOK;

public:

    TYPEINFO();

    XMLCountFieldImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nPrfx,
                               const ::rtl::OUString& sLocalName,
                               sal_uInt16 nToken);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);

    static const sal_Char* MapTokenToServiceName(sal_uInt16 nToken);
};


/** import page variable fields (<text:get-page-variable>) */
class XMLPageVarGetFieldImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyNumberingType;

    ::rtl::OUString sNumberFormat;
    ::rtl::OUString sLetterSync;

    sal_Bool bNumberFormatOK;

public:

    TYPEINFO();

    XMLPageVarGetFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** import page variable fields (<text:get-page-variable>) */
class XMLPageVarSetFieldImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyOn;
    const ::rtl::OUString sPropertyOffset;

    sal_Int16 nAdjust;
    sal_Bool bActive;

public:

    TYPEINFO();

    XMLPageVarSetFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** import macro fields (<text:execute-macro>) */
class XMLMacroFieldImportContext : public XMLTextFieldImportContext
{

    const ::rtl::OUString sPropertyHint;
    const ::rtl::OUString sPropertyMacroName;
    const ::rtl::OUString sPropertyScriptURL;
    const ::rtl::OUString sPropertyLibraryName;

    ::rtl::OUString sDescription;
    SvXMLImportContextRef xEventContext;

    ::rtl::OUString sMacro; // macro for old documents (pre 638i)

    sal_Bool bDescriptionOK;

public:

    TYPEINFO();

    XMLMacroFieldImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nPrfx,
                               const ::rtl::OUString& sLocalName);

protected:

    /// for <office:events> children
    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import reference fields (<text:reference-get>) */
class XMLReferenceFieldImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyReferenceFieldPart;
    const ::rtl::OUString sPropertyReferenceFieldSource;
    const ::rtl::OUString sPropertySourceName;
    const ::rtl::OUString sPropertyCurrentPresentation;

    ::rtl::OUString sName;
    sal_uInt16 nElementToken;
    sal_Int16 nSource;
    sal_Int16 nType;

    sal_Bool bNameOK;
    sal_Bool bTypeOK;

public:

    TYPEINFO();

    XMLReferenceFieldImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp,
                                   sal_uInt16 nToken,
                                   sal_uInt16 nPrfx,
                                   const ::rtl::OUString& sLocalName);

protected:

    /// start element
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};



/** import dde field declaration container (<text:dde-connection-decls>) */
class XMLDdeFieldDeclsImportContext : public SvXMLImportContext
{

    SvXMLTokenMap aTokenMap;

public:

    TYPEINFO();

    XMLDdeFieldDeclsImportContext(SvXMLImport& rImport,
                                  sal_uInt16 nPrfx,
                                  const ::rtl::OUString& sLocalName);

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList );


};



/** import dde field declaration (<text:dde-connection-decl>) */
class XMLDdeFieldDeclImportContext : public SvXMLImportContext
{
    const ::rtl::OUString sPropertyIsAutomaticUpdate;
    const ::rtl::OUString sPropertyName;
    const ::rtl::OUString sPropertyDDECommandType;
    const ::rtl::OUString sPropertyDDECommandFile;
    const ::rtl::OUString sPropertyDDECommandElement;

    const SvXMLTokenMap& rTokenMap;

public:

    TYPEINFO();

    XMLDdeFieldDeclImportContext(SvXMLImport& rImport,
                                 sal_uInt16 nPrfx,
                                 const ::rtl::OUString& sLocalName,
                                 const SvXMLTokenMap& rMap);

    // create fieldmaster
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList);
};



/** import dde fields (<text:dde-connection>) */
class XMLDdeFieldImportContext : public XMLTextFieldImportContext
{
    ::rtl::OUString sName;
    ::rtl::OUString sPropertyContent;
public:

    TYPEINFO();

    XMLDdeFieldImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp,
                             sal_uInt16 nPrfx,
                             const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// create textfield, attach master, and insert into document
    virtual void EndElement();

    /// empty method
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);

};


/** import sheet name fields (Calc) dde fields (<text:sheet-name>) */
class XMLSheetNameImportContext : public XMLTextFieldImportContext
{

public:

    TYPEINFO();

    XMLSheetNameImportContext(SvXMLImport& rImport,
                              XMLTextImportHelper& rHlp,
                              sal_uInt16 nPrfx,
                              const ::rtl::OUString& sLocalName);

protected:

    /// no attributes -> empty method
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// no atributes -> empty method
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

/** import hyperlinks as URL fields (Calc, Impress, Draw) (<office:a>) */
class XMLUrlFieldImportContext : public XMLTextFieldImportContext
{

    const ::rtl::OUString sPropertyURL;
    const ::rtl::OUString sPropertyTargetFrame;
    const ::rtl::OUString sPropertyRepresentation;

    ::rtl::OUString sURL;
    ::rtl::OUString sFrame;
    sal_Bool bFrameOK;

public:

    TYPEINFO();

    XMLUrlFieldImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp,
                             sal_uInt16 nPrfx,
                             const ::rtl::OUString& sLocalName);

protected:

    /// no attributes -> empty method
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// no atributes -> empty method
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

/** import bibliography info fields (<text:bibliography-mark>) */
class XMLBibliographyFieldImportContext : public XMLTextFieldImportContext
{

    const ::rtl::OUString sPropertyFields;

    ::std::vector< ::com::sun::star::beans::PropertyValue> aValues;

public:

    TYPEINFO();

    XMLBibliographyFieldImportContext(SvXMLImport& rImport,
                                      XMLTextImportHelper& rHlp,
                                      sal_uInt16 nPrfx,
                                      const ::rtl::OUString& sLocalName);

protected:

    /// process attributes (fill aValues)
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    /// empty method; all attributes are handled in StartElement
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// convert aValues into sequence and set property
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);

    static const sal_Char* MapBibliographyFieldName(::rtl::OUString sName);

};


/** Import an annotation field (<text:annotation>) */
class XMLAnnotationImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyAuthor;
    const ::rtl::OUString sPropertyInitials;
    const ::rtl::OUString sPropertyContent;
    const ::rtl::OUString sPropertyDate;
    const ::rtl::OUString sPropertyTextRange;
    const ::rtl::OUString sPropertyName;

    ::rtl::OUStringBuffer aAuthorBuffer;
    ::rtl::OUStringBuffer aInitialsBuffer;
    OUString aName;
    ::rtl::OUStringBuffer aTextBuffer;
    ::rtl::OUStringBuffer aDateBuffer;

    com::sun::star::uno::Reference < com::sun::star::beans::XPropertySet > mxField;
    com::sun::star::uno::Reference < com::sun::star::text::XTextCursor >  mxCursor;
    com::sun::star::uno::Reference < com::sun::star::text::XTextCursor >  mxOldCursor;
    /// If this is an annotation end, then position of the start.
    com::sun::star::uno::Reference < com::sun::star::text::XTextContent >  m_xStart;
    sal_uInt16 m_nToken;

public:

    TYPEINFO();

    XMLAnnotationImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_uInt16 nToken,
                               sal_uInt16 nPrfx,
                               const ::rtl::OUString& sLocalName);

protected:

    /// process attributes
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// set properties
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();
};


/** Import a script field (<text:script>) */
class XMLScriptImportContext : public XMLTextFieldImportContext
{
    const ::rtl::OUString sPropertyScriptType;
    const ::rtl::OUString sPropertyURLContent;
    const ::rtl::OUString sPropertyContent;

    ::rtl::OUString sContent;
    ::rtl::OUString sScriptType;

    sal_Bool bContentOK;
    sal_Bool bScriptTypeOK;

public:

    TYPEINFO();

    XMLScriptImportContext(SvXMLImport& rImport,
                           XMLTextImportHelper& rHlp,
                           sal_uInt16 nPrfx,
                           const ::rtl::OUString& sLocalName);

protected:

    /// process attributes
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// set properties
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

/** import measure fields (<text:measure>) */
class XMLMeasureFieldImportContext : public XMLTextFieldImportContext
{
    sal_Int16 mnKind;
public:

    TYPEINFO();

    XMLMeasureFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


/** dropdown field (filter legacy) */
class XMLDropDownFieldImportContext : public XMLTextFieldImportContext
{
    std::vector<rtl::OUString> aLabels;
    rtl::OUString sName;
    rtl::OUString sHelp;
    rtl::OUString sHint;
    sal_Int32 nSelected;
    bool bNameOK;
    bool bHelpOK;
    bool bHintOK;

    const rtl::OUString sPropertyItems;
    const rtl::OUString sPropertySelectedItem;
    const rtl::OUString sPropertyName;
    const rtl::OUString sPropertyHelp;
    const rtl::OUString sPropertyToolTip;

public:
    TYPEINFO();

    XMLDropDownFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName);

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList >& xAttrList );

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

/** import header fields (<draw:header>) */
class XMLHeaderFieldImportContext : public XMLTextFieldImportContext
{
public:
    TYPEINFO();

    XMLHeaderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

/** import footer fields (<draw:footer>) */
class XMLFooterFieldImportContext : public XMLTextFieldImportContext
{
public:
    TYPEINFO();

    XMLFooterFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

/** import footer fields (<draw:date-and-time>) */
class XMLDateTimeFieldImportContext : public XMLTextFieldImportContext
{
public:
    TYPEINFO();

    XMLDateTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        sal_uInt16 nPrfx,                       /// namespace prefix
        const ::rtl::OUString& sLocalName);     /// element name w/o prefix

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};

class XMLCustomPropertyFieldImportContext : public XMLTextFieldImportContext
{
    ::rtl::OUString sName;
    ::com::sun::star::uno::Any aValue;
    const ::rtl::OUString sPropertyName;
    const ::rtl::OUString sPropertyValue;

public:

    TYPEINFO();

    XMLCustomPropertyFieldImportContext (SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_uInt16 nPrfx,
                                    const ::rtl::OUString& sLocalName);

protected:

    /// process attribute values
    virtual void ProcessAttribute( sal_uInt16 nAttrToken,
                                   const ::rtl::OUString& sAttrValue );

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & xPropertySet);
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
