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

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "ximpshow.hxx"

using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::xml;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::presentation;
using namespace ::xmloff::token;

class ShowsImpImpl
{
public:
    Reference< XSingleServiceFactory > mxShowFactory;
    Reference< XNameContainer > mxShows;
    Reference< XPropertySet > mxPresProps;
    Reference< XNameAccess > mxPages;
    OUString maCustomShowName;
};


SdXMLShowsContext::SdXMLShowsContext( SdXMLImport& rImport,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList )
:   SvXMLImportContext(rImport, nPrfx, rLocalName),
    mpImpl(new ShowsImpImpl )
{

    Reference< XCustomPresentationSupplier > xShowsSupplier( rImport.GetModel(), UNO_QUERY );
    if( xShowsSupplier.is() )
    {
        mpImpl->mxShows = xShowsSupplier->getCustomPresentations();
        mpImpl->mxShowFactory.set( mpImpl->mxShows, UNO_QUERY );
    }

    Reference< XDrawPagesSupplier > xDrawPagesSupplier( rImport.GetModel(), UNO_QUERY );
    if( xDrawPagesSupplier.is() )
        mpImpl->mxPages.set( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );

    Reference< XPresentationSupplier > xPresentationSupplier( rImport.GetModel(), UNO_QUERY );
    if( xPresentationSupplier.is() )
        mpImpl->mxPresProps.set( xPresentationSupplier->getPresentation(), UNO_QUERY );

    if( mpImpl->mxPresProps.is() )
    {
        bool bAll = true;
        uno::Any aAny;
        // Per ODF this is default, but we did it wrong before LO 6.0 (tdf#108824)
        bool bIsMouseVisible = true;
        if (rImport.getGeneratorVersion() < SvXMLImport::LO_6x)
            bIsMouseVisible = false;

        // read attributes
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            OUString sValue = xAttrList->getValueByIndex( i );

            switch( nPrefix )
            {
            case XML_NAMESPACE_PRESENTATION:
                if( IsXMLToken( aLocalName, XML_START_PAGE ) )
                {
                    mpImpl->mxPresProps->setPropertyValue("FirstPage", Any(sValue) );
                    bAll = false;
                }
                else if( IsXMLToken( aLocalName, XML_SHOW ) )
                {
                    mpImpl->maCustomShowName = sValue;
                    bAll = false;
                }
                else if( IsXMLToken( aLocalName, XML_PAUSE ) )
                {
                    Duration aDuration;
                    if (!::sax::Converter::convertDuration(aDuration,  sValue))
                        continue;

                    const sal_Int32 nMS = (aDuration.Hours * 60 +
                            aDuration.Minutes) * 60 + aDuration.Seconds;
                    mpImpl->mxPresProps->setPropertyValue("Pause", Any(nMS) );
                }
                else if( IsXMLToken( aLocalName, XML_ANIMATIONS ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_ENABLED );
                    mpImpl->mxPresProps->setPropertyValue("AllowAnimations", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_STAY_ON_TOP ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_TRUE );
                    mpImpl->mxPresProps->setPropertyValue("IsAlwaysOnTop", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_FORCE_MANUAL ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_TRUE );
                    mpImpl->mxPresProps->setPropertyValue("IsAutomatic", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_ENDLESS ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_TRUE );
                    mpImpl->mxPresProps->setPropertyValue("IsEndless", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_FULL_SCREEN ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_TRUE );
                    mpImpl->mxPresProps->setPropertyValue("IsFullScreen", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_MOUSE_VISIBLE ) )
                {
                    bIsMouseVisible = IsXMLToken( sValue, XML_TRUE );
                }
                else if( IsXMLToken( aLocalName, XML_START_WITH_NAVIGATOR ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_TRUE );
                    mpImpl->mxPresProps->setPropertyValue("StartWithNavigator", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_MOUSE_AS_PEN ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_TRUE );
                    mpImpl->mxPresProps->setPropertyValue("UsePen", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_TRANSITION_ON_CLICK ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_ENABLED );
                    mpImpl->mxPresProps->setPropertyValue("IsTransitionOnClick", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_SHOW_LOGO ) )
                {
                    aAny <<= IsXMLToken( sValue, XML_TRUE );
                    mpImpl->mxPresProps->setPropertyValue("IsShowLogo", aAny );
                }
            }
        }
        mpImpl->mxPresProps->setPropertyValue("IsShowAll", Any(bAll) );
        mpImpl->mxPresProps->setPropertyValue("IsMouseVisible", Any(bIsMouseVisible) );
    }
}

SdXMLShowsContext::~SdXMLShowsContext()
{
    if( mpImpl && !mpImpl->maCustomShowName.isEmpty() )
    {
        uno::Any aAny;
        aAny <<= mpImpl->maCustomShowName;
        mpImpl->mxPresProps->setPropertyValue("CustomShow", aAny );
    }
}

SvXMLImportContextRef SdXMLShowsContext::CreateChildContext( sal_uInt16 p_nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
{
    if( mpImpl && p_nPrefix == XML_NAMESPACE_PRESENTATION && IsXMLToken( rLocalName, XML_SHOW ) )
    {
        OUString aName;
        OUString aPages;

        // read attributes
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
            OUString sValue = xAttrList->getValueByIndex( i );

            switch( nPrefix )
            {
            case XML_NAMESPACE_PRESENTATION:
                if( IsXMLToken( aLocalName, XML_NAME ) )
                {
                    aName = sValue;
                }
                else if( IsXMLToken( aLocalName, XML_PAGES ) )
                {
                    aPages = sValue;
                }
            }
        }

        if( !aName.isEmpty() && !aPages.isEmpty() )
        {
            Reference< XIndexContainer > xShow( mpImpl->mxShowFactory->createInstance(), UNO_QUERY );
            if( xShow.is() )
            {
                SvXMLTokenEnumerator aPageNames( aPages, ',' );
                OUString sPageName;

                while( aPageNames.getNextToken( sPageName ) )
                {
                    if( !mpImpl->mxPages->hasByName( sPageName ) )
                        continue;

                    Reference< XDrawPage > xPage;
                    mpImpl->mxPages->getByName( sPageName ) >>= xPage;
                    if( xPage.is() )
                    {
                        xShow->insertByIndex( xShow->getCount(), Any(xPage) );
                    }
                }

                Any aAny;
                aAny <<= xShow;
                if( mpImpl->mxShows->hasByName( aName ) )
                {
                    mpImpl->mxShows->replaceByName( aName, aAny );
                }
                else
                {
                    mpImpl->mxShows->insertByName( aName, aAny );
                }
            }
        }
    }

    return new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
