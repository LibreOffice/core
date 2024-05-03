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

#pragma once

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/PageNumberType.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <xmloff/txtimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <vector>

namespace com::sun::star {
    namespace xml::sax { class XAttributeList; }
    namespace text { class XTextField; }
    namespace beans { class XPropertySet; struct PropertyValue; }
}

class SvXMLImport;
class XMLTextImportHelper;
class SvXMLTokenMap;

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

    XMLTextFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp,              /// Text import helper
        OUString aService);              /// name of SO API service

    /// process character data: will be collected in member sContentBuffer
    virtual void SAL_CALL characters( const OUString& sContent ) override;

    /// parses attributes and calls ProcessAttribute
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList) override;

    /// create XTextField and insert into document; calls PrepareTextField
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    /// create the appropriate field context from
    /// (for use in paragraph import)
    static XMLTextFieldImportContext* CreateTextFieldImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp,
        sal_Int32 nElement);

protected:
    /// get helper
    XMLTextImportHelper& GetImportHelper() { return rTextImportHelper; }

    const OUString& GetServiceName() const { return sServiceName; }
    void SetServiceName(const OUString& sStr) { sServiceName = sStr; }

    OUString const & GetContent();

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) = 0;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) = 0;

    /// create field from ServiceName
    bool CreateField(css::uno::Reference< css::beans::XPropertySet> & xField,
                         const OUString& sServiceName);

    /// force an update of the field's value
    /// call update on optional XUpdatable interface; (disable Fixed property)
    static void ForceUpdate(
        const css::uno::Reference< css::beans::XPropertySet> & rPropertySet);
};

class XMLSenderFieldImportContext : public XMLTextFieldImportContext
{

    sal_Int16 nSubType;         /// API subtype for ExtUser field

    const OUString sPropertyFixed;
    const OUString sPropertyContent;

protected:
    // variables for access in subclass
    bool bFixed;

public:

    XMLSenderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

protected:
    /// start element
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList) override;

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

/** inherit sender field because of fixed attribute in ProcessAttributes */
class XMLAuthorFieldImportContext final : public XMLSenderFieldImportContext
{
    bool bAuthorFullName;
    const OUString sPropertyFixed;
    const OUString sPropertyContent;

public:

    XMLAuthorFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

private:
    /// start element
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList) override;

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

class XMLPlaceholderFieldImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertyHint;

    OUString sDescription;

    sal_Int16 nPlaceholderType;

public:

    XMLPlaceholderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

class XMLTimeFieldImportContext : public XMLTextFieldImportContext
{
protected:
    const OUString sPropertyNumberFormat;
    const OUString sPropertyFixed;
    const OUString sPropertyDateTimeValue;
    const OUString sPropertyDateTime;
    const OUString sPropertyIsDate;
    const OUString sPropertyIsFixedLanguage;

    css::util::DateTime aDateTimeValue;
    sal_Int32 nAdjust;
    sal_Int32 nFormatKey;
    bool bTimeOK;
    bool bFormatOK;
    bool bFixed;
    bool bIsDate;           // is this a date?
                                // (for XMLDateFieldImportContext, really)
    bool     bIsDefaultLanguage;

public:

    XMLTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

/** import date fields (<text:date>);
    inherit from TimeField to reuse implementation */
class XMLDateFieldImportContext final : public XMLTimeFieldImportContext
{
public:

    XMLDateFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;
};

/** import page continuation fields (<text:page-continuation-string>) */
class XMLPageContinuationImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertySubType;
    const OUString sPropertyNumberingType;

    OUString sString;            /// continuation string
    css::text::PageNumberType eSelectPage;   /// previous, current
                                                        /// or next page
    bool sStringOK;                 /// continuation string encountered?

public:

    XMLPageContinuationImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

/** import page number fields (<text:page-number>) */
class XMLPageNumberImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertySubType;
    const OUString sPropertyNumberingType;
    const OUString sPropertyOffset;

    OUString sNumberFormat;
    OUString sNumberSync;
    sal_Int16 nPageAdjust;
    css::text::PageNumberType eSelectPage;   /// previous, current
                                                        /// or next page
    bool sNumberFormatOK;

public:

    XMLPageNumberImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper


    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

/** superclass for database fields: handle database and table names */
class XMLDatabaseFieldImportContext : public XMLTextFieldImportContext
{
    OUString m_sDatabaseName;
    OUString m_sDatabaseURL;
    OUString m_sTableName;

    sal_Int32 m_nCommandType;
    bool m_bCommandTypeOK;

    bool m_bDisplay;
    bool m_bDisplayOK;
    bool m_bUseDisplay;

protected:
    bool m_bDatabaseOK;
    bool m_bDatabaseNameOK;
    bool m_bDatabaseURLOK;
    bool m_bTableOK;

    /// protected constructor: only for subclasses
    XMLDatabaseFieldImportContext(SvXMLImport& rImport,
                                  XMLTextImportHelper& rHlp,
                                  const OUString& pServiceName,
                                  bool bUseDisplay );

public:

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;

    /// handle database-location children
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

/** import database name fields (<text:database-name>) */
class XMLDatabaseNameImportContext final : public XMLDatabaseFieldImportContext
{
public:

    XMLDatabaseNameImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp);

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;
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
                                 const OUString& pServiceName);

public:

    XMLDatabaseNextImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp);

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

/** import database select fields (<text:database-select>) */
class XMLDatabaseSelectImportContext final : public XMLDatabaseNextImportContext
{
    const OUString sPropertySetNumber;
    sal_Int32 nNumber;
    bool bNumberOK;

public:

    XMLDatabaseSelectImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp);

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<
        css::beans::XPropertySet> & xPropertySet) override;
};

/** import database display number fields (<text:database-row-number>) */
class XMLDatabaseNumberImportContext final : public XMLDatabaseFieldImportContext
{
    const OUString sPropertyNumberingType;
    const OUString sPropertySetNumber;
    OUString sNumberFormat;
    OUString sNumberSync;
    sal_Int32 nValue;
    bool bValueOK;

public:

    XMLDatabaseNumberImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp);

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
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

    XMLSimpleDocInfoImportContext(SvXMLImport& rImport,
                                  XMLTextImportHelper& rHlp,
                                  sal_Int32 nElementToken,
                                  bool bContent,
                                  bool bAuthor);

protected:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;

    static OUString MapTokenToServiceName(sal_Int32 nElementToken);
};

/** import docinfo fields with date or time attributes and numberformats */
class XMLDateTimeDocInfoImportContext final : public XMLSimpleDocInfoImportContext
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

    XMLDateTimeDocInfoImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_Int32 nElement);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet> & xPropertySet) override;
};

/** import revision field (<text:editing-cycles>) */
class XMLRevisionDocInfoImportContext final : public XMLSimpleDocInfoImportContext
{
public:

    XMLRevisionDocInfoImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp,
                                    sal_Int32 nElement);

private:
    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import user docinfo field (<text:user-defined>) */
class XMLUserDocInfoImportContext final : public XMLSimpleDocInfoImportContext
{
    OUString aName;
    const OUString sPropertyName;
    const OUString sPropertyNumberFormat;
    const OUString sPropertyIsFixedLanguage;
    sal_Int32   nFormat;
    bool    bFormatOK;
    bool        bIsDefaultLanguage;

public:

    XMLUserDocInfoImportContext(SvXMLImport& rImport,
                                XMLTextImportHelper& rHlp,
                                sal_Int32 nElement);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import hidden paragraph fields (<text:hidden-paragraph>) */
class XMLHiddenParagraphImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertyCondition;
    const OUString sPropertyIsHidden;

    OUString sCondition;
    bool bIsHidden;

public:

    XMLHiddenParagraphImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import conditional text fields (<text:conditional-text>) */
class XMLConditionalTextImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertyCondition;
    const OUString sPropertyCurrentPresentation;

    OUString sCondition;
    OUString sTrueContent;
    OUString sFalseContent;

    bool bConditionOK;
    bool bTrueOK;
    bool bFalseOK;
    bool bCurrentValue;

public:

    XMLConditionalTextImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import conditional text fields (<text:hidden-text>) */
class XMLHiddenTextImportContext final : public XMLTextFieldImportContext
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

    XMLHiddenTextImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import file name fields (<text:file-name>) */
class XMLFileNameImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertyFixed;
    const OUString sPropertyFileFormat;
    const OUString sPropertyCurrentPresentation;

    sal_Int16 nFormat;
    bool bFixed;

public:

    XMLFileNameImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import document template name fields (<text:template-name>) */
class XMLTemplateNameImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertyFileFormat;

    sal_Int16 nFormat;

public:

    XMLTemplateNameImportContext(SvXMLImport& rImport,
                                 XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import chapter fields (<text:chapter>) */
class XMLChapterImportContext final : public XMLTextFieldImportContext
{
    sal_Int16 nFormat;
    sal_Int8 nLevel;

public:

    XMLChapterImportContext(SvXMLImport& rImport,
                            XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import count fields (<text:[XXX]-count>) */
class XMLCountFieldImportContext final : public XMLTextFieldImportContext
{
    const OUString sPropertyNumberingType;

    OUString sNumberFormat;
    OUString sLetterSync;

    bool bNumberFormatOK;

public:

    XMLCountFieldImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_Int32 nElement);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;

    static OUString MapTokenToServiceName(sal_Int32 nElement);
};

/** import page variable fields (<text:get-page-variable>) */
class XMLPageVarGetFieldImportContext final : public XMLTextFieldImportContext
{
    OUString sNumberFormat;
    OUString sLetterSync;

    bool bNumberFormatOK;

public:

    XMLPageVarGetFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import page variable fields (<text:get-page-variable>) */
class XMLPageVarSetFieldImportContext final : public XMLTextFieldImportContext
{
    sal_Int16 nAdjust;
    bool bActive;

public:

    XMLPageVarSetFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import macro fields (<text:execute-macro>) */
class XMLMacroFieldImportContext final : public XMLTextFieldImportContext
{
    OUString sDescription;
    rtl::Reference<XMLEventsImportContext> xEventContext;

    OUString sMacro; // macro for old documents (pre 638i)

    bool bDescriptionOK;

public:

    XMLMacroFieldImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp);

private:
    /// for <office:events> children
    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import reference fields (<text:reference-get>) */
class XMLReferenceFieldImportContext final : public XMLTextFieldImportContext
{
    OUString sName;
    OUString sLanguage;
    sal_Int32 nElementToken;
    sal_Int16 nSource;
    sal_Int16 nType;
    sal_uInt16 nFlags;

    bool bNameOK;
    bool bTypeOK;

public:

    XMLReferenceFieldImportContext(SvXMLImport& rImport,
                                   XMLTextImportHelper& rHlp,
                                   sal_Int32 nToken);

private:
    /// start element
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList) override;

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import dde field declaration container (<text:dde-connection-decls>) */
class XMLDdeFieldDeclsImportContext final : public SvXMLImportContext
{
public:

    XMLDdeFieldDeclsImportContext(SvXMLImport& rImport);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
};

/** import dde field declaration (<text:dde-connection-decl>) */
class XMLDdeFieldDeclImportContext final : public SvXMLImportContext
{
public:

    XMLDdeFieldDeclImportContext(SvXMLImport& rImport);

    // create fieldmaster
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference<css::xml::sax::XFastAttributeList> & xAttrList) override;
};

/** import dde fields (<text:dde-connection>) */
class XMLDdeFieldImportContext final : public XMLTextFieldImportContext
{
    OUString sName;
    OUString sPropertyContent;

public:

    XMLDdeFieldImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// create textfield, attach master, and insert into document
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

    /// empty method
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import sheet name fields (Calc) dde fields (<text:sheet-name>) */
class XMLSheetNameImportContext final : public XMLTextFieldImportContext
{

public:

    XMLSheetNameImportContext(SvXMLImport& rImport,
                              XMLTextImportHelper& rHlp);

private:
    /// no attributes -> empty method
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// no attributes -> empty method
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import page|slide name fields (<text:page-name>) */
class XMLPageNameFieldImportContext final : public XMLTextFieldImportContext
{
public:

    XMLPageNameFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import hyperlinks as URL fields (Calc, Impress, Draw) (<office:a>) */
class XMLUrlFieldImportContext final : public XMLTextFieldImportContext
{
    OUString sURL;
    OUString sFrame;
    bool bFrameOK;

public:

    XMLUrlFieldImportContext(SvXMLImport& rImport,
                             XMLTextImportHelper& rHlp);

private:
    /// no attributes -> empty method
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// no attributes -> empty method
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import bibliography info fields (<text:bibliography-mark>) */
class XMLBibliographyFieldImportContext final : public XMLTextFieldImportContext
{
    ::std::vector< css::beans::PropertyValue> aValues;

public:

    XMLBibliographyFieldImportContext(SvXMLImport& rImport,
                                      XMLTextImportHelper& rHlp);

private:
    /// process attributes (fill aValues)
    virtual void SAL_CALL startFastElement(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList) override;

    /// empty method; all attributes are handled in StartElement
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// convert aValues into sequence and set property
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;

    static OUString MapBibliographyFieldName(sal_Int32 nElement);
};

/** Import an annotation field (<text:annotation>) */
class XMLAnnotationImportContext final : public XMLTextFieldImportContext
{
    OUStringBuffer aAuthorBuffer;
    OUStringBuffer aInitialsBuffer;
    OUString aName;
    OUString aParentName;
    OUStringBuffer aTextBuffer;
    OUStringBuffer aDateBuffer;
    OUString aResolved;

    css::uno::Reference < css::beans::XPropertySet > mxField;
    css::uno::Reference < css::text::XTextCursor >  mxCursor;
    css::uno::Reference < css::text::XTextCursor >  mxOldCursor;

    sal_Int32 mnElement;

public:

    XMLAnnotationImportContext(SvXMLImport& rImport,
                               XMLTextImportHelper& rHlp,
                               sal_Int32 nElement);

private:
    /// process attributes
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// set properties
    virtual void PrepareField(
        const css::uno::Reference< css::beans::XPropertySet > & xPropertySet) override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;
};

/** Import a script field (<text:script>) */
class XMLScriptImportContext final : public XMLTextFieldImportContext
{
    OUString sContent;
    OUString sScriptType;

    bool bContentOK;

public:

    XMLScriptImportContext(SvXMLImport& rImport,
                           XMLTextImportHelper& rHlp);

private:
    /// process attributes
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// set properties
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import measure fields (<text:measure>) */
class XMLMeasureFieldImportContext final : public XMLTextFieldImportContext
{
    sal_Int16 mnKind;

public:

    XMLMeasureFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp);

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** dropdown field (filter legacy) */
class XMLDropDownFieldImportContext final : public XMLTextFieldImportContext
{
    std::vector<OUString> aLabels;
    OUString sName;
    OUString sHelp;
    OUString sHint;
    sal_Int32 nSelected;
    bool bNameOK;
    bool bHelpOK;
    bool bHintOK;

public:

    XMLDropDownFieldImportContext(SvXMLImport& rImport,
                                    XMLTextImportHelper& rHlp);

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

private:
    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import header fields (<draw:header>) */
class XMLHeaderFieldImportContext final : public XMLTextFieldImportContext
{
public:

    XMLHeaderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import footer fields (<draw:footer>) */
class XMLFooterFieldImportContext final : public XMLTextFieldImportContext
{
public:

    XMLFooterFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/** import footer fields (<draw:date-and-time>) */
class XMLDateTimeFieldImportContext final : public XMLTextFieldImportContext
{
public:

    XMLDateTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp);              /// Text import helper

    /// process attribute values
    virtual void ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue ) override;

    /// prepare XTextField for insertion into document
    virtual void PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
