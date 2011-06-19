/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_xmloff.hxx"
#include "xmloff/MarkerStyle.hxx"
#include "xexptran.hxx"
#include <xmloff/attrlist.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmluconv.hxx>
#include "xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmlimp.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>

using namespace ::com::sun::star;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::xmloff::token;


//-------------------------------------------------------------
// Import
//-------------------------------------------------------------

XMLMarkerStyleImport::XMLMarkerStyleImport( SvXMLImport& rImp )
    : rImport( rImp )
{
}

XMLMarkerStyleImport::~XMLMarkerStyleImport()
{
}

sal_Bool XMLMarkerStyleImport::importXML(
    const uno::Reference< xml::sax::XAttributeList >& xAttrList,
    uno::Any& rValue,
    OUString& rStrName )
{
    sal_Bool bHasViewBox    = sal_False;
    sal_Bool bHasPathData   = sal_False;
    OUString aDisplayName;

    SdXMLImExViewBox* pViewBox = NULL;

    SvXMLNamespaceMap& rNamespaceMap = rImport.GetNamespaceMap();
    SvXMLUnitConverter& rUnitConverter = rImport.GetMM100UnitConverter();

    OUString strPathData;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for( sal_Int16 i = 0; i < nAttrCount; i++ )
    {
        OUString aStrFullAttrName = xAttrList->getNameByIndex( i );
        OUString aStrAttrName;
        rNamespaceMap.GetKeyByAttrName( aStrFullAttrName, &aStrAttrName );
        OUString aStrValue = xAttrList->getValueByIndex( i );

        if( IsXMLToken( aStrAttrName, XML_NAME ) )
        {
            rStrName = aStrValue;
        }
        else if( IsXMLToken( aStrAttrName, XML_DISPLAY_NAME ) )
        {
            aDisplayName = aStrValue;
        }
        else if( IsXMLToken( aStrAttrName, XML_VIEWBOX ) )
        {
            pViewBox = new SdXMLImExViewBox( aStrValue, rUnitConverter );
            bHasViewBox = sal_True;

        }
        else if( IsXMLToken( aStrAttrName, XML_D ) )
        {
            strPathData = aStrValue;
            bHasPathData = sal_True;
        }
    }

    if( bHasViewBox && bHasPathData )
    {
        SdXMLImExSvgDElement aPoints(strPathData, *pViewBox, awt::Point( 0, 0 ),
            awt::Size( pViewBox->GetWidth(), pViewBox->GetHeight() ),
            rUnitConverter );

        if(aPoints.IsCurve())
        {
            drawing::PolyPolygonBezierCoords aSourcePolyPolygon(
                aPoints.GetPointSequenceSequence(),
                aPoints.GetFlagSequenceSequence());
            rValue <<= aSourcePolyPolygon;
        }
        else
        {
            drawing::PolyPolygonBezierCoords aSourcePolyPolygon;
            aSourcePolyPolygon.Coordinates = aPoints.GetPointSequenceSequence();
            aSourcePolyPolygon.Flags.realloc(aSourcePolyPolygon.Coordinates.getLength());

            // Zeiger auf innere sequences holen
            const drawing::PointSequence* pInnerSequence = aSourcePolyPolygon.Coordinates.getConstArray();
            drawing::FlagSequence* pInnerSequenceFlags = aSourcePolyPolygon.Flags.getArray();

            for(sal_Int32 a(0); a < aSourcePolyPolygon.Coordinates.getLength(); a++)
            {
                pInnerSequenceFlags->realloc(pInnerSequence->getLength());
                drawing::PolygonFlags* pPolyFlags = pInnerSequenceFlags->getArray();

                for(sal_Int32 b(0); b < pInnerSequence->getLength(); b++)
                    *pPolyFlags++ = drawing::PolygonFlags_NORMAL;

                // next run
                pInnerSequence++;
                pInnerSequenceFlags++;
            }

            rValue <<= aSourcePolyPolygon;
        }

        if( aDisplayName.getLength() )
        {
            rImport.AddStyleDisplayName( XML_STYLE_FAMILY_SD_MARKER_ID, rStrName,
                                        aDisplayName );
            rStrName = aDisplayName;
        }

    }

    if( pViewBox )
        delete pViewBox;

    return bHasViewBox && bHasPathData;
}


//-------------------------------------------------------------
// Export
//-------------------------------------------------------------

XMLMarkerStyleExport::XMLMarkerStyleExport( SvXMLExport& rExp )
    : rExport( rExp )
{
}

XMLMarkerStyleExport::~XMLMarkerStyleExport()
{
}

sal_Bool XMLMarkerStyleExport::exportXML(
    const OUString& rStrName,
    const uno::Any& rValue )
{
    sal_Bool bRet(sal_False);

    if(rStrName.getLength())
    {
        drawing::PolyPolygonBezierCoords aBezier;

        if(rValue >>= aBezier)
        {
            OUString aStrValue;
            OUStringBuffer aOut;

            /////////////////
            // Name
            sal_Bool bEncoded = sal_False;
            OUString aStrName( rStrName );
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME,
                                  rExport.EncodeStyleName( aStrName,
                                                           &bEncoded ) );
            if( bEncoded )
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_DISPLAY_NAME,
                                      aStrName );

            /////////////////
            // Viewbox (viewBox="0 0 1500 1000")
            sal_Int32 nMinX(0x7fffffff);
            sal_Int32 nMaxX(0x80000000);
            sal_Int32 nMinY(0x7fffffff);
            sal_Int32 nMaxY(0x80000000);
            sal_Int32 nOuterCnt(aBezier.Coordinates.getLength());
            drawing::PointSequence* pOuterSequence = aBezier.Coordinates.getArray();
            sal_Int32 a, b;
            sal_Bool bClosed(sal_False);

            for (a = 0; a < nOuterCnt; a++)
            {
                drawing::PointSequence* pSequence = pOuterSequence++;
                const awt::Point *pPoints = pSequence->getConstArray();
                sal_Int32 nPointCount(pSequence->getLength());

                if(nPointCount)
                {
                    const awt::Point aStart = pPoints[0];
                    const awt::Point aEnd = pPoints[nPointCount - 1];

                    if(aStart.X == aEnd.X && aStart.Y == aEnd.Y)
                    {
                        bClosed = sal_True;
                    }
                }

                for (b = 0; b < nPointCount; b++)
                {
                    const awt::Point aPoint = pPoints[b];

                    if( aPoint.X < nMinX )
                        nMinX = aPoint.X;

                    if( aPoint.X > nMaxX )
                        nMaxX = aPoint.X;

                    if( aPoint.Y < nMinY )
                        nMinY = aPoint.Y;

                    if( aPoint.Y > nMaxY )
                        nMaxY = aPoint.Y;
                }
            }

            sal_Int32 nDifX(nMaxX - nMinX);
            sal_Int32 nDifY(nMaxY - nMinY);

            SdXMLImExViewBox aViewBox( 0, 0, nDifX, nDifY );
            rExport.AddAttribute( XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString() );

            /////////////////
            // Pathdata
            pOuterSequence = aBezier.Coordinates.getArray();
            drawing::FlagSequence*  pOuterFlags = aBezier.Flags.getArray();
            SdXMLImExSvgDElement aSvgDElement(aViewBox);

            for (a = 0; a < nOuterCnt; a++)
            {
                drawing::PointSequence* pSequence = pOuterSequence++;
                drawing::FlagSequence* pFlags = pOuterFlags++;

                aSvgDElement.AddPolygon(pSequence, pFlags,
                    awt::Point( 0, 0 ),
                    awt::Size( aViewBox.GetWidth(), aViewBox.GetHeight() ),
                    bClosed);
            }

            // write point array
            rExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());

            /////////////////
            // Do Write
            SvXMLElementExport rElem( rExport, XML_NAMESPACE_DRAW, XML_MARKER,
                                      sal_True, sal_False );
        }
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
