/*************************************************************************
 *
 *  $RCSfile: ximpshap.cxx,v $
 *
 *  $Revision: 1.30 $
 *
 *  last change: $Author: bm $ $Date: 2001-02-27 12:53:38 $
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

#pragma hdrstop

#include <tools/debug.hxx>

#ifndef _CPPUHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif

#ifndef _XIMPSHAPE_HXX
#include "ximpshap.hxx"
#endif

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#include "XMLShapeStyleContext.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX3_HPP_
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include"xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _XMLOFF_EVENTIMP_HXX
#include "eventimp.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLShapeContext, SvXMLImportContext );

SdXMLShapeContext::SdXMLShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SvXMLImportContext( rImport, nPrfx, rLocalName ),
    mxShapes( rShapes ),
    mnStyleFamily(XML_STYLE_FAMILY_SD_GRAPHICS_ID),
    mbIsPlaceholder(FALSE),
    mbIsUserTransformed(FALSE),
    mxAttrList(xAttrList),
    mnZOrder(-1),
    mnShapeId(-1),
    maPosition(0, 0),
    maSize(1, 1)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLShapeContext::~SdXMLShapeContext()
{
    if(mxCursor.is())
    {
        // delete addition newline
        const OUString aEmpty;
        mxCursor->gotoEnd( sal_False );
        mxCursor->goLeft( 1, sal_True );
        mxCursor->setString( aEmpty );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if(mxOldCursor.is())
        GetImport().GetTextImport()->SetCursor( mxOldCursor );
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLShapeContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = NULL;

    if( nPrefix == XML_NAMESPACE_OFFICE && rLocalName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( sXML_events ) ) )
    {
        pContext = new SdXMLEventsContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    else
    {
        // create text cursor on demand
        if( !mxCursor.is() )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            if( xText.is() )
            {
                mxOldCursor = GetImport().GetTextImport()->GetCursor();
                mxCursor = xText->createTextCursor();
                if( mxCursor.is() )
                {
                    GetImport().GetTextImport()->SetCursor( mxCursor );
                }
            }
        }

        // if we have a text cursor, lets  try to import some text
        if( mxCursor.is() )
        {
            pContext = GetImport().GetTextImport()->CreateTextChildContext(
                GetImport(), nPrefix, rLocalName, xAttrList );
        }
    }

    // call parent for content
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::AddShape(uno::Reference< drawing::XShape >& xShape)
{
    if(xShape.is())
    {
        // set shape local
        mxShape = xShape;

        if(maShapeName.getLength())
        {
            uno::Reference< container::XNamed > xNamed( mxShape, uno::UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( maShapeName );
        }

        UniReference< XMLShapeImportHelper > xImp( GetImport().GetShapeImport() );
        xImp->addShape( xShape, mxAttrList, mxShapes );
        xImp->shapeWithZIndexAdded( xShape, mnZOrder );

        if( mnShapeId != -1 )
            xImp->createShapeId( xShape, mnShapeId );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::AddShape(const char* pServiceName )
{
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(OUString::createFromAscii(pServiceName)), uno::UNO_QUERY);
        if( xShape.is() )
            AddShape( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::SetTransformation()
{
    if(mxShape.is())
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            Matrix3D aTransformation;

            if(maSize.Width != 1 || maSize.Height != 1)
            {
                // take care there are no zeros used by error
                if(0 == maSize.Width)
                    maSize.Width = 1;
                if(0 == maSize.Height)
                    maSize.Height = 1;

                // set global size. This should always be used.
                aTransformation.Scale(maSize.Width, maSize.Height);
            }

            if(maPosition.X != 0 || maPosition.Y != 0)
            {
                // if global position is used, add it to transformation
                aTransformation.Translate(maPosition.X, maPosition.Y);
            }

            if(mnTransform.NeedsAction())
            {
                // transformation is used, apply to object.
                // NOTICE: The transformation is applied AFTER evtl. used
                // global positioning and scaling is used, so any shear or
                // rotate used herein is applied around the (0,0) position
                // of the PAGE object !!!
                Matrix3D aMat;
                mnTransform.GetFullTransform(aMat);

                // now add to transformation
                aTransformation *= aMat;
            }

            // now set transformation for this object
            uno::Any aAny;
            drawing::HomogenMatrix3 aMatrix;

            aMatrix.Line1.Column1 = aTransformation[0].X();
            aMatrix.Line1.Column2 = aTransformation[0].Y();
            aMatrix.Line1.Column3 = aTransformation[0].W();

            aMatrix.Line2.Column1 = aTransformation[1].X();
            aMatrix.Line2.Column2 = aTransformation[1].Y();
            aMatrix.Line2.Column3 = aTransformation[1].W();

            aMatrix.Line3.Column1 = aTransformation[2].X();
            aMatrix.Line3.Column2 = aTransformation[2].Y();
            aMatrix.Line3.Column3 = aTransformation[2].W();

            aAny <<= aMatrix;

            xPropSet->setPropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Transformation")), aAny);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::SetStyle()
{
    do
    {
        // set style on shape
        if(maDrawStyleName.getLength() == 0 || !mxShape.is())
            break;

        const SvXMLStyleContext* pStyle = 0L;
        sal_Bool bAutoStyle(FALSE);

        if(GetImport().GetShapeImport()->GetAutoStylesContext())
            pStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(mnStyleFamily, maDrawStyleName);

        if(pStyle)
            bAutoStyle = TRUE;

        if(!pStyle && GetImport().GetShapeImport()->GetStylesContext())
            pStyle = GetImport().GetShapeImport()->GetStylesContext()->FindStyleChildContext(mnStyleFamily, maDrawStyleName);

        if(NULL == pStyle || !pStyle->ISA(XMLShapeStyleContext) )
            break;

        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if( !xPropSet.is() )
            break;

        XMLPropStyleContext* pDocStyle =
            PTR_CAST( XMLShapeStyleContext, pStyle );

        if( pDocStyle->GetStyle().is() )
        {
            // set style on object
            uno::Any aAny;
            aAny <<= pDocStyle->GetStyle();
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Style")), aAny);
        }

        // we are finished if this is not a auto style
        if(!bAutoStyle)
            break;

        // check if this is a control shape
        // if so we must split the style for the shape and its control model
        uno::Reference< drawing::XControlShape > xControl( mxShape, uno::UNO_QUERY );
        if( xControl.is() )
        {
            uno::Reference< beans::XPropertySet > xControlModel( xControl->getControl(), uno::UNO_QUERY );
            if( !xControlModel.is() )
                break;

            xPropSet = PropertySetMerger_CreateInstance( xPropSet, xControlModel );
        }

        // set PropertySet on object
        pDocStyle->FillPropertySet(xPropSet);

    } while(0);
}

void SdXMLShapeContext::SetLayer()
{
    if( maLayerName.getLength() )
    {
        try
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is() )
            {
                uno::Any aAny;
                aAny <<= maLayerName;

                xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("LayerName")), aAny);
                return;
            }
        }
        catch( uno::Exception e )
        {
        }
        DBG_ERROR( "could not attach shape to layer!" );
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_zindex)) )
        {
            mnZOrder = rValue.toInt32();
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_id)) )
        {
            mnShapeId = rValue.toInt32();
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_name)) )
        {
            maShapeName = rValue;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_style_name)) )
        {
            maDrawStyleName = rValue;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_layer)) )
        {
            maLayerName = rValue;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_transform)) )
        {
            mnTransform.SetString(rValue, GetImport().GetMM100UnitConverter());
        }
    }
    else if( XML_NAMESPACE_PRESENTATION == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_user_transformed)) )
        {
            mbIsUserTransformed = rValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_true));
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_placeholder)) )
        {
            mbIsPlaceholder = rValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_true));
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_class)) )
        {
            maPresentationClass = rValue;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_style_name)) )
        {
            maDrawStyleName = rValue;
            mnStyleFamily = XML_STYLE_FAMILY_SD_PRESENTATION_ID;
        }
    }
    else if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_x)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maPosition.X, rValue);
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_y)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maPosition.Y, rValue);
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_width)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maSize.Width, rValue);
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_height)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maSize.Height, rValue);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLRectShapeContext, SdXMLShapeContext );

SdXMLRectShapeContext::SdXMLRectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnRadius( 0L )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLRectShapeContext::~SdXMLRectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLRectShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_corner_radius)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRadius, rValue);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLRectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.RectangleShape");
    if(mxShape.is())
    {
        // Add, set Style and properties from base shape
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        if(mnRadius)
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                uno::Any aAny;
                aAny <<= mnRadius;
                xPropSet->setPropertyValue(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius")), aAny);
            }
        }
        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLLineShapeContext, SdXMLShapeContext );

SdXMLLineShapeContext::SdXMLLineShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnX1( 0L ),
    mnY1( 0L ),
    mnX2( 1L ),
    mnY2( 1L )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLLineShapeContext::~SdXMLLineShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLLineShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_x1)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnX1, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_y1)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnY1, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_x2)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnX2, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_y2)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnY2, rValue);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLLineShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.LineShape");
    if(mxShape.is())
    {
        // Add, set Style and properties from base shape
        SetStyle();
        SetLayer();

        // set local parameters on shape
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            drawing::PointSequenceSequence aPolyPoly(1L);
            drawing::PointSequence* pOuterSequence = aPolyPoly.getArray();
            pOuterSequence->realloc(2L);
            awt::Point* pInnerSequence = pOuterSequence->getArray();
            uno::Any aAny;

            *pInnerSequence = awt::Point( mnX1, mnY1 );
            pInnerSequence++;
            *pInnerSequence = awt::Point( mnX2, mnY2 );

            aAny <<= aPolyPoly;
            xPropSet->setPropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygon")), aAny);
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLEllipseShapeContext, SdXMLShapeContext );

SdXMLEllipseShapeContext::SdXMLEllipseShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnCX( 0L ),
    mnCY( 0L ),
    mnRX( 1L ),
    mnRY( 1L ),
    meKind( drawing::CircleKind_FULL ),
    mnStartAngle( 0 ),
    mnEndAngle( 0 )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLEllipseShapeContext::~SdXMLEllipseShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLEllipseShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_rx)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRX, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_ry)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRY, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_cx)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnCX, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_cy)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnCY, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_r)) )
        {
            // single radius, it's a circle and both radii are the same
            GetImport().GetMM100UnitConverter().convertMeasure(mnRX, rValue);
            mnRY = mnRX;
            return;
        }
    }
    else if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_kind)) )
        {
            USHORT eKind;
            if( SvXMLUnitConverter::convertEnum( eKind, rValue, aXML_CircleKind_EnumMap ) )
            {
                meKind = eKind;
            }
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_start_angle)) )
        {
            double dStartAngle;
            if( SvXMLUnitConverter::convertDouble( dStartAngle, rValue ) )
                mnStartAngle = (sal_Int32)(dStartAngle * 100.0);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_end_angle)) )
        {
            double dEndAngle;
            if( SvXMLUnitConverter::convertDouble( dEndAngle, rValue ) )
                mnEndAngle = (sal_Int32)(dEndAngle * 100.0);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLEllipseShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.EllipseShape");
    if(mxShape.is())
    {
        // Add, set Style and properties from base shape
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        if( meKind != drawing::CircleKind_FULL )
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                uno::Any aAny;
                aAny <<= (drawing::CircleKind)meKind;
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleKind")), aAny );

                aAny <<= mnStartAngle;
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleStartAngle")), aAny );

                aAny <<= mnEndAngle;
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleEndAngle")), aAny );
            }
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPolygonShapeContext, SdXMLShapeContext );

SdXMLPolygonShapeContext::SdXMLPolygonShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes, sal_Bool bClosed)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mbClosed( bClosed )
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPolygonShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_viewBox)) )
        {
            maViewBox = rValue;
            return;
        }
    }
    else if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_points)) )
        {
            maPoints = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPolygonShapeContext::~SdXMLPolygonShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLPolygonShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // Add, set Style and properties from base shape
    if(mbClosed)
        AddShape("com.sun.star.drawing.PolyPolygonShape");
    else
        AddShape("com.sun.star.drawing.PolyLineShape");

    if( mxShape.is() )
    {
        SetStyle();
        SetLayer();

        // set local parameters on shape
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            // set polygon
            if(maPoints.getLength() && maViewBox.getLength())
            {
                SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
                awt::Size aSize(aViewBox.GetWidth(), aViewBox.GetHeight());
                awt::Point aPosition(aViewBox.GetX(), aViewBox.GetY());
                SdXMLImExPointsElement aPoints(maPoints, aViewBox,
                    aPosition, aSize, GetImport().GetMM100UnitConverter());

                uno::Any aAny;
                aAny <<= aPoints.GetPointSequenceSequence();
                xPropSet->setPropertyValue(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry")), aAny);
            }
        }

        // set pos, size, shear and rotate and get copy of matrix
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPathShapeContext, SdXMLShapeContext );

SdXMLPathShapeContext::SdXMLPathShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mbClosed( TRUE )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPathShapeContext::~SdXMLPathShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPathShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_viewBox)) )
        {
            maViewBox = rValue;
            return;
        }
        else if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_d)) )
        {
            maD = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLPathShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create polygon shape
    if(maD.getLength())
    {
        // prepare some of the parameters
        SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
        awt::Size aSize(aViewBox.GetWidth(), aViewBox.GetHeight());
        awt::Point aPosition(aViewBox.GetX(), aViewBox.GetY());
        SdXMLImExSvgDElement aPoints(maD, aViewBox,
            aPosition, aSize, GetImport().GetMM100UnitConverter());

        char* pService;
        // now create shape
        if(aPoints.IsCurve())
        {
            if(aPoints.IsClosed())
            {
                pService = "com.sun.star.drawing.ClosedBezierShape";
            }
            else
            {
                pService = "com.sun.star.drawing.OpenBezierShape";
            }
        }
        else
        {
            if(aPoints.IsClosed())
            {
                pService = "com.sun.star.drawing.PolyPolygonShape";
            }
            else
            {
                pService = "com.sun.star.drawing.PolyLineShape";
            }
        }

        // Add, set Style and properties from base shape
        AddShape(pService);
        if( mxShapes.is() )
        {
            SetStyle();
            SetLayer();

            // set local parameters on shape
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                uno::Any aAny;

                // set svg:d
                if(maD.getLength())
                {
                    if(aPoints.IsCurve())
                    {
                        drawing::PolyPolygonBezierCoords aSourcePolyPolygon(
                            aPoints.GetPointSequenceSequence(),
                            aPoints.GetFlagSequenceSequence());

                        aAny <<= aSourcePolyPolygon;
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry")), aAny);
                    }
                    else
                    {
                        aAny <<= aPoints.GetPointSequenceSequence();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry")), aAny);
                    }
                }
            }

            // set pos, size, shear and rotate
            SetTransformation();

            SdXMLShapeContext::StartElement(xAttrList);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLTextBoxShapeContext, SdXMLShapeContext );

SdXMLTextBoxShapeContext::SdXMLTextBoxShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLTextBoxShapeContext::~SdXMLTextBoxShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLTextBoxShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create textbox shape
    sal_Bool bIsPresShape(FALSE);

    char *pService;

    if(maPresentationClass.getLength())
    {
        if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_subtitle))))
        {
            // XmlShapeTypePresSubtitleShape
            pService = "com.sun.star.presentation.SubtitleShape";
        }
        else if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_outline))))
        {
            // XmlShapeTypePresOutlinerShape
            pService = "com.sun.star.presentation.OutlinerShape";
        }
        else if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_notes))))
        {
            // XmlShapeTypePresNotesShape
            pService = "com.sun.star.presentation.NotesShape";
        }
        else // if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_title))))
        {
            // XmlShapeTypePresTitleTextShape
            pService = "com.sun.star.presentation.TitleTextShape";
        }
        bIsPresShape = TRUE;
    }
    else
    {
        // normal text shape
        pService = "com.sun.star.drawing.TextShape";
    }

    // Add, set Style and properties from base shape
    AddShape(pService);

    if( mxShape.is() )
    {
        SetStyle();
        SetLayer();

        if(bIsPresShape)
        {
            uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
            if(xProps.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                if( xPropsInfo.is() )
                {
                    if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
                }
            }
        }

        // set parameters on shape
//A AW->CL: Eventually You need to strip scale and translate from the transformation
//A to reach the same goal again.
//A     if(!bIsPresShape || mbIsUserTransformed)
//A     {
//A         // set pos and size on shape, this should remove binding
//A         // to pres object on masterpage
//A         SetSizeAndPosition();
//A     }

        // set pos, size, shear and rotate
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLControlShapeContext, SdXMLShapeContext );

SdXMLControlShapeContext::SdXMLControlShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLControlShapeContext::~SdXMLControlShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLControlShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_FORM == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_id)) )
        {
            maFormId = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLControlShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Control shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.ControlShape");
    if( mxShape.is() )
    {
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        DBG_ASSERT( maFormId.getLength(), "draw:control without a form:id attribute!" );
        if( maFormId.getLength() )
        {
            uno::Reference< awt::XControlModel > xControlModel( GetImport().GetFormImport()->lookupControl( maFormId ), uno::UNO_QUERY );
            if( xControlModel.is() )
            {
                uno::Reference< drawing::XControlShape > xControl( mxShape, uno::UNO_QUERY );
                if( xControl.is() )
                    xControl->setControl(  xControlModel );

            }
        }
        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLConnectorShapeContext, SdXMLShapeContext );

SdXMLConnectorShapeContext::SdXMLConnectorShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maStart(0,0),
    maEnd(1,1),
    mnType( (USHORT)drawing::ConnectorType_STANDARD ),
    mnStartShapeId(-1),
    mnStartGlueId(-1),
    mnEndShapeId(-1),
    mnEndGlueId(-1),
    mnDelta1(0),
    mnDelta2(0),
    mnDelta3(0)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLConnectorShapeContext::~SdXMLConnectorShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLConnectorShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_start_shape)) )
        {
            mnStartShapeId = rValue.toInt32();
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_start_glue_point)) )
        {
            mnStartGlueId = rValue.toInt32();
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_end_shape)) )
        {
            mnEndShapeId = rValue.toInt32();
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_end_glue_point)) )
        {
            mnEndGlueId = rValue.toInt32();
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_line_skew)) )
        {
            SvXMLTokenEnumerator aTokenEnum( rValue );
            OUString aToken;
            if( aTokenEnum.getNextToken( aToken ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnDelta1, aToken);
                if( aTokenEnum.getNextToken( aToken ) )
                {
                    GetImport().GetMM100UnitConverter().convertMeasure(mnDelta2, aToken);
                    if( aTokenEnum.getNextToken( aToken ) )
                    {
                        GetImport().GetMM100UnitConverter().convertMeasure(mnDelta3, aToken);
                    }
                }
            }
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_type)) )
        {
            SvXMLUnitConverter::convertEnum( mnType, rValue, aXML_ConnectionKind_EnumMap );
            return;
        }
    }
    case XML_NAMESPACE_SVG:
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_x1)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.X, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_y1)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.Y, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_x2)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.X, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_y2)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.Y, rValue);
            return;
        }
    }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLConnectorShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Connector shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.ConnectorShape");
    if(mxShape.is())
    {
        // add connection ids
        if( mnStartShapeId != -1 )
            GetImport().GetShapeImport()->addShapeConnection( mxShape, sal_True, mnStartShapeId, mnStartGlueId );
        if( mnEndShapeId != -1 )
            GetImport().GetShapeImport()->addShapeConnection( mxShape, sal_False, mnEndShapeId, mnEndGlueId );

        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
        if( xProps.is() )
        {
            uno::Any aAny;
            aAny <<= maStart;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")), aAny);

            aAny <<= maEnd;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")), aAny );

            aAny <<= (drawing::ConnectorType)mnType;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeKind")), aAny );

            aAny <<= mnDelta1;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine1Delta")), aAny );

            aAny <<= mnDelta2;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine2Delta")), aAny );

            aAny <<= mnDelta3;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine3Delta")), aAny );
        }
        SetStyle();
        SetLayer();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLMeasureShapeContext, SdXMLShapeContext );

SdXMLMeasureShapeContext::SdXMLMeasureShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maStart(0,0),
    maEnd(1,1)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLMeasureShapeContext::~SdXMLMeasureShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLMeasureShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_SVG:
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_x1)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.X, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_y1)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.Y, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_x2)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.X, rValue);
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_y2)) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.Y, rValue);
            return;
        }
    }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLMeasureShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Measure shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.MeasureShape");
    if(mxShape.is())
    {
        SetStyle();
        SetLayer();
        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
        if( xProps.is() )
        {
            uno::Any aAny;
            aAny <<= maStart;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")), aAny);

            aAny <<= maEnd;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")), aAny );
        }

        // delete pre created fields
        uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
        if( xText.is() )
        {
            const OUString aEmpty( RTL_CONSTASCII_USTRINGPARAM( " " ) );
            xText->setString( aEmpty );
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

void SdXMLMeasureShapeContext::EndElement()
{
    SdXMLShapeContext::EndElement();

    do
    {
        // delete pre created fields
        uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
        if( !xText.is() )
            break;

        uno::Reference< text::XTextCursor > xCursor( xText->createTextCursor() );
        if( !xCursor.is() )
            break;

        const OUString aEmpty;
        xCursor->collapseToStart();
        xCursor->goRight( 1, sal_True );
        xCursor->setString( aEmpty );
    }
    while(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPageShapeContext, SdXMLShapeContext );

SdXMLPageShapeContext::SdXMLPageShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPageShapeContext::~SdXMLPageShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLPageShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Page shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.PageShape");
    if(mxShape.is())
    {
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLCaptionShapeContext, SdXMLShapeContext );

SdXMLCaptionShapeContext::SdXMLCaptionShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLCaptionShapeContext::~SdXMLCaptionShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLCaptionShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Caption shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.CaptionShape");
    if( mxShape.is() )
    {
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLGraphicObjectShapeContext, SdXMLShapeContext );

SdXMLGraphicObjectShapeContext::SdXMLGraphicObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maURL()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLGraphicObjectShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_XLINK == nPrefix )
    {
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_href)) )
        {
            maURL = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLGraphicObjectShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    // create graphic object shape
    AddShape( "com.sun.star.drawing.GraphicObjectShape" );
    if(mxShape.is())
    {
        SetStyle();
        SetLayer();
        if( !mbIsPlaceholder )
        {
            uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
            if(xProps.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                    xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                if( maURL.getLength() )
                {
                    uno::Any aAny;
                    aAny <<= GetImport().ResolveGraphicObjectURL( maURL, sal_True );
                    try
                    {
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL") ), aAny );
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicStreamURL") ), aAny );
                    }
                    catch (lang::IllegalArgumentException const &)
                    {
                    }
                }
            }
        }

        if(mbIsUserTransformed)
        {
            uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
            if(xProps.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                if( xPropsInfo.is() )
                {
                    if( xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
                }
            }
        }

        // set pos, size, shear and rotate
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLGraphicObjectShapeContext::~SdXMLGraphicObjectShapeContext()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLChartShapeContext, SdXMLShapeContext );

SdXMLChartShapeContext::SdXMLChartShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mpChartContext( NULL )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLChartShapeContext::~SdXMLChartShapeContext()
{
    if( mpChartContext )
        delete mpChartContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLChartShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    const sal_Bool bIsPresentation = maPresentationClass.getLength() != 0;
    AddShape( bIsPresentation ? "com.sun.star.presentation.ChartShape" : "com.sun.star.drawing.OLE2Shape" );

    if(mxShape.is())
    {
        SetStyle();
        SetLayer();

        if( !mbIsPlaceholder )
        {
            uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
            if(xProps.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                    xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                uno::Any aAny;

                const OUString aCLSID( RTL_CONSTASCII_USTRINGPARAM("12DCAE26-281F-416F-a234-c3086127382e"));

                aAny <<= aCLSID;
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID") ), aAny );

                aAny = xProps->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) );
                uno::Reference< frame::XModel > xChartModel;
                if( aAny >>= xChartModel )
                {
                    mpChartContext = GetImport().GetChartImport()->CreateChartContext( GetImport(), XML_NAMESPACE_SVG, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_chart)), xChartModel, xAttrList );
                }
            }
        }

        if(mbIsUserTransformed)
        {
            uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
            if(xProps.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                if( xPropsInfo.is() )
                {
                    if( xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
                }
            }
        }


        // set pos, size, shear and rotate
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);

        if( mpChartContext )
            mpChartContext->StartElement( xAttrList );
    }
}

void SdXMLChartShapeContext::EndElement()
{
    if( mpChartContext )
        mpChartContext->EndElement();
}

void SdXMLChartShapeContext::Characters( const ::rtl::OUString& rChars )
{
    if( mpChartContext )
        mpChartContext->Characters( rChars );
}

SvXMLImportContext * SdXMLChartShapeContext::CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if( mpChartContext )
        return mpChartContext->CreateChildContext( nPrefix, rLocalName, xAttrList );

    return NULL;
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLObjectShapeContext, SdXMLShapeContext );

SdXMLObjectShapeContext::SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
  mbPlaceHolder( sal_False ), mbUserTransformed( sal_False )
{
}

SdXMLObjectShapeContext::~SdXMLObjectShapeContext()
{
}

void SdXMLObjectShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    char* pService = "com.sun.star.drawing.OLE2Shape";

    sal_Bool bIsPresShape = maPresentationClass.getLength() != 0;
    if( bIsPresShape )
    {
        if( maPresentationClass.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_presentation_chart)) )
        {
            pService = "com.sun.star.presentation.ChartShape";
        }
        else if( maPresentationClass.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_presentation_table)) )
        {
            pService = "com.sun.star.presentation.TableShape";
        }
        else if( maPresentationClass.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_presentation_object)) )
        {
            pService = "com.sun.star.presentation.OLE2Shape";
        }
    }

    AddShape( pService );

    if( mxShape.is() )
    {
        SetLayer();

        if(bIsPresShape)
        {
            uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
            if(xProps.is())
            {
                uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                if( xPropsInfo.is() )
                {
                    if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
                }
            }
        }

        if( !mbIsPlaceholder && maHref.getLength() )
        {
            uno::Reference< container::XNamed > xNamed( mxShape, uno::UNO_QUERY );
            if( xNamed.is() )
            {
                OUString aName( GetImport().ResolveEmbeddedObjectURL( maHref, maCLSID ) );
                const OUString sURL(RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ));
                aName = aName.copy( sURL.getLength() );
                xNamed->setName( aName );
            }
        }

        // set pos, size, shear and rotate
        SetTransformation();
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLObjectShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_class_id)) )
        {
            maCLSID = rValue;
            return;
        }
        break;
    case XML_NAMESPACE_PRESENTATION:
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_class)) )
        {
            maClass = rValue;
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_placeholder)) )
        {
            mbPlaceHolder = rValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_true));
            return;
        }
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_user_transformed)) )
        {
            mbUserTransformed = rValue.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_true));
            return;
        }
        break;
    case XML_NAMESPACE_XLINK:
        if( rLocalName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM(sXML_href)) )
        {
            maHref = rValue;
            return;
        }
        break;
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}
