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

#include <memory>
#include <xmloff/MarkerStyle.hxx>
#include <xexptran.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <sal/log.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

using namespace ::com::sun::star;

using namespace ::xmloff::token;

// Import

XMLMarkerStyleImport::XMLMarkerStyleImport( SvXMLImport& rImp )
    : m_rImport( rImp )
{
}

void XMLMarkerStyleImport::importXML(
    const uno::Reference< xml::sax::XFastAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    bool bHasViewBox    = false;
    bool bHasPathData   = false;
    OUString aDisplayName;

    std::unique_ptr<SdXMLImExViewBox> xViewBox;

    SvXMLUnitConverter& rUnitConverter = m_rImport.GetMM100UnitConverter();

    OUString strPathData;

    for (auto &aIter : sax_fastparser::castToFastAttributeList( xAttrList ))
    {
        OUString aStrValue = aIter.toString();

        switch (aIter.getToken() & TOKEN_MASK)
        {
            case XML_NAME:
                rStrName = aStrValue;
                break;
            case XML_DISPLAY_NAME:
                aDisplayName = aStrValue;
                break;
            case XML_VIEWBOX:
                xViewBox.reset(new SdXMLImExViewBox(aStrValue, rUnitConverter));
                bHasViewBox = true;
                break;
            case XML_D:
                strPathData = aStrValue;
                bHasPathData = true;
                break;
            default:
                XMLOFF_WARN_UNKNOWN("xmloff.style", aIter);
        }
    }

    if( bHasViewBox && bHasPathData )
    {
        basegfx::B2DPolyPolygon aPolyPolygon;

        if(basegfx::utils::importFromSvgD(aPolyPolygon, strPathData, m_rImport.needFixPositionAfterZ(), nullptr))
        {
            if(aPolyPolygon.count())
            {
                // ViewBox probably not used, but stay with former processing inside of
                // SdXMLImExSvgDElement
                const basegfx::B2DRange aSourceRange(
                    xViewBox->GetX(), xViewBox->GetY(),
                    xViewBox->GetX() + xViewBox->GetWidth(),
                    xViewBox->GetY() + xViewBox->GetHeight());
                const basegfx::B2DRange aTargetRange(
                    0.0, 0.0,
                    xViewBox->GetWidth(), xViewBox->GetHeight());

                if(!aSourceRange.equal(aTargetRange))
                {
                    aPolyPolygon.transform(
                        basegfx::utils::createSourceRangeTargetRangeTransform(
                            aSourceRange,
                            aTargetRange));
                }

                // always use PolyPolygonBezierCoords here
                drawing::PolyPolygonBezierCoords aSourcePolyPolygon;

                basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords(
                    aPolyPolygon,
                    aSourcePolyPolygon);
                rValue <<= aSourcePolyPolygon;
            }
        }

        if( !aDisplayName.isEmpty() )
        {
            m_rImport.AddStyleDisplayName( XmlStyleFamily::SD_MARKER_ID, rStrName,
                                        aDisplayName );
            rStrName = aDisplayName;
        }
    }

    xViewBox.reset();
}

// Export

XMLMarkerStyleExport::XMLMarkerStyleExport( SvXMLExport& rExp )
    : m_rExport( rExp )
{
}

void XMLMarkerStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    if(rStrName.isEmpty())
        return;

    drawing::PolyPolygonBezierCoords aBezier;

    if(!(rValue >>= aBezier))
        return;

    // Name
    bool bEncoded(false);

    m_rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_NAME, m_rExport.EncodeStyleName( rStrName, &bEncoded ) );

    if( bEncoded )
    {
        m_rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME, rStrName );
    }

    const basegfx::B2DPolyPolygon aPolyPolygon(
        basegfx::utils::UnoPolyPolygonBezierCoordsToB2DPolyPolygon(
            aBezier));
    const basegfx::B2DRange aPolyPolygonRange(aPolyPolygon.getB2DRange());


    // Viewbox (viewBox="0 0 1500 1000")

    SdXMLImExViewBox aViewBox(
        aPolyPolygonRange.getMinX(),
        aPolyPolygonRange.getMinY(),
        aPolyPolygonRange.getWidth(),
        aPolyPolygonRange.getHeight());
    m_rExport.AddAttribute( XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString() );

    // Pathdata
    const OUString aPolygonString(
        basegfx::utils::exportToSvgD(
            aPolyPolygon,
            true,           // bUseRelativeCoordinates
            false,          // bDetectQuadraticBeziers: not used in old, but maybe activated now
            true));         // bHandleRelativeNextPointCompatible

    // write point array
    m_rExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aPolygonString);

    // Do Write
    SvXMLElementExport rElem( m_rExport, XML_NAMESPACE_DRAW, XML_MARKER, true, false );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
