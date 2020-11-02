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

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <o3tl/any.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <xmloff/namespacemap.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/prstylei.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>
#include <XMLTextHeaderFooterContext.hxx>
#include <PageMasterImportContext.hxx>
#include <xmloff/xmlimp.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;

Reference < XStyle > XMLTextMasterPageContext::Create()
{
    Reference < XStyle > xNewStyle;

    Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    UNO_QUERY );
    if( xFactory.is() )
    {
        Reference < XInterface > xIfc =
            xFactory->createInstance("com.sun.star.style.PageStyle");
        if( xIfc.is() )
            xNewStyle.set( xIfc, UNO_QUERY );
    }

    return xNewStyle;
}

constexpr OUStringLiteral gsFollowStyle( u"FollowStyle" );

XMLTextMasterPageContext::XMLTextMasterPageContext( SvXMLImport& rImport,
        sal_Int32 /*nElement*/,
        const Reference< XFastAttributeList > & xAttrList,
        bool bOverwrite )
:   SvXMLStyleContext( rImport, XmlStyleFamily::MASTER_PAGE )
,   bInsertHeader( false )
,   bInsertFooter( false )
,   bInsertHeaderLeft( false )
,   bInsertFooterLeft( false )
,   bInsertHeaderFirst( false )
,   bInsertFooterFirst( false )
,   bHeaderInserted( false )
,   bFooterInserted( false )
{
    OUString sName, sDisplayName;
    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        const OUString aValue = aIter.toString();
        switch (aIter.getToken())
        {
            case XML_ELEMENT(STYLE, XML_NAME):
                sName = aValue;
                break;
            case XML_ELEMENT(STYLE, XML_DISPLAY_NAME):
                sDisplayName = aValue;
                break;
            case XML_ELEMENT(STYLE, XML_NEXT_STYLE_NAME):
                sFollow = aValue;
                break;
            case XML_ELEMENT(STYLE, XML_PAGE_LAYOUT_NAME):
                sPageMasterName = aValue;
                break;
            case XML_ELEMENT(DRAW, XML_STYLE_NAME):
                m_sDrawingPageStyle = aValue;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( !sDisplayName.isEmpty() )
    {
        rImport.AddStyleDisplayName( XmlStyleFamily::MASTER_PAGE, sName,
                                     sDisplayName );
    }
    else
    {
        sDisplayName = sName;
    }

    if( sDisplayName.isEmpty() )
        return;

    Reference < XNameContainer > xPageStyles =
            GetImport().GetTextImport()->GetPageStyles();
    if( !xPageStyles.is() )
        return;

    Any aAny;
    bool bNew = false;
    if( xPageStyles->hasByName( sDisplayName ) )
    {
        aAny = xPageStyles->getByName( sDisplayName );
        aAny >>= xStyle;
    }
    else
    {
        xStyle = Create();
        if( !xStyle.is() )
            return;

        xPageStyles->insertByName( sDisplayName, Any(xStyle) );
        bNew = true;
    }

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    OUString sIsPhysical( "IsPhysical" );
    if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        bNew = !*o3tl::doAccess<bool>(aAny);
    }
    SetNew( bNew );

    if( !(bOverwrite || bNew) )
        return;

    Reference < XMultiPropertyStates > xMultiStates( xPropSet,
                                                     UNO_QUERY );
    OSL_ENSURE( xMultiStates.is(),
                "text page style does not support multi property set" );
    if( xMultiStates.is() )
        xMultiStates->setAllPropertiesToDefault();

    if ( xPropSetInfo->hasPropertyByName( "GridDisplay" ) )
        xPropSet->setPropertyValue( "GridDisplay", Any(false) );

    if ( xPropSetInfo->hasPropertyByName( "GridPrint" ) )
        xPropSet->setPropertyValue( "GridPrint", Any(false) );

    bInsertHeader = bInsertFooter = true;
    bInsertHeaderLeft = bInsertFooterLeft = true;
    bInsertHeaderFirst = bInsertFooterFirst = true;
}

XMLTextMasterPageContext::~XMLTextMasterPageContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > XMLTextMasterPageContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;

    bool bInsert = false, bFooter = false, bLeft = false, bFirst = false;
    switch( nElement )
    {
    case XML_ELEMENT(STYLE, XML_HEADER):
        if( bInsertHeader && !bHeaderInserted )
        {
            bInsert = true;
            bHeaderInserted = true;
        }
        break;
    case XML_ELEMENT(STYLE, XML_FOOTER):
        if( bInsertFooter && !bFooterInserted )
        {
            bInsert = bFooter = true;
            bFooterInserted = true;
        }
        break;
    case XML_ELEMENT(STYLE, XML_HEADER_LEFT):
        if( bInsertHeaderLeft && bHeaderInserted )
            bInsert = bLeft = true;
        break;
    case XML_ELEMENT(STYLE, XML_FOOTER_LEFT):
        if( bInsertFooterLeft && bFooterInserted )
            bInsert = bFooter = bLeft = true;
        break;
    case XML_ELEMENT(LO_EXT, XML_HEADER_FIRST):
    case XML_ELEMENT(STYLE, XML_HEADER_FIRST):
        if( bInsertHeaderFirst && bHeaderInserted )
            bInsert = bFirst = true;
        break;
    case XML_ELEMENT(LO_EXT, XML_FOOTER_FIRST):
    case XML_ELEMENT(STYLE, XML_FOOTER_FIRST):
        if( bInsertFooterFirst && bFooterInserted )
            bInsert = bFooter = bFirst = true;
        break;
    }

    if( bInsert && xStyle.is() )
    {
        xContext = CreateHeaderFooterContext( nElement, xAttrList,
                                              bFooter, bLeft, bFirst );
    }

    return xContext.get();
}

SvXMLImportContext *XMLTextMasterPageContext::CreateHeaderFooterContext(
            sal_Int32 /*nElement*/,
            const css::uno::Reference< css::xml::sax::XFastAttributeList > & /*xAttrList*/,
            const bool bFooter,
            const bool bLeft,
            const bool bFirst )
{
    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    return new XMLTextHeaderFooterContext( GetImport(), xPropSet, bFooter, bLeft, bFirst );
}

void XMLTextMasterPageContext::Finish( bool bOverwrite )
{
    if( !(xStyle.is() && (IsNew() || bOverwrite)) )
        return;

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    XMLPropStyleContext * pDrawingPageStyle(nullptr);
    if (!m_sDrawingPageStyle.isEmpty())
    {
        pDrawingPageStyle = GetImport().GetTextImport()->FindDrawingPage(m_sDrawingPageStyle);
    }
    PageStyleContext * pPageLayout(nullptr);
    if( !sPageMasterName.isEmpty() )
    {
        pPageLayout = static_cast<PageStyleContext *>(GetImport().GetTextImport()->FindPageMaster(sPageMasterName));
    }
    if (pPageLayout)
    {
        pPageLayout->FillPropertySet_PageStyle(xPropSet, pDrawingPageStyle);
    }
    else if (pDrawingPageStyle)
    {
        // don't need to care about old background attributes in this case
        pDrawingPageStyle->FillPropertySet(xPropSet);
    }

    Reference < XNameContainer > xPageStyles =
        GetImport().GetTextImport()->GetPageStyles();
    if( !xPageStyles.is() )
        return;

    Reference< XPropertySetInfo > xPropSetInfo =
        xPropSet->getPropertySetInfo();
    if( xPropSetInfo->hasPropertyByName( gsFollowStyle ) )
    {
        OUString sDisplayFollow(
            GetImport().GetStyleDisplayName(
                    XmlStyleFamily::MASTER_PAGE, sFollow ) );
        if( sDisplayFollow.isEmpty() ||
            !xPageStyles->hasByName( sDisplayFollow ) )
            sDisplayFollow = xStyle->getName();

        Any aAny = xPropSet->getPropertyValue( gsFollowStyle );
        OUString sCurrFollow;
        aAny >>= sCurrFollow;
        if( sCurrFollow != sDisplayFollow )
        {
            xPropSet->setPropertyValue( gsFollowStyle, Any(sDisplayFollow) );
        }
    }

    if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
    {
        xPropSet->setPropertyValue( "Hidden", uno::makeAny( IsHidden( ) ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
