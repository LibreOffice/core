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

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/style/PageStyleLayout.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <o3tl/any.hxx>
#include <osl/diagnose.h>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>
#include <XMLTextHeaderFooterContext.hxx>
#include <xmloff/xmlimp.hxx>
#include <PageMasterImportContext.hxx>


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

static const OUStringLiteral gsFollowStyle( "FollowStyle" );

XMLTextMasterPageContext::XMLTextMasterPageContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        bool bOverwrite )
:   SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_MASTER_PAGE )
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
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i=0; i < nAttrCount; i++ )
    {
        const OUString& rAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( rAttrName, &aLocalName );
        if( XML_NAMESPACE_STYLE == nPrefix )
        {
            if( IsXMLToken( aLocalName, XML_NAME ) )
            {
                sName = xAttrList->getValueByIndex( i );
            }
            else if( IsXMLToken( aLocalName, XML_DISPLAY_NAME ) )
            {
                sDisplayName = xAttrList->getValueByIndex( i );
            }
            else if( IsXMLToken( aLocalName, XML_NEXT_STYLE_NAME ) )
            {
                sFollow = xAttrList->getValueByIndex( i );
            }
            else if( IsXMLToken( aLocalName, XML_PAGE_LAYOUT_NAME ) )
            {
                sPageMasterName = xAttrList->getValueByIndex( i );
            }
        }
    }

    if( !sDisplayName.isEmpty() )
    {
        rImport.AddStyleDisplayName( XML_STYLE_FAMILY_MASTER_PAGE, sName,
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

    if( bOverwrite || bNew )
    {
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
}

XMLTextMasterPageContext::~XMLTextMasterPageContext()
{
}

SvXMLImportContextRef XMLTextMasterPageContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext;

    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextMasterPageElemTokenMap();

    bool bInsert = false, bFooter = false, bLeft = false, bFirst = false;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_MP_HEADER:
        if( bInsertHeader && !bHeaderInserted )
        {
            bInsert = true;
            bHeaderInserted = true;
        }
        break;
    case XML_TOK_TEXT_MP_FOOTER:
        if( bInsertFooter && !bFooterInserted )
        {
            bInsert = bFooter = true;
            bFooterInserted = true;
        }
        break;
    case XML_TOK_TEXT_MP_HEADER_LEFT:
        if( bInsertHeaderLeft && bHeaderInserted )
            bInsert = bLeft = true;
        break;
    case XML_TOK_TEXT_MP_FOOTER_LEFT:
        if( bInsertFooterLeft && bFooterInserted )
            bInsert = bFooter = bLeft = true;
        break;
    case XML_TOK_TEXT_MP_HEADER_FIRST:
        if( bInsertHeaderFirst && bHeaderInserted )
            bInsert = bFirst = true;
        break;
    case XML_TOK_TEXT_MP_FOOTER_FIRST:
        if( bInsertFooterFirst && bFooterInserted )
            bInsert = bFooter = bFirst = true;
        break;
    }

    if( bInsert && xStyle.is() )
    {
        xContext = CreateHeaderFooterContext( nPrefix, rLocalName,
                                                    xAttrList,
                                                    bFooter, bLeft, bFirst );
    }
    else
    {
        xContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );
    }

    return xContext;
}

SvXMLImportContext *XMLTextMasterPageContext::CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const OUString& rLocalName,
            const css::uno::Reference< css::xml::sax::XAttributeList > & xAttrList,
            const bool bFooter,
            const bool bLeft,
            const bool bFirst )
{
    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    return new XMLTextHeaderFooterContext( GetImport(),
                                                nPrefix, rLocalName,
                                                xAttrList,
                                                xPropSet,
                                                bFooter, bLeft, bFirst );
}

void XMLTextMasterPageContext::Finish( bool bOverwrite )
{
    if( xStyle.is() && (IsNew() || bOverwrite) )
    {
        Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
        if( !sPageMasterName.isEmpty() )
        {
            XMLPropStyleContext* pStyle =
                GetImport().GetTextImport()->FindPageMaster( sPageMasterName );
            if (pStyle)
            {
                pStyle->FillPropertySet(xPropSet);
            }
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
                        XML_STYLE_FAMILY_MASTER_PAGE, sFollow ) );
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
