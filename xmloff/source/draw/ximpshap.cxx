/*************************************************************************
 *
 *  $RCSfile: ximpshap.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: cl $ $Date: 2000-11-02 10:56:21 $
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
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

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
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
    mnRotate( 0L ),
    mnStyleFamily(XML_STYLE_FAMILY_SD_GRAPHICS_ID),
    mbIsPlaceholder(FALSE),
    mbIsUserTransformed(FALSE)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_SHAPE_IS_USER_TRANSFORMED:
            {
                if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true))))
                    mbIsUserTransformed = TRUE;
                break;
            }
            case XML_TOK_SHAPE_IS_PLACEHOLDER:
            {
                if(sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true))))
                    mbIsPlaceholder = TRUE;
                break;
            }
            case XML_TOK_SHAPE_DRAWSTYLE_NAME_GRAPHICS:
            {
                maDrawStyleName = sValue;
                break;
            }
            case XML_TOK_SHAPE_PRESENTATION_CLASS:
            {
                maPresentationClass = sValue;
                break;
            }
            case XML_TOK_SHAPE_DRAWSTYLE_NAME_PRESENTATION:
            {
                maDrawStyleName = sValue;
                mnStyleFamily = XML_STYLE_FAMILY_SD_PRESENTATION_ID;
                break;
            }
            case XML_TOK_SHAPE_TRANSFORM:
            {
                SdXMLImExTransform aTransform(sValue, GetImport().GetMM100UnitConverter());
                if(aTransform.NeedsAction())
                {
                    double fVal(0.0);
                    if(aTransform.FindRotate(fVal) && fVal != 0.0)
                        mnRotate = (sal_Int32)(fVal * 100.0);
                }
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLShapeContext::~SdXMLShapeContext()
{
    if(mxCursor.is())
        GetImport().GetTextImport()->ResetCursor();

    if(mxOldCursor.is())
        GetImport().GetTextImport()->SetCursor( mxOldCursor );
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLShapeContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
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

    SvXMLImportContext * pContext = NULL;

    // if we have a text cursor, lets  try to import some text
    if( mxCursor.is() )
    {
        pContext = GetImport().GetTextImport()->CreateTextChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList );
    }

    // call parent for content
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // set parameters on shape
    if(mnRotate != 0L)
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            uno::Any aAny;
            aAny <<= mnRotate;
            xPropSet->setPropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("RotateAngle")), aAny);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::AddShape(uno::Reference< drawing::XShape >& xShape)
{
    if(xShape.is() && mxShapes.is())
    {
        // set shape local
        mxShape = xShape;

        // add new shape to parent
        mxShapes->add( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::SetStyle()
{
    // set style on shape
    if(maDrawStyleName.getLength() && mxShape.is())
    {
        const SvXMLStyleContext* pStyle = 0L;
        sal_Bool bAutoStyle(FALSE);

        if(GetImport().GetShapeImport()->GetAutoStylesContext())
            pStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(
            mnStyleFamily, maDrawStyleName);

        if(pStyle)
            bAutoStyle = TRUE;

        if(!pStyle && GetImport().GetShapeImport()->GetStylesContext())
            pStyle = GetImport().GetShapeImport()->GetStylesContext()->
            FindStyleChildContext(mnStyleFamily, maDrawStyleName);

        if(pStyle && pStyle->ISA(XMLShapeStyleContext))
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                XMLShapeStyleContext* pDocStyle = (XMLShapeStyleContext*)pStyle;

                if(pDocStyle->GetStyle().is())
                {
                    // set style on object
                    uno::Any aAny;
                    aAny <<= pDocStyle->GetStyle();
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Style")), aAny);
                }

                if(bAutoStyle)
                {
                    // set PropertySet on object
                    pDocStyle->FillPropertySet(xPropSet);
                }
            }
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
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L ),
    mnRadius( 0L )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetRectShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_RECTSHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_RECTSHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_RECTSHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_RECTSHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
            case XML_TOK_RECTSHAPE_CORNER_RADIUS:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnRadius, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLRectShapeContext::~SdXMLRectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLRectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.RectangleShape"))),
            uno::UNO_QUERY);
        if(xShape.is())
        {
            // Add, set Style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);

            // set local parameters on shape
            awt::Point aPoint(mnX, mnY);
            awt::Size aSize(mnWidth, mnHeight);
            xShape->setPosition(aPoint);
            xShape->setSize(aSize);

            if(mnRadius)
            {
                uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
                if(xPropSet.is())
                {
                    uno::Any aAny;
                    aAny <<= mnRadius;
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius")), aAny);
                }
            }
        }
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
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetLineShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_LINESHAPE_X1:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX1, sValue);
                break;
            }
            case XML_TOK_LINESHAPE_Y1:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY1, sValue);
                break;
            }
            case XML_TOK_LINESHAPE_X2:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX2, sValue);
                break;
            }
            case XML_TOK_LINESHAPE_Y2:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY2, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLLineShapeContext::~SdXMLLineShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLLineShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.LineShape"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // Add, set Style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);

            // set local parameters on shape
            uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
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
        }
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
    mnRY( 1L )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetEllipseShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_ELLIPSESHAPE_CX:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnCX, sValue);
                break;
            }
            case XML_TOK_ELLIPSESHAPE_CY:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnCY, sValue);
                break;
            }
            case XML_TOK_ELLIPSESHAPE_RX:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnRX, sValue);
                break;
            }
            case XML_TOK_ELLIPSESHAPE_RY:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnRY, sValue);
                break;
            }
            case XML_TOK_ELLIPSESHAPE_R:
            {
                // single radius, it's a circle and both radii are the same
                GetImport().GetMM100UnitConverter().convertMeasure(mnRX, sValue);
                mnRY = mnRX;
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLEllipseShapeContext::~SdXMLEllipseShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLEllipseShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.EllipseShape"))),
            uno::UNO_QUERY);
        if(xShape.is())
        {
            // Add, set Style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);

            // set local parameters on shape
            awt::Point aPoint(mnCX - mnRX, mnCY - mnRY);
            awt::Size aSize(mnRX + mnRX, mnRY + mnRY);
            xShape->setPosition(aPoint);
            xShape->setSize(aSize);
        }
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
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L ),
    mbClosed( bClosed )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetPolygonShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_POLYGONSHAPE_VIEWBOX:
            {
                maViewBox = sValue;
                break;
            }
            case XML_TOK_POLYGONSHAPE_POINTS:
            {
                maPoints = sValue;
                break;
            }
            case XML_TOK_POLYGONSHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_POLYGONSHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_POLYGONSHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_POLYGONSHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPolygonShapeContext::~SdXMLPolygonShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLPolygonShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            mbClosed ? OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyPolygonShape"))
            : OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyLineShape"))),
            uno::UNO_QUERY);
        if(xShape.is())
        {
            // Add, set Style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);

            // set local parameters on shape
            uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                // set parameters on shape
                awt::Point aPoint(mnX, mnY);
                awt::Size aSize(mnWidth, mnHeight);
                uno::Any aAny;

                xShape->setPosition(aPoint);
                xShape->setSize(aSize);

                // set polygon
                if(maPoints.getLength())
                {
                    SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
                    SdXMLImExPointsElement aPoints(maPoints, aViewBox, aPoint, aSize, GetImport().GetMM100UnitConverter());

                    aAny <<= aPoints.GetPointSequenceSequence();
                    xPropSet->setPropertyValue(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygon")), aAny);
                }
            }
        }
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
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L ),
    mbClosed( TRUE )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetPathShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PATHSHAPE_VIEWBOX:
            {
                maViewBox = sValue;
                break;
            }
            case XML_TOK_PATHSHAPE_D:
            {
                maD = sValue;
                break;
            }
            case XML_TOK_PATHSHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_PATHSHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_PATHSHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_PATHSHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPathShapeContext::~SdXMLPathShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLPathShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create polygon shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        if(maD.getLength())
        {
            // prepare some of the parameters
            uno::Reference< drawing::XShape > xShape;
            awt::Point aPoint(mnX, mnY);
            awt::Size aSize(mnWidth, mnHeight);
            SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
            SdXMLImExSvgDElement aPoints(maD, aViewBox, aPoint, aSize, GetImport().GetMM100UnitConverter());

            // now create shape
            if(aPoints.IsCurve())
            {
                if(aPoints.IsClosed())
                {
                    xShape = uno::Reference< drawing::XShape > (
                        xServiceFact->createInstance(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ClosedBezierShape"))),
                        uno::UNO_QUERY);
                }
                else
                {
                    xShape = uno::Reference< drawing::XShape > (
                        xServiceFact->createInstance(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OpenBezierShape"))),
                        uno::UNO_QUERY);
                }
            }
            else
            {
                if(aPoints.IsClosed())
                {
                    xShape = uno::Reference< drawing::XShape > (
                        xServiceFact->createInstance(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyPolygonShape"))),
                        uno::UNO_QUERY);
                }
                else
                {
                    xShape = uno::Reference< drawing::XShape > (
                        xServiceFact->createInstance(
                        OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PolyLineShape"))),
                        uno::UNO_QUERY);
                }
            }

            if(xShape.is())
            {
                // Add, set Style and properties from base shape
                AddShape(xShape);
                SetStyle();
                SdXMLShapeContext::StartElement(xAttrList);

                // set local parameters on shape
                uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
                if(xPropSet.is())
                {
                    uno::Any aAny;

                    xShape->setPosition(aPoint);
                    xShape->setSize(aSize);

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
                                OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygonBezier")), aAny);
                        }
                        else
                        {
                            aAny <<= aPoints.GetPointSequenceSequence();
                            xPropSet->setPropertyValue(
                                OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygon")), aAny);
                        }
                    }
                }
            }
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
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetTextBoxShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_TEXTBOXSHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_TEXTBOXSHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_TEXTBOXSHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_TEXTBOXSHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLTextBoxShapeContext::~SdXMLTextBoxShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLTextBoxShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create textbox shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape;
        sal_Bool bIsPresShape(FALSE);

        if(maPresentationClass.getLength())
        {
            if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_subtitle))))
            {
                // XmlShapeTypePresSubtitleShape
                xShape = uno::Reference< drawing::XShape > (xServiceFact->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.SubtitleShape"))), uno::UNO_QUERY);
                bIsPresShape = TRUE;
            }
            else if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_title))))
            {
                // XmlShapeTypePresTitleTextShape
                xShape = uno::Reference< drawing::XShape > (xServiceFact->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.TitleTextShape"))), uno::UNO_QUERY);
                bIsPresShape = TRUE;
            }
            else if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_outline))))
            {
                // XmlShapeTypePresOutlinerShape
                xShape = uno::Reference< drawing::XShape > (xServiceFact->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.OutlinerShape"))), uno::UNO_QUERY);
                bIsPresShape = TRUE;
            }
            else if(maPresentationClass.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_notes))))
            {
                // XmlShapeTypePresNotesShape
                xShape = uno::Reference< drawing::XShape > (xServiceFact->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.NotesShape"))), uno::UNO_QUERY);
                bIsPresShape = TRUE;
            }
        }
        else
        {
            // normal text shape
            xShape = uno::Reference< drawing::XShape > (xServiceFact->createInstance(
                OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.TextShape"))), uno::UNO_QUERY);
        }

        if(xShape.is())
        {
            // Add, set Style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);

            if(bIsPresShape && mbIsPlaceholder)
            {
                // Do something special if this is a placeholder (?)



            }

            if(!bIsPresShape || mbIsUserTransformed)
            {
                // set pos and size on shape, this should remove binding
                // to pres object on masterpage
                awt::Point aPoint(mnX, mnY);
                awt::Size aSize(mnWidth, mnHeight);
                xShape->setPosition(aPoint);
                xShape->setSize(aSize);
            }
        }
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
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetControlShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_CONTROLSHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_CONTROLSHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_CONTROLSHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_CONTROLSHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLControlShapeContext::~SdXMLControlShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLControlShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Control shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ControlShape"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);

            // set local parameters on shape
            awt::Point aPoint(mnX, mnY);
            awt::Size aSize(mnWidth, mnHeight);
            xShape->setPosition(aPoint);
            xShape->setSize(aSize);
        }
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
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLConnectorShapeContext::~SdXMLConnectorShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLConnectorShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Connector shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.ConnectorShape"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);
        }
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
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLMeasureShapeContext::~SdXMLMeasureShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLMeasureShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Measure shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.MeasureShape"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);
        }
    }
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
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetPageShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_PAGESHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_PAGESHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_PAGESHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_PAGESHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPageShapeContext::~SdXMLPageShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLPageShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Page shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.PageShape"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);

            // set local parameters on shape
            awt::Point aPoint(mnX, mnY);
            awt::Size aSize(mnWidth, mnHeight);
            xShape->setPosition(aPoint);
            xShape->setSize(aSize);
        }
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
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXMLShapeContext::StartElement(xAttrList);
        }
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
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L )
{
    OUString aURL;

    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetGraphicObjectShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_GOSHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_GOSHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_GOSHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_GOSHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
            case XML_TOK_GOSHAPE_URL:
            {
                aURL = sValue;
            }
        }
    }

    // create graphic object shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        OUString aType(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.GraphicObjectShape"));
        if( maPresentationClass.getLength() )
            aType = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.GraphicObjectShape"));

        uno::Reference< drawing::XShape > xShape( xServiceFact->createInstance( aType ), uno::UNO_QUERY);
        if(xShape.is())
        {
            rShapes->add( xShape );

            if( !mbIsPlaceholder )
            {
                uno::Reference< beans::XPropertySet > xProps(xShape, uno::UNO_QUERY);
                if(xProps.is())
                {
                    uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                    if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                    if( aURL.getLength() )
                    {
                        uno::Any aAny;
                        aAny <<= aURL;
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL") ), aAny );
                    }
                }
            }
        }

        SetStyle();

        // set local parameters on shape
        awt::Point aPoint(mnX, mnY);
        awt::Size aSize(mnWidth, mnHeight);
        xShape->setPosition(aPoint);
        xShape->setSize(aSize);

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
    mnX( 0L ),
    mnY( 0L ),
    mnWidth( 1L ),
    mnHeight( 1L ),
    mpChartContext( NULL )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->GetRectShapeAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_RECTSHAPE_X:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnX, sValue);
                break;
            }
            case XML_TOK_RECTSHAPE_Y:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnY, sValue);
                break;
            }
            case XML_TOK_RECTSHAPE_WIDTH:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnWidth, sValue);
                break;
            }
            case XML_TOK_RECTSHAPE_HEIGHT:
            {
                GetImport().GetMM100UnitConverter().convertMeasure(mnHeight, sValue);
                break;
            }
        }
    }

    // create rectangle shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        OUString aType(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.OLE2Shape"));
        if( maPresentationClass.getLength() )
            aType = OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.ChartShape"));

        uno::Reference< drawing::XShape > xShape( xServiceFact->createInstance( aType ), uno::UNO_QUERY);
        if(xShape.is())
        {
            rShapes->add( xShape );

            if( !mbIsPlaceholder )
            {
                uno::Reference< beans::XPropertySet > xProps(xShape, uno::UNO_QUERY);
                if(xProps.is())
                {
                    uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
                    if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                    uno::Any aAny;

                    const OUString aCLSID( RTL_CONSTASCII_USTRINGPARAM("BF884321-85DD-11D1-89d0-008029e4b0b1"));
                    aAny <<= aCLSID;
                    xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID") ), aAny );

                    aAny = xProps->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) );
                    uno::Reference< frame::XModel > xChartModel;
                    if( aAny >>= xChartModel )
                    {
                        mpChartContext = rImport.GetChartImport()->CreateChartContext( rImport, nPrfx, rLocalName, xChartModel, xAttrList );
                    }
                }
            }
        }

        SetStyle();

        // set local parameters on shape
        awt::Point aPoint(mnX, mnY);
        awt::Size aSize(mnWidth, mnHeight);
        xShape->setPosition(aPoint);
        xShape->setSize(aSize);
    }
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
    if( mpChartContext )
        mpChartContext->StartElement( xAttrList );
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

