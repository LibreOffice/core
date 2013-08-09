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

#include <tools/debug.hxx>
#include <xmloff/XMLShapeStyleContext.hxx>
#include "XMLShapePropertySetContext.hxx"
#include <xmloff/contextid.hxx>
#include <com/sun/star/drawing/XControlShape.hpp>
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include "xmloff/xmlerror.hxx"
#include <xmloff/maptype.hxx>

#include "sdpropls.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_TEXT_PROPERTIES;
using ::xmloff::token::XML_GRAPHIC_PROPERTIES;
using ::xmloff::token::XML_PARAGRAPH_PROPERTIES;

TYPEINIT1( XMLShapeStyleContext, XMLPropStyleContext );

XMLShapeStyleContext::XMLShapeStyleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    SvXMLStylesContext& rStyles,
    sal_uInt16 nFamily)
:   XMLPropStyleContext(rImport, nPrfx, rLName, xAttrList, rStyles, nFamily ),
    m_bIsNumRuleAlreadyConverted( sal_False )
{
}

XMLShapeStyleContext::~XMLShapeStyleContext()
{
}

void XMLShapeStyleContext::SetAttribute( sal_uInt16 nPrefixKey, const OUString& rLocalName, const OUString& rValue )
{
    if (m_sControlDataStyleName.isEmpty() && (::xmloff::token::GetXMLToken(::xmloff::token::XML_DATA_STYLE_NAME) == rLocalName))
    {
        m_sControlDataStyleName = rValue;
    }
    else if( (XML_NAMESPACE_STYLE == nPrefixKey) && IsXMLToken( rLocalName, ::xmloff::token::XML_LIST_STYLE_NAME ) )
    {
        m_sListStyleName = rValue;
    }
    else
    {
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );

        if( (XML_NAMESPACE_STYLE == nPrefixKey) &&
            ( IsXMLToken( rLocalName, ::xmloff::token::XML_NAME ) || IsXMLToken( rLocalName, ::xmloff::token::XML_DISPLAY_NAME ) ) )
        {
            if( !GetName().isEmpty() && !GetDisplayName().isEmpty() && GetName() != GetDisplayName() )
            {
                const_cast< SvXMLImport&>( GetImport() ).
                    AddStyleDisplayName( GetFamily(), GetName(), GetDisplayName() );
            }
        }
    }
}

SvXMLImportContext *XMLShapeStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;

    if( XML_NAMESPACE_STYLE == nPrefix )
    {
        sal_uInt32 nFamily = 0;
        if( IsXMLToken( rLocalName, XML_TEXT_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_TEXT;
        else if( IsXMLToken( rLocalName, XML_PARAGRAPH_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_PARAGRAPH;
        else if( IsXMLToken( rLocalName, XML_GRAPHIC_PROPERTIES ) )
            nFamily = XML_TYPE_PROP_GRAPHIC;
        if( nFamily )
        {
            UniReference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                pContext = new XMLShapePropertySetContext( GetImport(), nPrefix,
                                                        rLocalName, xAttrList,
                                                        nFamily,
                                                        GetProperties(),
                                                        xImpPrMap );
        }
    }

    if( !pContext )
        pContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return pContext;
}

void XMLShapeStyleContext::FillPropertySet( const Reference< beans::XPropertySet > & rPropSet )
{
    if( !m_bIsNumRuleAlreadyConverted )
    {
        m_bIsNumRuleAlreadyConverted = sal_True;

        // for compatibility to beta files, search for CTF_SD_NUMBERINGRULES_NAME to
        // import numbering rules from the style:properties element
        const UniReference< XMLPropertySetMapper >&rMapper = GetStyles()->GetImportPropertyMapper( GetFamily() )->getPropertySetMapper();

        ::std::vector< XMLPropertyState > &rProperties = GetProperties();
        ::std::vector< XMLPropertyState >::iterator end( rProperties.end() );
        ::std::vector< XMLPropertyState >::iterator property;

        // first, look for the old format, where we had a text:list-style-name
        // attribute in the style:properties element
        for( property = rProperties.begin(); property != end; ++property )
        {
            // find properties with context
            if( (property->mnIndex != -1) && (rMapper->GetEntryContextId( property->mnIndex ) == CTF_SD_NUMBERINGRULES_NAME) )
                break;
        }

        // if we did not find an old list-style-name in the properties, and we need one
        // because we got a style:list-style attribute in the style-style element
        // we generate one
        if( (property == end) && ( !m_sListStyleName.isEmpty() ) )
        {
            sal_Int32 nIndex = rMapper->FindEntryIndex( CTF_SD_NUMBERINGRULES_NAME );
            DBG_ASSERT( -1 != nIndex, "can't find numbering rules property entry, can't set numbering rule!" );

            XMLPropertyState aNewState( nIndex );
            rProperties.push_back( aNewState );
            end = rProperties.end();
            property = end - 1;
        }

        // so, if we have an old or a new list style name, we set its value to
        // a numbering rule
        if( property != end )
        {
            if( m_sListStyleName.isEmpty() )
            {
                property->maValue >>= m_sListStyleName;
            }

            const SvxXMLListStyleContext *pListStyle = GetImport().GetTextImport()->FindAutoListStyle( m_sListStyleName );

            DBG_ASSERT( pListStyle, "list-style not found for shape style" );
            if( pListStyle )
            {
                uno::Reference< container::XIndexReplace > xNumRule( pListStyle->CreateNumRule( GetImport().GetModel() ) );
                pListStyle->FillUnoNumRule(xNumRule, NULL /* const SvI18NMap * ??? */ );
                property->maValue <<= xNumRule;
            }
            else
            {
                property->mnIndex = -1;
            }
        }
    }

    struct _ContextID_Index_Pair aContextIDs[] =
    {
        { CTF_DASHNAME , -1 },
        { CTF_LINESTARTNAME , -1 },
        { CTF_LINEENDNAME , -1 },
        { CTF_FILLGRADIENTNAME, -1 },
        { CTF_FILLTRANSNAME , -1 },
        { CTF_FILLHATCHNAME , -1 },
        { CTF_FILLBITMAPNAME , -1 },
        { CTF_SD_OLE_VIS_AREA_IMPORT_LEFT, -1 },
        { CTF_SD_OLE_VIS_AREA_IMPORT_TOP, -1 },
        { CTF_SD_OLE_VIS_AREA_IMPORT_WIDTH, -1 },
        { CTF_SD_OLE_VIS_AREA_IMPORT_HEIGHT, -1 },
        { -1, -1 }
    };
    static const sal_uInt16 aFamilies[] =
    {
        XML_STYLE_FAMILY_SD_STROKE_DASH_ID,
        XML_STYLE_FAMILY_SD_MARKER_ID,
        XML_STYLE_FAMILY_SD_MARKER_ID,
        XML_STYLE_FAMILY_SD_GRADIENT_ID,
        XML_STYLE_FAMILY_SD_GRADIENT_ID,
        XML_STYLE_FAMILY_SD_HATCH_ID,
        XML_STYLE_FAMILY_SD_FILL_IMAGE_ID
    };

    UniReference < SvXMLImportPropertyMapper > xImpPrMap =
        GetStyles()->GetImportPropertyMapper( GetFamily() );
    DBG_ASSERT( xImpPrMap.is(), "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xImpPrMap->FillPropertySet( GetProperties(), rPropSet, aContextIDs );

    Reference< XPropertySetInfo > xInfo;
    // get property set mapper
    UniReference<XMLPropertySetMapper> xPropMapper( xImpPrMap->getPropertySetMapper() );

    for( sal_uInt16 i=0; aContextIDs[i].nContextID != -1; i++ )
    {
        sal_Int32 nIndex = aContextIDs[i].nIndex;
        if( nIndex != -1 ) switch( aContextIDs[i].nContextID )
        {
        case CTF_DASHNAME:
        case CTF_LINESTARTNAME:
        case CTF_LINEENDNAME:
        case CTF_FILLGRADIENTNAME:
        case CTF_FILLTRANSNAME:
        case CTF_FILLHATCHNAME:
        case CTF_FILLBITMAPNAME:
        {
            struct XMLPropertyState& rState = GetProperties()[nIndex];
            OUString sStyleName;
            rState.maValue >>= sStyleName;
            sStyleName = GetImport().GetStyleDisplayName( aFamilies[i], sStyleName );
            try
            {

                // set property
                const OUString& rPropertyName = xPropMapper->GetEntryAPIName(rState.mnIndex);
                if( !xInfo.is() )
                    xInfo = rPropSet->getPropertySetInfo();
                if ( xInfo->hasPropertyByName( rPropertyName ) )
                {
                    rPropSet->setPropertyValue( rPropertyName, Any( sStyleName ) );
                }
            }
            catch ( const ::com::sun::star::lang::IllegalArgumentException& e )
            {
                Sequence<OUString> aSeq(1);
                aSeq[0] = sStyleName;
                GetImport().SetError(
                    XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_WARNING,
                    aSeq, e.Message, NULL );
            }
            break;
        }
        case CTF_SD_OLE_VIS_AREA_IMPORT_LEFT:
        case CTF_SD_OLE_VIS_AREA_IMPORT_TOP:
        case CTF_SD_OLE_VIS_AREA_IMPORT_WIDTH:
        case CTF_SD_OLE_VIS_AREA_IMPORT_HEIGHT:
        {
            struct XMLPropertyState& rState = GetProperties()[nIndex];
            const OUString& rPropertyName = xPropMapper->GetEntryAPIName(rState.mnIndex);
            try
            {
                if( !xInfo.is() )
                    xInfo = rPropSet->getPropertySetInfo();
                if ( xInfo->hasPropertyByName( rPropertyName ) )
                {
                    rPropSet->setPropertyValue( rPropertyName, rState.maValue );
                }
            }
            catch ( const ::com::sun::star::lang::IllegalArgumentException& e )
            {
                Sequence<OUString> aSeq;
                GetImport().SetError(
                    XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_WARNING,
                    aSeq, e.Message, NULL );
            }
            break;
        }
        }
    }

    if (!m_sControlDataStyleName.isEmpty())
    {   // we had a data-style-name attribute

        // set the formatting on the control model of the control shape
        uno::Reference< drawing::XControlShape > xControlShape(rPropSet, uno::UNO_QUERY);
        DBG_ASSERT(xControlShape.is(), "XMLShapeStyleContext::FillPropertySet: data style for a non-control shape!");
        if (xControlShape.is())
        {
            uno::Reference< beans::XPropertySet > xControlModel(xControlShape->getControl(), uno::UNO_QUERY);
            DBG_ASSERT(xControlModel.is(), "XMLShapeStyleContext::FillPropertySet: no control model for the shape!");
            if (xControlModel.is())
            {
                GetImport().GetFormImport()->applyControlNumberStyle(xControlModel, m_sControlDataStyleName);
            }
        }
    }
}

void XMLShapeStyleContext::Finish( sal_Bool /*bOverwrite*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
