/*************************************************************************
 *
 *  $RCSfile: MarkerStyle.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 08:32:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_MARKERSTYLE_HXX
#include "MarkerStyle.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _XMLOFF_ATTRLIST_HXX
#include "attrlist.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
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

#ifndef SVX_LIGHT

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

            SvXMLUnitConverter& rUnitConverter =
                rExport.GetMM100UnitConverter();

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

            for(a = 0L; a < nOuterCnt; a++)
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

                for(b = 0L; b < nPointCount; b++)
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
            rExport.AddAttribute( XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString( rUnitConverter ) );

            /////////////////
            // Pathdata
            pOuterSequence = aBezier.Coordinates.getArray();
            drawing::FlagSequence*  pOuterFlags = aBezier.Flags.getArray();
            SdXMLImExSvgDElement aSvgDElement(aViewBox);

            for(a = 0L; a < nOuterCnt; a++)
            {
                drawing::PointSequence* pSequence = pOuterSequence++;
                drawing::FlagSequence* pFlags = pOuterFlags++;

                aSvgDElement.AddPolygon(pSequence, pFlags,
                    awt::Point( 0, 0 ),
                    awt::Size( aViewBox.GetWidth(), aViewBox.GetHeight() ),
                    rUnitConverter, bClosed);
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

#endif // #ifndef SVX_LIGHT
