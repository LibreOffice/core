/*************************************************************************
 *
 *  $RCSfile: shapeexport2.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-02 11:14:37 $
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

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX
#include "shapeexport.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#include "xmlkywd.hxx"
#include "xmlnmspe.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportSize( const uno::Reference< drawing::XShape >& xShape, sal_Int32 nFeatures )
{
    OUStringBuffer sStringBuffer;

    awt::Size aSize( xShape->getSize() );

    if( nFeatures & SEF_EXPORT_WIDTH )
    {
        // svg: width
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aSize.Width);
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_width, sStringBuffer.makeStringAndClear());
    }

    if( nFeatures & SEF_EXPORT_HEIGHT )
    {
        // svg: height
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aSize.Height);
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_height, sStringBuffer.makeStringAndClear());
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPosition( const uno::Reference< drawing::XShape >& xShape, sal_Int32 nFeatures, awt::Point* pRefPoint )
{
    OUStringBuffer sStringBuffer;

    awt::Point aPoint( xShape->getPosition() );
    if( pRefPoint )
    {
        aPoint.X -= pRefPoint->X;
        aPoint.Y -= pRefPoint->Y;
    }

    if( nFeatures & SEF_EXPORT_X )
    {
        // svg: x
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aPoint.X);
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x, sStringBuffer.makeStringAndClear());
    }

    if( nFeatures & SEF_EXPORT_Y )
    {
        // svg: y
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aPoint.Y);
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y, sStringBuffer.makeStringAndClear());
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportTransformation( const uno::Reference< beans::XPropertySet >& xPropSet )
{
    OUStringBuffer sStringBuffer;

    SdXMLImExTransform2D aTransform;

    // evtl. rotation (100'th degree, part of transformation)?
    sal_Int32 nRotAngle(0L);
    uno::Any aAny = xPropSet->getPropertyValue(
        OUString(RTL_CONSTASCII_USTRINGPARAM("RotateAngle")));
    aAny >>= nRotAngle;
    if(nRotAngle)
        aTransform.AddRotate(nRotAngle / 100.0);

    // does transformation need to be exported?
    if(aTransform.NeedsAction())
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_transform, aTransform.GetExportString(rExport.GetMM100UnitConverter()));
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool XMLShapeExport::ImpExportPresentationAttributes( const uno::Reference< beans::XPropertySet >& xPropSet, const rtl::OUString& rClass )
{
    sal_Bool bIsEmpty = sal_False;

    OUStringBuffer sStringBuffer;

    // write presentation class entry
    rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_class, rClass);

    if( xPropSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        sal_Bool bTemp;

        // is empty pes shape?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject"))))
        {
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject"))) >>= bIsEmpty;
            if( bIsEmpty )
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_placeholder, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        }

        // is user-transformed?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent"))))
        {
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent"))) >>= bTemp;
            if(!bTemp)
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_user_transformed, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        }
    }

    return bIsEmpty;
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportText( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() && xText->getString().getLength() )
        rExport.GetTextParagraphExport()->exportText( xText );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportGroupShape( const uno::Reference< drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY);
    if(xShapes.is() && xShapes->getCount())
    {
        // write group shape
        SvXMLElementExport aPGR(rExport, XML_NAMESPACE_DRAW, sXML_g, sal_True, sal_True);

        // write members
        exportShapes( xShapes, nFeatures, pRefPoint );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportTextBoxShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */ )
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        OUString aStr;
        OUStringBuffer sStringBuffer;

        // presentation attribute (if presentation)
        sal_Bool bIsPresShape(FALSE);
        sal_Bool bIsEmptyPresObj(FALSE);

        switch(eShapeType)
        {
            case XmlShapeTypePresSubtitleShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_subtitle));
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresTitleTextShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_title));
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresOutlinerShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_outline));
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresNotesShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_notes));
                bIsPresShape = TRUE;
                break;
            }
        }

        ImpExportPosition( xShape, nFeatures, pRefPoint );
        ImpExportSize( xShape, nFeatures );
        ImpExportTransformation( xPropSet );

        if(bIsPresShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, aStr );

        // write text-box
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_text_box, sal_True, sal_True);

        // export text
        if(!bIsEmptyPresObj)
            ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportRectangleShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /*= SEF_DEFAULT */,    com::sun::star::awt::Point* pRefPoint /* = NULL */ )
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUStringBuffer sStringBuffer;

        ImpExportPosition( xShape, nFeatures, pRefPoint );
        ImpExportSize( xShape, nFeatures );
        ImpExportTransformation( xPropSet );

        // evtl. corner radius?
        sal_Int32 nCornerRadius(0L);
        xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius"))) >>= nCornerRadius;
        if(nCornerRadius)
        {
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nCornerRadius);
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_corner_radius, sStringBuffer.makeStringAndClear());
        }

        // write rectangle
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_rect, sal_True, sal_True);

        // export text
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportLineShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */ )
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        SdXMLImExTransform2D aTransform;
        OUString aStr;
        OUStringBuffer sStringBuffer;
        awt::Point aStart(0,0);
        awt::Point aEnd(1,1);

        drawing::PointSequenceSequence* pSourcePolyPolygon = 0L;
        uno::Any aAny = xPropSet->getPropertyValue(
            OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygon")));
        pSourcePolyPolygon = (drawing::PointSequenceSequence*)aAny.getValue();

        if(pSourcePolyPolygon)
        {
            drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
            if(pOuterSequence)
            {
                drawing::PointSequence* pInnerSequence = pOuterSequence++;
                if(pInnerSequence)
                {
                    awt::Point* pArray = pInnerSequence->getArray();
                    if(pArray)
                    {
                        if(pInnerSequence->getLength() > 0)
                        {
                            aStart = awt::Point(pArray->X, pArray->Y);
                            pArray++;
                        }

                        if(pInnerSequence->getLength() > 1)
                        {
                            aEnd = awt::Point(pArray->X, pArray->Y);
                        }
                    }
                }
            }
        }

        if( pRefPoint )
        {
            aStart.X -= pRefPoint->X;
            aStart.Y -= pRefPoint->Y;
            aEnd.X -= pRefPoint->X;
            aEnd.Y -= pRefPoint->Y;
        }

        if( nFeatures & SEF_EXPORT_X )
        {
            // svg: x1
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.X);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x1, aStr);
        }
        else
        {
            aEnd.X -= aStart.X;
        }

        if( nFeatures & SEF_EXPORT_Y )
        {
            // svg: y1
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.Y);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y1, aStr);
        }
        else
        {
            aEnd.Y -= aStart.Y;
        }

        // svg: x2
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x2, aStr);

        // svg: y2
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y2, aStr);

        // write line
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_line, sal_True, sal_True);

        // export text
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportEllipseShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        SdXMLImExTransform2D aTransform;
        // get size to decide between Circle and Ellipse
        awt::Point aPoint = xShape->getPosition();
        if( pRefPoint )
        {
            aPoint.X -= pRefPoint->X;
            aPoint.Y -= pRefPoint->Y;
        }

        awt::Size aSize = xShape->getSize();
        sal_Int32 nRx((aSize.Width + 1) / 2);
        sal_Int32 nRy((aSize.Height + 1) / 2);
        BOOL bCircle(nRx == nRy);
        OUString aStr;
        OUStringBuffer sStringBuffer;

        if( nFeatures & SEF_EXPORT_X )
        {
            // svg: cx
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aPoint.X + nRx);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_cx, aStr);
        }

        if( nFeatures & SEF_EXPORT_Y )
        {
            // svg: cy
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aPoint.Y + nRy);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_cy, aStr);
        }

        ImpExportTransformation( xPropSet );

        drawing::CircleKind eKind = drawing::CircleKind_FULL;
        xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleKind")) ) >>= eKind;
        if( eKind != drawing::CircleKind_FULL )
        {
            sal_Int32 nStartAngle;
            sal_Int32 nEndAngle;
            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleStartAngle")) ) >>= nStartAngle;
            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleEndAngle")) ) >>= nEndAngle;

            const double dStartAngle = nStartAngle / 100.0;
            const double dEndAngle = nEndAngle / 100.0;

            SvXMLUnitConverter::convertEnum( sStringBuffer, (USHORT)eKind, aXML_CircleKind_EnumMap );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_kind, sStringBuffer.makeStringAndClear() );

            SvXMLUnitConverter::convertNumber( sStringBuffer, dStartAngle );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_start_angle, sStringBuffer.makeStringAndClear() );

            SvXMLUnitConverter::convertNumber( sStringBuffer, dEndAngle );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_end_angle, sStringBuffer.makeStringAndClear() );
        }

        if(bCircle)
        {
            // svg: r
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nRx);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_r, aStr);

            // write circle
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_circle, sal_True, sal_True);

            // export text
            ImpExportText( xShape );
        }
        else
        {
            // svg: rx
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nRx);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_rx, aStr);

            // svg: ry
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nRy);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_ry, aStr);

            // write ellipse
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_ellipse, sal_True, sal_True);

            // export text
            ImpExportText( xShape );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPolygonShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        BOOL bClosed(eShapeType == XmlShapeTypeDrawPolyPolygonShape
            || eShapeType == XmlShapeTypeDrawClosedBezierShape);
        BOOL bBezier(eShapeType == XmlShapeTypeDrawClosedBezierShape
            || eShapeType == XmlShapeTypeDrawOpenBezierShape);

        OUStringBuffer sStringBuffer;

        ImpExportPosition( xShape, nFeatures, pRefPoint );
        ImpExportSize( xShape, nFeatures );
        ImpExportTransformation( xPropSet );

        // create and export ViewBox
        awt::Point aPoint( xShape->getPosition() );
        awt::Size aSize( xShape->getSize() );
        SdXMLImExViewBox aViewBox(0, 0, aSize.Width, aSize.Height);
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_viewBox, aViewBox.GetExportString(rExport.GetMM100UnitConverter()));

        if(bBezier)
        {
            // get PolygonBezier
            uno::Any aAny( xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygonBezier"))) );
            drawing::PolyPolygonBezierCoords* pSourcePolyPolygon =
                (drawing::PolyPolygonBezierCoords*)aAny.getValue();

            if(pSourcePolyPolygon && pSourcePolyPolygon->Coordinates.getLength())
            {
                sal_Int32 nOuterCnt(pSourcePolyPolygon->Coordinates.getLength());
                drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
                drawing::FlagSequence*  pOuterFlags = pSourcePolyPolygon->Flags.getArray();

                if(pOuterSequence && pOuterFlags)
                {
                    // prepare svx:d element export
                    SdXMLImExSvgDElement aSvgDElement(aViewBox);

                    for(sal_Int32 a(0L); a < nOuterCnt; a++)
                    {
                        drawing::PointSequence* pSequence = pOuterSequence++;
                        drawing::FlagSequence* pFlags = pOuterFlags++;

                        if(pSequence && pFlags)
                        {
                            aSvgDElement.AddPolygon(pSequence, pFlags,
                                aPoint, aSize, rExport.GetMM100UnitConverter(), bClosed);
                        }
                    }

                    // write point array
                    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_d, aSvgDElement.GetExportString());
                }

                // write object now
                SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_path, sal_True, sal_True);

                // export text
                uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
                if( xText.is() && xText->getString().getLength() )
                    rExport.GetTextParagraphExport()->exportText( xText );
            }
        }
        else
        {
            // get non-bezier polygon
            uno::Any aAny( xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygon"))) );
            drawing::PointSequenceSequence* pSourcePolyPolygon = (drawing::PointSequenceSequence*)aAny.getValue();

            if(pSourcePolyPolygon && pSourcePolyPolygon->getLength())
            {
                sal_Int32 nOuterCnt(pSourcePolyPolygon->getLength());

                if(1L == nOuterCnt && !bBezier)
                {
                    // simple polygon shape, can be written as svg:points sequence
                    drawing::PointSequence* pSequence = pSourcePolyPolygon->getArray();
                    if(pSequence)
                    {
                        SdXMLImExPointsElement aPoints(pSequence, aViewBox, aPoint, aSize, rExport.GetMM100UnitConverter());

                        // write point array
                        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_points, aPoints.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW,
                        bClosed ? sXML_polygon : sXML_polyline , sal_True, sal_True);

                    // export text
                    ImpExportText( xShape );
                }
                else
                {
                    // polypolygon or bezier, needs to be written as a svg:path sequence
                    drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
                    if(pOuterSequence)
                    {
                        // prepare svx:d element export
                        SdXMLImExSvgDElement aSvgDElement(aViewBox);

                        for(sal_Int32 a(0L); a < nOuterCnt; a++)
                        {
                            drawing::PointSequence* pSequence = pOuterSequence++;
                            if(pSequence)
                            {
                                aSvgDElement.AddPolygon(pSequence, 0L, aPoint,
                                    aSize, rExport.GetMM100UnitConverter(), bClosed);
                            }
                        }

                        // write point array
                        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_d, aSvgDElement.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_path, sal_True, sal_True);

                    // export text
                    ImpExportText( xShape );
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportGraphicObjectShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUStringBuffer sStringBuffer;

        sal_Bool bIsEmptyPresObj = sal_False;
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        ImpExportPosition( xShape, nFeatures, pRefPoint );
        ImpExportSize( xShape, nFeatures  );
        ImpExportTransformation( xPropSet );

        if(eShapeType == XmlShapeTypePresGraphicObjectShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_graphic)) );

        if( !bIsEmptyPresObj )
        {
            OUString aStreamURL;
            OUString aStr;

            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL"))) >>= aStr;
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_href, aStr = rExport.AddEmbeddedGraphicObject( aStr ) );

            if( aStr.getLength() && aStr[ 0 ] == '#' )
            {
                aStreamURL = OUString::createFromAscii( "vnd.sun.star.Package:" );
                aStreamURL = aStreamURL.concat( aStr.copy( 1, aStr.getLength() - 1 ) );
            }

            // update stream URL for load on demand
            uno::Any aAny;
            aAny <<= aStreamURL;
            xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicStreamURL")), aAny );

            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_simple));
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_type, aStr );

            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_embed));
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_show, aStr );

            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_onLoad));
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_actuate, aStr );
        }
        // write graphic object
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_image, sal_True, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportChartShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        sal_Bool bIsEmptyPresObj = sal_False;
        if(eShapeType == XmlShapeTypePresChartShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_chart)) );

        OUStringBuffer sStringBuffer;

        ImpExportPosition( xShape, nFeatures, pRefPoint );
        ImpExportSize( xShape, nFeatures );

        uno::Reference< chart::XChartDocument > xChartDoc;
        if( !bIsEmptyPresObj )
            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) ) >>= xChartDoc;

        if( xChartDoc.is() )
        {
            // export chart data if the flag is not set (default)
            sal_Bool bExportOwnData = (( nFeatures & SEF_EXPORT_NO_CHART_DATA ) == 0 );
            rExport.GetChartExport()->exportChart( xChartDoc, bExportOwnData );
        }
        else
        {
            // write chart object (fake for now, replace later)
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_CHART, sXML_chart, sal_True, sal_True);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportSpreadsheetShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        if(eShapeType == XmlShapeTypePresTableShape)
            ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_table)) );

        DBG_ERROR( "presentation spreadsheets not yet implemented!" );
        // write spreadsheet object (fake for now, replace later)
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML__unknown_, sal_True, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportControlShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    OUStringBuffer sStringBuffer;

    ImpExportPosition( xShape, nFeatures, pRefPoint );
    ImpExportSize( xShape, nFeatures );

    uno::Reference< drawing::XControlShape > xControl( xShape, uno::UNO_QUERY );
    DBG_ASSERT( xControl.is(), "Control shape is not supporting XControlShape" );
    if( xControl.is() )
    {
        uno::Reference< beans::XPropertySet > xControlModel( xControl->getControl(), uno::UNO_QUERY );
        DBG_ASSERT( xControlModel.is(), "Control shape has not XControlModel" );
        if( xControlModel.is() )
        {
            rExport.AddAttribute( XML_NAMESPACE_FORM, sXML_id, rExport.GetFormExport()->getControlId( xControlModel ) );
        }
    }

    // this is a control shape, in this place the database team
    // would have to export the control abilities. Add Export later
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_control, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportConnectorShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export connection kind
    drawing::ConnectorType eType = drawing::ConnectorType_STANDARD;
    uno::Any aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeKind")));
    aAny >>= eType;

    if( eType != drawing::ConnectorType_STANDARD )
    {
        SvXMLUnitConverter::convertEnum( sStringBuffer, (sal_uInt16)eType, aXML_ConnectionKind_EnumMap );
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_type, aStr);
    }

    // export line skew
    sal_Int32 nDelta1 = 0, nDelta2 = 0, nDelta3 = 0;

    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine1Delta")));
    aAny >>= nDelta1;
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine2Delta")));
    aAny >>= nDelta2;
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine3Delta")));
    aAny >>= nDelta3;

    if( nDelta1 != 0 || nDelta2 != 0 || nDelta3 != 0 )
    {
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDelta1);
        if( nDelta2 != 0 || nDelta3 != 0 )
        {
            const char aSpace = ' ';
            sStringBuffer.appendAscii( &aSpace, 1 );
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDelta2);
            if( nDelta3 != 0 )
            {
                sStringBuffer.appendAscii( &aSpace, 1 );
                rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDelta3);
            }
        }

        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_line_skew, aStr);
    }

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition"))) >>= aStart;
    xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition"))) >>= aEnd;

    if( pRefPoint )
    {
        aStart.X -= pRefPoint->X;
        aStart.Y -= pRefPoint->Y;
        aEnd.X -= pRefPoint->X;
        aEnd.Y -= pRefPoint->Y;
    }

    if( nFeatures & SEF_EXPORT_X )
    {
        // svg: x1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & SEF_EXPORT_Y )
    {
        // svg: y1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x2, aStr);

    // svg: y2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y2, aStr);

    uno::Reference< drawing::XShape > xTempShape;

    // export start connection
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartShape") ) );
    if( aAny >>= xTempShape )
    {
        sal_Int32 nShapeId = rExport.GetShapeExport()->getShapeId( xTempShape );
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_start_shape, OUString::valueOf( nShapeId ));

        aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartGluePointIndex")) );
        sal_Int32 nGluePointId;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_start_glue_point, OUString::valueOf( nGluePointId ));
            }
        }
    }

    // export end connection
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndShape")) );
    if( aAny >>= xTempShape )
    {
        sal_Int32 nShapeId = rExport.GetShapeExport()->getShapeId( xTempShape );
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_end_shape, OUString::valueOf( nShapeId ));

        aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndGluePointIndex")) );
        sal_Int32 nGluePointId;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_end_glue_point, OUString::valueOf( nGluePointId ));
            }
        }
    }

    // write connector shape. Add Export later.
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_connector, sal_True, sal_True);

    // export text
    ImpExportText( xShape );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportMeasureShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    uno::Any aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")));
    aAny >>= aStart;

    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")));
    aAny >>= aEnd;

    if( pRefPoint )
    {
        aStart.X -= pRefPoint->X;
        aStart.Y -= pRefPoint->Y;
        aEnd.X -= pRefPoint->X;
        aEnd.Y -= pRefPoint->Y;
    }

    if( nFeatures & SEF_EXPORT_X )
    {
        // svg: x1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & SEF_EXPORT_Y )
    {
        // svg: y1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x2, aStr);

    // svg: y2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y2, aStr);

    // write measure shape
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_measure, sal_True, sal_True);

    // export text
    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() )
        rExport.GetTextParagraphExport()->exportText( xText );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportOLE2Shape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        if(eShapeType == XmlShapeTypePresOLE2Shape)
            ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_object)));

        DBG_ERROR("presentation ole2 not yet supported");

        // write object
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML__unknown_, sal_True, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPageShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    OUString aStr;

    // a presentation page shape, normally used on notes pages only. If
    // it is used not as presentation shape, it may have been created with
    // copy-paste exchange between draw and impress (this IS possible...)
    if(eShapeType == XmlShapeTypePresPageShape)
    {
        rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_class,
            OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_page)));
    }

    // write Page shape
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_page_thumbnail, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportCaptionShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    // write Caption shape. Add export later.
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_caption, sal_True, sal_True);

    // export text
    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() && xText->getString().getLength() )
        rExport.GetTextParagraphExport()->exportText( xText );
}

