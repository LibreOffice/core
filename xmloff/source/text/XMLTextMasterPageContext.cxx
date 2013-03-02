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
#include <xmloff/nmspmap.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/XMLTextMasterPageContext.hxx>
#include "XMLTextHeaderFooterContext.hxx"
#include <xmloff/xmlimp.hxx>
#include "PageMasterImportContext.hxx"


using ::rtl::OUString;
using ::rtl::OUStringBuffer;

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
            xFactory->createInstance(OUString("com.sun.star.style.PageStyle"));
        if( xIfc.is() )
            xNewStyle = Reference < XStyle >( xIfc, UNO_QUERY );
    }

    return xNewStyle;
}
TYPEINIT1( XMLTextMasterPageContext, SvXMLStyleContext );

XMLTextMasterPageContext::XMLTextMasterPageContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        sal_Bool bOverwrite )
:   SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, XML_STYLE_FAMILY_MASTER_PAGE )
,   sIsPhysical( "IsPhysical" )
,   sPageStyleLayout( "PageStyleLayout" )
,   sFollowStyle( "FollowStyle" )
,   bInsertHeader( sal_False )
,   bInsertFooter( sal_False )
,   bInsertHeaderLeft( sal_False )
,   bInsertFooterLeft( sal_False )
,   bInsertHeaderFirst( sal_False )
,   bInsertFooterFirst( sal_False )
,   bHeaderInserted( sal_False )
,   bFooterInserted( sal_False )
,   bHeaderLeftInserted( sal_False )
,   bFooterLeftInserted( sal_False )
,   bHeaderFirstInserted( sal_False )
,   bFooterFirstInserted( sal_False )
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
    sal_Bool bNew = sal_False;
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

        aAny <<= xStyle;
        xPageStyles->insertByName( sDisplayName, aAny );
        bNew = sal_True;
    }

    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    Reference< XPropertySetInfo > xPropSetInfo =
                xPropSet->getPropertySetInfo();
    if( !bNew && xPropSetInfo->hasPropertyByName( sIsPhysical ) )
    {
        aAny = xPropSet->getPropertyValue( sIsPhysical );
        bNew = !*(sal_Bool *)aAny.getValue();
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

        bInsertHeader = bInsertFooter = sal_True;
        bInsertHeaderLeft = bInsertFooterLeft = sal_True;
        bInsertHeaderFirst = bInsertFooterFirst = sal_True;
    }
}

XMLTextMasterPageContext::~XMLTextMasterPageContext()
{
}

SvXMLImportContext *XMLTextMasterPageContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap =
        GetImport().GetTextImport()->GetTextMasterPageElemTokenMap();

    sal_Bool bInsert = sal_False, bFooter = sal_False, bLeft = sal_False, bFirst = sal_False;
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
    case XML_TOK_TEXT_MP_HEADER:
        if( bInsertHeader && !bHeaderInserted )
        {
            bInsert = sal_True;
            bHeaderInserted = sal_True;
        }
        break;
    case XML_TOK_TEXT_MP_FOOTER:
        if( bInsertFooter && !bFooterInserted )
        {
            bInsert = bFooter = sal_True;
            bFooterInserted = sal_True;
        }
        break;
    case XML_TOK_TEXT_MP_HEADER_LEFT:
        if( bInsertHeaderLeft && bHeaderInserted && !bHeaderLeftInserted )
            bInsert = bLeft = sal_True;
        break;
    case XML_TOK_TEXT_MP_FOOTER_LEFT:
        if( bInsertFooterLeft && bFooterInserted && !bFooterLeftInserted )
            bInsert = bFooter = bLeft = sal_True;
        break;
    case XML_TOK_TEXT_MP_HEADER_FIRST:
        if( bInsertHeaderFirst && bHeaderInserted && !bHeaderFirstInserted )
            bInsert = bFirst = sal_True;
        break;
    case XML_TOK_TEXT_MP_FOOTER_FIRST:
        if( bInsertFooterFirst && bFooterInserted && !bFooterFirstInserted )
            bInsert = bFooter = bFirst = sal_True;
        break;
    }

    if( bInsert && xStyle.is() )
    {
        pContext = CreateHeaderFooterContext( nPrefix, rLocalName,
                                                    xAttrList,
                                                    bFooter, bLeft, bFirst );
    }
    else
    {
        pContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );
    }

    return pContext;
}

SvXMLImportContext *XMLTextMasterPageContext::CreateHeaderFooterContext(
            sal_uInt16 nPrefix,
            const ::rtl::OUString& rLocalName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > & xAttrList,
            const sal_Bool bFooter,
            const sal_Bool bLeft,
            const sal_Bool bFirst )
{
    Reference < XPropertySet > xPropSet( xStyle, UNO_QUERY );
    return new XMLTextHeaderFooterContext( GetImport(),
                                                nPrefix, rLocalName,
                                                xAttrList,
                                                xPropSet,
                                                bFooter, bLeft, bFirst );
}

void XMLTextMasterPageContext::Finish( sal_Bool bOverwrite )
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
        if( xPropSetInfo->hasPropertyByName( sFollowStyle ) )
        {
            OUString sDisplayFollow(
                GetImport().GetStyleDisplayName(
                        XML_STYLE_FAMILY_MASTER_PAGE, sFollow ) );
            if( sDisplayFollow.isEmpty() ||
                !xPageStyles->hasByName( sDisplayFollow ) )
                sDisplayFollow = xStyle->getName();

            Any aAny = xPropSet->getPropertyValue( sFollowStyle );
            OUString sCurrFollow;
            aAny >>= sCurrFollow;
            if( sCurrFollow != sDisplayFollow )
            {
                aAny <<= sDisplayFollow;
                xPropSet->setPropertyValue( sFollowStyle, aAny );
            }
        }

        if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
        {
            xPropSet->setPropertyValue( "Hidden", uno::makeAny( IsHidden( ) ) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
