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


#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <com/sun/star/drawing/XLayerManager.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/drawing/XLayerSupplier.hpp>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "layerimp.hxx"


#include <XMLStringBufferImportContext.hxx>

using namespace ::std;
using namespace ::cppu;
using namespace ::xmloff::token;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using ::xmloff::token::IsXMLToken;

namespace {

class SdXMLLayerContext : public SvXMLImportContext
{
public:
    SdXMLLayerContext( SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, const Reference< XNameAccess >& xLayerManager );

    virtual SvXMLImportContextRef CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList ) override;
    virtual void EndElement() override;

private:
    css::uno::Reference< css::container::XNameAccess > mxLayerManager;
    OUString msName;
    OUStringBuffer sDescriptionBuffer;
    OUStringBuffer sTitleBuffer;
    OUString msDisplay;
    OUString msProtected;
};

}

SdXMLLayerContext::SdXMLLayerContext( SvXMLImport& rImport, sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& xAttrList, const Reference< XNameAccess >& xLayerManager )
: SvXMLImportContext(rImport, nPrefix, rLocalName)
, mxLayerManager( xLayerManager )
{
    const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString aLocalName;
        if( GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( i ), &aLocalName ) == XML_NAMESPACE_DRAW )
        {
            const OUString sValue( xAttrList->getValueByIndex( i ) );

            if( IsXMLToken( aLocalName, XML_NAME ) )
            {
                msName = sValue;
            }
            else if ( IsXMLToken( aLocalName, XML_DISPLAY))
            {
                msDisplay = sValue;
            }
            else if ( IsXMLToken( aLocalName, XML_PROTECTED))
            {
                msProtected = sValue;
            }
        }
    }

}

SvXMLImportContextRef SdXMLLayerContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList >& )
{
    if( (XML_NAMESPACE_SVG == nPrefix) && IsXMLToken(rLocalName, XML_TITLE) )
    {
        return new XMLStringBufferImportContext( GetImport(), nPrefix, rLocalName, sTitleBuffer);
    }
    else if( (XML_NAMESPACE_SVG == nPrefix) && IsXMLToken(rLocalName, XML_DESC) )
    {
        return new XMLStringBufferImportContext( GetImport(), nPrefix, rLocalName, sDescriptionBuffer);
    }
    return nullptr;
}

void SdXMLLayerContext::EndElement()
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
                xLayer->setPropertyValue("Name", Any( msName ) );
        }

        if( xLayer.is() )
        {
            xLayer->setPropertyValue("Title", Any( sTitleBuffer.makeStringAndClear() ) );
            xLayer->setPropertyValue("Description", Any( sDescriptionBuffer.makeStringAndClear() ) );
            bool bIsVisible( true );
            bool bIsPrintable( true );
            if ( !msDisplay.isEmpty() )
            {
                bIsVisible = (msDisplay == "always") || (msDisplay == "screen");
                bIsPrintable = (msDisplay == "always") || (msDisplay == "printer");
            }
            xLayer->setPropertyValue("IsVisible", Any( bIsVisible ) );
            xLayer->setPropertyValue("IsPrintable", Any( bIsPrintable ) );
            bool bIsLocked( false );
            if ( !msProtected.isEmpty() )
                bIsLocked = (msProtected == "true");
            xLayer->setPropertyValue("IsLocked", Any( bIsLocked ) );

            // tdf#129898 repair layer "DrawnInSlideshow", which was wrongly written
            // in LO 6.2 to 6.4. It should always have ODF defaults.
            if (msName == "DrawnInSlideshow")
            {
                xLayer->setPropertyValue("IsVisible", Any(true));
                xLayer->setPropertyValue("IsPrintable", Any(true));
                xLayer->setPropertyValue("IsLocked", Any(false));
            }
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("SdXMLLayerContext::EndElement(), exception caught!");
    }
}


SdXMLLayerSetContext::SdXMLLayerSetContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>&)
: SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    Reference< XLayerSupplier > xLayerSupplier( rImport.GetModel(), UNO_QUERY );
    SAL_WARN_IF( !xLayerSupplier.is(), "xmloff", "xmloff::SdXMLLayerSetContext::SdXMLLayerSetContext(), XModel is not supporting XLayerSupplier!" );
    if( xLayerSupplier.is() )
        mxLayerManager = xLayerSupplier->getLayerManager();
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

SvXMLImportContextRef SdXMLLayerSetContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    return new SdXMLLayerContext( GetImport(), nPrefix, rLocalName, xAttrList, mxLayerManager );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
