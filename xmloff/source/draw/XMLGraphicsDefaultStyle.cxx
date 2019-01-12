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

#include <xmloff/XMLGraphicsDefaultStyle.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <tools/color.hxx>

#include <xmloff/xmlimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/families.hxx>
#include <xmloff/xmltypes.hxx>
#include <xmloff/maptype.hxx>
#include <xmloff/xmlimppr.hxx>
#include <xmloff/xmlprmap.hxx>

#include <XMLShapePropertySetContext.hxx>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::xml::sax;

using ::xmloff::token::IsXMLToken;
using ::xmloff::token::XML_TEXT_PROPERTIES;
using ::xmloff::token::XML_GRAPHIC_PROPERTIES;
using ::xmloff::token::XML_PARAGRAPH_PROPERTIES;


XMLGraphicsDefaultStyle::XMLGraphicsDefaultStyle( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLName, const Reference< XAttributeList >& xAttrList, SvXMLStylesContext& rStyles )
: XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, XML_STYLE_FAMILY_SD_GRAPHICS_ID, true )
{
}

XMLGraphicsDefaultStyle::~XMLGraphicsDefaultStyle()
{
}

SvXMLImportContextRef XMLGraphicsDefaultStyle::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const Reference< XAttributeList > & xAttrList )
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
            rtl::Reference < SvXMLImportPropertyMapper > xImpPrMap = GetStyles()->GetImportPropertyMapper( GetFamily() );
            if( xImpPrMap.is() )
                xContext = new XMLShapePropertySetContext( GetImport(), nPrefix, rLocalName, xAttrList, nFamily, GetProperties(), xImpPrMap );
        }
    }

    if (!xContext)
        xContext = XMLPropStyleContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return xContext;
}

struct XMLPropertyByIndex {
    sal_Int32 const m_nIndex;
    explicit XMLPropertyByIndex(sal_Int32 const nIndex) : m_nIndex(nIndex) {}
    bool operator()(XMLPropertyState const& rProp) {
        return m_nIndex == rProp.mnIndex;
    }
};

// This method is called for every default style
void XMLGraphicsDefaultStyle::SetDefaults()
{
    Reference< XMultiServiceFactory > xFact( GetImport().GetModel(), UNO_QUERY );
    if( !xFact.is() )
        return;

    Reference< XPropertySet > xDefaults( xFact->createInstance( "com.sun.star.drawing.Defaults" ), UNO_QUERY );
    if( !xDefaults.is() )
        return;
                                            // SJ: #i114750#
    bool bWordWrapDefault = true;   // initializing with correct ODF fo:wrap-option default
    sal_Int32 nUPD( 0 );
    sal_Int32 nBuild( 0 );
    const bool bBuildIdFound = GetImport().getBuildIds( nUPD, nBuild );
    if ( bBuildIdFound && (
        ((nUPD >= 600) &&  (nUPD < 700))
        ||
        ((nUPD == 300) && (nBuild <= 9535))
        ||
        ((nUPD > 300) && (nUPD <= 330))
    ) )
        bWordWrapDefault = false;

    const OUString sTextWordWrap( "TextWordWrap" );
    Reference< XPropertySetInfo > xInfo( xDefaults->getPropertySetInfo() );
    if ( xInfo->hasPropertyByName( sTextWordWrap ) )
        xDefaults->setPropertyValue( sTextWordWrap, Any( bWordWrapDefault ) );

    if (GetImport().IsOOoXML()
        && xInfo->hasPropertyByName("IsFollowingTextFlow"))
    {
        // OOo 1.x only supported "true" so that is the more appropriate
        // default for OOoXML format documents.
        xDefaults->setPropertyValue("IsFollowingTextFlow", uno::makeAny(true));
    }

    // NOTE: the only reason why it's legal to check "==" (not "<") against
    // arbitrary versions here is that the default value of these attributes
    // is not defined by ODF, therefore it is implementation-defined
    // (and we of course must not override any attributes that are actually
    // in the document, so check for that)
    bool const bIsAOO4(
           GetImport().getGeneratorVersion() >= SvXMLImport::AOO_40x
        && GetImport().getGeneratorVersion() <= SvXMLImport::AOO_4x);

    // fdo#75872: backward compatibility for pool defaults change
    if (GetImport().isGeneratorVersionOlderThan(
                SvXMLImport::AOO_40x, SvXMLImport::LO_42x)
        // argh... it turns out that LO has also changed defaults for these
        // since LO 4.0, and so even the _new_ AOO 4.0+ default needs
        // special handling since AOO still does _not_ write it into the file
        || bIsAOO4)
    {
        rtl::Reference<XMLPropertySetMapper> const pImpPrMap(
            GetStyles()->GetImportPropertyMapper(GetFamily())
                ->getPropertySetMapper());
        sal_Int32 const nStrokeIndex(
            pImpPrMap->GetEntryIndex(XML_NAMESPACE_SVG, "stroke-color", 0));
        if (std::none_of(GetProperties().begin(), GetProperties().end(),
                         XMLPropertyByIndex(nStrokeIndex)))
        {
            Color const nStroke(
                    bIsAOO4 ? Color(128, 128, 128) : COL_BLACK);
            xDefaults->setPropertyValue("LineColor", makeAny(nStroke));
        }
        Color const nFillColor( bIsAOO4
            ? Color(0xCF, 0xE7, 0xF5) : Color(153, 204, 255));
        sal_Int32 const nFillIndex(
            pImpPrMap->GetEntryIndex(XML_NAMESPACE_DRAW, "fill-color", 0));
        if (std::none_of(GetProperties().begin(), GetProperties().end(),
                         XMLPropertyByIndex(nFillIndex)))
        {
            xDefaults->setPropertyValue("FillColor", makeAny(nFillColor));
        }
        if (xInfo->hasPropertyByName("FillColor2"))
        {
            sal_Int32 const nFill2Index(pImpPrMap->GetEntryIndex(
                        XML_NAMESPACE_DRAW, "secondary-fill-color", 0));
            if (std::none_of(GetProperties().begin(), GetProperties().end(),
                             XMLPropertyByIndex(nFill2Index)))
            {
                xDefaults->setPropertyValue("FillColor2", makeAny(sal_Int32(nFillColor)));
            }
        }
    }

    FillPropertySet( xDefaults );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
