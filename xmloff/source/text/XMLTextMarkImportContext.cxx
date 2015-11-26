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

#include "XMLTextMarkImportContext.hxx"


#include <rtl/ustring.hxx>
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/odffields.hxx>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/rdf/XMetadatable.hpp>

#include <com/sun/star/text/XFormField.hpp>

#include "RDFaImportHelper.hxx"



using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::xml::sax;
using namespace ::xmloff::token;



XMLFieldParamImportContext::XMLFieldParamImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrefix,
    const OUString& rLocalName ) :
        SvXMLImportContext(rImport, nPrefix, rLocalName),
        rHelper(rHlp)
{
}


void XMLFieldParamImportContext::StartElement(const css::uno::Reference< css::xml::sax::XAttributeList> & xAttrList)
{
    SvXMLImport& rImport = GetImport();
    OUString sName;
    OUString sValue;

    sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        if ( (XML_NAMESPACE_FIELD == nPrefix) &&
             IsXMLToken(sLocalName, XML_NAME)   )
        {
            sName = xAttrList->getValueByIndex(nAttr);
        }
        if ( (XML_NAMESPACE_FIELD == nPrefix) &&
             IsXMLToken(sLocalName, XML_VALUE)   )
        {
            sValue = xAttrList->getValueByIndex(nAttr);
        }
    }
    if (rHelper.hasCurrentFieldCtx() && !sName.isEmpty()) {
        rHelper.addFieldParam(sName, sValue);
    }
}



XMLTextMarkImportContext::XMLTextMarkImportContext(
    SvXMLImport& rImport,
    XMLTextImportHelper& rHlp,
    sal_uInt16 nPrefix,
    const OUString& rLocalName )
    : SvXMLImportContext(rImport, nPrefix, rLocalName)
    , m_rHelper(rHlp)
    , m_bHaveAbout(false)
{
}

enum lcl_MarkType { TypeReference, TypeReferenceStart, TypeReferenceEnd,
                    TypeBookmark, TypeBookmarkStart, TypeBookmarkEnd,
                    TypeFieldmark, TypeFieldmarkStart, TypeFieldmarkEnd
                  };

static SvXMLEnumMapEntry const lcl_aMarkTypeMap[] =
{
    { XML_REFERENCE_MARK,           TypeReference },
    { XML_REFERENCE_MARK_START,     TypeReferenceStart },
    { XML_REFERENCE_MARK_END,       TypeReferenceEnd },
    { XML_BOOKMARK,                 TypeBookmark },
    { XML_BOOKMARK_START,           TypeBookmarkStart },
    { XML_BOOKMARK_END,             TypeBookmarkEnd },
    { XML_FIELDMARK,                TypeFieldmark },
    { XML_FIELDMARK_START,          TypeFieldmarkStart },
    { XML_FIELDMARK_END,            TypeFieldmarkEnd },
    { XML_TOKEN_INVALID,            0 },
};


static const char *lcl_getFormFieldmarkName(OUString &name)
{
    static const char sCheckbox[]=ODF_FORMCHECKBOX;
    static const char sFormDropDown[]=ODF_FORMDROPDOWN;
    if (name == "msoffice.field.FORMCHECKBOX" ||
        name == "ecma.office-open-xml.field.FORMCHECKBOX")
        return sCheckbox;
    else if (name == ODF_FORMCHECKBOX)
        return sCheckbox;
    if (name == ODF_FORMDROPDOWN ||
        name == "ecma.office-open-xml.field.FORMDROPDOWN")
        return sFormDropDown;
    else
        return nullptr;
}

static OUString lcl_getFieldmarkName(OUString const& name)
{
    if (name == "msoffice.field.FORMTEXT" ||
        name == "ecma.office-open-xml.field.FORMTEXT")
        return OUString(ODF_FORMTEXT);
    else
        return name;
}


void XMLTextMarkImportContext::StartElement(
    const Reference<XAttributeList> & xAttrList)
{
    if (!FindName(GetImport(), xAttrList))
    {
        m_sBookmarkName.clear();
    }

    if (IsXMLToken(GetLocalName(), XML_FIELDMARK_END))
    {
        m_sBookmarkName = m_rHelper.FindActiveBookmarkName();
    }

    if (IsXMLToken(GetLocalName(), XML_FIELDMARK_START) || IsXMLToken(GetLocalName(), XML_FIELDMARK))
    {
        if (m_sBookmarkName.isEmpty())
        {
            m_sBookmarkName = "Unknown";
        }
        m_rHelper.pushFieldCtx( m_sBookmarkName, m_sFieldName );
    }
}

void XMLTextMarkImportContext::EndElement()
{
    SvXMLImportContext::EndElement();

    static const char sAPI_reference_mark[] = "com.sun.star.text.ReferenceMark";
    static const char sAPI_bookmark[] = "com.sun.star.text.Bookmark";
    static const char sAPI_fieldmark[] = "com.sun.star.text.Fieldmark";
    static const char sAPI_formfieldmark[] = "com.sun.star.text.FormFieldmark";

    if (!m_sBookmarkName.isEmpty())
    {
        sal_uInt16 nTmp;
        if (SvXMLUnitConverter::convertEnum(nTmp, GetLocalName(),
                                            lcl_aMarkTypeMap))
        {
            switch ((lcl_MarkType)nTmp)
            {
                case TypeReference:
                    // export point reference mark
                    CreateAndInsertMark(GetImport(),
                        sAPI_reference_mark,
                        m_sBookmarkName,
                        m_rHelper.GetCursorAsRange()->getStart(),
                        OUString());
                    break;

                case TypeFieldmark:
                case TypeBookmark:
                    {
                        const char *formFieldmarkName=lcl_getFormFieldmarkName(m_sFieldName);
                        bool bImportAsField=((lcl_MarkType)nTmp==TypeFieldmark && formFieldmarkName!=nullptr); //@TODO handle abbreviation cases..
                        // export point bookmark
                        const Reference<XInterface> xContent(
                            CreateAndInsertMark(GetImport(),
                                        (bImportAsField ? OUString(sAPI_formfieldmark) : OUString(sAPI_bookmark)),
                                m_sBookmarkName,
                                m_rHelper.GetCursorAsRange()->getStart(),
                                m_sXmlId) );
                        if ((lcl_MarkType)nTmp==TypeFieldmark) {
                            if (xContent.is() && bImportAsField) {
                                // setup fieldmark...
                                Reference< css::text::XFormField> xFormField(xContent, UNO_QUERY);
                                xFormField->setFieldType(OUString::createFromAscii(formFieldmarkName));
                                if (xFormField.is() && m_rHelper.hasCurrentFieldCtx()) {
                                    m_rHelper.setCurrentFieldParamsTo(xFormField);
                                }
                            }
                            m_rHelper.popFieldCtx();
                        }
                    }
                    break;

                case TypeFieldmarkStart:
                case TypeBookmarkStart:
                    // save XTextRange for later construction of bookmark
                    {
                        std::shared_ptr< ::xmloff::ParsedRDFaAttributes >
                            xRDFaAttributes;
                        if (m_bHaveAbout && (TypeBookmarkStart
                                == static_cast<lcl_MarkType>(nTmp)))
                        {
                            xRDFaAttributes =
                                GetImport().GetRDFaImportHelper().ParseRDFa(
                                    m_sAbout, m_sProperty,
                                    m_sContent, m_sDatatype);
                        }
                        m_rHelper.InsertBookmarkStartRange(
                            m_sBookmarkName,
                            m_rHelper.GetCursorAsRange()->getStart(),
                            m_sXmlId, xRDFaAttributes);
                    }
                    break;

                case TypeFieldmarkEnd:
                case TypeBookmarkEnd:
                {
                    // get old range, and construct
                    Reference<XTextRange> xStartRange;
                    std::shared_ptr< ::xmloff::ParsedRDFaAttributes >
                        xRDFaAttributes;
                    if (m_rHelper.FindAndRemoveBookmarkStartRange(
                            m_sBookmarkName, xStartRange,
                            m_sXmlId, xRDFaAttributes))
                    {
                        Reference<XTextRange> xEndRange(
                            m_rHelper.GetCursorAsRange()->getStart());

                        // check if beginning and end are in same XText
                        if (xStartRange->getText() == xEndRange->getText())
                        {
                            // create range for insertion
                            Reference<XTextCursor> xInsertionCursor =
                                m_rHelper.GetText()->createTextCursorByRange(
                                    xEndRange);
                            try {
                            xInsertionCursor->gotoRange(xStartRange, sal_True);
                            } catch (uno::Exception&) {
                                OSL_ENSURE(false,
                                    "cannot go to end position of bookmark");
                            }

                            //DBG_ASSERT(! xInsertionCursor->isCollapsed(),
                            //              "we want no point mark");
                            // can't assert, because someone could
                            // create a file with subsequence
                            // start/end elements

                            bool bImportAsField=((lcl_MarkType)nTmp==TypeFieldmarkEnd && m_rHelper.hasCurrentFieldCtx());

                            // fdo#86795 check if it's actually a checkbox first
                            bool isInvalid(false);
                            OUString fieldmarkTypeName;
                            if (bImportAsField && m_rHelper.hasCurrentFieldCtx())
                            {

                                OUString const type(m_rHelper.getCurrentFieldType());
                                fieldmarkTypeName = lcl_getFieldmarkName(type);
                                if (fieldmarkTypeName == ODF_FORMCHECKBOX ||
                                    fieldmarkTypeName == ODF_FORMDROPDOWN)
                                {   // sw can't handle checkbox with start+end
                                    SAL_INFO("xmloff.text", "invalid fieldmark-start/fieldmark-end ignored");
                                    isInvalid = true;
                                }
                            }

                            Reference<XInterface> xContent;
                            if (!isInvalid)
                            {
                                // insert reference
                                xContent = CreateAndInsertMark(GetImport(),
                                        (bImportAsField
                                            ? OUString(sAPI_fieldmark)
                                            : OUString(sAPI_bookmark)),
                                        m_sBookmarkName,
                                        xInsertionCursor,
                                        m_sXmlId);
                                if (xRDFaAttributes)
                                {
                                    const Reference<rdf::XMetadatable>
                                        xMeta(xContent, UNO_QUERY);
                                    GetImport().GetRDFaImportHelper().AddRDFa(
                                        xMeta, xRDFaAttributes);
                                }
                            }

                            if ((lcl_MarkType)nTmp==TypeFieldmarkEnd) {
                                if (xContent.is() && bImportAsField) {
                                    // setup fieldmark...
                                    Reference< css::text::XFormField> xFormField(xContent, UNO_QUERY);
                                    if (xFormField.is() && m_rHelper.hasCurrentFieldCtx()) {

                                        xFormField->setFieldType(fieldmarkTypeName);
                                        m_rHelper.setCurrentFieldParamsTo(xFormField);
                                    }
                                }
                                m_rHelper.popFieldCtx();
                            }
                        }
                        // else: beginning/end in different XText -> ignore!
                    }
                    // else: no start found -> ignore!
                    break;
                }

                case TypeReferenceStart:
                case TypeReferenceEnd:
                    OSL_FAIL("reference start/end are handled in txtparai !");
                    break;

                default:
                    OSL_FAIL("unknown mark type");
                    break;
            }
        }
    }
}

SvXMLImportContext *XMLTextMarkImportContext::CreateChildContext( sal_uInt16 nPrefix,
                                        const OUString& rLocalName,
                                        const css::uno::Reference< css::xml::sax::XAttributeList >&  )
{
    return new XMLFieldParamImportContext(GetImport(), m_rHelper,
                nPrefix, rLocalName);
}


Reference<XTextContent> XMLTextMarkImportContext::CreateAndInsertMark(
    SvXMLImport& rImport,
    const OUString& sServiceName,
    const OUString& sMarkName,
    const Reference<XTextRange> & rRange,
    const OUString& i_rXmlId)
{
    // create mark
    const Reference<XMultiServiceFactory> xFactory(rImport.GetModel(),
        UNO_QUERY);
    Reference<XInterface> xIfc;

    if (xFactory.is())
    {
        xIfc = xFactory->createInstance(sServiceName);

        if (!xIfc.is())
        {
            OSL_FAIL("CreateAndInsertMark: cannot create service?");
            return nullptr;
        }

        // set name (unless there is no name (text:meta))
        const Reference<XNamed> xNamed(xIfc, UNO_QUERY);
        if (xNamed.is())
        {
            xNamed->setName(sMarkName);
        }
        else
        {
            if (!sMarkName.isEmpty())
            {
                OSL_FAIL("name given, but XNamed not supported?");
                return nullptr;
            }
        }

        // cast to XTextContent and attach to document
        const Reference<XTextContent> xTextContent(xIfc, UNO_QUERY);
        if (xTextContent.is())
        {
            try
            {
                // if inserting marks, bAbsorb==sal_False will cause
                // collapsing of the given XTextRange.
                rImport.GetTextImport()->GetText()->insertTextContent(rRange,
                    xTextContent, sal_True);

                // xml:id for RDF metadata -- after insertion!
                rImport.SetXmlId(xIfc, i_rXmlId);

                return xTextContent;
            }
            catch (css::lang::IllegalArgumentException &)
            {
                OSL_FAIL("CreateAndInsertMark: cannot insert?");
                return nullptr;
            }
        }
    }
    return nullptr;
}

bool XMLTextMarkImportContext::FindName(
    SvXMLImport& rImport,
    const Reference<XAttributeList> & xAttrList)
{
    bool bNameOK = false;

    // find name attribute first
    const sal_Int16 nLength = xAttrList->getLength();
    for(sal_Int16 nAttr = 0; nAttr < nLength; nAttr++)
    {
        OUString sLocalName;
        const sal_uInt16 nPrefix = rImport.GetNamespaceMap().
            GetKeyByAttrName( xAttrList->getNameByIndex(nAttr),
                              &sLocalName );

        if ( (XML_NAMESPACE_TEXT == nPrefix) &&
             IsXMLToken(sLocalName, XML_NAME)   )
        {
            m_sBookmarkName = xAttrList->getValueByIndex(nAttr);
            bNameOK = true;
        }
        else if ( (XML_NAMESPACE_XML == nPrefix) &&
             IsXMLToken(sLocalName, XML_ID)   )
        {
            m_sXmlId = xAttrList->getValueByIndex(nAttr);
        }
        else if ( XML_NAMESPACE_XHTML == nPrefix )
        {
            // RDFa
            if ( IsXMLToken( sLocalName, XML_ABOUT) )
            {
                m_sAbout = xAttrList->getValueByIndex(nAttr);
                m_bHaveAbout = true;
            }
            else if ( IsXMLToken( sLocalName, XML_PROPERTY) )
            {
                m_sProperty = xAttrList->getValueByIndex(nAttr);
            }
            else if ( IsXMLToken( sLocalName, XML_CONTENT) )
            {
                m_sContent = xAttrList->getValueByIndex(nAttr);
            }
            else if ( IsXMLToken( sLocalName, XML_DATATYPE) )
            {
                m_sDatatype = xAttrList->getValueByIndex(nAttr);
            }
        }
        else if ( (XML_NAMESPACE_FIELD == nPrefix) &&
             IsXMLToken(sLocalName, XML_TYPE)   )
        {
            m_sFieldName = xAttrList->getValueByIndex(nAttr);
        }
    }

    return bNameOK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
