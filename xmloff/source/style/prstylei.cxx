/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <tools/debug.hxx>
#include <set>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlprcon.hxx>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XAutoStyleFamily.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <xmloff/xmlimp.hxx>

#include <xmloff/prstylei.hxx>
#include <xmloff/attrlist.hxx>
#include <xmloff/xmlerror.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::xmloff::token;


void XMLPropStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    if( XML_NAMESPACE_STYLE == nPrefixKey && IsXMLToken( rLocalName, XML_FAMILY ) )
    {
        DBG_ASSERT( GetFamily() == ((SvXMLStylesContext *)&mxStyles)->GetFamily( rValue ), "unexpected style family" );
    }
    else
    {
        SvXMLStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
    }
}

TYPEINIT1( XMLPropStyleContext, SvXMLStyleContext );

XMLPropStyleContext::XMLPropStyleContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily,
        sal_Bool bDefault )
:   SvXMLStyleContext( rImport, nPrfx, rLName, xAttrList, nFamily, bDefault )
,   msIsPhysical(  "IsPhysical"  )
,   msFollowStyle(  "FollowStyle"  )
,   mxStyles( &rStyles )
{
}

XMLPropStyleContext::~XMLPropStyleContext()
{
}

SvXMLImportContext *XMLPropStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    sal_uInt32 nFamily = 0;
    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        else if( IsXMLToken( rLocalName, XML_DRAWING_PAGE_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_DRAWING_PAGE;
        else if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_RUBY_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_RUBY;
        else if( IsXMLToken( rLocalName, XML_SECTION_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_SECTION;
        else if( IsXMLToken( rLocalName, XML_TABLE_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE;
        else if( IsXMLToken( rLocalName, XML_TABLE_COLUMN_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_COLUMN;
        else if( IsXMLToken( rLocalName, XML_TABLE_ROW_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_ROW;
        else if( IsXMLToken( rLocalName, XML_TABLE_CELL_PROPERTIES )  )
            nFamily = XML_TYPE_PROP_TABLE_CELL;
        else if( IsXMLToken( rLocalName, XML_CHART_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_CHART;
    }
    if( nFamily )
    {
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            ((SvXMLStylesContext *)&mxStyles)->GetImportPropertyMapper(
                                                        GetFamily() );
        if( xImpPrMap.is() )
            pContext = new SvXMLPropertySetContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    nFamily,
                                                    maProperties,
                                                    xImpPrMap );
    }

    if( !pContext )
        pContext = SvXMLStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLPropStyleContext::FillPropertySet(
            const Reference< XPropertySet > & rPropSet )
{
    UniReference < SvXMLImportPropertyMapper > xImpPrMap =
        ((SvXMLStylesContext *)&mxStyles)->GetImportPropertyMapper(
                                                                GetFamily() );
    DBG_ASSERT( xImpPrMap.is(), "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xImpPrMap->FillPropertySet( maProperties, rPropSet );
}

void XMLPropStyleContext::SetDefaults()
{
}

Reference < XStyle > XMLPropStyleContext::Create()
{
    Reference < XStyle > xNewStyle;

    OUString sServiceName(
        ((SvXMLStylesContext *)&mxStyles)->GetServiceName( GetFamily() ) );
    if( !sServiceName.isEmpty() )
    {
        Reference< XMultiServiceFactory > xFactory( GetImport().GetModel(),
                                                    UNO_QUERY );
        if( xFactory.is() )
        {
            Reference < XInterface > xIfc =
                xFactory->createInstance( sServiceName );
            if( xIfc.is() )
                xNewStyle = Reference < XStyle >( xIfc, UNO_QUERY );
        }
    }

    return xNewStyle;
}

typedef ::std::set < OUString > PropertyNameSet;

void XMLPropStyleContext::CreateAndInsert( sal_Bool bOverwrite )
{
    if( ((SvXMLStylesContext *)&mxStyles)->IsAutomaticStyle()
        && ( GetFamily() == XML_STYLE_FAMILY_TEXT_TEXT || GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH ) )
    {
        Reference < XAutoStyleFamily > xAutoFamily =
                ((SvXMLStylesContext *)&mxStyles)->GetAutoStyles( GetFamily() );
        if( !xAutoFamily.is() )
            return;
        UniReference < SvXMLImportPropertyMapper > xImpPrMap =
            ((SvXMLStylesContext *)&mxStyles)->GetImportPropertyMapper( GetFamily() );
        DBG_ASSERT( xImpPrMap.is(), "There is no import prop mapper" );
        if( xImpPrMap.is() )
        {
            Sequence< PropertyValue > aValues;
            xImpPrMap->FillPropertySequence( maProperties, aValues );

            sal_Int32 nLen = aValues.getLength();
            if( nLen )
            {
                if( GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH )
                {
                    aValues.realloc( nLen + 2 );
                    PropertyValue *pProps = aValues.getArray() + nLen;
                    pProps->Name = "ParaStyleName";
                    OUString sParent( GetParentName() );
                    if( !sParent.isEmpty() )
                        sParent = GetImport().GetStyleDisplayName( GetFamily(), sParent );
                    else
                        sParent = "Standard";
                    pProps->Value <<= sParent;
                    ++pProps;
                    pProps->Name = "ParaConditionalStyleName";
                    pProps->Value <<= sParent;
                }

                Reference < XAutoStyle > xAutoStyle = xAutoFamily->insertStyle( aValues );
                if( xAutoStyle.is() )
                {
                    Sequence< OUString > aPropNames(1);
                    aPropNames[0] = GetFamily() == XML_STYLE_FAMILY_TEXT_PARAGRAPH ?
                        OUString("ParaAutoStyleName") :
                        OUString("CharAutoStyleName");
                    Sequence< Any > aAny = xAutoStyle->getPropertyValues( aPropNames );
                    if( aAny.hasElements() )
                    {
                        OUString aName;
                        aAny[0] >>= aName;
                        SetAutoName( aName );
                    }
                }
            }
        }
    }
    else
    {
        const OUString& rName = GetDisplayName();
        if( rName.isEmpty() || IsDefaultStyle() )
            return;

        Reference < XNameContainer > xFamilies =
                ((SvXMLStylesContext *)&mxStyles)->GetStylesContainer( GetFamily() );
        if( !xFamilies.is() )
            return;

        sal_Bool bNew = sal_False;
        if( xFamilies->hasByName( rName ) )
        {
            Any aAny = xFamilies->getByName( rName );
            aAny >>= mxStyle;
        }
        else
        {
            mxStyle = Create();
            if( !mxStyle.is() )
                return;

            Any aAny;
            aAny <<= mxStyle;
            xFamilies->insertByName( rName, aAny );
            bNew = sal_True;
        }

        Reference < XPropertySet > xPropSet( mxStyle, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo =
                    xPropSet->getPropertySetInfo();
        if( !bNew && xPropSetInfo->hasPropertyByName( msIsPhysical ) )
        {
            Any aAny = xPropSet->getPropertyValue( msIsPhysical );
            bNew = !*(sal_Bool *)aAny.getValue();
        }
        SetNew( bNew );
        if( rName != GetName() )
            GetImport().AddStyleDisplayName( GetFamily(), GetName(), rName );


        if( bOverwrite || bNew )
        {
            Reference< XPropertyState > xPropState( xPropSet, uno::UNO_QUERY );

            UniReference < XMLPropertySetMapper > xPrMap;
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                ((SvXMLStylesContext *)&mxStyles)->GetImportPropertyMapper(
                                                                    GetFamily() );
            DBG_ASSERT( xImpPrMap.is(), "There is the import prop mapper" );
            if( xImpPrMap.is() )
                xPrMap = xImpPrMap->getPropertySetMapper();
            if( xPrMap.is() )
            {
                Reference < XMultiPropertyStates > xMultiStates( xPropSet,
                                                                 UNO_QUERY );
                if( xMultiStates.is() )
                {
                    xMultiStates->setAllPropertiesToDefault();
                }
                else
                {
                    PropertyNameSet aNameSet;
                    sal_Int32 nCount = xPrMap->GetEntryCount();
                    sal_Int32 i;
                    for( i = 0; i < nCount; i++ )
                    {
                        const OUString& rPrName = xPrMap->GetEntryAPIName( i );
                        if( xPropSetInfo->hasPropertyByName( rPrName ) )
                            aNameSet.insert( rPrName );
                    }

                    nCount = aNameSet.size();
                    Sequence < OUString > aNames( nCount );
                    OUString *pNames = aNames.getArray();
                    PropertyNameSet::iterator aIter = aNameSet.begin();
                    while( aIter != aNameSet.end() )
                        *pNames++ = *aIter++;

                    Sequence < PropertyState > aStates(
                        xPropState->getPropertyStates( aNames ) );
                    const PropertyState *pStates = aStates.getConstArray();
                    pNames = aNames.getArray();

                    for( i = 0; i < nCount; i++ )
                    {
                        if( PropertyState_DIRECT_VALUE == *pStates++ )
                            xPropState->setPropertyToDefault( pNames[i] );
                    }
                }
            }

            if (mxStyle.is())
                mxStyle->setParentStyle(OUString());

            FillPropertySet( xPropSet );
        }
        else
        {
            SetValid( sal_False );
        }
    }
}

void XMLPropStyleContext::Finish( bool bOverwrite )
{
    if( mxStyle.is() && (IsNew() || bOverwrite) )
    {
        
        Reference < XNameContainer > xFamilies =
            ((SvXMLStylesContext *)&mxStyles)->GetStylesContainer( GetFamily() );
        DBG_ASSERT( xFamilies.is(), "Families lost" );
        if( !xFamilies.is() )
            return;

        
        OUString sParent( GetParentName() );
        if( !sParent.isEmpty() )
            sParent = GetImport().GetStyleDisplayName( GetFamily(), sParent );
        if( !sParent.isEmpty() && !xFamilies->hasByName( sParent ) )
            sParent = OUString();

        if( sParent != mxStyle->getParentStyle() )
        {
            
            
            
            try
            {
                mxStyle->setParentStyle( sParent );
            }
            catch(const uno::Exception& e)
            {
                
                
                
                

                
                
                
                Sequence<OUString> aSequence(2);

                
                aSequence[0] = mxStyle->getName();
                aSequence[1] = sParent;

                GetImport().SetError(
                    XMLERROR_FLAG_ERROR | XMLERROR_PARENT_STYLE_NOT_ALLOWED,
                    aSequence, e.Message, NULL );
            }
        }

        
        OUString sFollow( GetFollow() );
        if( !sFollow.isEmpty() )
            sFollow = GetImport().GetStyleDisplayName( GetFamily(), sFollow );
        if( sFollow.isEmpty() || !xFamilies->hasByName( sFollow ) )
            sFollow = mxStyle->getName();

        Reference < XPropertySet > xPropSet( mxStyle, UNO_QUERY );
        Reference< XPropertySetInfo > xPropSetInfo =
            xPropSet->getPropertySetInfo();
        if( xPropSetInfo->hasPropertyByName( msFollowStyle ) )
        {
            Any aAny = xPropSet->getPropertyValue( msFollowStyle );
            OUString sCurrFollow;
            aAny >>= sCurrFollow;
            if( sCurrFollow != sFollow )
            {
                aAny <<= sFollow;
                xPropSet->setPropertyValue( msFollowStyle, aAny );
            }
        }

        if ( xPropSetInfo->hasPropertyByName( "Hidden" ) )
        {
            xPropSet->setPropertyValue( "Hidden", uno::makeAny( IsHidden( ) ) );
        }

    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
