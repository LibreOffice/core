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
#include <comphelper/extract.hxx>
#include "xmloff/xmlnmspe.hxx"
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
    SdXMLImport& mrImport;

    ShowsImpImpl( SdXMLImport& rImport )
    :   mrImport( rImport )
    {}
};

TYPEINIT1( SdXMLShowsContext, SvXMLImportContext );

SdXMLShowsContext::SdXMLShowsContext( SdXMLImport& rImport,  sal_uInt16 nPrfx, const OUString& rLocalName,  const Reference< XAttributeList >& xAttrList )
:   SvXMLImportContext(rImport, nPrfx, rLocalName)
{
    mpImpl = new ShowsImpImpl( rImport );

    Reference< XCustomPresentationSupplier > xShowsSupplier( rImport.GetModel(), UNO_QUERY );
    if( xShowsSupplier.is() )
    {
        mpImpl->mxShows = xShowsSupplier->getCustomPresentations();
        mpImpl->mxShowFactory = Reference< XSingleServiceFactory >::query( mpImpl->mxShows );
    }

    Reference< XDrawPagesSupplier > xDrawPagesSupplier( rImport.GetModel(), UNO_QUERY );
    if( xDrawPagesSupplier.is() )
        mpImpl->mxPages = Reference< XNameAccess >::query( xDrawPagesSupplier->getDrawPages() );

    Reference< XPresentationSupplier > xPresentationSupplier( rImport.GetModel(), UNO_QUERY );
    if( xPresentationSupplier.is() )
        mpImpl->mxPresProps = Reference< XPropertySet >::query( xPresentationSupplier->getPresentation() );

    if( mpImpl->mxPresProps.is() )
    {
        sal_Bool bAll = sal_True;
        uno::Any aAny;

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
                    aAny <<= sValue;
                    mpImpl->mxPresProps->setPropertyValue("FirstPage", aAny );
                    bAll = sal_False;
                }
                else if( IsXMLToken( aLocalName, XML_SHOW ) )
                {
                    mpImpl->maCustomShowName = sValue;
                    bAll = sal_False;
                }
                else if( IsXMLToken( aLocalName, XML_PAUSE ) )
                {
                    Duration aDuration;
                    if (!::sax::Converter::convertDuration(aDuration,  sValue))
                        continue;

                    const sal_Int32 nMS = (aDuration.Hours * 60 +
                            aDuration.Minutes) * 60 + aDuration.Seconds;
                    aAny <<= nMS;
                    mpImpl->mxPresProps->setPropertyValue("Pause", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_ANIMATIONS ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_ENABLED ) );
                    mpImpl->mxPresProps->setPropertyValue("AllowAnimations", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_STAY_ON_TOP ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("IsAlwaysOnTop", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_FORCE_MANUAL ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("IsAutomatic", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_ENDLESS ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("IsEndless", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_FULL_SCREEN ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("IsFullScreen", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_MOUSE_VISIBLE ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("IsMouseVisible", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_START_WITH_NAVIGATOR ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("StartWithNavigator", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_MOUSE_AS_PEN ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("UsePen", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_TRANSITION_ON_CLICK ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_ENABLED ) );
                    mpImpl->mxPresProps->setPropertyValue("IsTransitionOnClick", aAny );
                }
                else if( IsXMLToken( aLocalName, XML_SHOW_LOGO ) )
                {
                    aAny = bool2any( IsXMLToken( sValue, XML_TRUE ) );
                    mpImpl->mxPresProps->setPropertyValue("IsShowLogo", aAny );
                }
            }
        }
        aAny = bool2any( bAll );
        mpImpl->mxPresProps->setPropertyValue("IsShowAll", aAny );
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

    delete mpImpl;
}

SvXMLImportContext * SdXMLShowsContext::CreateChildContext( sal_uInt16 p_nPrefix, const OUString& rLocalName, const Reference< XAttributeList>& xAttrList )
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
                SvXMLTokenEnumerator aPageNames( aPages, sal_Unicode(',') );
                OUString sPageName;
                Any aAny;

                while( aPageNames.getNextToken( sPageName ) )
                {
                    if( !mpImpl->mxPages->hasByName( sPageName ) )
                        continue;

                    Reference< XDrawPage > xPage;
                    mpImpl->mxPages->getByName( sPageName ) >>= xPage;
                    if( xPage.is() )
                    {
                        aAny <<= xPage;
                        xShow->insertByIndex( xShow->getCount(), aAny );
                    }
                }

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
