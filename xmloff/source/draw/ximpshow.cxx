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

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/Duration.hpp>
#include <com/sun/star/presentation/XCustomPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmluconv.hxx>
#include "ximpshow.hxx"

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

SdXMLShowsContext::SdXMLShowsContext( SdXMLImport& rImport, const Reference< XFastAttributeList >& xAttrList )
:   SvXMLImportContext(rImport)
{

    Reference< XCustomPresentationSupplier > xShowsSupplier( rImport.GetModel(), UNO_QUERY );
    if( xShowsSupplier.is() )
    {
        mxShows = xShowsSupplier->getCustomPresentations();
        mxShowFactory.set( mxShows, UNO_QUERY );
    }

    Reference< XDrawPagesSupplier > xDrawPagesSupplier( rImport.GetModel(), UNO_QUERY );
    if( xDrawPagesSupplier.is() )
        mxPages.set( xDrawPagesSupplier->getDrawPages(), UNO_QUERY );

    Reference< XPresentationSupplier > xPresentationSupplier( rImport.GetModel(), UNO_QUERY );
    if( xPresentationSupplier.is() )
        mxPresProps.set( xPresentationSupplier->getPresentation(), UNO_QUERY );

    if( !mxPresProps.is() )
        return;

    bool bAll = true;
    uno::Any aAny;
    // Per ODF this is default, but we did it wrong before LO 6.0 (tdf#108824)
    bool bIsMouseVisible = true;
    if (rImport.getGeneratorVersion() < SvXMLImport::LO_6x)
        bIsMouseVisible = false;

    // read attributes
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        switch( aIter.getToken() )
        {
            case XML_ELEMENT(PRESENTATION, XML_START_PAGE):
            {
                mxPresProps->setPropertyValue(u"FirstPage"_ustr, Any(aIter.toString()) );
                bAll = false;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_SHOW):
            {
                maCustomShowName = aIter.toString();
                bAll = false;
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_PAUSE):
            {
                Duration aDuration;
                if (!::sax::Converter::convertDuration(aDuration, aIter.toView()))
                    continue;

                const sal_Int32 nMS = (aDuration.Hours * 60 +
                        aDuration.Minutes) * 60 + aDuration.Seconds;
                mxPresProps->setPropertyValue(u"Pause"_ustr, Any(nMS) );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_ANIMATIONS):
            {
                aAny <<= IsXMLToken( aIter, XML_ENABLED );
                mxPresProps->setPropertyValue(u"AllowAnimations"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_STAY_ON_TOP):
            {
                aAny <<= IsXMLToken( aIter, XML_TRUE );
                mxPresProps->setPropertyValue(u"IsAlwaysOnTop"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_FORCE_MANUAL):
            {
                aAny <<= IsXMLToken( aIter, XML_TRUE );
                mxPresProps->setPropertyValue(u"IsAutomatic"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_ENDLESS):
            {
                aAny <<= IsXMLToken( aIter, XML_TRUE );
                mxPresProps->setPropertyValue(u"IsEndless"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_FULL_SCREEN):
            {
                aAny <<= IsXMLToken( aIter, XML_TRUE );
                mxPresProps->setPropertyValue(u"IsFullScreen"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_MOUSE_VISIBLE):
            {
                bIsMouseVisible = IsXMLToken( aIter, XML_TRUE );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_START_WITH_NAVIGATOR):
            {
                aAny <<= IsXMLToken( aIter, XML_TRUE );
                mxPresProps->setPropertyValue(u"StartWithNavigator"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_MOUSE_AS_PEN):
            {
                aAny <<= IsXMLToken( aIter, XML_TRUE );
                mxPresProps->setPropertyValue(u"UsePen"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_TRANSITION_ON_CLICK):
            {
                aAny <<= IsXMLToken( aIter, XML_ENABLED );
                mxPresProps->setPropertyValue(u"IsTransitionOnClick"_ustr, aAny );
                break;
            }
            case XML_ELEMENT(PRESENTATION, XML_SHOW_LOGO):
            {
                aAny <<= IsXMLToken( aIter, XML_TRUE );
                mxPresProps->setPropertyValue(u"IsShowLogo"_ustr, aAny );
                break;
            }
        }
    }
    mxPresProps->setPropertyValue(u"IsShowAll"_ustr, Any(bAll) );
    mxPresProps->setPropertyValue(u"IsMouseVisible"_ustr, Any(bIsMouseVisible) );
}

SdXMLShowsContext::~SdXMLShowsContext()
{
    if( !maCustomShowName.isEmpty() )
    {
        uno::Any aAny;
        aAny <<= maCustomShowName;
        mxPresProps->setPropertyValue(u"CustomShow"_ustr, aAny );
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLShowsContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(PRESENTATION, XML_SHOW) )
    {
        OUString aName;
        OUString aPages;

        // read attributes
        for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
        {
            OUString sValue = aIter.toString();

            switch( aIter.getToken() )
            {
                case XML_ELEMENT(PRESENTATION, XML_NAME):
                    aName = sValue;
                    break;
                case XML_ELEMENT(PRESENTATION, XML_PAGES):
                    aPages = sValue;
                    break;
            }
        }

        if( !aName.isEmpty() && !aPages.isEmpty() )
        {
            Reference< XIndexContainer > xShow( mxShowFactory->createInstance(), UNO_QUERY );
            if( xShow.is() )
            {
                SvXMLTokenEnumerator aPageNames( aPages, ',' );
                std::u16string_view sPageNameView;

                while( aPageNames.getNextToken( sPageNameView ) )
                {
                    OUString sPageName(sPageNameView);
                    if( !mxPages->hasByName( sPageName ) )
                        continue;

                    Reference< XDrawPage > xPage;
                    mxPages->getByName( sPageName ) >>= xPage;
                    if( xPage.is() )
                    {
                        xShow->insertByIndex( xShow->getCount(), Any(xPage) );
                    }
                }

                Any aAny;
                aAny <<= xShow;
                if( mxShows->hasByName( aName ) )
                {
                    mxShows->replaceByName( aName, aAny );
                }
                else
                {
                    mxShows->insertByName( aName, aAny );
                }
            }
        }
    }

    return nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
