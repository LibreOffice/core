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
#include <sal/log.hxx>
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
using ::com::sun::star::xml::sax::XFastAttributeList;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::container::XNamed;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::xmloff::token;


// section import: This one is fairly tricky due to a variety of
// limits of the core or the API. The main problem is that if you
// insert a section within another section, you can't move the cursor
// between the ends of the inner and the enclosing section. To avoid
// these problems, additional markers are first inserted and later deleted.
XMLSectionImportContext::XMLSectionImportContext( SvXMLImport& rImport )
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

void XMLSectionImportContext::startFastElement( sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    // process attributes
    ProcessAttributes(xAttrList);

    // process index headers:
    bool bIsIndexHeader = (nElement & TOKEN_MASK) == XML_INDEX_TITLE;
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
        xFactory->createInstance( bIsIndexHeader ? u"com.sun.star.text.IndexHeaderSection"_ustr
                                                 : u"com.sun.star.text.TextSection"_ustr );
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
        xPropSet->setPropertyValue( u"IsVisible"_ustr, Any(bIsVisible) );

        // #97450# hidden sections must be hidden on reload
        // For backwards compatibility, set flag only if it is
        // present
        if( bIsCurrentlyVisibleOK )
        {
            xPropSet->setPropertyValue( u"IsCurrentlyVisible"_ustr, Any(bIsCurrentlyVisible));
        }

        if (bCondOK)
        {
            xPropSet->setPropertyValue( u"Condition"_ustr, Any(sCond) );
        }
    }

    // password (only for regular sections)
    if ( bSequenceOK &&
         (nElement & TOKEN_MASK) == XML_SECTION )
    {
        xPropSet->setPropertyValue(u"ProtectionKey"_ustr, Any(aSequence));
    }

    // protection
    xPropSet->setPropertyValue( u"IsProtected"_ustr, Any(bProtect) );

    // insert marker, <paragraph>, marker; then insert
    // section over the first marker character, and delete the
    // last paragraph (and marker) when closing a section.
    Reference<XTextRange> xStart =
        rHelper->GetCursor()->getStart();
#ifndef DBG_UTIL
    OUString sMarkerString(" ");
#else
    OUString sMarkerString(u"X"_ustr);
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
        rHelper->GetCursorAsRange(), u""_ustr, true);

    // finally, check for redlines that should start at
    // the section start node
    rHelper->RedlineAdjustStartNodeCursor(); // start ???

    // xml:id for RDF metadata
    GetImport().SetXmlId(xIfc, sXmlId);
}

void XMLSectionImportContext::ProcessAttributes(
    const Reference<XFastAttributeList> & xAttrList )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch (aIter.getToken())
        {
            case XML_ELEMENT(XML, XML_ID):
                sXmlId = aIter.toString();
                break;
            case XML_ELEMENT(TEXT, XML_STYLE_NAME):
                sStyleName = aIter.toString();
                break;
            case XML_ELEMENT(TEXT, XML_NAME):
                sName = aIter.toString();
                bValid = true;
                break;
            case XML_ELEMENT(TEXT, XML_CONDITION):
                {
                    OUString sValue = aIter.toString();
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
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    bIsVisible = true;
                }
                else if ( IsXMLToken(aIter, XML_NONE) ||
                          IsXMLToken(aIter, XML_CONDITION) )
                {
                    bIsVisible = false;
                }
                // else: ignore
                break;
            case XML_ELEMENT(TEXT, XML_IS_HIDDEN):
                {
                    bool bTmp(false);
                    if (::sax::Converter::convertBool(bTmp, aIter.toView()))
                    {
                        bIsCurrentlyVisible = !bTmp;
                        bIsCurrentlyVisibleOK = true;
                    }
                }
                break;
            case XML_ELEMENT(TEXT, XML_PROTECTION_KEY):
                ::comphelper::Base64::decode(aSequence, aIter.toString());
                bSequenceOK = true;
                break;
            case XML_ELEMENT(TEXT, XML_PROTECTED):
            // compatibility with SRC629 (or earlier) versions
            case XML_ELEMENT(TEXT, XML_PROTECT):
            {
                bool bTmp(false);
                if (::sax::Converter::convertBool(bTmp, aIter.toView()))
                {
                    bProtect = bTmp;
                }
                break;
            }
            default:
                // ignore
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
                break;
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
                                         u""_ustr, true);
    }

    // and delete second marker
    rHelper->GetCursor()->goRight(1, true);
    rHelper->GetText()->insertString(rHelper->GetCursorAsRange(),
                                     u""_ustr, true);

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
        return new XMLSectionSourceImportContext(GetImport(),
                                                     xSectionPropertySet);
    }
    else if ( nElement == XML_ELEMENT(OFFICE, XML_DDE_SOURCE) )
    {
        return new XMLSectionSourceDDEImportContext(GetImport(),
                                                        xSectionPropertySet);
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
            XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
        return pContext;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
