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
 *  Import of all text fields except those from txtvfldi.cxx
 *  (variable related text fields and database display fields)
 */

#include <sal/config.h>

#include <cassert>

#include <txtfld.hxx>
#include <txtfldi.hxx>
#include <txtvfldi.hxx>
#include <utility>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <XMLStringBufferImportContext.hxx>
#include <xmloff/XMLEventsImportContext.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/UserDataPart.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/PlaceholderType.hpp>
#include <com/sun/star/text/ReferenceFieldPart.hpp>
#include <com/sun/star/text/ReferenceFieldSource.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/XTextFieldsSupplier.hpp>
#include <com/sun/star/text/XDependentTextField.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>
#include <com/sun/star/text/ChapterFormat.hpp>
#include <com/sun/star/text/TemplateDisplayFormat.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/text/BibliographyDataType.hpp>
#include <com/sun/star/util/XUpdatable.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XUniqueIDAccess.hpp>

#include <sax/tools/converter.hxx>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <rtl/math.hxx>
#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;


// SO API string constants


// service prefix and service names
constexpr OUString sAPI_textfield_prefix = u"com.sun.star.text.TextField."_ustr;
constexpr char16_t sAPI_fieldmaster_prefix[] = u"com.sun.star.text.FieldMaster.";
constexpr OUString sAPI_presentation_prefix = u"com.sun.star.presentation.TextField."_ustr;

constexpr OUString sAPI_date_time        = u"DateTime"_ustr;
constexpr OUString sAPI_page_number      = u"PageNumber"_ustr;
constexpr OUString sAPI_docinfo_change_date_time = u"DocInfo.ChangeDateTime"_ustr;
constexpr OUString sAPI_docinfo_create_date_time = u"DocInfo.CreateDateTime"_ustr;
constexpr OUString sAPI_docinfo_custom   = u"DocInfo.Custom"_ustr;
constexpr OUString sAPI_docinfo_print_date_time = u"DocInfo.PrintDateTime"_ustr;
constexpr OUString sAPI_dde              = u"DDE"_ustr;
constexpr OUString sAPI_url              = u"URL"_ustr;

// property names
constexpr OUString sAPI_is_fixed = u"IsFixed"_ustr;
constexpr OUString sAPI_content  = u"Content"_ustr;
constexpr OUString sAPI_author   = u"Author"_ustr;
constexpr OUString sAPI_hint     = u"Hint"_ustr;
constexpr OUString sAPI_name     = u"Name"_ustr;
constexpr OUStringLiteral sAPI_parent_name = u"ParentName";
constexpr OUString sAPI_sub_type = u"SubType"_ustr;
constexpr OUString sAPI_date_time_value = u"DateTimeValue"_ustr;
constexpr OUString sAPI_number_format = u"NumberFormat"_ustr;
constexpr OUString sAPI_numbering_type = u"NumberingType"_ustr;
constexpr OUString sAPI_offset   = u"Offset"_ustr;
constexpr OUString sAPI_condition = u"Condition"_ustr;
constexpr OUString sAPI_set_number = u"SetNumber"_ustr;
constexpr OUString sAPI_file_format = u"FileFormat"_ustr;
constexpr OUString sAPI_is_date  = u"IsDate"_ustr;
constexpr OUString sAPI_current_presentation = u"CurrentPresentation"_ustr;
constexpr OUString sAPI_is_hidden = u"IsHidden"_ustr;
constexpr OUString sAPI_is_fixed_language = u"IsFixedLanguage"_ustr;

constexpr OUString sAPI_true = u"TRUE"_ustr;


XMLTextFieldImportContext::XMLTextFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    OUString aService)
:   SvXMLImportContext( rImport )
,   sServiceName(std::move(aService))
,   rTextImportHelper(rHlp)
,   sServicePrefix(sAPI_textfield_prefix)
,   bValid(false)
{
}

void XMLTextFieldImportContext::startFastElement(
        sal_Int32 /*nElement*/,
        const Reference<XFastAttributeList> & xAttrList)
{
    // process attributes
    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
        ProcessAttribute(aIter.getToken(), aIter.toView() );
}

OUString const & XMLTextFieldImportContext::GetContent()
{
    if (sContent.isEmpty())
    {
        sContent = sContentBuffer.makeStringAndClear();
    }

    return sContent;
}

void XMLTextFieldImportContext::endFastElement(sal_Int32 )
{
    if (bValid)
    {

        // create field/Service
        Reference<XPropertySet> xPropSet;
        if (CreateField(xPropSet, sServicePrefix + GetServiceName()))
        {
            // set field properties
            PrepareField(xPropSet);

            // attach field to document
            Reference<XTextContent> xTextContent(xPropSet, UNO_QUERY);

            // workaround for #80606#
            try
            {
                rTextImportHelper.InsertTextContent(xTextContent);
            }
            catch (const lang::IllegalArgumentException&)
            {
                // ignore
            }
            return;
        }
    }

    // in case of error: write element content
    rTextImportHelper.InsertString(GetContent());
}

void XMLTextFieldImportContext::characters(const OUString& rContent)
{
    sContentBuffer.append(rContent);
}

bool XMLTextFieldImportContext::CreateField(
    Reference<XPropertySet> & xField,
    const OUString& rServiceName)
{
    // instantiate new XTextField:
    // ask import for model, model is factory, ask factory to create service

    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),UNO_QUERY);
    if( xFactory.is() )
    {
        Reference<XInterface> xIfc = xFactory->createInstance(rServiceName);
        if( xIfc.is() )
        {
            xField.set(xIfc, UNO_QUERY);
        } else {
            return false;   // can't create instance
        }
    } else {
        return false;   // can't get MultiServiceFactory
    }

    return true;
}

/// create the appropriate field context from
XMLTextFieldImportContext*
XMLTextFieldImportContext::CreateTextFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_Int32 nToken)
{
    XMLTextFieldImportContext* pContext = nullptr;

    switch (nToken)
    {
        case XML_ELEMENT(TEXT, XML_SENDER_FIRSTNAME):
        case XML_ELEMENT(TEXT, XML_SENDER_LASTNAME):
        case XML_ELEMENT(LO_EXT, XML_SENDER_INITIALS):
        case XML_ELEMENT(TEXT, XML_SENDER_INITIALS):
        case XML_ELEMENT(TEXT, XML_SENDER_TITLE):
        case XML_ELEMENT(TEXT, XML_SENDER_POSITION):
        case XML_ELEMENT(TEXT, XML_SENDER_EMAIL):
        case XML_ELEMENT(TEXT, XML_SENDER_PHONE_PRIVATE):

        case XML_ELEMENT(TEXT, XML_SENDER_FAX):
        case XML_ELEMENT(TEXT, XML_SENDER_COMPANY):
        case XML_ELEMENT(TEXT, XML_SENDER_PHONE_WORK):
        case XML_ELEMENT(TEXT, XML_SENDER_STREET):
        case XML_ELEMENT(TEXT, XML_SENDER_CITY):
        case XML_ELEMENT(TEXT, XML_SENDER_POSTAL_CODE):
        case XML_ELEMENT(TEXT, XML_SENDER_COUNTRY):
        case XML_ELEMENT(TEXT, XML_SENDER_STATE_OR_PROVINCE):
            pContext =
                new XMLSenderFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_AUTHOR_NAME):
        case XML_ELEMENT(TEXT, XML_AUTHOR_INITIALS):
            pContext =
                new XMLAuthorFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_PLACEHOLDER):
            pContext =
                new XMLPlaceholderFieldImportContext( rImport, rHlp);
            break;
        case XML_ELEMENT(TEXT, XML_SEQUENCE):
            pContext =
                new XMLSequenceFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_TEXT_INPUT):
            pContext =
                new XMLTextInputFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_EXPRESSION):
            pContext =
                new XMLExpressionFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_VARIABLE_SET):
            pContext =
                new XMLVariableSetFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_VARIABLE_INPUT):
            pContext =
                new XMLVariableInputFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_VARIABLE_GET):
            pContext =
                new XMLVariableGetFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_USER_FIELD_GET):
            pContext = new XMLUserFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_USER_FIELD_INPUT):
            pContext = new XMLUserFieldInputImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_TIME):
            pContext = new XMLTimeFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_PAGE_CONTINUATION_STRING):
        case XML_ELEMENT(TEXT, XML_PAGE_CONTINUATION):
            pContext = new XMLPageContinuationImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_PAGE_NUMBER):
            pContext = new XMLPageNumberImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_DATE):
            pContext = new XMLDateFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_DATABASE_NAME):
            pContext = new XMLDatabaseNameImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_DATABASE_NEXT):
            pContext = new XMLDatabaseNextImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_DATABASE_ROW_SELECT):
            pContext = new XMLDatabaseSelectImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_DATABASE_ROW_NUMBER):
            pContext = new XMLDatabaseNumberImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_DATABASE_DISPLAY):
            pContext = new XMLDatabaseDisplayImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_CONDITIONAL_TEXT):
            pContext = new XMLConditionalTextImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_HIDDEN_TEXT):
            pContext = new XMLHiddenTextImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_HIDDEN_PARAGRAPH):
            pContext = new XMLHiddenParagraphImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_DESCRIPTION):
        case XML_ELEMENT(TEXT, XML_TITLE):
        case XML_ELEMENT(TEXT, XML_SUBJECT):
        case XML_ELEMENT(TEXT, XML_KEYWORDS):
            pContext = new XMLSimpleDocInfoImportContext( rImport, rHlp,
                                                          nToken, true,
                                                          false );
            break;
        case XML_ELEMENT(TEXT, XML_INITIAL_CREATOR):
        case XML_ELEMENT(TEXT, XML_PRINTED_BY):
        case XML_ELEMENT(TEXT, XML_CREATOR):
            pContext = new XMLSimpleDocInfoImportContext( rImport, rHlp,
                                                          nToken, false,
                                                          true );
            break;

        case XML_ELEMENT(TEXT, XML_CREATION_DATE):
        case XML_ELEMENT(TEXT, XML_CREATION_TIME):
        case XML_ELEMENT(TEXT, XML_PRINT_DATE):
        case XML_ELEMENT(TEXT, XML_PRINT_TIME):
        case XML_ELEMENT(TEXT, XML_MODIFICATION_DATE):
        case XML_ELEMENT(TEXT, XML_MODIFICATION_TIME):
        case XML_ELEMENT(TEXT, XML_EDITING_DURATION):
            pContext = new XMLDateTimeDocInfoImportContext( rImport, rHlp,
                                                            nToken );
            break;

        case XML_ELEMENT(TEXT, XML_EDITING_CYCLES):
            pContext = new XMLRevisionDocInfoImportContext( rImport, rHlp,
                                                            nToken );
            break;

        case XML_ELEMENT(TEXT, XML_USER_DEFINED):
            pContext = new XMLUserDocInfoImportContext( rImport, rHlp,
                                                        nToken );
            break;

        case XML_ELEMENT(TEXT, XML_FILE_NAME):
            pContext = new XMLFileNameImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_CHAPTER):
            pContext = new XMLChapterImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_TEMPLATE_NAME):
            pContext = new XMLTemplateNameImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_WORD_COUNT):
        case XML_ELEMENT(TEXT, XML_PARAGRAPH_COUNT):
        case XML_ELEMENT(TEXT, XML_TABLE_COUNT):
        case XML_ELEMENT(TEXT, XML_CHARACTER_COUNT):
        case XML_ELEMENT(TEXT, XML_IMAGE_COUNT):
        case XML_ELEMENT(TEXT, XML_OBJECT_COUNT):
        case XML_ELEMENT(TEXT, XML_PAGE_COUNT):
            pContext = new XMLCountFieldImportContext( rImport, rHlp, nToken);
            break;

        case XML_ELEMENT(TEXT, XML_PAGE_VARIABLE_GET):
            pContext = new XMLPageVarGetFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_PAGE_VARIABLE_SET):
            pContext = new XMLPageVarSetFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_EXECUTE_MACRO):
            pContext = new XMLMacroFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_DDE_CONNECTION):
            pContext = new XMLDdeFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_REFERENCE_REF):
        case XML_ELEMENT(TEXT, XML_BOOKMARK_REF):
        case XML_ELEMENT(TEXT, XML_NOTE_REF):
        case XML_ELEMENT(TEXT, XML_SEQUENCE_REF):
        case XML_ELEMENT(TEXT, XML_STYLE_REF):
        case XML_ELEMENT(LO_EXT, XML_STYLE_REF):
            pContext = new XMLReferenceFieldImportContext( rImport, rHlp, nToken );
            break;

        case XML_ELEMENT(TEXT, XML_SHEET_NAME):
            pContext = new XMLSheetNameImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_PAGE_NAME):
        case XML_ELEMENT(LO_EXT, XML_PAGE_NAME):
            pContext = new XMLPageNameFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_BIBLIOGRAPHY_MARK):
            pContext = new XMLBibliographyFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(OFFICE, XML_ANNOTATION):
        case XML_ELEMENT(OFFICE, XML_ANNOTATION_END):
            pContext = new XMLAnnotationImportContext( rImport, rHlp, nToken);
            break;

        case XML_ELEMENT(TEXT, XML_SCRIPT):
            pContext = new XMLScriptImportContext( rImport, rHlp);
            break;

        case XML_ELEMENT(TEXT, XML_MEASURE):
            pContext = new XMLMeasureFieldImportContext( rImport, rHlp );
            break;

        case XML_ELEMENT(TEXT, XML_TABLE_FORMULA):
            pContext = new XMLTableFormulaImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(TEXT, XML_DROP_DOWN):
            pContext = new XMLDropDownFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(PRESENTATION, XML_HEADER):
            pContext = new XMLHeaderFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(PRESENTATION, XML_FOOTER):
            pContext = new XMLFooterFieldImportContext( rImport, rHlp );
            break;
        case XML_ELEMENT(PRESENTATION, XML_DATE_TIME):
            pContext = new XMLDateTimeFieldImportContext( rImport, rHlp );
            break;

        default:
            // ignore! May not even be a textfield.
            // (Reminder: This method is called inside default:-branch)
            pContext = nullptr;
            break;
    }

    return pContext;
}


void XMLTextFieldImportContext::ForceUpdate(
    const Reference<XPropertySet> & rPropertySet)
{
    // force update
    Reference<XUpdatable> xUpdate(rPropertySet, UNO_QUERY);
    if (xUpdate.is())
    {
        xUpdate->update();
    }
    else
    {
        OSL_FAIL("Expected XUpdatable support!");
    }
}


// XMLSenderFieldImportContext


constexpr OUStringLiteral gsPropertyFieldSubType(u"UserDataType");

XMLSenderFieldImportContext::XMLSenderFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp)
    : XMLTextFieldImportContext(rImport, rHlp, u"ExtendedUser"_ustr)
    , nSubType(0)
    , sPropertyFixed(sAPI_is_fixed)
    , sPropertyContent(sAPI_content)
    , bFixed(true)
{
}

void XMLSenderFieldImportContext::startFastElement(
        sal_Int32 nElement,
        const Reference<XFastAttributeList> & xAttrList)
{
    bValid = true;
    switch (nElement) {
    case XML_ELEMENT(TEXT, XML_SENDER_FIRSTNAME):
        nSubType = UserDataPart::FIRSTNAME;
        break;
    case  XML_ELEMENT(TEXT, XML_SENDER_LASTNAME):
        nSubType = UserDataPart::NAME;
        break;
    case XML_ELEMENT(LO_EXT, XML_SENDER_INITIALS):
    case XML_ELEMENT(TEXT,   XML_SENDER_INITIALS):
        nSubType = UserDataPart::SHORTCUT;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_TITLE):
        nSubType = UserDataPart::TITLE;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_POSITION):
        nSubType = UserDataPart::POSITION;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_EMAIL):
        nSubType = UserDataPart::EMAIL;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_PHONE_PRIVATE):
        nSubType = UserDataPart::PHONE_PRIVATE;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_FAX):
        nSubType = UserDataPart::FAX;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_COMPANY):
        nSubType = UserDataPart::COMPANY;
        break;
    case  XML_ELEMENT(TEXT, XML_SENDER_PHONE_WORK):
        nSubType = UserDataPart::PHONE_COMPANY;
        break;
    case  XML_ELEMENT(TEXT, XML_SENDER_STREET):
        nSubType = UserDataPart::STREET;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_CITY):
        nSubType = UserDataPart::CITY;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_POSTAL_CODE):
        nSubType = UserDataPart::ZIP;
        break;
    case XML_ELEMENT(TEXT, XML_SENDER_COUNTRY):
        nSubType = UserDataPart::COUNTRY;
        break;
    case  XML_ELEMENT(TEXT, XML_SENDER_STATE_OR_PROVINCE):
        nSubType = UserDataPart::STATE;
        break;
    default:
        bValid = false;
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
        break;
    }

    // process Attributes
    XMLTextFieldImportContext::startFastElement(nElement, xAttrList);
}

void XMLSenderFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue)
{
    if (XML_ELEMENT(TEXT, XML_FIXED) == nAttrToken) {

        // set bVal
        bool bVal(false);
        bool const bRet = ::sax::Converter::convertBool(bVal, sAttrValue);

        // set bFixed if successful
        if (bRet) {
            bFixed = bVal;
        }
    }
    else
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

void XMLSenderFieldImportContext::PrepareField(
    const Reference<XPropertySet> & rPropSet)
{
    // set members
    rPropSet->setPropertyValue(gsPropertyFieldSubType, Any(nSubType));

    // set fixed
    rPropSet->setPropertyValue(sPropertyFixed, Any(bFixed));

    // set content if fixed
    if (!bFixed)
        return;

    // in organizer or styles-only mode: force update
    if (GetImport().GetTextImport()->IsOrganizerMode() ||
        GetImport().GetTextImport()->IsStylesOnlyMode()   )
    {
        ForceUpdate(rPropSet);
    }
    else
    {
        rPropSet->setPropertyValue(sPropertyContent, Any(GetContent()));
    }
}


// XMLAuthorFieldImportContext

constexpr OUStringLiteral gsPropertyAuthorFullName(u"FullName");

XMLAuthorFieldImportContext::XMLAuthorFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp)
:   XMLSenderFieldImportContext(rImport, rHlp)
,   bAuthorFullName(true)
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyContent(sAPI_content)
{
    // overwrite service name from XMLSenderFieldImportContext
    SetServiceName(sAPI_author);
}

void XMLAuthorFieldImportContext::startFastElement(
        sal_Int32 nElement,
        const Reference<XFastAttributeList> & xAttrList)
{
    bAuthorFullName = ( XML_ELEMENT(TEXT, XML_AUTHOR_INITIALS) != nElement);
    bValid = true;

    // process Attributes
    XMLTextFieldImportContext::startFastElement(nElement, xAttrList);
}

void XMLAuthorFieldImportContext::ProcessAttribute(sal_Int32 nAttrToken, std::string_view sAttrValue)
{
    if(nAttrToken == XML_ELEMENT(TEXT, XML_FIXED))
    {
        bool bTmp(false);
        if (::sax::Converter::convertBool(bTmp, sAttrValue))
            bFixed = bTmp;
    }
    else
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

void XMLAuthorFieldImportContext::PrepareField(
    const Reference<XPropertySet> & rPropSet)
{
    // set members
    Any aAny;
    rPropSet->setPropertyValue(gsPropertyAuthorFullName, Any(bAuthorFullName));

    rPropSet->setPropertyValue(sPropertyFixed, Any(bFixed));

    // set content if fixed
    if (!bFixed)
        return;

    // organizer or styles-only mode: force update
    if (GetImport().GetTextImport()->IsOrganizerMode() ||
        GetImport().GetTextImport()->IsStylesOnlyMode()   )
    {
        ForceUpdate(rPropSet);
    }
    else
    {
        aAny <<= GetContent();
        rPropSet->setPropertyValue(sPropertyContent, aAny);
    }
}


// page continuation string


SvXMLEnumMapEntry<PageNumberType> const lcl_aSelectPageAttrMap[] =
{
    { XML_PREVIOUS,      PageNumberType_PREV },
    { XML_CURRENT,       PageNumberType_CURRENT },
    { XML_NEXT,          PageNumberType_NEXT },
    { XML_TOKEN_INVALID, PageNumberType(0) },
};

constexpr OUStringLiteral gsPropertyUserText(u"UserText");

XMLPageContinuationImportContext::XMLPageContinuationImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_page_number)
,   sPropertySubType(sAPI_sub_type)
,   sPropertyNumberingType(sAPI_numbering_type)
,   eSelectPage(PageNumberType_CURRENT)
,   sStringOK(false)
{
    bValid = true;
}

void XMLPageContinuationImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    switch(nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_SELECT_PAGE):
        {
            PageNumberType nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                lcl_aSelectPageAttrMap)
                && (PageNumberType_CURRENT != nTmp) )
            {
                eSelectPage = nTmp;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_STRING_VALUE):
        case XML_ELEMENT(OFFICE, XML_STRING_VALUE):
            sString = OUString::fromUtf8(sAttrValue);
            sStringOK = true;
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLPageContinuationImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    xPropertySet->setPropertyValue(sPropertySubType, Any(eSelectPage));

    aAny <<= (sStringOK ? sString : GetContent());
    xPropertySet->setPropertyValue(gsPropertyUserText, aAny);

    aAny <<= style::NumberingType::CHAR_SPECIAL;
    xPropertySet->setPropertyValue(sPropertyNumberingType, aAny);
}


// page number field


XMLPageNumberImportContext::XMLPageNumberImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_page_number)
,   sPropertySubType(sAPI_sub_type)
,   sPropertyNumberingType(sAPI_numbering_type)
,   sPropertyOffset(sAPI_offset)
,   sNumberSync(GetXMLToken(XML_FALSE))
,   nPageAdjust(0)
,   eSelectPage(PageNumberType_CURRENT)
,   sNumberFormatOK(false)
{
    bValid = true;
}

void XMLPageNumberImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            sNumberFormat = OUString::fromUtf8(sAttrValue);
            sNumberFormatOK = true;
            break;
        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            sNumberSync = OUString::fromUtf8(sAttrValue);
            break;
        case XML_ELEMENT(TEXT, XML_SELECT_PAGE):
            SvXMLUnitConverter::convertEnum(eSelectPage, sAttrValue,
                                                lcl_aSelectPageAttrMap);
            break;
        case XML_ELEMENT(TEXT, XML_PAGE_ADJUST):
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(nTmp, sAttrValue))
            {
                nPageAdjust = static_cast<sal_Int16>(nTmp);
            }
            break;
        }
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLPageNumberImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    // all properties are optional
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());

    if (xPropertySetInfo->hasPropertyByName(sPropertyNumberingType))
    {
        sal_Int16 nNumType;
        if( sNumberFormatOK )
        {
            nNumType= style::NumberingType::ARABIC;
            GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sNumberSync );
        }
        else
            nNumType = style::NumberingType::PAGE_DESCRIPTOR;

        xPropertySet->setPropertyValue(sPropertyNumberingType, Any(nNumType));
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertyOffset))
    {
        // adjust offset
        switch (eSelectPage)
        {
            case PageNumberType_PREV:
                nPageAdjust--;
                break;
            case PageNumberType_CURRENT:
                break;
            case PageNumberType_NEXT:
                nPageAdjust++;
                break;
            default:
                SAL_WARN("xmloff.text", "unknown page number type");
        }
        xPropertySet->setPropertyValue(sPropertyOffset, Any(nPageAdjust));
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertySubType))
    {
        xPropertySet->setPropertyValue(sPropertySubType, Any(eSelectPage));
    }
}


// Placeholder


constexpr OUStringLiteral gsPropertyPlaceholderType(u"PlaceHolderType");
constexpr OUStringLiteral gsPropertyPlaceholder(u"PlaceHolder");

XMLPlaceholderFieldImportContext::XMLPlaceholderFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp)
:   XMLTextFieldImportContext(rImport, rHlp, u"JumpEdit"_ustr)
,   sPropertyHint(sAPI_hint)
,   nPlaceholderType(PlaceholderType::TEXT)
{
}

/// process attribute values
void XMLPlaceholderFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    switch (nAttrToken) {
    case XML_ELEMENT(TEXT, XML_DESCRIPTION):
        sDescription = OUString::fromUtf8(sAttrValue);
        break;

    case XML_ELEMENT(TEXT, XML_PLACEHOLDER_TYPE):
        bValid = true;
        if (IsXMLToken(sAttrValue, XML_TABLE))
        {
            nPlaceholderType = PlaceholderType::TABLE;
        }
        else if (IsXMLToken(sAttrValue, XML_TEXT))
        {
            nPlaceholderType = PlaceholderType::TEXT;
        }
        else if (IsXMLToken(sAttrValue, XML_TEXT_BOX))
        {
            nPlaceholderType = PlaceholderType::TEXTFRAME;
        }
        else if (IsXMLToken(sAttrValue, XML_IMAGE))
        {
            nPlaceholderType = PlaceholderType::GRAPHIC;
        }
        else if (IsXMLToken(sAttrValue, XML_OBJECT))
        {
            nPlaceholderType = PlaceholderType::OBJECT;
        }
        else
        {
            bValid = false;
        }
        break;

    default:
        // ignore
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLPlaceholderFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet) {

    Any aAny;
    xPropertySet->setPropertyValue(sPropertyHint, Any(sDescription));

    // remove <...> around content (if present)
    OUString aContent = GetContent();
    sal_Int32 nStart = 0;
    sal_Int32 nLength = aContent.getLength();
    if (aContent.startsWith("<"))
    {
        --nLength;
        ++nStart;
    }
    if (aContent.endsWith(">"))
    {
        --nLength;
    }
    aAny <<= aContent.copy(nStart, nLength);
    xPropertySet->setPropertyValue(gsPropertyPlaceholder, aAny);

    xPropertySet->setPropertyValue(gsPropertyPlaceholderType, Any(nPlaceholderType));
}


// time field

constexpr OUString gsPropertyAdjust(u"Adjust"_ustr);

XMLTimeFieldImportContext::XMLTimeFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp)
:   XMLTextFieldImportContext(rImport, rHlp, sAPI_date_time)
,   sPropertyNumberFormat(sAPI_number_format)
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyDateTimeValue(sAPI_date_time_value)
,   sPropertyDateTime(sAPI_date_time)
,   sPropertyIsDate(sAPI_is_date)
,   sPropertyIsFixedLanguage(sAPI_is_fixed_language)
,   nAdjust(0)
,   nFormatKey(0)
,   bTimeOK(false)
,   bFormatOK(false)
,   bFixed(false)
,   bIsDate(false)
,   bIsDefaultLanguage( true )
{
    bValid = true;  // always valid!
}

void XMLTimeFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_TIME_VALUE):
        case XML_ELEMENT(OFFICE, XML_TIME_VALUE):
        {
            if (::sax::Converter::parseTimeOrDateTime(aDateTimeValue, sAttrValue))
            {
                bTimeOK = true;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_FIXED):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bFixed = bTmp;
            }
            break;
        }
        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
        {
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(
                                               OUString::fromUtf8(sAttrValue), &bIsDefaultLanguage);
            if (-1 != nKey)
            {
                nFormatKey = nKey;
                bFormatOK = true;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_TIME_ADJUST):
        {
            double fTmp;

            if (::sax::Converter::convertDuration(fTmp, sAttrValue))
            {
                // convert to minutes
                nAdjust = static_cast<sal_Int32>(::rtl::math::approxFloor(fTmp * 60 * 24));
            }
            break;
        }
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLTimeFieldImportContext::PrepareField(
    const Reference<XPropertySet> & rPropertySet)
{
    // all properties are optional (except IsDate)
    Reference<XPropertySetInfo> xPropertySetInfo(
        rPropertySet->getPropertySetInfo());

    if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
    {
        rPropertySet->setPropertyValue(sPropertyFixed, Any(bFixed));
    }

    rPropertySet->setPropertyValue(sPropertyIsDate, Any(bIsDate));

    if (xPropertySetInfo->hasPropertyByName(gsPropertyAdjust))
    {
        rPropertySet->setPropertyValue(gsPropertyAdjust, Any(nAdjust));
    }

    // set value
    if (bFixed)
    {
        // organizer or styles-only mode: force update
        if (GetImport().GetTextImport()->IsOrganizerMode() ||
            GetImport().GetTextImport()->IsStylesOnlyMode()   )
        {
            ForceUpdate(rPropertySet);
        }
        else
        {
            // normal mode: set value (if present)
            if (bTimeOK)
            {
               if (xPropertySetInfo->hasPropertyByName(sPropertyDateTimeValue))
               {
                   rPropertySet->setPropertyValue(sPropertyDateTimeValue, Any(aDateTimeValue));
               }
               else if (xPropertySetInfo->hasPropertyByName(sPropertyDateTime))
               {
                   rPropertySet->setPropertyValue(sPropertyDateTime, Any(aDateTimeValue));
               }
            }
        }
    }

    if (bFormatOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyNumberFormat))
    {
        rPropertySet->setPropertyValue(sPropertyNumberFormat, Any(nFormatKey));

        if( xPropertySetInfo->hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            bool bIsFixedLanguage = ! bIsDefaultLanguage;
            rPropertySet->setPropertyValue( sPropertyIsFixedLanguage, Any(bIsFixedLanguage) );
        }
    }
}


// date field


XMLDateFieldImportContext::XMLDateFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTimeFieldImportContext(rImport, rHlp)
{
    bIsDate = true; // always a date!
}

void XMLDateFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_DATE_VALUE):
        case XML_ELEMENT(OFFICE, XML_DATE_VALUE):
        {
            if (::sax::Converter::parseDateTime(aDateTimeValue, sAttrValue))
            {
                bTimeOK = true;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_DATE_ADJUST):
            // delegate to superclass, pretending it was a time-adjust attr.
            XMLTimeFieldImportContext::ProcessAttribute(
                XML_ELEMENT(TEXT, XML_TIME_ADJUST),
                sAttrValue);
            break;
        case XML_ELEMENT(TEXT, XML_TIME_VALUE):
        case XML_ELEMENT(OFFICE, XML_TIME_VALUE):
        case XML_ELEMENT(TEXT, XML_TIME_ADJUST):
            ; // ignore time-adjust and time-value attributes
            break;
        default:
            // all others: delegate to super-class
            return XMLTimeFieldImportContext::ProcessAttribute(nAttrToken,
                                                        sAttrValue);
            break;
    }
}


// database field superclass


constexpr OUStringLiteral gsPropertyDataBaseName(u"DataBaseName");
constexpr OUStringLiteral gsPropertyDataBaseURL(u"DataBaseURL");
constexpr OUStringLiteral gsPropertyTableName(u"DataTableName");
constexpr OUStringLiteral gsPropertyDataCommandType(u"DataCommandType");
constexpr OUStringLiteral gsPropertyIsVisible(u"IsVisible");

XMLDatabaseFieldImportContext::XMLDatabaseFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const OUString& pServiceName, bool bUseDisplay)
:   XMLTextFieldImportContext(rImport, rHlp, pServiceName)
,   m_nCommandType( sdb::CommandType::TABLE )
,   m_bCommandTypeOK(false)
,   m_bDisplay( true )
,   m_bDisplayOK( false )
,   m_bUseDisplay( bUseDisplay )
,   m_bDatabaseOK(false)
,   m_bDatabaseNameOK(false)
,   m_bDatabaseURLOK(false)
,   m_bTableOK(false)
{
}

void XMLDatabaseFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_DATABASE_NAME):
            m_sDatabaseName = OUString::fromUtf8(sAttrValue);
            m_bDatabaseOK = true;
            m_bDatabaseNameOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_TABLE_NAME):
            m_sTableName = OUString::fromUtf8(sAttrValue);
            m_bTableOK = true;
            break;
        case  XML_ELEMENT(TEXT, XML_TABLE_TYPE):
            if( IsXMLToken( sAttrValue, XML_TABLE ) )
            {
                m_nCommandType = sdb::CommandType::TABLE;
                m_bCommandTypeOK = true;
            }
            else if( IsXMLToken( sAttrValue, XML_QUERY ) )
            {
                m_nCommandType = sdb::CommandType::QUERY;
                m_bCommandTypeOK = true;
            }
            else if( IsXMLToken( sAttrValue, XML_COMMAND ) )
            {
                m_nCommandType = sdb::CommandType::COMMAND;
                m_bCommandTypeOK = true;
            }
            break;
        case XML_ELEMENT(TEXT, XML_DISPLAY):
            if( IsXMLToken( sAttrValue, XML_NONE ) )
            {
                m_bDisplay = false;
                m_bDisplayOK = true;
            }
            else if( IsXMLToken( sAttrValue, XML_VALUE ) )
            {
                m_bDisplay = true;
                m_bDisplayOK = true;
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLDatabaseFieldImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if (nElement == XML_ELEMENT(FORM, XML_CONNECTION_RESOURCE) )
    {
        for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(XLINK, XML_HREF):
                {
                    m_sDatabaseURL = aIter.toString();
                    m_bDatabaseOK = true;
                    m_bDatabaseURLOK = true;
                }
                break;
                default:;
            }
        }

        // we call ProcessAttribute in order to set bValid appropriately
        ProcessAttribute( XML_TOKEN_INVALID, "" );
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}


void XMLDatabaseFieldImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(gsPropertyTableName, Any(m_sTableName));

    if( m_bDatabaseNameOK )
    {
        xPropertySet->setPropertyValue(gsPropertyDataBaseName, Any(m_sDatabaseName));
    }
    else if( m_bDatabaseURLOK )
    {
        xPropertySet->setPropertyValue(gsPropertyDataBaseURL, Any(m_sDatabaseURL));
    }

    // #99980# load/save command type for all fields; also load
    //         old documents without command type
    if( m_bCommandTypeOK )
    {
        xPropertySet->setPropertyValue( gsPropertyDataCommandType, Any(m_nCommandType) );
    }

    if( m_bUseDisplay && m_bDisplayOK )
    {
        xPropertySet->setPropertyValue( gsPropertyIsVisible, Any(m_bDisplay) );
    }
}


// database name field


XMLDatabaseNameImportContext::XMLDatabaseNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLDatabaseFieldImportContext(rImport, rHlp, u"DatabaseName"_ustr, true)
{
}

void XMLDatabaseNameImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    // delegate to superclass and check for success
    XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken, sAttrValue);
    bValid = m_bDatabaseOK && m_bTableOK;
}


// database next field


XMLDatabaseNextImportContext::XMLDatabaseNextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    const OUString& pServiceName) :
        XMLDatabaseFieldImportContext(rImport, rHlp, pServiceName, false),
        sPropertyCondition(sAPI_condition),
        sTrue(sAPI_true),
        bConditionOK(false)
{
}

XMLDatabaseNextImportContext::XMLDatabaseNextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp)
: XMLDatabaseFieldImportContext(rImport, rHlp, u"DatabaseNextSet"_ustr, false)
,   sPropertyCondition(sAPI_condition)
,   sTrue(sAPI_true)
,   bConditionOK(false)
{
}

void XMLDatabaseNextImportContext::ProcessAttribute(
    sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    if (XML_ELEMENT(TEXT, XML_CONDITION) == nAttrToken)
    {
        OUString sTmp;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrValueQName(
                                    OUString::fromUtf8(sAttrValue), &sTmp );
        if( XML_NAMESPACE_OOOW == nPrefix )
        {
            sCondition = sTmp;
            bConditionOK = true;
        }
        else
            sCondition = OUString::fromUtf8(sAttrValue);
    }
    else
    {
        XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken,
                                                        sAttrValue);
    }

    bValid = m_bDatabaseOK && m_bTableOK;
}

void XMLDatabaseNextImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;

    aAny <<= bConditionOK ? sCondition : sTrue;
    xPropertySet->setPropertyValue(sPropertyCondition, aAny);

    XMLDatabaseFieldImportContext::PrepareField(xPropertySet);
}


// database select field


XMLDatabaseSelectImportContext::XMLDatabaseSelectImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLDatabaseNextImportContext(rImport, rHlp, u"DatabaseNumberOfSet"_ustr),
        sPropertySetNumber(sAPI_set_number),
        nNumber(0),
        bNumberOK(false)
{
}

void XMLDatabaseSelectImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    if (XML_ELEMENT(TEXT, XML_ROW_NUMBER) == nAttrToken)
    {
        sal_Int32 nTmp;
        if (::sax::Converter::convertNumber( nTmp, sAttrValue
                                               /* , nMin, nMax ??? */ ))
        {
            nNumber = nTmp;
            bNumberOK = true;
        }
    }
    else
    {
        XMLDatabaseNextImportContext::ProcessAttribute(nAttrToken, sAttrValue);
    }

    bValid = m_bTableOK && m_bDatabaseOK && bNumberOK;
}

void XMLDatabaseSelectImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertySetNumber, Any(nNumber));

    XMLDatabaseNextImportContext::PrepareField(xPropertySet);
}


// database display row number field


XMLDatabaseNumberImportContext::XMLDatabaseNumberImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLDatabaseFieldImportContext(rImport, rHlp, u"DatabaseSetNumber"_ustr, true),
        sPropertyNumberingType(
            sAPI_numbering_type),
        sPropertySetNumber(sAPI_set_number),
        sNumberFormat(u"1"_ustr),
        sNumberSync(GetXMLToken(XML_FALSE)),
        nValue(0),
        bValueOK(false)
{
}

void XMLDatabaseNumberImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            sNumberFormat = OUString::fromUtf8(sAttrValue);
            break;
        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            sNumberSync = OUString::fromUtf8(sAttrValue);
            break;
        case XML_ELEMENT(TEXT, XML_VALUE_TYPE):
        case XML_ELEMENT(OFFICE, XML_VALUE_TYPE):
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber( nTmp, sAttrValue ))
            {
                nValue = nTmp;
                bValueOK = true;
            }
            break;
        }
        default:
            XMLDatabaseFieldImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
    }

    bValid = m_bTableOK && m_bDatabaseOK;
}

void XMLDatabaseNumberImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    sal_Int16 nNumType = style::NumberingType::ARABIC;
    GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sNumberSync );
    xPropertySet->setPropertyValue(sPropertyNumberingType, Any(nNumType));

    if (bValueOK)
    {
        xPropertySet->setPropertyValue(sPropertySetNumber, Any(nValue));
    }

    XMLDatabaseFieldImportContext::PrepareField(xPropertySet);
}


// Simple doc info fields


XMLSimpleDocInfoImportContext::XMLSimpleDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_Int32 nElementToken,
    bool bContent, bool bAuthor)
:   XMLTextFieldImportContext(rImport, rHlp, MapTokenToServiceName(nElementToken) )
,   sPropertyFixed(sAPI_is_fixed)
,   sPropertyContent(sAPI_content)
,   sPropertyAuthor(sAPI_author)
,   sPropertyCurrentPresentation(sAPI_current_presentation)
,   bFixed(false)
,   bHasAuthor(bAuthor)
,   bHasContent(bContent)
{
    bValid = true;
}

void XMLSimpleDocInfoImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    if (XML_ELEMENT(TEXT, XML_FIXED) == nAttrToken)
    {
        bool bTmp(false);
        if (::sax::Converter::convertBool(bTmp, sAttrValue))
        {
            bFixed = bTmp;
        }
    }
    else
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

void XMLSimpleDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & rPropertySet)
{
    //  title field in Calc has no Fixed property
    Reference<XPropertySetInfo> xPropertySetInfo(rPropertySet->getPropertySetInfo());
    if (!xPropertySetInfo->hasPropertyByName(sPropertyFixed))
        return;

    Any aAny;
    rPropertySet->setPropertyValue(sPropertyFixed, Any(bFixed));

    // set Content and CurrentPresentation (if fixed)
    if (!bFixed)
        return;

    // in organizer-mode or styles-only-mode, only force update
    if (GetImport().GetTextImport()->IsOrganizerMode() ||
        GetImport().GetTextImport()->IsStylesOnlyMode()   )
    {
        ForceUpdate(rPropertySet);
    }
    else
    {
        // set content (author, if that's the name) and current
        // presentation
        aAny <<= GetContent();

        if (bFixed && bHasAuthor)
        {
            rPropertySet->setPropertyValue(sPropertyAuthor, aAny);
        }

        if (bFixed && bHasContent)
        {
            rPropertySet->setPropertyValue(sPropertyContent, aAny);
        }

        rPropertySet->setPropertyValue(sPropertyCurrentPresentation, aAny);
    }
}

OUString XMLSimpleDocInfoImportContext::MapTokenToServiceName(
    sal_Int32 nElementToken)
{
    OUString pServiceName;

    switch(nElementToken)
    {
        case XML_ELEMENT(TEXT, XML_INITIAL_CREATOR):
            pServiceName = "DocInfo.CreateAuthor";
            break;
        case XML_ELEMENT(TEXT, XML_CREATION_DATE):
            pServiceName = sAPI_docinfo_create_date_time;
            break;
        case XML_ELEMENT(TEXT, XML_CREATION_TIME):
            pServiceName = sAPI_docinfo_create_date_time;
            break;
        case XML_ELEMENT(TEXT, XML_DESCRIPTION):
            pServiceName = "DocInfo.Description";
            break;
        case XML_ELEMENT(TEXT, XML_EDITING_DURATION):
            pServiceName = "DocInfo.EditTime";
            break;
        case XML_ELEMENT(TEXT, XML_USER_DEFINED):
            pServiceName = sAPI_docinfo_custom;
            break;
        case XML_ELEMENT(TEXT, XML_PRINTED_BY):
            pServiceName = "DocInfo.PrintAuthor";
            break;
        case XML_ELEMENT(TEXT, XML_PRINT_DATE):
            pServiceName = sAPI_docinfo_print_date_time;
            break;
        case XML_ELEMENT(TEXT, XML_PRINT_TIME):
            pServiceName = sAPI_docinfo_print_date_time;
            break;
        case XML_ELEMENT(TEXT, XML_KEYWORDS):
            pServiceName = "DocInfo.KeyWords";
            break;
        case XML_ELEMENT(TEXT, XML_SUBJECT):
            pServiceName = "DocInfo.Subject";
            break;
        case XML_ELEMENT(TEXT, XML_EDITING_CYCLES):
            pServiceName = "DocInfo.Revision";
            break;
        case XML_ELEMENT(TEXT, XML_CREATOR):
            pServiceName = "DocInfo.ChangeAuthor";
            break;
        case XML_ELEMENT(TEXT, XML_MODIFICATION_DATE):
            pServiceName = sAPI_docinfo_change_date_time;
            break;
        case XML_ELEMENT(TEXT, XML_MODIFICATION_TIME):
            pServiceName = sAPI_docinfo_change_date_time;
            break;
        case XML_ELEMENT(TEXT, XML_TITLE):
            pServiceName = "DocInfo.Title";
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElementToken);
            assert(false);
    }

    return pServiceName;
}


// revision field

constexpr OUStringLiteral sPropertyRevision(u"Revision");

XMLRevisionDocInfoImportContext::XMLRevisionDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_Int32 nElement) :
        XMLSimpleDocInfoImportContext(rImport, rHlp, nElement, false, false)
{
    bValid = true;
}

void XMLRevisionDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & rPropertySet)
{
    XMLSimpleDocInfoImportContext::PrepareField(rPropertySet);

    // set revision number
    // if fixed, if not in organizer-mode, if not in styles-only-mode
    if (!bFixed)
        return;

    if ( GetImport().GetTextImport()->IsOrganizerMode() ||
         GetImport().GetTextImport()->IsStylesOnlyMode()   )
    {
        ForceUpdate(rPropertySet);
    }
    else
    {
        sal_Int32 nTmp;
        if (::sax::Converter::convertNumber(nTmp, GetContent()))
        {
            rPropertySet->setPropertyValue(sPropertyRevision, Any(nTmp));
        }
    }
}


// DocInfo fields with date/time attributes


XMLDateTimeDocInfoImportContext::XMLDateTimeDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp, sal_Int32 nElement)
    : XMLSimpleDocInfoImportContext(rImport, rHlp, nElement, false, false)
    , sPropertyNumberFormat(sAPI_number_format)
    , sPropertyIsDate(sAPI_is_date)
    , sPropertyIsFixedLanguage(sAPI_is_fixed_language)
    , nFormat(0)
    , bFormatOK(false)
    , bIsDate(false)
    , bHasDateTime(false)
    , bIsDefaultLanguage(true)
{
    // we allow processing of EDIT_DURATION here, because import of actual
    // is not supported anyway. If it was, we'd need an extra import class
    // because times and time durations are presented differently!

    bValid = true;
    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_CREATION_DATE):
        case XML_ELEMENT(TEXT, XML_PRINT_DATE):
        case XML_ELEMENT(TEXT, XML_MODIFICATION_DATE):
            bIsDate = true;
            bHasDateTime = true;
            break;
        case XML_ELEMENT(TEXT, XML_CREATION_TIME):
        case XML_ELEMENT(TEXT, XML_PRINT_TIME):
        case XML_ELEMENT(TEXT, XML_MODIFICATION_TIME):
            bIsDate = false;
            bHasDateTime = true;
            break;
        case XML_ELEMENT(TEXT, XML_EDITING_DURATION):
            bIsDate = false;
            bHasDateTime = false;
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
            OSL_FAIL("XMLDateTimeDocInfoImportContext needs date/time doc. fields");
            bValid = false;
            break;
    }
}

void XMLDateTimeDocInfoImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
        {
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(
                                               OUString::fromUtf8(sAttrValue), &bIsDefaultLanguage);
            if (-1 != nKey)
            {
                nFormat = nKey;
                bFormatOK = true;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_FIXED):
            XMLSimpleDocInfoImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
        default:
            // ignore -> we can't set date/time value anyway!
            break;
    }
}

void XMLDateTimeDocInfoImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // process fixed and presentation
    XMLSimpleDocInfoImportContext::PrepareField(xPropertySet);

    if (bHasDateTime)
    {
        xPropertySet->setPropertyValue(sPropertyIsDate, Any(bIsDate));
    }

    if (bFormatOK)
    {
        xPropertySet->setPropertyValue(sPropertyNumberFormat, Any(nFormat));

        if( xPropertySet->getPropertySetInfo()->
                hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            bool bIsFixedLanguage = ! bIsDefaultLanguage;
            xPropertySet->setPropertyValue( sPropertyIsFixedLanguage, Any(bIsFixedLanguage) );
        }
    }

    // can't set date/time/duration value! Sorry.
}


// user defined docinfo fields


XMLUserDocInfoImportContext::XMLUserDocInfoImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_Int32 nElement) :
        XMLSimpleDocInfoImportContext(rImport, rHlp, nElement, false, false)
    , sPropertyName(sAPI_name)
    , sPropertyNumberFormat(sAPI_number_format)
    , sPropertyIsFixedLanguage(sAPI_is_fixed_language)
    , nFormat(0)
    , bFormatOK(false)
    , bIsDefaultLanguage( true )
{
    bValid = false;
}

void XMLUserDocInfoImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(STYLE, XML_DATA_STYLE_NAME):
        {
            sal_Int32 nKey = GetImportHelper().GetDataStyleKey(
                                               OUString::fromUtf8(sAttrValue), &bIsDefaultLanguage);
            if (-1 != nKey)
            {
                nFormat = nKey;
                bFormatOK = true;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_NAME):
        {
            if (!bValid)
            {
                SetServiceName(sAPI_docinfo_custom );
                aName = OUString::fromUtf8(sAttrValue);
                bValid = true;
            }
            break;
        }

        default:
            XMLSimpleDocInfoImportContext::ProcessAttribute(nAttrToken,
                                                            sAttrValue);
            break;
    }
}

void XMLUserDocInfoImportContext::PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> & xPropertySet)
{
    if ( !aName.isEmpty() )
    {
        xPropertySet->setPropertyValue(sPropertyName, Any(aName));
    }
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());
    if (bFormatOK &&
        xPropertySetInfo->hasPropertyByName(sPropertyNumberFormat))
    {
        xPropertySet->setPropertyValue(sPropertyNumberFormat, Any(nFormat));

        if( xPropertySetInfo->hasPropertyByName( sPropertyIsFixedLanguage ) )
        {
            bool bIsFixedLanguage = ! bIsDefaultLanguage;
            xPropertySet->setPropertyValue( sPropertyIsFixedLanguage, Any(bIsFixedLanguage) );
        }
    }

    // call superclass to handle "fixed"
    XMLSimpleDocInfoImportContext::PrepareField(xPropertySet);
}


// import hidden paragraph fields


XMLHiddenParagraphImportContext::XMLHiddenParagraphImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"HiddenParagraph"_ustr),
        sPropertyCondition(sAPI_condition),
        sPropertyIsHidden(sAPI_is_hidden),
        bIsHidden(false)
{
}

void XMLHiddenParagraphImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    if ( XML_ELEMENT(TEXT, XML_CONDITION) == nAttrToken)
    {
        OUString sTmp;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrValueQName(
                                    OUString::fromUtf8(sAttrValue), &sTmp );
        if( XML_NAMESPACE_OOOW == nPrefix )
        {
            sCondition = sTmp;
            bValid = true;
        }
        else
            sCondition = OUString::fromUtf8(sAttrValue);
    }
    else if ( XML_ELEMENT(TEXT, XML_IS_HIDDEN) == nAttrToken)
    {
        bool bTmp(false);
        if (::sax::Converter::convertBool(bTmp, sAttrValue))
        {
            bIsHidden = bTmp;
        }
    }
    else
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

void XMLHiddenParagraphImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertyCondition, Any(sCondition));
    xPropertySet->setPropertyValue(sPropertyIsHidden, Any(bIsHidden));
}


// import conditional text (<text:conditional-text>)

constexpr OUStringLiteral gsPropertyTrueContent(u"TrueContent");
constexpr OUStringLiteral gsPropertyFalseContent(u"FalseContent");
constexpr OUStringLiteral gsPropertyIsConditionTrue(u"IsConditionTrue");

XMLConditionalTextImportContext::XMLConditionalTextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"ConditionalText"_ustr),
        sPropertyCondition(sAPI_condition),
        sPropertyCurrentPresentation(sAPI_current_presentation),
        bConditionOK(false),
        bTrueOK(false),
        bFalseOK(false),
        bCurrentValue(false)
{
}

void XMLConditionalTextImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_CONDITION):
            {
                OUString sTmp;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                        GetKeyByAttrValueQName(OUString::fromUtf8(sAttrValue), &sTmp);
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sCondition = sTmp;
                    bConditionOK = true;
                }
                else
                    sCondition = OUString::fromUtf8(sAttrValue);
            }
            break;
        case XML_ELEMENT(TEXT, XML_STRING_VALUE_IF_FALSE):
            sFalseContent = OUString::fromUtf8(sAttrValue);
            bFalseOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_STRING_VALUE_IF_TRUE):
            sTrueContent = OUString::fromUtf8(sAttrValue);
            bTrueOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_CURRENT_VALUE):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bCurrentValue = bTmp;
            }
            break;
        }
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }

    bValid = bConditionOK && bFalseOK && bTrueOK;
}

void XMLConditionalTextImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertyCondition, Any(sCondition));
    xPropertySet->setPropertyValue(gsPropertyFalseContent, Any(sFalseContent));
    xPropertySet->setPropertyValue(gsPropertyTrueContent, Any(sTrueContent));
    xPropertySet->setPropertyValue(gsPropertyIsConditionTrue, Any(bCurrentValue));
    xPropertySet->setPropertyValue(sPropertyCurrentPresentation, Any(GetContent()));
}


// hidden text


XMLHiddenTextImportContext::XMLHiddenTextImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"HiddenText"_ustr),
        sPropertyCondition(sAPI_condition),
        sPropertyContent(sAPI_content),
        sPropertyIsHidden(sAPI_is_hidden),
        bConditionOK(false),
        bStringOK(false),
        bIsHidden(false)
{
}

void XMLHiddenTextImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_CONDITION):
            {
                OUString sTmp;
                sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                            GetKeyByAttrValueQName(OUString::fromUtf8(sAttrValue), &sTmp);
                if( XML_NAMESPACE_OOOW == nPrefix )
                {
                    sCondition = sTmp;
                    bConditionOK = true;
                }
                else
                    sCondition = OUString::fromUtf8(sAttrValue);
            }
            break;
        case XML_ELEMENT(TEXT, XML_STRING_VALUE):
        case XML_ELEMENT(OFFICE, XML_STRING_VALUE):
            sString = OUString::fromUtf8(sAttrValue);
            bStringOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_IS_HIDDEN):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bIsHidden = bTmp;
            }
            break;
        }
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }

    bValid = bConditionOK && bStringOK;
}

void XMLHiddenTextImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertyCondition, Any(sCondition));
    xPropertySet->setPropertyValue(sPropertyContent, Any(sString));
    xPropertySet->setPropertyValue(sPropertyIsHidden, Any(bIsHidden));
}


// file name fields


const SvXMLEnumMapEntry<sal_uInt16> aFilenameDisplayMap[] =
{
    { XML_PATH,                 FilenameDisplayFormat::PATH },
    { XML_NAME,                 FilenameDisplayFormat::NAME },
    { XML_NAME_AND_EXTENSION,   FilenameDisplayFormat::NAME_AND_EXT },
    { XML_FULL,                 FilenameDisplayFormat::FULL },
    { XML_TOKEN_INVALID, 0 }
};

XMLFileNameImportContext::XMLFileNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"FileName"_ustr),
        sPropertyFixed(sAPI_is_fixed),
        sPropertyFileFormat(sAPI_file_format),
        sPropertyCurrentPresentation(
            sAPI_current_presentation),
        nFormat(FilenameDisplayFormat::FULL),
        bFixed(false)
{
    bValid = true;
}

void XMLFileNameImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_FIXED):
        {
            bool bTmp(false);
            if (::sax::Converter::convertBool(bTmp, sAttrValue))
            {
                bFixed = bTmp;
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_DISPLAY):
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                aFilenameDisplayMap))
            {
                nFormat = nTmp;
            }
            break;
        }
        default:
            // unknown attribute: ignore
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
            break;
    }
}

void XMLFileNameImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // properties are optional
    Reference<XPropertySetInfo> xPropertySetInfo(
        xPropertySet->getPropertySetInfo());

    if (xPropertySetInfo->hasPropertyByName(sPropertyFixed))
    {
         xPropertySet->setPropertyValue(sPropertyFixed, Any(bFixed));
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertyFileFormat))
    {
        xPropertySet->setPropertyValue(sPropertyFileFormat, Any(nFormat));
    }

    if (xPropertySetInfo->hasPropertyByName(sPropertyCurrentPresentation))
    {
        xPropertySet->setPropertyValue(sPropertyCurrentPresentation, Any(GetContent()));
    }
}


// template name field


const SvXMLEnumMapEntry<sal_uInt16> aTemplateDisplayMap[] =
{
    { XML_FULL,                 TemplateDisplayFormat::FULL },
    { XML_PATH,                 TemplateDisplayFormat::PATH },
    { XML_NAME,                 TemplateDisplayFormat::NAME },
    { XML_NAME_AND_EXTENSION,   TemplateDisplayFormat::NAME_AND_EXT },
    { XML_AREA,                 TemplateDisplayFormat::AREA },
    { XML_TITLE,                TemplateDisplayFormat::TITLE },
    { XML_TOKEN_INVALID, 0 }
};


XMLTemplateNameImportContext::XMLTemplateNameImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"TemplateName"_ustr),
        sPropertyFileFormat(sAPI_file_format),
        nFormat(TemplateDisplayFormat::FULL)
{
    bValid = true;
}

void XMLTemplateNameImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_DISPLAY):
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                aTemplateDisplayMap))
            {
                nFormat = nTmp;
            }
            break;
        }
        default:
            // unknown attribute: ignore
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
            break;
    }
}

void XMLTemplateNameImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sPropertyFileFormat, Any(nFormat));
}


// import chapter fields


const SvXMLEnumMapEntry<sal_uInt16> aChapterDisplayMap[] =
{
    { XML_NAME,                     ChapterFormat::NAME },
    { XML_NUMBER,                   ChapterFormat::NUMBER },
    { XML_NUMBER_AND_NAME,          ChapterFormat::NAME_NUMBER },
    { XML_PLAIN_NUMBER_AND_NAME,    ChapterFormat::NO_PREFIX_SUFFIX },
    { XML_PLAIN_NUMBER,             ChapterFormat::DIGIT },
    { XML_TOKEN_INVALID, 0 }
};

constexpr OUStringLiteral gsPropertyChapterFormat(u"ChapterFormat");
constexpr OUStringLiteral gsPropertyLevel(u"Level");

XMLChapterImportContext::XMLChapterImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"Chapter"_ustr),
        nFormat(ChapterFormat::NAME_NUMBER),
        nLevel(0)
{
    bValid = true;
}

void XMLChapterImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_DISPLAY):
        {
            sal_uInt16 nTmp;
            if (SvXMLUnitConverter::convertEnum(nTmp, sAttrValue,
                                                aChapterDisplayMap))
            {
                nFormat = static_cast<sal_Int16>(nTmp);
            }
            break;
        }
        case XML_ELEMENT(TEXT, XML_OUTLINE_LEVEL):
        {
            sal_Int32 nTmp;
            if (::sax::Converter::convertNumber(
                nTmp, sAttrValue, 1,
                GetImport().GetTextImport()->GetChapterNumbering()->getCount()
                ))
            {
                // API numbers 0..9, we number 1..10
                nLevel = static_cast<sal_Int8>(nTmp);
                nLevel--;
            }
            break;
        }
        default:
            // unknown attribute: ignore
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
            break;
    }
}

void XMLChapterImportContext::PrepareField(
        const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(gsPropertyChapterFormat, Any(nFormat));
    xPropertySet->setPropertyValue(gsPropertyLevel, Any(nLevel));
}


// counting fields


XMLCountFieldImportContext::XMLCountFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_Int32 nElement) :
        XMLTextFieldImportContext(rImport, rHlp, MapTokenToServiceName(nElement)),
        sPropertyNumberingType(
            sAPI_numbering_type),
        bNumberFormatOK(false)
{
    bValid = true;
}

void XMLCountFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            sNumberFormat = OUString::fromUtf8(sAttrValue);
            bNumberFormatOK = true;
            break;
        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            sLetterSync = OUString::fromUtf8(sAttrValue);
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLCountFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // properties optional
    // (only page count, but do for all to save common implementation)

    if (!xPropertySet->getPropertySetInfo()->
        hasPropertyByName(sPropertyNumberingType))
        return;

    sal_Int16 nNumType;
    if( bNumberFormatOK )
    {
        nNumType= style::NumberingType::ARABIC;
        GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                sNumberFormat,
                                                sLetterSync );
    }
    else
        nNumType = style::NumberingType::PAGE_DESCRIPTOR;
    xPropertySet->setPropertyValue(sPropertyNumberingType, Any(nNumType));
}

OUString XMLCountFieldImportContext::MapTokenToServiceName(
    sal_Int32 nElement)
{
    OUString pServiceName;

    switch (nElement)
    {
        case XML_ELEMENT(TEXT, XML_WORD_COUNT):
            pServiceName = "WordCount";
            break;
        case XML_ELEMENT(TEXT, XML_PARAGRAPH_COUNT):
            pServiceName = "ParagraphCount";
            break;
        case XML_ELEMENT(TEXT, XML_TABLE_COUNT):
            pServiceName = "TableCount";
            break;
        case XML_ELEMENT(TEXT, XML_CHARACTER_COUNT):
            pServiceName = "CharacterCount";
            break;
        case XML_ELEMENT(TEXT, XML_IMAGE_COUNT):
            pServiceName = "GraphicObjectCount";
            break;
        case XML_ELEMENT(TEXT, XML_OBJECT_COUNT):
            pServiceName = "EmbeddedObjectCount";
            break;
        case XML_ELEMENT(TEXT, XML_PAGE_COUNT):
            pServiceName = "PageCount";
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
            assert(false);
    }

    return pServiceName;
}


// page variable import


XMLPageVarGetFieldImportContext::XMLPageVarGetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"ReferencePageGet"_ustr),
        bNumberFormatOK(false)
{
    bValid = true;
}

void XMLPageVarGetFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(STYLE, XML_NUM_FORMAT):
            sNumberFormat = OUString::fromUtf8(sAttrValue);
            bNumberFormatOK = true;
            break;
        case XML_ELEMENT(STYLE, XML_NUM_LETTER_SYNC):
            sLetterSync = OUString::fromUtf8(sAttrValue);
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLPageVarGetFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    sal_Int16 nNumType;
    if( bNumberFormatOK )
    {
        nNumType= style::NumberingType::ARABIC;
        GetImport().GetMM100UnitConverter().convertNumFormat( nNumType,
                                                    sNumberFormat,
                                                    sLetterSync );
    }
    else
        nNumType = style::NumberingType::PAGE_DESCRIPTOR;
    xPropertySet->setPropertyValue(sAPI_numbering_type, Any(nNumType));

    // display old content (#96657#)
    xPropertySet->setPropertyValue( sAPI_current_presentation, Any(GetContent()) );
}


// page variable set fields


XMLPageVarSetFieldImportContext::XMLPageVarSetFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"ReferencePageSet"_ustr),
        nAdjust(0),
        bActive(true)
{
    bValid = true;
}

void XMLPageVarSetFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
    case XML_ELEMENT(TEXT, XML_ACTIVE):
    {
        bool bTmp(false);
        if (::sax::Converter::convertBool(bTmp, sAttrValue))
        {
            bActive = bTmp;
        }
        break;
    }
    case XML_ELEMENT(TEXT, XML_PAGE_ADJUST):
    {
        sal_Int32 nTmp(0);
        if (::sax::Converter::convertNumber(nTmp, sAttrValue))
        {
            nAdjust = static_cast<sal_Int16>(nTmp);
        }
        break;
    }
    default:
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
        break;
    }
}

void XMLPageVarSetFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(u"On"_ustr, Any(bActive));
    xPropertySet->setPropertyValue(sAPI_offset, Any(nAdjust));
}


// macro fields


XMLMacroFieldImportContext::XMLMacroFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"Macro"_ustr),
        bDescriptionOK(false)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLMacroFieldImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if ( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        // create events context and remember it!
        xEventContext = new XMLEventsImportContext( GetImport() );
        bValid = true;
        return xEventContext;
    }
    XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return nullptr;
}

void XMLMacroFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_DESCRIPTION):
            sDescription = OUString::fromUtf8(sAttrValue);
            bDescriptionOK = true;
            break;
        case XML_ELEMENT(TEXT, XML_NAME):
            sMacro = OUString::fromUtf8(sAttrValue);
            bValid = true;
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLMacroFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    Any aAny;
    aAny <<= (bDescriptionOK ? sDescription : GetContent());
    xPropertySet->setPropertyValue(sAPI_hint, aAny);

    // if we have an events child element, we'll look for the OnClick
    // event if not, it may be an old (pre-638i) document. Then, we'll
    // have to look at the name attribute.
    OUString sMacroName;
    OUString sLibraryName;
    OUString sScriptURL;

    if ( xEventContext.is() )
    {
        // get event sequence
        XMLEventsImportContext* pEvents = xEventContext.get();
        Sequence<PropertyValue> aValues;
        pEvents->GetEventSequence( u"OnClick"_ustr, aValues );

        for (const auto& rValue : aValues)
        {
            if ( rValue.Name == "ScriptType" )
            {
                // ignore ScriptType
            }
            else if ( rValue.Name == "Library" )
            {
                rValue.Value >>= sLibraryName;
            }
            else if ( rValue.Name == "MacroName" )
            {
                rValue.Value >>= sMacroName;
            }
            if ( rValue.Name == "Script" )
            {
                rValue.Value >>= sScriptURL;
            }
        }
    }
    else
    {
        // disassemble old-style macro-name: Everything before the
        // third-last dot is the library
        sal_Int32 nPos = sMacro.getLength() + 1;    // the loop starts with nPos--
        const sal_Unicode* pBuf = sMacro.getStr();
        for( sal_Int32 i = 0; (i < 3) && (nPos > 0); i++ )
        {
            nPos--;
            while ( (pBuf[nPos] != '.') && (nPos > 0) )
                nPos--;
        }

        if (nPos > 0)
        {
            sLibraryName = sMacro.copy(0, nPos);
            sMacroName = sMacro.copy(nPos+1);
        }
        else
            sMacroName = sMacro;
    }

    xPropertySet->setPropertyValue(u"ScriptURL"_ustr, Any(sScriptURL));
    xPropertySet->setPropertyValue(u"MacroName"_ustr, Any(sMacroName));
    xPropertySet->setPropertyValue(u"MacroLibrary"_ustr, Any(sLibraryName));
}


// reference field import


XMLReferenceFieldImportContext::XMLReferenceFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp,
    sal_Int32 nToken)
:   XMLTextFieldImportContext(rImport, rHlp, u"GetReference"_ustr)
,   nElementToken(nToken)
,   nSource(0)
,   nType(ReferenceFieldPart::PAGE_DESC)
,   nFlags(0)
,   bNameOK(false)
,   bTypeOK(false)
{
}

SvXMLEnumMapEntry<sal_uInt16> const lcl_aReferenceTypeTokenMap[] =
{
    { XML_PAGE,         ReferenceFieldPart::PAGE},
    { XML_CHAPTER,      ReferenceFieldPart::CHAPTER },
    { XML_TEXT,         ReferenceFieldPart::TEXT },
    { XML_DIRECTION,    ReferenceFieldPart::UP_DOWN },
    { XML_CATEGORY_AND_VALUE, ReferenceFieldPart::CATEGORY_AND_NUMBER },
    { XML_CAPTION,      ReferenceFieldPart::ONLY_CAPTION },
    { XML_VALUE,        ReferenceFieldPart::ONLY_SEQUENCE_NUMBER },
    // Core implementation for direct cross-references (#i81002#)
    { XML_NUMBER,               ReferenceFieldPart::NUMBER },
    { XML_NUMBER_NO_SUPERIOR,   ReferenceFieldPart::NUMBER_NO_CONTEXT },
    { XML_NUMBER_ALL_SUPERIOR,  ReferenceFieldPart::NUMBER_FULL_CONTEXT },
    { XML_TOKEN_INVALID, 0 }
};

void XMLReferenceFieldImportContext::startFastElement(
        sal_Int32 nElement,
        const Reference<XFastAttributeList> & xAttrList)
{
    bTypeOK = true;
    switch (nElementToken)
    {
        case XML_ELEMENT(TEXT, XML_REFERENCE_REF):
            nSource = ReferenceFieldSource::REFERENCE_MARK;
            break;
        case  XML_ELEMENT(TEXT, XML_BOOKMARK_REF):
            nSource = ReferenceFieldSource::BOOKMARK;
            break;
        case XML_ELEMENT(TEXT, XML_NOTE_REF):
            nSource = ReferenceFieldSource::FOOTNOTE;
            break;
        case XML_ELEMENT(TEXT, XML_SEQUENCE_REF):
            nSource = ReferenceFieldSource::SEQUENCE_FIELD;
            break;
        case XML_ELEMENT(TEXT, XML_STYLE_REF):
        case XML_ELEMENT(LO_EXT, XML_STYLE_REF):
            nSource = ReferenceFieldSource::STYLE;
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElementToken);
            bTypeOK = false;
            break;
    }

    XMLTextFieldImportContext::startFastElement(nElement, xAttrList);
}


void XMLReferenceFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_NOTE_CLASS):
            if( IsXMLToken( sAttrValue, XML_ENDNOTE ) )
                nSource = ReferenceFieldSource::ENDNOTE;
            break;
        case XML_ELEMENT(TEXT, XML_REF_NAME):
            sName = OUString::fromUtf8(sAttrValue);
            bNameOK = true;
            break;
        case  XML_ELEMENT(TEXT, XML_REFERENCE_FORMAT):
        {
            sal_uInt16 nToken;
            if (SvXMLUnitConverter::convertEnum(nToken, sAttrValue,
                                                lcl_aReferenceTypeTokenMap))
            {
                nType = nToken;
            }

            // check for sequence-only-attributes
            if ( (XML_ELEMENT(TEXT, XML_SEQUENCE_REF) != nElementToken) &&
                 ( (nType == ReferenceFieldPart::CATEGORY_AND_NUMBER) ||
                   (nType == ReferenceFieldPart::ONLY_CAPTION) ||
                   (nType == ReferenceFieldPart::ONLY_SEQUENCE_NUMBER) ) )
            {
                nType = ReferenceFieldPart::PAGE_DESC;
            }

            break;
        }
        case XML_ELEMENT(LO_EXT, XML_REFERENCE_LANGUAGE):
        case XML_ELEMENT(TEXT, XML_REFERENCE_LANGUAGE):
            sLanguage = OUString::fromUtf8(sAttrValue);
            break;
        case XML_ELEMENT(LO_EXT, XML_REFERENCE_HIDE_NON_NUMERICAL):
        case XML_ELEMENT(TEXT, XML_REFERENCE_HIDE_NON_NUMERICAL):
            if (OUString::fromUtf8(sAttrValue).toBoolean())
                nFlags |= REFFLDFLAG_STYLE_HIDE_NON_NUMERICAL;
            break;
        case XML_ELEMENT(LO_EXT, XML_REFERENCE_FROM_BOTTOM):
        case XML_ELEMENT(TEXT, XML_REFERENCE_FROM_BOTTOM):
            if (OUString::fromUtf8(sAttrValue).toBoolean())
                nFlags |= REFFLDFLAG_STYLE_FROM_BOTTOM;
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }

    // bValid: we need proper element type and name
    bValid = bTypeOK && bNameOK;
}

void XMLReferenceFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(u"ReferenceFieldPart"_ustr, Any(nType));

    xPropertySet->setPropertyValue(u"ReferenceFieldSource"_ustr, Any(nSource));

    xPropertySet->setPropertyValue(u"ReferenceFieldLanguage"_ustr, Any(sLanguage));
    switch (nElementToken)
    {
        case XML_ELEMENT(TEXT, XML_REFERENCE_REF):
        case XML_ELEMENT(TEXT, XML_BOOKMARK_REF):
        case XML_ELEMENT(TEXT, XML_STYLE_REF):
        case XML_ELEMENT(LO_EXT, XML_STYLE_REF):
            xPropertySet->setPropertyValue(u"SourceName"_ustr, Any(sName));
            xPropertySet->setPropertyValue(u"ReferenceFieldFlags"_ustr, Any(nFlags));
            break;

        case XML_ELEMENT(TEXT, XML_NOTE_REF):
            GetImportHelper().ProcessFootnoteReference(sName, xPropertySet);
            break;

        case XML_ELEMENT(TEXT, XML_SEQUENCE_REF):
            GetImportHelper().ProcessSequenceReference(sName, xPropertySet);
            break;
    }

    xPropertySet->setPropertyValue(sAPI_current_presentation, Any(GetContent()));
}


// field declarations container

XMLDdeFieldDeclsImportContext::XMLDdeFieldDeclsImportContext(SvXMLImport& rImport) :
        SvXMLImportContext(rImport)
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLDdeFieldDeclsImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if ( nElement == XML_ELEMENT(TEXT, XML_DDE_CONNECTION_DECL) )
    {
        return new XMLDdeFieldDeclImportContext(GetImport());
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}


// import dde field declaration


XMLDdeFieldDeclImportContext::XMLDdeFieldDeclImportContext(SvXMLImport& rImport)
:   SvXMLImportContext(rImport)
{
}

void XMLDdeFieldDeclImportContext::startFastElement(
        sal_Int32 /*nElement*/,
        const Reference<XFastAttributeList> & xAttrList)
{
    OUString sName;
    OUString sCommandApplication;
    OUString sCommandTopic;
    OUString sCommandItem;

    bool bUpdate = false;
    bool bNameOK = false;
    bool bCommandApplicationOK = false;
    bool bCommandTopicOK = false;
    bool bCommandItemOK = false;

    // process attributes
    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(OFFICE, XML_NAME):
                sName = aIter.toString();
                bNameOK = true;
                break;
            case XML_ELEMENT(OFFICE, XML_DDE_APPLICATION):
                sCommandApplication = aIter.toString();
                bCommandApplicationOK = true;
                break;
            case XML_ELEMENT(OFFICE, XML_DDE_TOPIC):
                sCommandTopic = aIter.toString();
                bCommandTopicOK = true;
                break;
            case XML_ELEMENT(OFFICE, XML_DDE_ITEM):
                sCommandItem = aIter.toString();
                bCommandItemOK = true;
                break;
            case XML_ELEMENT(OFFICE, XML_AUTOMATIC_UPDATE):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, aIter.toView()) )
                {
                    bUpdate = bTmp;
                }
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    // valid data?
    if (!(bNameOK && bCommandApplicationOK && bCommandTopicOK && bCommandItemOK))
        return;

    // create DDE TextFieldMaster
    Reference<XMultiServiceFactory> xFactory(GetImport().GetModel(),
                                             UNO_QUERY);
    if( !xFactory.is() )
        return;

    /* #i6432# There might be multiple occurrences of one DDE
       declaration if it is used in more than one of
       header/footer/body. createInstance will throw an exception if we
       try to create the second, third, etc. instance of such a
       declaration. Thus we ignore the exception. Otherwise this will
       lead to an unloadable document. */
    try
    {
        Reference<XInterface> xIfc =
            xFactory->createInstance(OUString::Concat(sAPI_fieldmaster_prefix) + sAPI_dde);
        if( xIfc.is() )
        {
            Reference<XPropertySet> xPropSet( xIfc, UNO_QUERY );
            if (xPropSet.is() &&
                xPropSet->getPropertySetInfo()->hasPropertyByName(
                                                                  u"DDECommandType"_ustr))
            {
                xPropSet->setPropertyValue(sAPI_name, Any(sName));

                xPropSet->setPropertyValue(u"DDECommandType"_ustr, Any(sCommandApplication));

                xPropSet->setPropertyValue(u"DDECommandFile"_ustr, Any(sCommandTopic));

                xPropSet->setPropertyValue(u"DDECommandElement"_ustr,
                                           Any(sCommandItem));

                xPropSet->setPropertyValue(u"IsAutomaticUpdate"_ustr,
                                           Any(bUpdate));
            }
            // else: ignore (can't get XPropertySet, or DDE
            //               properties are not supported)
        }
        // else: ignore
    }
    catch (const Exception&)
    {
        //ignore
    }
    // else: ignore
    // else: ignore
}


// DDE field import


XMLDdeFieldImportContext::XMLDdeFieldImportContext(
    SvXMLImport& rImport, XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_dde),
        sPropertyContent(sAPI_content)
{
}

void XMLDdeFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    if ( XML_ELEMENT(TEXT, XML_CONNECTION_NAME) == nAttrToken)
    {
        sName = OUString::fromUtf8(sAttrValue);
        bValid = true;
    }
    else
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}


void XMLDdeFieldImportContext::endFastElement(sal_Int32 )
{
    if (!bValid)
        return;

    // find master
    OUString sMasterName = OUString::Concat(sAPI_fieldmaster_prefix) + sAPI_dde + "." + sName;

    Reference<XTextFieldsSupplier> xTextFieldsSupp(GetImport().GetModel(),
                                                   UNO_QUERY);
    Reference<container::XNameAccess> xFieldMasterNameAccess =
        xTextFieldsSupp->getTextFieldMasters();

    if (!xFieldMasterNameAccess->hasByName(sMasterName))
        return;

    Reference<XPropertySet> xMaster;
    Any aAny = xFieldMasterNameAccess->getByName(sMasterName);
    aAny >>= xMaster;
    //apply the content to the master
    xMaster->setPropertyValue( sPropertyContent, uno::Any( GetContent()));
    // master exists: create text field and attach
    Reference<XPropertySet> xField;
    OUString sFieldName = OUString::Concat(sAPI_textfield_prefix) + sAPI_dde;
    if (!CreateField(xField, sFieldName))
        return;

    Reference<XDependentTextField> xDepTextField(xField,UNO_QUERY);
    xDepTextField->attachTextFieldMaster(xMaster);

    // attach field to document
    Reference<XTextContent> xTextContent(xField, UNO_QUERY);
    if (xTextContent.is())
    {
        GetImportHelper().InsertTextContent(xTextContent);

        // we're lucky. nothing else to prepare.
    }
    // else: fail, because text content could not be created
    // else: fail, because field could not be created
    // else: fail, because no master was found (faulty document?!)
    // not valid: ignore
}

void XMLDdeFieldImportContext::PrepareField(
    const Reference<XPropertySet> &)
{
    // empty, since not needed.
}


// sheet name fields


XMLSheetNameImportContext::XMLSheetNameImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"SheetName"_ustr)
{
    bValid = true;  // always valid!
}

void XMLSheetNameImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue)
{
    // no attributes -> nothing to be done
    XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

void XMLSheetNameImportContext::PrepareField(
    const Reference<XPropertySet> &)
{
    // no attributes -> nothing to be done
}

/** import page|slide name fields (<text:page-name>) */

XMLPageNameFieldImportContext::XMLPageNameFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp)              /// Text import helper
: XMLTextFieldImportContext(rImport, rHlp, u"PageName"_ustr )
{
    bValid = true;
}

/// process attribute values
void XMLPageNameFieldImportContext::ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue )
{
    XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

/// prepare XTextField for insertion into document
void XMLPageNameFieldImportContext::PrepareField(
        const css::uno::Reference<css::beans::XPropertySet> &)
{
}


// URL fields (Calc, Impress, Draw)


XMLUrlFieldImportContext::XMLUrlFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, sAPI_url),
        bFrameOK(false)
{
}

void XMLUrlFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(XLINK, XML_HREF):
            sURL = GetImport().GetAbsoluteReference( OUString::fromUtf8(sAttrValue) );
            bValid = true;
            break;
        case XML_ELEMENT(OFFICE, XML_TARGET_FRAME_NAME):
            sFrame = OUString::fromUtf8(sAttrValue);
            bFrameOK = true;
            break;
        default:
            // ignore
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
            break;
    }
}

void XMLUrlFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(sAPI_url, Any(sURL));

    if (bFrameOK)
    {
        xPropertySet->setPropertyValue(u"TargetFrame"_ustr, Any(sFrame));
    }

    xPropertySet->setPropertyValue(u"Representation"_ustr, Any(GetContent()));
}


XMLBibliographyFieldImportContext::XMLBibliographyFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"Bibliography"_ustr)
{
    bValid = true;
}

// TODO: this is the same map as is used in the text field export
SvXMLEnumMapEntry<sal_uInt16> const aBibliographyDataTypeMap[] =
{
    { XML_ARTICLE,          BibliographyDataType::ARTICLE },
    { XML_BOOK,             BibliographyDataType::BOOK },
    { XML_BOOKLET,          BibliographyDataType::BOOKLET },
    { XML_CONFERENCE,       BibliographyDataType::CONFERENCE },
    { XML_CUSTOM1,          BibliographyDataType::CUSTOM1 },
    { XML_CUSTOM2,          BibliographyDataType::CUSTOM2 },
    { XML_CUSTOM3,          BibliographyDataType::CUSTOM3 },
    { XML_CUSTOM4,          BibliographyDataType::CUSTOM4 },
    { XML_CUSTOM5,          BibliographyDataType::CUSTOM5 },
    { XML_EMAIL,            BibliographyDataType::EMAIL },
    { XML_INBOOK,           BibliographyDataType::INBOOK },
    { XML_INCOLLECTION,     BibliographyDataType::INCOLLECTION },
    { XML_INPROCEEDINGS,    BibliographyDataType::INPROCEEDINGS },
    { XML_JOURNAL,          BibliographyDataType::JOURNAL },
    { XML_MANUAL,           BibliographyDataType::MANUAL },
    { XML_MASTERSTHESIS,    BibliographyDataType::MASTERSTHESIS },
    { XML_MISC,             BibliographyDataType::MISC },
    { XML_PHDTHESIS,        BibliographyDataType::PHDTHESIS },
    { XML_PROCEEDINGS,      BibliographyDataType::PROCEEDINGS },
    { XML_TECHREPORT,       BibliographyDataType::TECHREPORT },
    { XML_UNPUBLISHED,      BibliographyDataType::UNPUBLISHED },
    { XML_WWW,              BibliographyDataType::WWW },
    { XML_TOKEN_INVALID, 0 }
};


// we'll process attributes on our own and for fit the standard
// textfield mechanism, because our attributes have zero overlap with
// all the other textfields.
void XMLBibliographyFieldImportContext::startFastElement(
        sal_Int32 /*nElement*/,
        const Reference<XFastAttributeList> & xAttrList)
{
    // iterate over attributes
    for( auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ) )
    {
        if (IsTokenInNamespace(aIter.getToken(), XML_NAMESPACE_TEXT)
            || IsTokenInNamespace(aIter.getToken(), XML_NAMESPACE_LO_EXT))
        {
            auto nToken = aIter.getToken() & TOKEN_MASK;
            PropertyValue aValue;
            aValue.Name = MapBibliographyFieldName(nToken);
            Any aAny;

            // special treatment for bibliography type
            // biblio vs bibilio: #96658#; also read old documents
            if (nToken == XML_BIBILIOGRAPHIC_TYPE ||
                nToken == XML_BIBLIOGRAPHY_TYPE    )
            {
                sal_uInt16 nTmp;
                if (SvXMLUnitConverter::convertEnum(
                    nTmp, aIter.toView(),
                    aBibliographyDataTypeMap))
                {
                    aAny <<= static_cast<sal_Int16>(nTmp);
                    aValue.Value = aAny;

                    aValues.push_back(aValue);
                }
            }
            else
            {
                OUString aStringValue = aIter.toString();
                if (nToken == XML_URL || nToken == XML_LOCAL_URL || nToken == XML_TARGET_URL)
                {
                    aStringValue = GetImport().GetAbsoluteReference(aStringValue);
                }
                aAny <<= aStringValue;
                aValue.Value = aAny;

                aValues.push_back(aValue);
            }
        }
        // else: unknown namespace -> ignore
    }
}

void XMLBibliographyFieldImportContext::ProcessAttribute(
    sal_Int32 ,
    std::string_view )
{
    // attributes are handled in StartElement
    assert(false && "This should not have happened.");
}


void XMLBibliographyFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // convert vector into sequence
    sal_Int32 nCount = aValues.size();
    Sequence<PropertyValue> aValueSequence(nCount);
    auto aValueSequenceRange = asNonConstRange(aValueSequence);
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        aValueSequenceRange[i] = aValues[i];
    }

    // set sequence
    xPropertySet->setPropertyValue(u"Fields"_ustr, Any(aValueSequence));
}

OUString XMLBibliographyFieldImportContext::MapBibliographyFieldName(
    sal_Int32 nElement)
{
    OUString pName;

    switch (nElement & TOKEN_MASK)
    {
        case XML_IDENTIFIER:
            pName = u"Identifier"_ustr;
            break;
        case XML_BIBILIOGRAPHIC_TYPE:
        case XML_BIBLIOGRAPHY_TYPE:
            // biblio... vs bibilio...: #96658#: also read old documents
            pName = u"BibiliographicType"_ustr;
            break;
        case XML_ADDRESS:
            pName = u"Address"_ustr;
            break;
        case XML_ANNOTE:
            pName = u"Annote"_ustr;
            break;
        case XML_AUTHOR:
            pName = u"Author"_ustr;
            break;
        case XML_BOOKTITLE:
            pName = u"Booktitle"_ustr;
            break;
        case XML_CHAPTER:
            pName = u"Chapter"_ustr;
            break;
        case XML_EDITION:
            pName = u"Edition"_ustr;
            break;
        case XML_EDITOR:
            pName = u"Editor"_ustr;
            break;
        case XML_HOWPUBLISHED:
            pName = u"Howpublished"_ustr;
            break;
        case XML_INSTITUTION:
            pName = u"Institution"_ustr;
            break;
        case XML_JOURNAL:
            pName = u"Journal"_ustr;
            break;
        case XML_MONTH:
            pName = u"Month"_ustr;
            break;
        case XML_NOTE:
            pName = u"Note"_ustr;
            break;
        case XML_NUMBER:
            pName = u"Number"_ustr;
            break;
        case XML_ORGANIZATIONS:
            pName = u"Organizations"_ustr;
            break;
        case XML_PAGES:
            pName = u"Pages"_ustr;
            break;
        case XML_PUBLISHER:
            pName = u"Publisher"_ustr;
            break;
        case XML_SCHOOL:
            pName = u"School"_ustr;
            break;
        case XML_SERIES:
            pName = u"Series"_ustr;
            break;
        case XML_TITLE:
            pName = u"Title"_ustr;
            break;
        case XML_REPORT_TYPE:
            pName = u"Report_Type"_ustr;
            break;
        case XML_VOLUME:
            pName = u"Volume"_ustr;
            break;
        case XML_YEAR:
            pName = u"Year"_ustr;
            break;
        case XML_URL:
            pName = u"URL"_ustr;
            break;
        case XML_CUSTOM1:
            pName = u"Custom1"_ustr;
            break;
        case XML_CUSTOM2:
            pName = u"Custom2"_ustr;
            break;
        case XML_CUSTOM3:
            pName = u"Custom3"_ustr;
            break;
        case XML_CUSTOM4:
            pName = u"Custom4"_ustr;
            break;
        case XML_CUSTOM5:
            pName = u"Custom5"_ustr;
            break;
        case XML_ISBN:
            pName = u"ISBN"_ustr;
            break;
        case XML_LOCAL_URL:
            pName = u"LocalURL"_ustr;
            break;
        case XML_TARGET_TYPE:
            pName = u"TargetType"_ustr;
            break;
        case XML_TARGET_URL:
            pName = u"TargetURL"_ustr;
            break;
        default:
            assert(false && "Unknown bibliography info data");
    }
    return pName;
}

// Annotation Field


XMLAnnotationImportContext::XMLAnnotationImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_Int32 nElement) :
        XMLTextFieldImportContext(rImport, rHlp, u"Annotation"_ustr),
        mnElement(nElement)
{
    bValid = true;

    // remember old list item and block (#91964#) and reset them
    // for the text frame
    // do this in the constructor, not in CreateChildContext (#i93392#)
    GetImport().GetTextImport()->PushListContext();
}

void XMLAnnotationImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    if (nAttrToken == XML_ELEMENT(OFFICE, XML_NAME))
        aName = OUString::fromUtf8(sAttrValue);
    else if (nAttrToken == XML_ELEMENT(LO_EXT, XML_RESOLVED))
        aResolved = OUString::fromUtf8(sAttrValue);
    else if (nAttrToken == XML_ELEMENT(LO_EXT, XML_PARENT_NAME))
        aParentName = OUString::fromUtf8(sAttrValue);
    else
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLAnnotationImportContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    if( nElement == XML_ELEMENT(DC, XML_CREATOR) )
        return new XMLStringBufferImportContext(GetImport(), aAuthorBuffer);
    else if( nElement == XML_ELEMENT(DC, XML_DATE) )
        return new XMLStringBufferImportContext(GetImport(), aDateBuffer);
    else if (nElement == XML_ELEMENT(TEXT,XML_SENDER_INITIALS) ||
             nElement == XML_ELEMENT(LO_EXT, XML_SENDER_INITIALS) ||
             nElement == XML_ELEMENT(META, XML_CREATOR_INITIALS))
        return new XMLStringBufferImportContext(GetImport(), aInitialsBuffer);

    try
    {
        bool bOK = true;
        if ( !mxField.is() )
            bOK = CreateField( mxField, sServicePrefix + GetServiceName() );
        if (bOK)
        {
            Any aAny = mxField->getPropertyValue( u"TextRange"_ustr );
            Reference< XText > xText;
            aAny >>= xText;
            if( xText.is() )
            {
                rtl::Reference < XMLTextImportHelper > xTxtImport = GetImport().GetTextImport();
                if( !mxCursor.is() )
                {
                    mxOldCursor = xTxtImport->GetCursor();
                    mxCursor = xText->createTextCursor();
                }

                if( mxCursor.is() )
                {
                    xTxtImport->SetCursor( mxCursor );
                    return xTxtImport->CreateTextChildContext( GetImport(), nElement, xAttrList );
                }
            }
        }
    }
    catch (const Exception&)
    {
    }

    return new XMLStringBufferImportContext(GetImport(), aTextBuffer);
}

void XMLAnnotationImportContext::endFastElement(sal_Int32 /*nElement*/)
{
    DBG_ASSERT(!GetServiceName().isEmpty(), "no service name for element!");
    if( mxCursor.is() )
    {
        // delete addition newline
        mxCursor->gotoEnd( false );
        mxCursor->goLeft( 1, true );
        mxCursor->setString( u""_ustr );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if( mxOldCursor.is() )
        GetImport().GetTextImport()->SetCursor( mxOldCursor );

    // reinstall old list item #91964#
    GetImport().GetTextImport()->PopListContext();

    if (!bValid)
    {
        GetImportHelper().InsertString(GetContent());
        return;
    }

    if ( mnElement == XML_ELEMENT(OFFICE, XML_ANNOTATION_END) )
    {
        // Search for a previous annotation with the same name.
        uno::Reference< text::XTextContent > xPrevField;
        {
            Reference<XTextFieldsSupplier> xTextFieldsSupplier(GetImport().GetModel(), UNO_QUERY);
            if (!xTextFieldsSupplier)
                return;
            uno::Reference<container::XUniqueIDAccess> xFieldsAccess(xTextFieldsSupplier->getTextFields(), UNO_QUERY_THROW);
            uno::Any aAny = xFieldsAccess->getByUniqueID(aName);
            aAny >>= xPrevField;
        }
        if ( xPrevField.is() )
        {
            // So we are ending a previous annotation,
            // let's create a text range covering the old and the current position.
            uno::Reference<text::XText> xText = GetImportHelper().GetText();
            uno::Reference<text::XTextCursor> xCursor =
                xText->createTextCursorByRange(GetImportHelper().GetCursorAsRange());
            try
            {
                xCursor->gotoRange(xPrevField->getAnchor(), true);
            }
            catch (const uno::RuntimeException&)
            {
                // Losing the start of the anchor is better than not opening the document at
                // all.
                TOOLS_WARN_EXCEPTION(
                    "xmloff.text",
                    "XMLAnnotationImportContext::endFastElement: gotoRange() failed: ");
            }

            xText->insertTextContent(xCursor, xPrevField, !xCursor->isCollapsed());
        }
        return;
    }

    if ( mxField.is() || CreateField( mxField, sServicePrefix + GetServiceName() ) )
    {
        // set field properties
        PrepareField( mxField );

        // attach field to document
        Reference < XTextContent > xTextContent( mxField, UNO_QUERY );

        // workaround for #80606#
        try
        {
            GetImportHelper().InsertTextContent( xTextContent );
        }
        catch (const lang::IllegalArgumentException&)
        {
            // ignore
        }
    }
}

void XMLAnnotationImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet )
{
    // import (possibly empty) author
    OUString sAuthor( aAuthorBuffer.makeStringAndClear() );
    xPropertySet->setPropertyValue(sAPI_author, Any(sAuthor));

    // import (possibly empty) initials
    OUString sInitials( aInitialsBuffer.makeStringAndClear() );
    xPropertySet->setPropertyValue(u"Initials"_ustr, Any(sInitials));

    //import resolved flag
    bool bTmp(false);
    (void)::sax::Converter::convertBool(bTmp, aResolved);
    xPropertySet->setPropertyValue(u"Resolved"_ustr, Any(bTmp));

    util::DateTime aDateTime;
    if (::sax::Converter::parseDateTime(aDateTime, aDateBuffer))
    {
        /*
        Date aDate;
        aDate.Year = aDateTime.Year;
        aDate.Month = aDateTime.Month;
        aDate.Day = aDateTime.Day;
        xPropertySet->setPropertyValue(sPropertyDate, makeAny(aDate));
        */
        // why is there no UNO_NAME_DATE_TIME, but only UNO_NAME_DATE_TIME_VALUE?
        xPropertySet->setPropertyValue(sAPI_date_time_value, Any(aDateTime));
    }
    aDateBuffer.setLength(0);

    if ( aTextBuffer.getLength() )
    {
        // delete last paragraph mark (if necessary)
        if (char(0x0a) == aTextBuffer[aTextBuffer.getLength()-1])
            aTextBuffer.setLength(aTextBuffer.getLength()-1);
        xPropertySet->setPropertyValue(sAPI_content, Any(aTextBuffer.makeStringAndClear()));
    }

    if (!aName.isEmpty())
        xPropertySet->setPropertyValue(sAPI_name, Any(aName));

    if (!aParentName.isEmpty())
        xPropertySet->setPropertyValue(sAPI_parent_name, Any(aParentName));
}


// script field


XMLScriptImportContext::XMLScriptImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp)
:   XMLTextFieldImportContext(rImport, rHlp, u"Script"_ustr)
,   bContentOK(false)
{
}

void XMLScriptImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(XLINK, XML_HREF):
            sContent = GetImport().GetAbsoluteReference( OUString::fromUtf8(sAttrValue) );
            bContentOK = true;
            break;

        case XML_ELEMENT(SCRIPT, XML_LANGUAGE):
            sScriptType = OUString::fromUtf8(sAttrValue);
            break;

        default:
            // ignore
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
            break;
    }

    // always valid (even without ScriptType; cf- #96531#)
    bValid = true;
}

void XMLScriptImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    // if href attribute was present, we use it. Else we use element content
    if (! bContentOK)
    {
        sContent = GetContent();
    }
    xPropertySet->setPropertyValue(sAPI_content, Any(sContent));

    // URL or script text? We use URL if we have an href-attribute
    xPropertySet->setPropertyValue(u"URLContent"_ustr, Any(bContentOK));

    xPropertySet->setPropertyValue(u"ScriptType"_ustr, Any(sScriptType));
}


// measure field


XMLMeasureFieldImportContext::XMLMeasureFieldImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp) :
        XMLTextFieldImportContext(rImport, rHlp, u"Measure"_ustr),
        mnKind( 0 )
{
}

void XMLMeasureFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    switch (nAttrToken)
    {
        case XML_ELEMENT(TEXT, XML_KIND):
            if( IsXMLToken( sAttrValue, XML_VALUE ) )
            {
                mnKind = 0; bValid = true;
            }
            else if( IsXMLToken( sAttrValue, XML_UNIT ) )
            {
                mnKind = 1; bValid = true;
            }
            else if( IsXMLToken( sAttrValue, XML_GAP ) )
            {
                mnKind = 2; bValid = true;
            }
            break;
        default:
            XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
    }
}

void XMLMeasureFieldImportContext::PrepareField(
    const Reference<XPropertySet> & xPropertySet)
{
    xPropertySet->setPropertyValue(u"Kind"_ustr, Any(mnKind));
}


// dropdown field


XMLDropDownFieldImportContext::XMLDropDownFieldImportContext(
        SvXMLImport& rImport,
        XMLTextImportHelper& rHlp) :
    XMLTextFieldImportContext( rImport, rHlp, u"DropDown"_ustr ),
    nSelected( -1 ),
    bNameOK( false ),
    bHelpOK(false),
    bHintOK(false)
{
    bValid = true;
}

static bool lcl_ProcessLabel(
                       const Reference<XFastAttributeList>& xAttrList,
                       OUString& rLabel,
                       bool& rIsSelected )
{
    bool bValid = false;
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(TEXT, XML_VALUE):
            {
                rLabel = aIter.toString();
                bValid = true;
                break;
            }
            case XML_ELEMENT(TEXT, XML_CURRENT_SELECTED):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool( bTmp, aIter.toView() ))
                    rIsSelected = bTmp;
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }
    return bValid;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLDropDownFieldImportContext::createFastChildContext(
    sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(TEXT, XML_LABEL) )
    {
        OUString sLabel;
        bool bIsSelected = false;
        if( lcl_ProcessLabel( xAttrList, sLabel, bIsSelected ) )
        {
            if( bIsSelected )
                nSelected = static_cast<sal_Int32>( aLabels.size() );
            aLabels.push_back( sLabel );
        }
    }
    return new SvXMLImportContext( GetImport() );
}

void XMLDropDownFieldImportContext::ProcessAttribute(
    sal_Int32 nAttrToken,
    std::string_view sAttrValue )
{
    if( nAttrToken == XML_ELEMENT(TEXT, XML_NAME))
    {
        sName = OUString::fromUtf8(sAttrValue);
        bNameOK = true;
    }
    else if (nAttrToken ==  XML_ELEMENT(TEXT, XML_HELP))
    {
        sHelp = OUString::fromUtf8(sAttrValue);
        bHelpOK = true;
    }
    else if (nAttrToken ==  XML_ELEMENT(TEXT, XML_HINT))
    {
        sHint = OUString::fromUtf8(sAttrValue);
        bHintOK = true;
    }
    else
        XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

void XMLDropDownFieldImportContext::PrepareField(
    const Reference<XPropertySet>& xPropertySet)
{
    // create sequence
    sal_Int32 nLength = static_cast<sal_Int32>( aLabels.size() );
    Sequence<OUString> aSequence( nLength );
    OUString* pSequence = aSequence.getArray();
    for( sal_Int32 n = 0; n < nLength; n++ )
        pSequence[n] = aLabels[n];

    // now set values:

    xPropertySet->setPropertyValue( u"Items"_ustr, Any(aSequence) );

    if( nSelected >= 0  &&  nSelected < nLength )
    {
        xPropertySet->setPropertyValue( u"SelectedItem"_ustr, Any(pSequence[nSelected]) );
    }

    // set name
    if( bNameOK )
    {
        xPropertySet->setPropertyValue( u"Name"_ustr, Any(sName) );
    }
    // set help
    if( bHelpOK )
    {
        xPropertySet->setPropertyValue( u"Help"_ustr, Any(sHelp) );
    }
    // set hint
    if( bHintOK )
    {
        xPropertySet->setPropertyValue( u"Tooltip"_ustr, Any(sHint) );
    }

}

/** import header fields (<draw:header>) */

XMLHeaderFieldImportContext::XMLHeaderFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp)              /// Text import helper
: XMLTextFieldImportContext(rImport, rHlp, u"Header"_ustr )
{
    sServicePrefix = sAPI_presentation_prefix;
    bValid = true;
}

/// process attribute values
void XMLHeaderFieldImportContext::ProcessAttribute( sal_Int32 nAttrToken, std::string_view sAttrValue )
{
    XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

/// prepare XTextField for insertion into document
void XMLHeaderFieldImportContext::PrepareField(const Reference<XPropertySet> &)
{
}

/** import footer fields (<draw:footer>) */

XMLFooterFieldImportContext::XMLFooterFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp)              /// Text import helper
: XMLTextFieldImportContext(rImport, rHlp, u"Footer"_ustr )
{
    sServicePrefix = sAPI_presentation_prefix;
    bValid = true;
}

/// process attribute values
void XMLFooterFieldImportContext::ProcessAttribute( sal_Int32 nAttrToken, std::string_view sAttrValue)
{
    XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

/// prepare XTextField for insertion into document
void XMLFooterFieldImportContext::PrepareField(const Reference<XPropertySet> &)
{
}


/** import footer fields (<draw:date-and-time>) */

XMLDateTimeFieldImportContext::XMLDateTimeFieldImportContext(
        SvXMLImport& rImport,                   /// XML Import
        XMLTextImportHelper& rHlp)              /// Text import helper
: XMLTextFieldImportContext(rImport, rHlp, u"DateTime"_ustr )
{
    sServicePrefix = sAPI_presentation_prefix;
    bValid = true;
}

/// process attribute values
void XMLDateTimeFieldImportContext::ProcessAttribute( sal_Int32 nAttrToken,
                                   std::string_view sAttrValue )
{
    XMLOFF_WARN_UNKNOWN_ATTR("xmloff", nAttrToken, sAttrValue);
}

/// prepare XTextField for insertion into document
void XMLDateTimeFieldImportContext::PrepareField(
        const css::uno::Reference<
        css::beans::XPropertySet> &)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
