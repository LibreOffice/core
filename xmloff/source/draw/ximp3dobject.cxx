/*************************************************************************
 *
 *  $RCSfile: ximp3dobject.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: sab $ $Date: 2001-03-16 14:35:56 $
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

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _XIMP3DOBJECT_HXX
#include "ximp3dobject.hxx"
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

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONSHAPE3D_HPP_
#include <com/sun/star/drawing/PolyPolygonShape3D.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_DOUBLESEQUENCE_HPP_
#include <com/sun/star/drawing/DoubleSequence.hpp>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DObjectContext, SvXMLImportContext );

SdXML3DObjectContext::SdXML3DObjectContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SvXMLImportContext( rImport, nPrfx, rLocalName ),
    mxShapes( rShapes ),
    mbSetTransform( FALSE )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DObjectAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_3DOBJECT_DRAWSTYLE_NAME:
            {
                maDrawStyleName = sValue;
                break;
            }
            case XML_TOK_3DOBJECT_TRANSFORM:
            {
                SdXMLImExTransform3D aTransform(sValue, GetImport().GetMM100UnitConverter());
                if(aTransform.NeedsAction())
                    mbSetTransform = aTransform.GetFullHomogenTransform(mxHomMat);
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DObjectContext::~SdXML3DObjectContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DObjectContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // set parameters
        if(mbSetTransform)
        {
            uno::Any aAny;
            aAny <<= mxHomMat;
            xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DTransformMatrix")), aAny);
        }

        // call parent
        SvXMLImportContext::StartElement(xAttrList);
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DObjectContext::EndElement()
{
    // call parent
    SvXMLImportContext::EndElement();
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DObjectContext::AddShape(uno::Reference< drawing::XShape >& xShape)
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

void SdXML3DObjectContext::SetStyle()
{
    // set style on shape
    if(maDrawStyleName.getLength() && mxShape.is())
    {
        const SvXMLStyleContext* pStyle = 0L;
        sal_Bool bAutoStyle(FALSE);

        if(GetImport().GetShapeImport()->GetAutoStylesContext())
            pStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(
            XML_STYLE_FAMILY_SD_GRAPHICS_ID, maDrawStyleName);

        if(pStyle)
            bAutoStyle = TRUE;

        if(!pStyle && GetImport().GetShapeImport()->GetStylesContext())
            pStyle = GetImport().GetShapeImport()->GetStylesContext()->
            FindStyleChildContext(XML_STYLE_FAMILY_SD_GRAPHICS_ID, maDrawStyleName);

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

TYPEINIT1( SdXML3DCubeObjectShapeContext, SdXML3DObjectContext);

SdXML3DCubeObjectShapeContext::SdXML3DCubeObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXML3DObjectContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maMinEdge(-2500.0, -2500.0, -2500.0),
    maMaxEdge(2500.0, 2500.0, 2500.0),
    mbMinEdgeUsed(FALSE),
    mbMaxEdgeUsed(FALSE)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DCubeObjectAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_3DCUBEOBJ_MINEDGE:
            {
                Vector3D aNewVec;
                GetImport().GetMM100UnitConverter().convertVector3D(aNewVec, sValue);

                if(aNewVec != maMinEdge)
                {
                    maMinEdge = aNewVec;
                    mbMinEdgeUsed = TRUE;
                }
                break;
            }
            case XML_TOK_3DCUBEOBJ_MAXEDGE:
            {
                Vector3D aNewVec;
                GetImport().GetMM100UnitConverter().convertVector3D(aNewVec, sValue);

                if(aNewVec != maMaxEdge)
                {
                    maMaxEdge = aNewVec;
                    mbMaxEdgeUsed = TRUE;
                }
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DCubeObjectShapeContext::~SdXML3DCubeObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DCubeObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DCubeObject"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXML3DObjectContext::StartElement(xAttrList);

            // set local parameters on shape
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                // set parameters
                drawing::Position3D aPosition3D;
                drawing::Direction3D aDirection3D;

                // convert from min, max to size to be set
                maMaxEdge = maMaxEdge - maMinEdge;

                aPosition3D.PositionX = maMinEdge.X();
                aPosition3D.PositionY = maMinEdge.Y();
                aPosition3D.PositionZ = maMinEdge.Z();

                aDirection3D.DirectionX = maMaxEdge.X();
                aDirection3D.DirectionY = maMaxEdge.Y();
                aDirection3D.DirectionZ = maMaxEdge.Z();

                uno::Any aAny;
                aAny <<= aPosition3D;
                xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")), aAny);
                aAny <<= aDirection3D;
                xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")), aAny);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DCubeObjectShapeContext::EndElement()
{
    // call parent
    SdXML3DObjectContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DSphereObjectShapeContext, SdXML3DObjectContext);

SdXML3DSphereObjectShapeContext::SdXML3DSphereObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXML3DObjectContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maCenter(0.0, 0.0, 0.0),
    maSize(5000.0, 5000.0, 5000.0),
    mbCenterUsed(FALSE),
    mbSizeUsed(FALSE)
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DSphereObjectAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_3DSPHEREOBJ_CENTER:
            {
                Vector3D aNewVec;
                GetImport().GetMM100UnitConverter().convertVector3D(aNewVec, sValue);

                if(aNewVec != maCenter)
                {
                    maCenter = aNewVec;
                    mbCenterUsed = TRUE;
                }
                break;
            }
            case XML_TOK_3DSPHEREOBJ_SIZE:
            {
                Vector3D aNewVec;
                GetImport().GetMM100UnitConverter().convertVector3D(aNewVec, sValue);

                if(aNewVec != maSize)
                {
                    maSize = aNewVec;
                    mbSizeUsed = TRUE;
                }
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DSphereObjectShapeContext::~SdXML3DSphereObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSphereObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DSphereObject"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXML3DObjectContext::StartElement(xAttrList);

            // set local parameters on shape
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                // set parameters
                drawing::Position3D aPosition3D;
                drawing::Direction3D aDirection3D;

                aPosition3D.PositionX = maCenter.X();
                aPosition3D.PositionY = maCenter.Y();
                aPosition3D.PositionZ = maCenter.Z();

                aDirection3D.DirectionX = maSize.X();
                aDirection3D.DirectionY = maSize.Y();
                aDirection3D.DirectionZ = maSize.Z();

                uno::Any aAny;
                aAny <<= aPosition3D;
                xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPosition")), aAny);
                aAny <<= aDirection3D;
                xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("D3DSize")), aAny);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DSphereObjectShapeContext::EndElement()
{
    // call parent
    SdXML3DObjectContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DPolygonBasedShapeContext, SdXML3DObjectContext );

SdXML3DPolygonBasedShapeContext::SdXML3DPolygonBasedShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXML3DObjectContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        OUString sValue = xAttrList->getValueByIndex( i );
        const SvXMLTokenMap& rAttrTokenMap = GetImport().GetShapeImport()->Get3DPolygonBasedAttrTokenMap();

        switch(rAttrTokenMap.Get(nPrefix, aLocalName))
        {
            case XML_TOK_3DPOLYGONBASED_VIEWBOX:
            {
                maViewBox = sValue;
                break;
            }
            case XML_TOK_3DPOLYGONBASED_D:
            {
                maPoints = sValue;
                break;
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DPolygonBasedShapeContext::~SdXML3DPolygonBasedShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DPolygonBasedShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // set parameters
        if(maPoints.getLength() && maViewBox.getLength())
        {
            SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
            awt::Point aMinPoint(aViewBox.GetX(), aViewBox.GetY());
            awt::Size aMaxSize(aViewBox.GetWidth(), aViewBox.GetHeight());
            SdXMLImExSvgDElement aPoints(maPoints, aViewBox,
                aMinPoint, aMaxSize, GetImport().GetMM100UnitConverter());

            // convert to double sequences
            drawing::PointSequenceSequence& xPoSeSe =
                (drawing::PointSequenceSequence&)aPoints.GetPointSequenceSequence();
            sal_Int32 nOuterSequenceCount = xPoSeSe.getLength();
            drawing::PointSequence* pInnerSequence = xPoSeSe.getArray();

            drawing::PolyPolygonShape3D xPolyPolygon3D;
            xPolyPolygon3D.SequenceX.realloc(nOuterSequenceCount);
            xPolyPolygon3D.SequenceY.realloc(nOuterSequenceCount);
            xPolyPolygon3D.SequenceZ.realloc(nOuterSequenceCount);
            drawing::DoubleSequence* pOuterSequenceX = xPolyPolygon3D.SequenceX.getArray();
            drawing::DoubleSequence* pOuterSequenceY = xPolyPolygon3D.SequenceY.getArray();
            drawing::DoubleSequence* pOuterSequenceZ = xPolyPolygon3D.SequenceZ.getArray();

            for(sal_Int32 a(0L); a < nOuterSequenceCount; a++)
            {
                sal_Int32 nInnerSequenceCount(pInnerSequence->getLength());
                awt::Point* pArray = pInnerSequence->getArray();

                pOuterSequenceX->realloc(nInnerSequenceCount);
                pOuterSequenceY->realloc(nInnerSequenceCount);
                pOuterSequenceZ->realloc(nInnerSequenceCount);
                double* pInnerSequenceX = pOuterSequenceX->getArray();
                double* pInnerSequenceY = pOuterSequenceY->getArray();
                double* pInnerSequenceZ = pOuterSequenceZ->getArray();

                for(sal_Int32 b(0L); b < nInnerSequenceCount; b++)
                {
                    *pInnerSequenceX++ = pArray->X;
                    *pInnerSequenceY++ = pArray->Y;
                    *pInnerSequenceZ++ = 0.0;
                    pArray++;
                }
                pInnerSequence++;

                pOuterSequenceX++;
                pOuterSequenceY++;
                pOuterSequenceZ++;
            }

            // set poly
            uno::Any aAny;
            aAny <<= xPolyPolygon3D;
            xPropSet->setPropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("D3DPolyPolygon3D")), aAny);
        }

        // call parent
        SdXML3DObjectContext::StartElement(xAttrList);
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DPolygonBasedShapeContext::EndElement()
{
    // call parent
    SdXML3DObjectContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DLatheObjectShapeContext, SdXML3DPolygonBasedShapeContext);

SdXML3DLatheObjectShapeContext::SdXML3DLatheObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXML3DPolygonBasedShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DLatheObjectShapeContext::~SdXML3DLatheObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DLatheObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DLatheObject"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXML3DPolygonBasedShapeContext::StartElement(xAttrList);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DLatheObjectShapeContext::EndElement()
{
    // call parent
    SdXML3DPolygonBasedShapeContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXML3DExtrudeObjectShapeContext, SdXML3DPolygonBasedShapeContext);

SdXML3DExtrudeObjectShapeContext::SdXML3DExtrudeObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXML3DPolygonBasedShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXML3DExtrudeObjectShapeContext::~SdXML3DExtrudeObjectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DExtrudeObjectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create shape
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.Shape3DExtrudeObject"))), uno::UNO_QUERY);
        if(xShape.is())
        {
            // add, set style and properties from base shape
            AddShape(xShape);
            SetStyle();
            SdXML3DPolygonBasedShapeContext::StartElement(xAttrList);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXML3DExtrudeObjectShapeContext::EndElement()
{
    // call parent
    SdXML3DPolygonBasedShapeContext::EndElement();
}

// EOF
