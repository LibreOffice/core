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

#include "XMLSectionImportContext.hxx"
#include "XMLSectionSourceImportContext.hxx"
#include "XMLSectionSourceDDEImportContext.hxx"
#include <comphelper/base64.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/prstylei.hxx>
#include <sax/tools/converter.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>


using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::xml::sax::XAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNamed;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;

namespace {

enum XMLSectionToken
{
    XML_TOK_SECTION_XMLID,
    XML_TOK_SECTION_STYLE_NAME,
    XML_TOK_SECTION_NAME,
    XML_TOK_SECTION_CONDITION,
    XML_TOK_SECTION_DISPLAY,
    XML_TOK_SECTION_PROTECT,
    XML_TOK_SECTION_PROTECTION_KEY,
    XML_TOK_SECTION_IS_HIDDEN
};

}

const SvXMLTokenMapEntry aSectionTokenMap[] =
{
    { XML_NAMESPACE_XML , XML_ID, XML_TOK_SECTION_XMLID },
    { XML_NAMESPACE_TEXT, XML_STYLE_NAME, XML_TOK_SECTION_STYLE_NAME },
    { XML_NAMESPACE_TEXT, XML_NAME, XML_TOK_SECTION_NAME },
    { XML_NAMESPACE_TEXT, XML_CONDITION, XML_TOK_SECTION_CONDITION },
    { XML_NAMESPACE_TEXT, XML_DISPLAY, XML_TOK_SECTION_DISPLAY },
    { XML_NAMESPACE_TEXT, XML_PROTECTED, XML_TOK_SECTION_PROTECT },
    { XML_NAMESPACE_TEXT, XML_PROTECTION_KEY, XML_TOK_SECTION_PROTECTION_KEY},
    { XML_NAMESPACE_TEXT, XML_IS_HIDDEN, XML_TOK_SECTION_IS_HIDDEN },
    // compatibility with SRC629 (or earlier) versions
    { XML_NAMESPACE_TEXT, XML_PROTECT, XML_TOK_SECTION_PROTECT },
    XML_TOKEN_MAP_END
};


// section import: This one is fairly tricky due to a variety of
// limits of the core or the API. The main problem is that if you
// insert a section within another section, you can't move the cursor
// between the ends of the inner and the enclosing section. To avoid
// these problems, additional markers are first inserted and later deleted.
XMLSectionImportContext::XMLSectionImportContext(
    SvXMLImport& rImport )
:   SvXMLImportContext(rImport)
,   bProtect(false)
,   bCondOK(false)
,   bIsVisible(true)
,   bValid(false)
,   bSequenceOK(false)
,   bIsCurrentlyVisible(true)
,   bIsCurrentlyVisibleOK(false)
,   bHasContent(false)
{
}

XMLSectionImportContext::~XMLSectionImportContext()
{
}

void XMLSectionImportContext::startFastElement(
    sal_Int32 /*nElement*/,
    const Reference<css::xml::sax::XFastAttributeList> & xAttrList)
{
    // process attributes
    ProcessAttributes(xAttrList);

    // process index headers:
    bool bIsIndexHeader = IsXMLToken( GetLocalName(), XML_INDEX_TITLE );
    if (bIsIndexHeader)
    {
        bValid = true;
    }

    rtl::Reference<XMLTextImportHelper> rHelper = GetImport().GetTextImport();

    // valid?
    if (!bValid)
        return;

    // create text section (as XPropertySet)
    Reference<XMultiServiceFactory> xFactory(
        GetImport().GetModel(),UNO_QUERY);
    if (!xFactory.is())
        return;

    Reference<XInterface> xIfc =
        xFactory->createInstance( bIsIndexHeader ? OUString("com.sun.star.text.IndexHeaderSection")
                                                 : OUString("com.sun.star.text.TextSection") );
    if (!xIfc.is())
        return;

    Reference<XPropertySet> xPropSet(xIfc, UNO_QUERY);

    // save PropertySet (for CreateChildContext)
    xSectionPropertySet = xPropSet;

    // name
    Reference<XNamed> xNamed(xPropSet, UNO_QUERY);
    xNamed->setName(sName);

    // stylename?
    if (!sStyleName.isEmpty())
    {
        XMLPropStyleContext* pStyle = rHelper->
            FindSectionStyle(sStyleName);

        if (pStyle != nullptr)
        {
            pStyle->FillPropertySet( xPropSet );
        }
    }

    // IsVisible and condition (not for index headers)
    if (! bIsIndexHeader)
    {
        xPropSet->setPropertyValue( "IsVisible", Any(bIsVisible) );

        // #97450# hidden sections must be hidden on reload
        // For backwards compatibility, set flag only if it is
        // present
        if( bIsCurrentlyVisibleOK )
        {
            xPropSet->setPropertyValue( "IsCurrentlyVisible", Any(bIsCurrentlyVisible));
        }

        if (bCondOK)
        {
            xPropSet->setPropertyValue( "Condition", Any(sCond) );
        }
    }

    // password (only for regular sections)
    if ( bSequenceOK &&
         IsXMLToken(GetLocalName(), XML_SECTION) )
    {
        xPropSet->setPropertyValue("ProtectionKey", Any(aSequence));
    }

    // protection
    xPropSet->setPropertyValue( "IsProtected", Any(bProtect) );

    // insert marker, <paragraph>, marker; then insert
    // section over the first marker character, and delete the
    // last paragraph (and marker) when closing a section.
    Reference<XTextRange> xStart =
        rHelper->GetCursor()->getStart();
#ifndef DBG_UTIL
    OUString sMarkerString(" ");
#else
    OUString sMarkerString("X");
#endif
    rHelper->InsertString(sMarkerString);
    rHelper->InsertControlCharacter(
        ControlCharacter::APPEND_PARAGRAPH );
    rHelper->InsertString(sMarkerString);

    // select first marker
    rHelper->GetCursor()->gotoRange(xStart, false);
    rHelper->GetCursor()->goRight(1, true);

    // convert section to XTextContent
    Reference<XTextContent> xTextContent(xSectionPropertySet,
                                         UNO_QUERY);

    // and insert (over marker)
    rHelper->GetText()->insertTextContent(
        rHelper->GetCursorAsRange(), xTextContent, true );

    // and delete first marker (in section)
    rHelper->GetText()->insertString(
        rHelper->GetCursorAsRange(), "", true);

    // finally, check for redlines that should start at
    // the section start node
    rHelper->RedlineAdjustStartNodeCursor(); // start ???

    // xml:id for RDF metadata
    GetImport().SetXmlId(xIfc, sXmlId);
}

void XMLSectionImportContext::ProcessAttributes(
    const Reference<css::xml::sax::XFastAttributeList> & xAttrList )
{
    for(auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        OUString sValue = aIter.toString();

        switch (aIter.getToken())
        {
            case XML_ELEMENT(XML, XML_ID):
                sXmlId = sValue;
                break;
            case XML_ELEMENT(TEXT, XML_STYLE_NAME):
                sStyleName = sValue;
                break;
            case XML_ELEMENT(TEXT, XML_NAME):
                sName = sValue;
                bValid = true;
                break;
            case XML_ELEMENT(TEXT, XML_CONDITION):
                {
                    OUString sTmp;
                    sal_uInt16 nPrefix = GetImport().GetNamespaceMap().
                                    GetKeyByAttrValueQName(sValue, &sTmp);
                    if( XML_NAMESPACE_OOOW == nPrefix )
                    {
                        sCond = sTmp;
                        bCondOK = true;
                    }
                    else
                        sCond = sValue;
                }
                break;
            case XML_ELEMENT(TEXT, XML_DISPLAY):
                if (IsXMLToken(sValue, XML_TRUE))
                {
                    bIsVisible = true;
                }
                else if ( IsXMLToken(sValue, XML_NONE) ||
                          IsXMLToken(sValue, XML_CONDITION) )
                {
                    bIsVisible = false;
                }
                // else: ignore
                break;
            case XML_ELEMENT(TEXT, XML_IS_HIDDEN):
                {
                    bool bTmp(false);
                    if (::sax::Converter::convertBool(bTmp, sValue))
                    {
                        bIsCurrentlyVisible = !bTmp;
                        bIsCurrentlyVisibleOK = true;
                    }
                }
                break;
            case XML_ELEMENT(TEXT, XML_PROTECTION_KEY):
                ::comphelper::Base64::decode(aSequence, sValue);
                bSequenceOK = true;
                break;
            case XML_ELEMENT(TEXT, XML_PROTECTED):
            // compatibility with SRC629 (or earlier) versions
            case XML_ELEMENT(TEXT, XML_PROTECT):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, sValue))
                {
                    bProtect = bTmp;
                }
                break;
            }
            default:
                SAL_WARN("xmloff", "unknown attribute " << SvXMLImport::getPrefixAndNameFromToken(aIter.getToken()) << "=" << sValue);
        }
    }
}

void XMLSectionImportContext::endFastElement(sal_Int32 )
{
    // get rid of last paragraph
    // (unless it's the only paragraph in the section)
    rtl::Reference<XMLTextImportHelper> rHelper = GetImport().GetTextImport();
    rHelper->GetCursor()->goRight(1, false);
    if (bHasContent)
    {
        rHelper->GetCursor()->goLeft(1, true);
        rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                         "", true);
    }

    // and delete second marker
    rHelper->GetCursor()->goRight(1, true);
    rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                     "", true);

    // check for redlines to our endnode
    rHelper->RedlineAdjustStartNodeCursor();
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLSectionImportContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // section-source (-dde) elements
    if ( nElement == XML_ELEMENT(TEXT, XML_SECTION_SOURCE) )
    {
    }
    else if ( nElement == XML_ELEMENT(OFFICE, XML_DDE_SOURCE) )
    {
    }
    else
    {
        // otherwise: text context
        auto pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nElement, xAttrList, XMLTextType::Section );

        // if that fails, default context
        if (pContext)
            bHasContent = true;
        else
            SAL_WARN("xmloff", "unknown element " << SvXMLImport::getPrefixAndNameFromToken(nElement));
        return pContext;
    }
    return nullptr;
}

SvXMLImportContextRef XMLSectionImportContext::CreateChildContext(
    sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const Reference<XAttributeList> & /*xAttrList*/ )
{
    SvXMLImportContext* pContext = nullptr;

    // section-source (-dde) elements
    if ( (XML_NAMESPACE_TEXT == nPrefix) &&
         IsXMLToken(rLocalName, XML_SECTION_SOURCE) )
    {
        pContext = new XMLSectionSourceImportContext(GetImport(),
                                                     nPrefix, rLocalName,
                                                     xSectionPropertySet);
    }
    else if ( (XML_NAMESPACE_OFFICE == nPrefix) &&
              IsXMLToken(rLocalName, XML_DDE_SOURCE) )
    {
        pContext = new XMLSectionSourceDDEImportContext(GetImport(),
                                                        nPrefix, rLocalName,
                                                        xSectionPropertySet);
    }

    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
