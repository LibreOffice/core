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

#include <sal/config.h>

#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include "layerimp.hxx"


#include <XMLStringBufferImportContext.hxx>

using namespace ::xmloff::token;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

namespace {

class SdXMLLayerContext : public SvXMLImportContext
{
public:
    SdXMLLayerContext( SvXMLImport& rImport, const Reference< XFastAttributeList >& xAttrList, const Reference< XNameAccess >& xLayerManager );

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement,
        const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;
    virtual void SAL_CALL endFastElement(sal_Int32 nElement) override;

private:
    css::uno::Reference< css::container::XNameAccess > mxLayerManager;
    OUString msName;
    OUStringBuffer sDescriptionBuffer;
    OUStringBuffer sTitleBuffer;
    OUString msDisplay;
    OUString msProtected;
};

}

SdXMLLayerContext::SdXMLLayerContext( SvXMLImport& rImport, const Reference< XFastAttributeList >& xAttrList, const Reference< XNameAccess >& xLayerManager )
: SvXMLImportContext(rImport)
, mxLayerManager( xLayerManager )
{
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        OUString sValue = aIter.toString();
        switch(aIter.getToken())
        {
            case XML_ELEMENT(DRAW, XML_NAME):
                msName = sValue;
                break;
            case XML_ELEMENT(DRAW, XML_DISPLAY):
                msDisplay = sValue;
                break;
            case XML_ELEMENT(DRAW, XML_PROTECTED):
                msProtected = sValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLLayerContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >&  )
{
    if( nElement == XML_ELEMENT(SVG, XML_TITLE) )
    {
        return new XMLStringBufferImportContext( GetImport(), sTitleBuffer);
    }
    else if( nElement == XML_ELEMENT(SVG, XML_DESC) )
    {
        return new XMLStringBufferImportContext( GetImport(), sDescriptionBuffer);
    }
    else
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);
    return nullptr;
}

void SdXMLLayerContext::endFastElement(sal_Int32 )
{
    SAL_WARN_IF( msName.isEmpty(), "xmloff", "xmloff::SdXMLLayerContext::EndElement(), draw:layer element without draw:name!" );
    if( msName.isEmpty() )
        return;

    try
    {
        Reference< XPropertySet > xLayer;

        if( mxLayerManager->hasByName( msName ) )
        {
            mxLayerManager->getByName( msName ) >>= xLayer;
            SAL_WARN_IF( !xLayer.is(), "xmloff", "xmloff::SdXMLLayerContext::EndElement(), failed to get existing XLayer!" );
        }
        else
        {
            Reference< XLayerManager > xLayerManager( mxLayerManager, UNO_QUERY );
            if( xLayerManager.is() )
                xLayer = xLayerManager->insertNewByIndex( xLayerManager->getCount() );
            SAL_WARN_IF( !xLayer.is(), "xmloff", "xmloff::SdXMLLayerContext::EndElement(), failed to create new XLayer!" );

            if( xLayer.is() )
                xLayer->setPropertyValue(u"Name"_ustr, Any( msName ) );
        }

        if( xLayer.is() )
        {
            xLayer->setPropertyValue(u"Title"_ustr, Any( sTitleBuffer.makeStringAndClear() ) );
            xLayer->setPropertyValue(u"Description"_ustr, Any( sDescriptionBuffer.makeStringAndClear() ) );
            bool bIsVisible( true );
            bool bIsPrintable( true );
            if ( !msDisplay.isEmpty() )
            {
                bIsVisible = (msDisplay == "always") || (msDisplay == "screen");
                bIsPrintable = (msDisplay == "always") || (msDisplay == "printer");
            }
            xLayer->setPropertyValue(u"IsVisible"_ustr, Any( bIsVisible ) );
            xLayer->setPropertyValue(u"IsPrintable"_ustr, Any( bIsPrintable ) );
            bool bIsLocked( false );
            if ( !msProtected.isEmpty() )
                bIsLocked = (msProtected == "true");
            xLayer->setPropertyValue(u"IsLocked"_ustr, Any( bIsLocked ) );

            // tdf#129898 repair layer "DrawnInSlideshow", which was wrongly written
            // in LO 6.2 to 6.4. It should always have ODF defaults.
            if (msName == "DrawnInSlideshow")
            {
                xLayer->setPropertyValue(u"IsVisible"_ustr, Any(true));
                xLayer->setPropertyValue(u"IsPrintable"_ustr, Any(true));
                xLayer->setPropertyValue(u"IsLocked"_ustr, Any(false));
            }
        }
    }
    catch( Exception& )
    {
        TOOLS_WARN_EXCEPTION("xmloff.draw", "");
    }
}


SdXMLLayerSetContext::SdXMLLayerSetContext( SvXMLImport& rImport )
: SvXMLImportContext(rImport)
{
    Reference< XLayerSupplier > xLayerSupplier( rImport.GetModel(), UNO_QUERY );
    SAL_WARN_IF( !xLayerSupplier.is(), "xmloff", "xmloff::SdXMLLayerSetContext::SdXMLLayerSetContext(), XModel is not supporting XLayerSupplier!" );
    if( xLayerSupplier.is() )
        mxLayerManager = xLayerSupplier->getLayerManager();
}

SdXMLLayerSetContext::~SdXMLLayerSetContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLLayerSetContext::createFastChildContext(
    sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    return new SdXMLLayerContext( GetImport(), xAttrList, mxLayerManager );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
