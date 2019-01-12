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
#include <sal/log.hxx>
#include <xmloff/XMLShapeStyleContext.hxx>
#include <XMLShapePropertySetContext.hxx>
#include <xmloff/contextid.hxx>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnumi.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>

#include "sdpropls.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_TEXT_PROPERTIES;
using ::xmloff::token::XML_GRAPHIC_PROPERTIES;
using ::xmloff::token::XML_PARAGRAPH_PROPERTIES;


XMLShapeStyleContext::XMLShapeStyleContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    SvXMLStylesContext& rStyles,
    sal_uInt16 nFamily)
:   XMLPropStyleContext(rImport, nPrfx, rLName, xAttrList, rStyles, nFamily ),
    m_bIsNumRuleAlreadyConverted( false )
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
                GetImport().
                    AddStyleDisplayName( GetFamily(), GetName(), GetDisplayName() );
            }
        }
    }
}

SvXMLImportContextRef XMLShapeStyleContext::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLImportContextRef xContext;

    if( XML_NAMESPACE_STYLE == nPrefix || XML_NAMESPACE_LO_EXT == nPrefix )
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
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
                GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                xContext = new XMLShapePropertySetContext( GetImport(), nPrefix,
                                                        rLocalName, xAttrList,
                                                        nFamily,
                                                        GetProperties(),
                                                        xImpPrMap );
        }
    }

    if (!xContext)
        xContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName,
                                                          xAttrList );

    return xContext;
}

void XMLShapeStyleContext::FillPropertySet( const Reference< beans::XPropertySet > & rPropSet )
{
    if( !m_bIsNumRuleAlreadyConverted )
    {
        m_bIsNumRuleAlreadyConverted = true;

        // for compatibility to beta files, search for CTF_SD_NUMBERINGRULES_NAME to
        // import numbering rules from the style:properties element
        const rtl::Reference< XMLPropertySetMapper >&rMapper = GetStyles()->GetImportPropertyMapper( GetFamily() )->getPropertySetMapper();

        ::std::vector< XMLPropertyState > &rProperties = GetProperties();
        ::std::vector< XMLPropertyState >::iterator end( rProperties.end() );

        // first, look for the old format, where we had a text:list-style-name
        // attribute in the style:properties element
        auto property = std::find_if(rProperties.begin(), end, [&rMapper](XMLPropertyState& rProp) {
            // find properties with context
            return (rProp.mnIndex != -1) && (rMapper->GetEntryContextId( rProp.mnIndex ) == CTF_SD_NUMBERINGRULES_NAME); });

        // if we did not find an old list-style-name in the properties, and we need one
        // because we got a style:list-style attribute in the style-style element
        // we generate one
        if( (property == end) && ( !m_sListStyleName.isEmpty() ) )
        {
            sal_Int32 nIndex = rMapper->FindEntryIndex( CTF_SD_NUMBERINGRULES_NAME );
            SAL_WARN_IF( -1 == nIndex, "xmloff", "can't find numbering rules property entry, can't set numbering rule!" );

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

            SAL_WARN_IF( !pListStyle, "xmloff", "list-style not found for shape style" );
            if( pListStyle )
            {
                uno::Reference< container::XIndexReplace > xNumRule( SvxXMLListStyleContext::CreateNumRule( GetImport().GetModel() ) );
                pListStyle->FillUnoNumRule(xNumRule);
                property->maValue <<= xNumRule;
            }
            else
            {
                property->mnIndex = -1;
            }
        }
    }

    struct ContextID_Index_Pair aContextIDs[] =
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

    rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap =
        GetStyles()->GetImportPropertyMapper( GetFamily() );
    SAL_WARN_IF( !xImpPrMap.is(), "xmloff", "There is the import prop mapper" );
    if( xImpPrMap.is() )
        xImpPrMap->FillPropertySet( GetProperties(), rPropSet, aContextIDs );

    Reference< XPropertySetInfo > xInfo;
    // get property set mapper
    rtl::Reference<XMLPropertySetMapper> xPropMapper( xImpPrMap->getPropertySetMapper() );

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
            // All of these attributes refer to something with draw:name
            // of type styleName = NCName which is non-empty.
            // tdf#89802: for Writer frames there would be no exception here but
            // it will fail later on attach() and take out the entire frame
            if (sStyleName.isEmpty() &&
                (   CTF_FILLGRADIENTNAME == aContextIDs[i].nContextID
                 || CTF_FILLTRANSNAME    == aContextIDs[i].nContextID
                 || CTF_FILLHATCHNAME    == aContextIDs[i].nContextID
                 || CTF_FILLBITMAPNAME   == aContextIDs[i].nContextID))
            {
                Sequence<OUString> const seq{ sStyleName };
                GetImport().SetError(
                    XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_WARNING,
                    seq, "empty style name reference", nullptr );
                break;
            }

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
            catch ( const css::lang::IllegalArgumentException& e )
            {
                Sequence<OUString> aSeq { sStyleName };
                GetImport().SetError(
                    XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_WARNING,
                    aSeq, e.Message, nullptr );
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
            catch ( const css::lang::IllegalArgumentException& e )
            {
                Sequence<OUString> aSeq;
                GetImport().SetError(
                    XMLERROR_STYLE_PROP_VALUE | XMLERROR_FLAG_WARNING,
                    aSeq, e.Message, nullptr );
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

void XMLShapeStyleContext::Finish( bool /*bOverwrite*/ )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
