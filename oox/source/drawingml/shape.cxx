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

#include "oox/drawingml/shape.hxx"
#include "drawingml/customshapeproperties.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "drawingml/graphicproperties.hxx"
#include <drawingml/scene3dcontext.hxx>
#include "oox/drawingml/lineproperties.hxx"
#include "effectproperties.hxx"
#include "oox/drawingml/shapepropertymap.hxx"
#include "drawingml/textbody.hxx"
#include <drawingml/ThemeOverrideFragmentHandler.hxx>
#include "drawingml/table/tableproperties.hxx"
#include "oox/drawingml/chart/chartconverter.hxx"
#include "drawingml/chart/chartspacefragment.hxx"
#include "drawingml/chart/chartspacemodel.hxx"
#include "oox/ppt/pptimport.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshape.hxx"
#include "oox/vml/vmlshapecontainer.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/graphichelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/modelobjecthelper.hxx"

#include <tools/gen.hxx>
#include <tools/mapunit.hxx>
#include <editeng/unoprnms.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/xml/AttributeData.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/GraphicExportFilter.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/BorderLine2.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/style/ParagraphAdjust.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/document/XActionLockable.hpp>
#include <com/sun/star/chart2/data/XDataReceiver.hpp>
#include <svl/outstrm.hxx>
#include <svx/svdtrans.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/fltrcfg.hxx>
#include <vcl/graphic.hxx>
#include <vcl/graphicfilter.hxx>
#include <vcl/svapp.hxx>

#include <vcl/wmf.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;

namespace oox { namespace drawingml {

#define PUT_PROP( aProperties, nPos, sPropName, aPropValue ) \
    aProperties[nPos].Name = sPropName; \
    aProperties[nPos].Value = Any( aPropValue );

Shape::Shape( const sal_Char* pServiceName, bool bDefaultHeight )
: mbIsChild( false )
, mpLinePropertiesPtr( new LineProperties )
, mpShapeRefLinePropPtr( new LineProperties )
, mpFillPropertiesPtr( new FillProperties )
, mpShapeRefFillPropPtr( new FillProperties )
, mpGraphicPropertiesPtr( new GraphicProperties )
, mpCustomShapePropertiesPtr( new CustomShapeProperties )
, mp3DPropertiesPtr( new Shape3DProperties )
, mpEffectPropertiesPtr( new EffectProperties )
, mpShapeRefEffectPropPtr( new EffectProperties )
, mpMasterTextListStyle( new TextListStyle )
, mnSubType( 0 )
, meFrameType( FRAMETYPE_GENERIC )
, mnRotation( 0 )
, mbFlipH( false )
, mbFlipV( false )
, mbHidden( false )
, mbHiddenMasterShape( false )
, mbLockedCanvas( false )
, mbWps( false )
, mbTextBox( false )
, mbHasLinkedTxbx( false )
, maDiagramDoms( 0 )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
    setDefaults(bDefaultHeight);
}

Shape::Shape( const ShapePtr& pSourceShape )
: maChildren()
, mbIsChild( pSourceShape->mbIsChild )
, mpTextBody(pSourceShape->mpTextBody)
, mpLinePropertiesPtr( pSourceShape->mpLinePropertiesPtr )
, mpShapeRefLinePropPtr( pSourceShape->mpShapeRefLinePropPtr )
, mpFillPropertiesPtr( pSourceShape->mpFillPropertiesPtr )
, mpShapeRefFillPropPtr( pSourceShape->mpShapeRefFillPropPtr )
, mpGraphicPropertiesPtr( pSourceShape->mpGraphicPropertiesPtr )
, mpCustomShapePropertiesPtr( pSourceShape->mpCustomShapePropertiesPtr )
, mpTablePropertiesPtr( pSourceShape->mpTablePropertiesPtr )
, mp3DPropertiesPtr( pSourceShape->mp3DPropertiesPtr )
, mpEffectPropertiesPtr (pSourceShape->mpEffectPropertiesPtr)
, mpShapeRefEffectPropPtr(pSourceShape->mpShapeRefEffectPropPtr)
, maShapeProperties( pSourceShape->maShapeProperties )
, mpMasterTextListStyle( pSourceShape->mpMasterTextListStyle )
, mxShape()
, msServiceName( pSourceShape->msServiceName )
, msName( pSourceShape->msName )
, msId( pSourceShape->msId )
, mnSubType( pSourceShape->mnSubType )
, moSubTypeIndex( pSourceShape->moSubTypeIndex )
, maShapeStyleRefs( pSourceShape->maShapeStyleRefs )
, maSize( pSourceShape->maSize )
, maPosition( pSourceShape->maPosition )
, meFrameType( pSourceShape->meFrameType )
, mnRotation( pSourceShape->mnRotation )
, mbFlipH( pSourceShape->mbFlipH )
, mbFlipV( pSourceShape->mbFlipV )
, mbHidden( pSourceShape->mbHidden )
, mbHiddenMasterShape( pSourceShape->mbHiddenMasterShape )
, mbLockedCanvas( pSourceShape->mbLockedCanvas )
, mbWps( pSourceShape->mbWps )
, mbTextBox( pSourceShape->mbTextBox )
, maLinkedTxbxAttr()
, mbHasLinkedTxbx(false)
, maDiagramDoms( pSourceShape->maDiagramDoms )
{}

Shape::~Shape()
{
}

table::TablePropertiesPtr Shape::getTableProperties()
{
    if ( !mpTablePropertiesPtr.get() )
        mpTablePropertiesPtr.reset( new table::TableProperties() );
    return mpTablePropertiesPtr;
}

void Shape::setDefaults(bool bHeight)
{
    maDefaultShapeProperties.setProperty(PROP_TextAutoGrowHeight, false);
    maDefaultShapeProperties.setProperty(PROP_TextWordWrap, true);
    maDefaultShapeProperties.setProperty(PROP_TextLeftDistance, static_cast< sal_Int32 >( 250 ));
    maDefaultShapeProperties.setProperty(PROP_TextUpperDistance, static_cast< sal_Int32 >( 125 ));
    maDefaultShapeProperties.setProperty(PROP_TextRightDistance, static_cast< sal_Int32 >( 250 ));
    maDefaultShapeProperties.setProperty(PROP_TextLowerDistance, static_cast< sal_Int32 >( 125 ));
    if (bHeight)
        maDefaultShapeProperties.setProperty(PROP_CharHeight, static_cast< float >( 18.0 ));
    maDefaultShapeProperties.setProperty(PROP_TextVerticalAdjust, TextVerticalAdjust_TOP);
    maDefaultShapeProperties.setProperty(PROP_ParaAdjust, static_cast< sal_Int16 >( ParagraphAdjust_LEFT )); // check for RTL?
}

::oox::vml::OleObjectInfo& Shape::setOleObjectType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setOleObjectType - multiple frame types" );
    meFrameType = FRAMETYPE_OLEOBJECT;
    mxOleObjectInfo.reset( new ::oox::vml::OleObjectInfo( true ) );
    return *mxOleObjectInfo;
}

ChartShapeInfo& Shape::setChartType( bool bEmbedShapes )
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setChartType - multiple frame types" );
    meFrameType = FRAMETYPE_CHART;
    msServiceName = "com.sun.star.drawing.OLE2Shape";
    mxChartShapeInfo.reset( new ChartShapeInfo( bEmbedShapes ) );
    return *mxChartShapeInfo;
}

void Shape::setDiagramType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setDiagramType - multiple frame types" );
    meFrameType = FRAMETYPE_DIAGRAM;
    msServiceName = "com.sun.star.drawing.GroupShape";
    mnSubType = 0;
}

void Shape::setTableType()
{
    OSL_ENSURE( meFrameType == FRAMETYPE_GENERIC, "Shape::setTableType - multiple frame types" );
    meFrameType = FRAMETYPE_TABLE;
    msServiceName = "com.sun.star.drawing.TableShape";
    mnSubType = 0;
}

void Shape::setServiceName( const sal_Char* pServiceName )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
}

const ShapeStyleRef* Shape::getShapeStyleRef( sal_Int32 nRefType ) const
{
    ShapeStyleRefMap::const_iterator aIt = maShapeStyleRefs.find( nRefType );
    return (aIt == maShapeStyleRefs.end()) ? nullptr : &aIt->second;
}

void Shape::addShape(
        ::oox::core::XmlFilterBase& rFilterBase,
        const Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        const basegfx::B2DHomMatrix& aTransformation,
        FillProperties& rShapeOrParentShapeFillProps,
        const awt::Rectangle* pShapeRect,
        ShapeIdMap* pShapeMap )
{
    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << " id: " << msId);

    try
    {
        OUString sServiceName( msServiceName );
        if( !sServiceName.isEmpty() )
        {
            basegfx::B2DHomMatrix aMatrix( aTransformation );
            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, pTheme, rxShapes, pShapeRect, false, false, aMatrix, rShapeOrParentShapeFillProps ) );

            if( pShapeMap && !msId.isEmpty() )
            {
                (*pShapeMap)[ msId ] = shared_from_this();
            }

            // if this is a group shape, we have to add also each child shape
            Reference< XShapes > xShapes( xShape, UNO_QUERY );
            if ( xShapes.is() )
                addChildren( rFilterBase, *this, pTheme, xShapes, pShapeRect ? *pShapeRect : awt::Rectangle( maPosition.X, maPosition.Y, maSize.Width, maSize.Height ), pShapeMap, aMatrix );

            if( meFrameType == FRAMETYPE_DIAGRAM )
            {
                if( !SvtFilterOptions::Get().IsSmartArt2Shape() )
                    keepDiagramCompatibilityInfo( rFilterBase );
            }
        }
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", OSL_THIS_FUNC << "Exception: " << e.Message );
    }
}

void Shape::setLockedCanvas(bool bLockedCanvas)
{
    mbLockedCanvas = bLockedCanvas;
}

void Shape::setWps(bool bWps)
{
    mbWps = bWps;
}

void Shape::setTextBox(bool bTextBox)
{
    mbTextBox = bTextBox;
}

void Shape::applyShapeReference( const Shape& rReferencedShape, bool bUseText )
{
    SAL_INFO("oox", OSL_THIS_FUNC << "apply shape reference: " << rReferencedShape.msId << " to shape id: " << msId);

    if ( rReferencedShape.mpTextBody.get() && bUseText )
        mpTextBody = TextBodyPtr( new TextBody( *rReferencedShape.mpTextBody.get() ) );
    else
        mpTextBody.reset();
    maShapeProperties = rReferencedShape.maShapeProperties;
    mpShapeRefLinePropPtr = LinePropertiesPtr( new LineProperties( *rReferencedShape.mpLinePropertiesPtr.get() ) );
    mpShapeRefFillPropPtr = FillPropertiesPtr( new FillProperties( *rReferencedShape.mpFillPropertiesPtr.get() ) );
    mpCustomShapePropertiesPtr = CustomShapePropertiesPtr( new CustomShapeProperties( *rReferencedShape.mpCustomShapePropertiesPtr.get() ) );
    mpTablePropertiesPtr = table::TablePropertiesPtr( rReferencedShape.mpTablePropertiesPtr.get() ? new table::TableProperties( *rReferencedShape.mpTablePropertiesPtr.get() ) : nullptr );
    mpShapeRefEffectPropPtr = EffectPropertiesPtr( new EffectProperties( *rReferencedShape.mpEffectPropertiesPtr.get() ) );
    mpMasterTextListStyle = TextListStylePtr( new TextListStyle( *rReferencedShape.mpMasterTextListStyle.get() ) );
    maShapeStyleRefs = rReferencedShape.maShapeStyleRefs;
    maSize = rReferencedShape.maSize;
    maPosition = rReferencedShape.maPosition;
    mnRotation = rReferencedShape.mnRotation;
    mbFlipH = rReferencedShape.mbFlipH;
    mbFlipV = rReferencedShape.mbFlipV;
    mbHidden = rReferencedShape.mbHidden;
}

void Shape::addChildren( ::oox::core::XmlFilterBase& rFilterBase,
                         const Theme* pTheme,
                         const Reference< XShapes >& rxShapes,
                         basegfx::B2DHomMatrix& aTransformation,
                         const awt::Rectangle* pShapeRect,
                         ShapeIdMap* pShapeMap )
{
    addChildren(rFilterBase, *this, pTheme, rxShapes,
                pShapeRect ?
                 *pShapeRect :
                 awt::Rectangle( maPosition.X, maPosition.Y, maSize.Width, maSize.Height ),
                pShapeMap, aTransformation);
}

struct ActionLockGuard
{
    explicit ActionLockGuard(Reference<drawing::XShape> const& xShape)
        : m_xLockable(xShape, UNO_QUERY)
    {
        if (m_xLockable.is()) {
            m_xLockable->addActionLock();
        }
    }
    ~ActionLockGuard()
    {
        if (m_xLockable.is()) {
            m_xLockable->removeActionLock();
        }
    }
private:
    Reference<document::XActionLockable> m_xLockable;
};

// for group shapes, the following method is also adding each child
void Shape::addChildren(
        XmlFilterBase& rFilterBase,
        Shape& rMaster,
        const Theme* pTheme,
        const Reference< XShapes >& rxShapes,
        const awt::Rectangle&,
        ShapeIdMap* pShapeMap,
        const basegfx::B2DHomMatrix& aTransformation )
{
    basegfx::B2DHomMatrix aChildTransformation;

    aChildTransformation.translate(-maChPosition.X, -maChPosition.Y);
    aChildTransformation.scale(1/(maChSize.Width ? maChSize.Width : 1.0), 1/(maChSize.Height ? maChSize.Height : 1.0));

    // Child position and size is typically non-zero, but it's allowed to have
    // it like that, and in that case Word ignores the parent transformation
    // (excluding translate component).
    if (!mbWps || maChPosition.X || maChPosition.Y || maChSize.Width || maChSize.Height)
    {
        aChildTransformation *= aTransformation;
    }
    else
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        aTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
        aChildTransformation.translate(aTranslate.getX(), aTranslate.getY());
    }

    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "parent matrix:\n"
             << aChildTransformation.get(0, 0) << " "
             << aChildTransformation.get(0, 1) << " "
             << aChildTransformation.get(0, 2) << "\n"
             << aChildTransformation.get(1, 0) << " "
             << aChildTransformation.get(1, 1) << " "
             << aChildTransformation.get(1, 2) << "\n"
             << aChildTransformation.get(2, 0) << " "
             << aChildTransformation.get(2, 1) << " "
             << aChildTransformation.get(2, 2));

    std::vector< ShapePtr >::iterator aIter( rMaster.maChildren.begin() );
    while( aIter != rMaster.maChildren.end() ) {
        (*aIter)->setMasterTextListStyle( mpMasterTextListStyle );
        (*aIter++)->addShape( rFilterBase, pTheme, rxShapes, aChildTransformation, getFillProperties(), nullptr, pShapeMap );
    }
}

Reference< XShape > Shape::createAndInsert(
        ::oox::core::XmlFilterBase& rFilterBase,
        const OUString& rServiceName,
        const Theme* pTheme,
        const css::uno::Reference< css::drawing::XShapes >& rxShapes,
        const awt::Rectangle* /* pShapeRect */,
        bool bClearText,
        bool bDoNotInsertEmptyTextBody,
        basegfx::B2DHomMatrix& aParentTransformation,
        FillProperties& rShapeOrParentShapeFillProps )
{
    bool bIsEmbMedia = false;
    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << " id: " << msId);

    // tdf#90403 PowerPoint ignores a:ext cx and cy values of p:xfrm, and uses real table width and height
    if ( mpTablePropertiesPtr.get() && rServiceName == "com.sun.star.drawing.TableShape" )
    {
        maSize.Width = 0;
        for( std::vector< sal_Int32 >::const_iterator aTableColIter(mpTablePropertiesPtr->getTableGrid().begin());
             aTableColIter != mpTablePropertiesPtr->getTableGrid().end(); ++aTableColIter )
        {
            maSize.Width += *aTableColIter;
        }
        maSize.Height = 0;
        for( std::vector< ::oox::drawingml::table::TableRow >::const_iterator aTableRowIter(mpTablePropertiesPtr->getTableRows().begin());
             aTableRowIter != mpTablePropertiesPtr->getTableRows().end(); ++aTableRowIter )
        {
            maSize.Height += (*aTableRowIter).getHeight();
        }
    }

    awt::Rectangle aShapeRectHmm( maPosition.X / EMU_PER_HMM, maPosition.Y / EMU_PER_HMM, maSize.Width / EMU_PER_HMM, maSize.Height / EMU_PER_HMM );

    OUString aServiceName;
    if( rServiceName == "com.sun.star.drawing.GraphicObjectShape" &&
        mpGraphicPropertiesPtr && !mpGraphicPropertiesPtr->m_sMediaPackageURL.isEmpty())
    {
        aServiceName = finalizeServiceName( rFilterBase, "com.sun.star.presentation.MediaShape", aShapeRectHmm );
        bIsEmbMedia = true;
    }
    else
    {
        aServiceName = finalizeServiceName( rFilterBase, rServiceName, aShapeRectHmm );
    }
    bool bIsCustomShape = ( aServiceName == "com.sun.star.drawing.CustomShape" ||
                            aServiceName == "com.sun.star.drawing.ConnectorShape" );
    bool bUseRotationTransform = ( !mbWps ||
            aServiceName == "com.sun.star.drawing.LineShape" ||
            aServiceName == "com.sun.star.drawing.GroupShape" ||
            mbFlipH ||
            mbFlipV );

    basegfx::B2DHomMatrix aTransformation;

    if( maSize.Width != 1 || maSize.Height != 1)
    {
        // take care there are no zeros used by error
        aTransformation.scale(
            maSize.Width ? maSize.Width : 1.0,
            maSize.Height ? maSize.Height : 1.0 );
    }

    if( mbFlipH || mbFlipV || mnRotation != 0)
    {
        // calculate object's center
        basegfx::B2DPoint aCenter(0.5, 0.5);
        aCenter *= aTransformation;

        // center object at origin
        aTransformation.translate( -aCenter.getX(), -aCenter.getY() );

        if( !bIsCustomShape && ( mbFlipH || mbFlipV ) )
        {
            // mirror around object's center
            aTransformation.scale( mbFlipH ? -1.0 : 1.0, mbFlipV ? -1.0 : 1.0 );
        }

        if( bUseRotationTransform && mnRotation != 0 )
        {
            // rotate around object's center
            aTransformation.rotate( F_PI180 * ( (double)mnRotation / 60000.0 ) );
        }

        // move object back from center
        aTransformation.translate( aCenter.getX(), aCenter.getY() );
    }

    bool bInGroup = !aParentTransformation.isIdentity();
    if( maPosition.X != 0 || maPosition.Y != 0)
    {
        // if global position is used, add it to transformation
        if (mbWps && !bInGroup)
            aTransformation.translate( maPosition.X * EMU_PER_HMM, maPosition.Y * EMU_PER_HMM);
        else
            aTransformation.translate( maPosition.X, maPosition.Y );
    }

    aTransformation = aParentTransformation*aTransformation;
    aParentTransformation = aTransformation;
    aTransformation.scale(1/double(EMU_PER_HMM), 1/double(EMU_PER_HMM));

    // special for lineshape
    if ( aServiceName == "com.sun.star.drawing.LineShape" )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        // now creating the corresponding PolyPolygon
        sal_Int32 i, nNumPoints = aPoly.count();
        uno::Sequence< awt::Point > aPointSequence( nNumPoints );
        awt::Point* pPoints = aPointSequence.getArray();
        uno::Reference<lang::XServiceInfo> xModelInfo(rFilterBase.getModel(), uno::UNO_QUERY);
        bool bIsWriter = xModelInfo->supportsService("com.sun.star.text.TextDocument");
        for( i = 0; i < nNumPoints; ++i )
        {
            basegfx::B2DPoint aPoint( aPoly.getB2DPoint( i ) );

            // Guard against zero width or height.
            if (i)
            {
                const basegfx::B2DPoint& rPreviousPoint = aPoly.getB2DPoint(i - 1);
                if (aPoint.getX() - rPreviousPoint.getX() == 0)
                    aPoint.setX(aPoint.getX() + 1);
                if (aPoint.getY() - rPreviousPoint.getY() == 0)
                    aPoint.setY(aPoint.getY() + 1);
            }

            if (bIsWriter && bInGroup)
                // Writer's draw page is in twips, and these points get passed
                // to core without any unit conversion when Writer
                // postprocesses only the group shape itself.
                pPoints[i] = awt::Point(static_cast<sal_Int32>(convertMm100ToTwip(aPoint.getX())), static_cast<sal_Int32>(convertMm100ToTwip(aPoint.getY())));
            else
                pPoints[i] = awt::Point(static_cast<sal_Int32>(aPoint.getX()), static_cast<sal_Int32>(aPoint.getY()));
        }
        uno::Sequence< uno::Sequence< awt::Point > > aPolyPolySequence( 1 );
        aPolyPolySequence.getArray()[ 0 ] = aPointSequence;

        maShapeProperties.setProperty(PROP_PolyPolygon, aPolyPolySequence);
    }
    else if ( aServiceName == "com.sun.star.drawing.ConnectorShape" )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        basegfx::B2DPoint aStartPosition( aPoly.getB2DPoint( 0 ) );
        basegfx::B2DPoint aEndPosition( aPoly.getB2DPoint( 1 ) );
        awt::Point aAWTStartPosition( static_cast< sal_Int32 >( aStartPosition.getX() ), static_cast< sal_Int32 >( aStartPosition.getY() ) );
        awt::Point aAWTEndPosition( static_cast< sal_Int32 >( aEndPosition.getX() ), static_cast< sal_Int32 >( aEndPosition.getY() ) );

        maShapeProperties.setProperty(PROP_StartPosition, aAWTStartPosition);
        maShapeProperties.setProperty(PROP_EndPosition, aAWTEndPosition);
    }
    else
    {
        // now set transformation for this object
        HomogenMatrix3 aMatrix;

        aMatrix.Line1.Column1 = aTransformation.get(0,0);
        aMatrix.Line1.Column2 = aTransformation.get(0,1);
        aMatrix.Line1.Column3 = aTransformation.get(0,2);

        aMatrix.Line2.Column1 = aTransformation.get(1,0);
        aMatrix.Line2.Column2 = aTransformation.get(1,1);
        aMatrix.Line2.Column3 = aTransformation.get(1,2);

        aMatrix.Line3.Column1 = aTransformation.get(2,0);
        aMatrix.Line3.Column2 = aTransformation.get(2,1);
        aMatrix.Line3.Column3 = aTransformation.get(2,2);

        maShapeProperties.setProperty(PROP_Transformation, aMatrix);
    }

    Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
    if ( !mxShape.is() )
        mxShape.set( xServiceFact->createInstance( aServiceName ), UNO_QUERY_THROW );

    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    if( mxShape.is() && xSet.is() )
    {
        if( !msName.isEmpty() )
        {
            Reference< container::XNamed > xNamed( mxShape, UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( msName );
        }
        if (aServiceName != "com.sun.star.text.TextFrame")
            rxShapes->add( mxShape );

        if ( mbHidden || mbHiddenMasterShape )
        {
            SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "invisible shape with id: " << msId);
            const OUString sVisible( "Visible" );
            xSet->setPropertyValue( sVisible, Any( sal_False ) );
        }

        ActionLockGuard const alg(mxShape);

        // sj: removing default text of placeholder objects such as SlideNumberShape or HeaderShape
        if ( bClearText )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            if ( xText.is() )
            {
                xText->setString( "" );
            }
        }

        const GraphicHelper& rGraphicHelper = rFilterBase.getGraphicHelper();

        LineProperties aLineProperties;
        aLineProperties.maLineFill.moFillType = XML_noFill;
        sal_Int32 nLinePhClr = -1;
        FillProperties aFillProperties;
        aFillProperties.moFillType = XML_noFill;
        sal_Int32 nFillPhClr = -1;
        EffectProperties aEffectProperties;
        // TODO: use ph color when applying effect properties
        //sal_Int32 nEffectPhClr = -1;

        // First apply reference shape's properties (shape on the master slide)
        aFillProperties.assignUsed( *mpShapeRefFillPropPtr );
        aLineProperties.assignUsed( *mpShapeRefLinePropPtr );
        aEffectProperties.assignUsed( *mpShapeRefEffectPropPtr );

        if( pTheme )
        {
            if( const ShapeStyleRef* pLineRef = getShapeStyleRef( XML_lnRef ) )
            {
                if( const LineProperties* pLineProps = pTheme->getLineStyle( pLineRef->mnThemedIdx ) )
                    aLineProperties.assignUsed( *pLineProps );
                nLinePhClr = pLineRef->maPhClr.getColor( rGraphicHelper );

                // Store style-related properties to InteropGrabBag to be able to export them back
                Sequence< PropertyValue > aProperties( 7 );
                PUT_PROP( aProperties, 0, "SchemeClr",      pLineRef->maPhClr.getSchemeName() );
                PUT_PROP( aProperties, 1, "Idx",            pLineRef->mnThemedIdx );
                PUT_PROP( aProperties, 2, "Color",          nLinePhClr );
                PUT_PROP( aProperties, 3, "LineStyle",      aLineProperties.getLineStyle() );
                PUT_PROP( aProperties, 4, "LineJoint",      aLineProperties.getLineJoint() );
                PUT_PROP( aProperties, 5, "LineWidth",      aLineProperties.getLineWidth() );
                PUT_PROP( aProperties, 6, "Transformations", pLineRef->maPhClr.getTransformations() );
                putPropertyToGrabBag( "StyleLnRef", Any( aProperties ) );
            }
            if( const ShapeStyleRef* pFillRef = getShapeStyleRef( XML_fillRef ) )
            {
                if( const FillProperties* pFillProps = pTheme->getFillStyle( pFillRef->mnThemedIdx ) )
                    aFillProperties.assignUsed( *pFillProps );
                nFillPhClr = pFillRef->maPhClr.getColor( rGraphicHelper );

                OUString sColorScheme = pFillRef->maPhClr.getSchemeName();
                if( !sColorScheme.isEmpty() )
                {
                    Sequence< PropertyValue > aProperties(4);
                    PUT_PROP( aProperties, 0, "SchemeClr",      sColorScheme );
                    PUT_PROP( aProperties, 1, "Idx",            pFillRef->mnThemedIdx );
                    PUT_PROP( aProperties, 2, "Color",          nFillPhClr );
                    PUT_PROP( aProperties, 3, "Transformations", pFillRef->maPhClr.getTransformations() );

                    putPropertyToGrabBag( "StyleFillRef", Any( aProperties ) );
                }
            }
            if( const ShapeStyleRef* pEffectRef = getShapeStyleRef( XML_effectRef ) )
            {
                if( const EffectProperties* pEffectProps = pTheme->getEffectStyle( pEffectRef->mnThemedIdx ) )
                    aEffectProperties.assignUsed( *pEffectProps );
                // TODO: use ph color when applying effect properties
                // nEffectPhClr = pEffectRef->maPhClr.getColor( rGraphicHelper );

                // Store style-related properties to InteropGrabBag to be able to export them back
                Sequence< PropertyValue > aProperties( 3 );
                PUT_PROP( aProperties, 0, "SchemeClr",      pEffectRef->maPhClr.getSchemeName() );
                PUT_PROP( aProperties, 1, "Idx",            pEffectRef->mnThemedIdx );
                PUT_PROP( aProperties, 2, "Transformations", pEffectRef->maPhClr.getTransformations() );
                putPropertyToGrabBag( "StyleEffectRef", Any( aProperties ) );
            }
        }

        aLineProperties.assignUsed( getLineProperties() );

        // group fill inherits from parent
        if ( getFillProperties().moFillType.has() && getFillProperties().moFillType.get() == XML_grpFill )
            getFillProperties().assignUsed( rShapeOrParentShapeFillProps );
        aFillProperties.assignUsed( getFillProperties() );
        aEffectProperties.assignUsed ( getEffectProperties() );

        ShapePropertyMap aShapeProps( rFilterBase.getModelObjectHelper() );

        // add properties from textbody to shape properties
        if( mpTextBody.get() )
        {
            mpTextBody->getTextProperties().pushRotationAdjustments();
            aShapeProps.assignUsed( mpTextBody->getTextProperties().maPropertyMap );
            // Push char properties as well - specifically useful when this is a placeholder
            if( mpMasterTextListStyle &&  mpMasterTextListStyle->getListStyle()[0]->getTextCharacterProperties().moHeight.has() )
                aShapeProps.setProperty(PROP_CharHeight, GetFontHeight( mpMasterTextListStyle->getListStyle()[0]->getTextCharacterProperties().moHeight.get() ));
        }

        // applying properties
        aShapeProps.assignUsed( getShapeProperties() );
        aShapeProps.assignUsed( maDefaultShapeProperties );
        if ( bIsEmbMedia || aServiceName == "com.sun.star.drawing.GraphicObjectShape" || aServiceName == "com.sun.star.drawing.OLE2Shape" )
            mpGraphicPropertiesPtr->pushToPropMap( aShapeProps, rGraphicHelper );
        if ( mpTablePropertiesPtr.get() && aServiceName == "com.sun.star.drawing.TableShape" )
            mpTablePropertiesPtr->pushToPropSet( rFilterBase, xSet, mpMasterTextListStyle );
        aFillProperties.pushToPropMap( aShapeProps, rGraphicHelper, mnRotation, nFillPhClr, mbFlipH, mbFlipV );
        aLineProperties.pushToPropMap( aShapeProps, rGraphicHelper, nLinePhClr );
        // TODO: use ph color when applying effect properties
        aEffectProperties.pushToPropMap( aShapeProps, rGraphicHelper );

        // applying autogrowheight property before setting shape size, because
        // the shape size might be changed if currently autogrowheight is true
        // we must also check that the PropertySet supports the property.
        Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
        const OUString& rPropName = PropertyMap::getPropertyName( PROP_TextAutoGrowHeight );
        if( xSetInfo.is() && xSetInfo->hasPropertyByName( rPropName ) )
            if( aShapeProps.hasProperty( PROP_TextAutoGrowHeight ) )
                xSet->setPropertyValue( rPropName, Any( false ) );

        // do not set properties at a group shape (this causes
        // assertions from svx) ...
        if( aServiceName != "com.sun.star.drawing.GroupShape" )
        {
            if (aServiceName == "com.sun.star.text.TextFrame")
            {
                if (mpCustomShapePropertiesPtr && mpCustomShapePropertiesPtr->getShapeTypeOverride())
                {
                    uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    propertySet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc( length+1);
                    aGrabBag[length].Name = "mso-orig-shape-type";
                    const uno::Sequence< sal_Int8 > aNameSeq =
                        mpCustomShapePropertiesPtr->getShapePresetTypeName();
                    OUString sShapePresetTypeName(reinterpret_cast< const char* >(
                        aNameSeq.getConstArray()), aNameSeq.getLength(), RTL_TEXTENCODING_UTF8);
                    aGrabBag[length].Value = uno::makeAny(sShapePresetTypeName);
                    propertySet->setPropertyValue("FrameInteropGrabBag",uno::makeAny(aGrabBag));
                }
                //If the text box has links then save the link information so that
                //it can be accessed in DomainMapper_Impl.cxx while chaining the text frames.
                if (this->isLinkedTxbx())
                {
                    uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    propertySet->getPropertyValue("FrameInteropGrabBag") >>= aGrabBag;
                    sal_Int32 length = aGrabBag.getLength();
                    aGrabBag.realloc( length + 3 );
                    aGrabBag[length].Name = "TxbxHasLink";
                    aGrabBag[length].Value = uno::makeAny(this->isLinkedTxbx());
                    aGrabBag[length + 1 ].Name = "Txbx-Id";
                    aGrabBag[length + 1 ].Value = uno::makeAny(this->getLinkedTxbxAttributes().id);
                    aGrabBag[length + 2 ].Name = "Txbx-Seq";
                    aGrabBag[length + 2 ].Value = uno::makeAny(this->getLinkedTxbxAttributes().seq);
                    propertySet->setPropertyValue("FrameInteropGrabBag",uno::makeAny(aGrabBag));
                }

                // TextFrames have BackColor, not FillColor
                if (aShapeProps.hasProperty(PROP_FillColor))
                {
                    aShapeProps.setProperty(PROP_BackColor, aShapeProps.getProperty(PROP_FillColor));
                    aShapeProps.erase(PROP_FillColor);
                }
                // TextFrames have BackColorTransparency, not FillTransparence
                if (aShapeProps.hasProperty(PROP_FillTransparence))
                {
                    aShapeProps.setProperty(PROP_BackColorTransparency, aShapeProps.getProperty(PROP_FillTransparence));
                    aShapeProps.erase(PROP_FillTransparence);
                }
                // TextFrames have BackGrahicURL, not FillBitmapURL
                if (aShapeProps.hasProperty(PROP_FillBitmapURL))
                {
                    aShapeProps.setProperty(PROP_BackGraphicURL, aShapeProps.getProperty(PROP_FillBitmapURL));
                    aShapeProps.erase(PROP_FillBitmapURL);
                }
                if (aShapeProps.hasProperty(PROP_FillBitmapName))
                {
                    uno::Any aAny = aShapeProps.getProperty(PROP_FillBitmapName);
                    aShapeProps.setProperty(PROP_BackGraphicURL, rFilterBase.getModelObjectHelper().getFillBitmapUrl( aAny.get<OUString>() ));
                    // aShapeProps.erase(PROP_FillBitmapName);  // Maybe, leave the name as well
                }
                // And no LineColor property; individual borders can have colors
                if (aShapeProps.hasProperty(PROP_LineColor))
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                    static const sal_Int32 aBorders[] =
                    {
                        PROP_TopBorder, PROP_LeftBorder, PROP_BottomBorder, PROP_RightBorder
                    };
                    for (unsigned int i = 0; i < SAL_N_ELEMENTS(aBorders); ++i)
                    {
                        css::table::BorderLine2 aBorderLine = xPropertySet->getPropertyValue(PropertyMap::getPropertyName(aBorders[i])).get<css::table::BorderLine2>();
                        aBorderLine.Color = aShapeProps.getProperty(PROP_LineColor).get<sal_Int32>();
                        if (aLineProperties.moLineWidth.has())
                            aBorderLine.LineWidth = convertEmuToHmm(aLineProperties.moLineWidth.get());
                        aShapeProps.setProperty(aBorders[i], uno::makeAny(aBorderLine));
                    }
                    aShapeProps.erase(PROP_LineColor);
                }
                if(mnRotation)
                {
                    uno::Reference<beans::XPropertySet> xPropertySet(mxShape, uno::UNO_QUERY);
                    const OUString aGrabBagPropName = "FrameInteropGrabBag";
                    uno::Sequence<beans::PropertyValue> aGrabBag;
                    xPropertySet->getPropertyValue(aGrabBagPropName) >>= aGrabBag;
                    beans::PropertyValue aPair;
                    aPair.Name = "mso-rotation-angle";
                    aPair.Value = uno::makeAny(mnRotation);
                    if (aGrabBag.hasElements())
                    {
                        sal_Int32 nLength = aGrabBag.getLength();
                        aGrabBag.realloc(nLength + 1);
                        aGrabBag[nLength] = aPair;
                    }
                    else
                    {
                        aGrabBag.realloc(1);
                        aGrabBag[0] = aPair;
                    }
                    xPropertySet->setPropertyValue(aGrabBagPropName, uno::makeAny(aGrabBag));
                }
                // TextFrames have ShadowFormat, not individual shadow properties.
                boost::optional<sal_Int32> oShadowDistance;
                if (aShapeProps.hasProperty(PROP_ShadowXDistance))
                {
                    oShadowDistance = aShapeProps.getProperty(PROP_ShadowXDistance).get<sal_Int32>();
                    aShapeProps.erase(PROP_ShadowXDistance);
                }
                if (aShapeProps.hasProperty(PROP_ShadowYDistance))
                {
                    // There is a single 'dist' attribute, so no need to count the avg of x and y.
                    aShapeProps.erase(PROP_ShadowYDistance);
                }
                boost::optional<sal_Int32> oShadowColor;
                if (aShapeProps.hasProperty(PROP_ShadowColor))
                {
                    oShadowColor = aShapeProps.getProperty(PROP_ShadowColor).get<sal_Int32>();
                    aShapeProps.erase(PROP_ShadowColor);
                }
                if (aShapeProps.hasProperty(PROP_Shadow))
                    aShapeProps.erase(PROP_Shadow);

                if (oShadowDistance || oShadowColor || aEffectProperties.maShadow.moShadowDir.has())
                {
                    css::table::ShadowFormat aFormat;
                    if (oShadowColor)
                        aFormat.Color = *oShadowColor;
                    if (aEffectProperties.maShadow.moShadowDir.has())
                    {
                        css::table::ShadowLocation nLocation = css::table::ShadowLocation_NONE;
                        switch (aEffectProperties.maShadow.moShadowDir.get())
                        {
                        case 13500000:
                            nLocation = css::table::ShadowLocation_TOP_LEFT;
                            break;
                        case 18900000:
                            nLocation = css::table::ShadowLocation_TOP_RIGHT;
                            break;
                        case 8100000:
                            nLocation = css::table::ShadowLocation_BOTTOM_LEFT;
                            break;
                        case 2700000:
                            nLocation = css::table::ShadowLocation_BOTTOM_RIGHT;
                            break;
                        }
                        aFormat.Location = nLocation;
                    }
                    aFormat.ShadowWidth = *oShadowDistance;
                    aShapeProps.setProperty(PROP_ShadowFormat, uno::makeAny(aFormat));
                }
            }
            else if (mbTextBox)
            {
                aShapeProps.setProperty(PROP_TextBox, uno::makeAny(true));
            }

            if (aServiceName != "com.sun.star.text.TextFrame" && isLinkedTxbx())
            {
                uno::Reference<beans::XPropertySet> propertySet (mxShape, uno::UNO_QUERY);
                uno::Sequence<beans::PropertyValue> aGrabBag;
                propertySet->getPropertyValue("InteropGrabBag") >>= aGrabBag;
                sal_Int32 length = aGrabBag.getLength();
                aGrabBag.realloc( length + 3 );
                aGrabBag[length].Name = "TxbxHasLink";
                aGrabBag[length].Value = uno::makeAny(this->isLinkedTxbx());
                aGrabBag[length + 1 ].Name = "Txbx-Id";
                aGrabBag[length + 1 ].Value = uno::makeAny(this->getLinkedTxbxAttributes().id);
                aGrabBag[length + 2 ].Name = "Txbx-Seq";
                aGrabBag[length + 2 ].Value = uno::makeAny(this->getLinkedTxbxAttributes().seq);
                propertySet->setPropertyValue("InteropGrabBag",uno::makeAny(aGrabBag));
            }

            PropertySet( xSet ).setProperties( aShapeProps );
            if (mbLockedCanvas)
            {
                putPropertyToGrabBag( "LockedCanvas", Any( true ) );
                if (aServiceName == "com.sun.star.drawing.LineShape")
                {
                    // It seems the position and size for lines inside a locked canvas is absolute.
                    mxShape->setPosition(awt::Point(aShapeRectHmm.X, aShapeRectHmm.Y));
                    mxShape->setSize(awt::Size(aShapeRectHmm.Width, aShapeRectHmm.Height));
                }
            }

            // Store original fill and line colors of the shape and the theme color name to InteropGrabBag
            Sequence< PropertyValue > aProperties( 6 );  //allocate the maximum possible number of slots
            sal_Int32 nSize = 2;
            PUT_PROP( aProperties, 0, "OriginalSolidFillClr", aShapeProps.getProperty(PROP_FillColor) );
            PUT_PROP( aProperties, 1, "OriginalLnSolidFillClr", aShapeProps.getProperty(PROP_LineColor) );
            OUString sColorFillScheme = aFillProperties.maFillColor.getSchemeName();
            if( !aFillProperties.maFillColor.isPlaceHolder() && !sColorFillScheme.isEmpty() )
            {
                PUT_PROP( aProperties, nSize, "SpPrSolidFillSchemeClr", sColorFillScheme );
                nSize++;
                PUT_PROP( aProperties, nSize, "SpPrSolidFillSchemeClrTransformations",
                          aFillProperties.maFillColor.getTransformations() );
                nSize++;
            }
            OUString sLnColorFillScheme = aLineProperties.maLineFill.maFillColor.getSchemeName();
            if( !aLineProperties.maLineFill.maFillColor.isPlaceHolder() && !sLnColorFillScheme.isEmpty() )
            {
                PUT_PROP( aProperties, nSize, "SpPrLnSolidFillSchemeClr", sLnColorFillScheme );
                nSize++;
                PUT_PROP( aProperties, nSize, "SpPrLnSolidFillSchemeClrTransformations",
                          aLineProperties.maLineFill.maFillColor.getTransformations() );
                nSize++;
            }
            aProperties.realloc( nSize ); //shrink the Sequence if we didn't use all the slots
            putPropertiesToGrabBag( aProperties );

            // Store original gradient fill of the shape to InteropGrabBag
            // LibreOffice doesn't support all the kinds of gradient so we save its complete definition
            if( aShapeProps.hasProperty( PROP_FillGradient ) )
            {
                Sequence< PropertyValue > aGradientStops( aFillProperties.maGradientProps.maGradientStops.size() );
                ::std::map< double, Color >::iterator aIt = aFillProperties.maGradientProps.maGradientStops.begin();
                for( size_t i = 0; i < aFillProperties.maGradientProps.maGradientStops.size(); ++i )
                { // for each stop in the gradient definition:

                    // save position
                    Sequence< PropertyValue > aGradientStop( 3 );
                    PUT_PROP( aGradientStop, 0, "Pos", aIt->first );

                    OUString sStopColorScheme = aIt->second.getSchemeName();
                    if( sStopColorScheme.isEmpty() )
                    {
                        // save RGB color
                        PUT_PROP( aGradientStop, 1, "RgbClr", aIt->second.getColor( rGraphicHelper, nFillPhClr ) );
                        // in the case of a RGB color, transformations are already applied to
                        // the color with the exception of alpha transformations. We only need
                        // to keep the transparency value to calculate the alpha value later.
                        if( aIt->second.hasTransparency() )
                        {
                            PUT_PROP( aGradientStop, 2, "Transparency", aIt->second.getTransparency() );
                        }
                    }
                    else
                    {
                        // save color with scheme name
                        PUT_PROP( aGradientStop, 1, "SchemeClr", sStopColorScheme );
                        // save all color transformations
                        PUT_PROP( aGradientStop, 2, "Transformations", aIt->second.getTransformations() );
                    }

                    PUT_PROP( aGradientStops, i, OUString::number( i ), aGradientStop );
                    ++aIt;
                }
                // If getFillProperties.moFillType is unused that means gradient is defined by a theme
                // which is already saved into StyleFillRef property, so no need to save the explicit values too
                if( getFillProperties().moFillType.has() )
                    putPropertyToGrabBag( "GradFillDefinition", Any( aGradientStops ) );
                putPropertyToGrabBag( "OriginalGradFill", aShapeProps.getProperty(PROP_FillGradient) );
            }

            // store unsupported effect attributes in the grab bag
            if (!aEffectProperties.m_Effects.empty())
            {
                Sequence<PropertyValue> aEffects(aEffectProperties.m_Effects.size());
                sal_uInt32 i = 0;
                for (auto const& it : aEffectProperties.m_Effects)
                {
                    PropertyValue aEffect = it->getEffect();
                    if( !aEffect.Name.isEmpty() )
                    {
                        Sequence< PropertyValue > aEffectsGrabBag( 3 );
                        PUT_PROP( aEffectsGrabBag, 0, "Attribs", aEffect.Value );

                        Color& aColor( it->moColor );
                        OUString sColorScheme = aColor.getSchemeName();
                        if( sColorScheme.isEmpty() )
                        {
                            // RGB color and transparency value
                            PUT_PROP( aEffectsGrabBag, 1, "RgbClr",
                                      aColor.getColor( rGraphicHelper, nFillPhClr ) );
                            PUT_PROP( aEffectsGrabBag, 2, "RgbClrTransparency",
                                      aColor.getTransparency() );
                        }
                        else
                        {
                            // scheme color with name and transformations
                            PUT_PROP( aEffectsGrabBag, 1, "SchemeClr", sColorScheme );
                            PUT_PROP( aEffectsGrabBag, 2, "SchemeClrTransformations",
                                      aColor.getTransformations() );
                        }
                        PUT_PROP( aEffects, i, aEffect.Name, aEffectsGrabBag );
                        ++i;
                    }
                }
                putPropertyToGrabBag( "EffectProperties", Any( aEffects ) );
            }

            // add 3D effects if any
            Sequence< PropertyValue > aCamera3DEffects = get3DProperties().getCameraAttributes();
            Sequence< PropertyValue > aLightRig3DEffects = get3DProperties().getLightRigAttributes();
            Sequence< PropertyValue > aShape3DEffects = get3DProperties().getShape3DAttributes( rGraphicHelper, nFillPhClr );
            if( aCamera3DEffects.getLength() > 0 || aLightRig3DEffects.getLength() > 0 || aShape3DEffects.getLength() > 0 )
            {
                Sequence< PropertyValue > a3DEffectsGrabBag( 3 );
                PUT_PROP( a3DEffectsGrabBag, 0, "Camera", Any( aCamera3DEffects ) );
                PUT_PROP( a3DEffectsGrabBag, 1, "LightRig", Any( aLightRig3DEffects ) );
                PUT_PROP( a3DEffectsGrabBag, 2, "Shape3D", Any( aShape3DEffects ) );
                putPropertyToGrabBag( "3DEffectProperties", Any( a3DEffectsGrabBag ) );
            }

            // store bitmap artistic effects in the grab bag
            if( !mpGraphicPropertiesPtr->maBlipProps.maEffect.isEmpty() )
                putPropertyToGrabBag( "ArtisticEffectProperties",
                                      Any( mpGraphicPropertiesPtr->maBlipProps.maEffect.getEffect() ) );
        }

        else if( mbLockedCanvas )
        {
            //If we have aServiceName as "com.sun.star.drawing.GroupShape" and lockedCanvas
            putPropertyToGrabBag( "LockedCanvas", Any( true ) );
        }

        // These can have a custom geometry, so position should be set here,
        // after creation but before custom shape handling, using the position
        // we got from the caller.
        if (mbWps && aServiceName == "com.sun.star.drawing.LineShape")
            mxShape->setPosition(maPosition);

        if( bIsCustomShape )
        {
            if ( mbFlipH )
                mpCustomShapePropertiesPtr->setMirroredX( true );
            if ( mbFlipV )
                mpCustomShapePropertiesPtr->setMirroredY( true );
            if( getTextBody() )
            {
                sal_Int32 nTextRotateAngle = static_cast< sal_Int32 >( getTextBody()->getTextProperties().moRotation.get( 0 ) );
                /* OOX measures text rotation clockwise in 1/60000th degrees,
                   relative to the containing shape. setTextRotateAngle wants
                   degrees anticlockwise. */
                mpCustomShapePropertiesPtr->setTextRotateAngle( -1 * nTextRotateAngle / 60000 );
            }

            SAL_INFO("oox.cscode", "==cscode== shape name: '" << msName << "'");
            SAL_INFO("oox.csdata", "==csdata== shape name: '" << msName << "'");
            mpCustomShapePropertiesPtr->pushToPropSet( rFilterBase, xSet, mxShape, maSize );
        }
        else if( getTextBody() )
            getTextBody()->getTextProperties().pushVertSimulation();

        PropertySet aPropertySet(mxShape);
        if ( !bUseRotationTransform && mnRotation != 0 )
        {
            // use the same logic for rotation from VML exporter (SimpleShape::implConvertAndInsert at vmlshape.cxx)
            aPropertySet.setAnyProperty( PROP_RotateAngle, makeAny( sal_Int32( NormAngle360( mnRotation / -600 ) ) ) );
            aPropertySet.setAnyProperty( PROP_HoriOrientPosition, makeAny( maPosition.X ) );
            aPropertySet.setAnyProperty( PROP_VertOrientPosition, makeAny( maPosition.Y ) );
        }

        // in some cases, we don't have any text body.
        if( getTextBody() && ( !bDoNotInsertEmptyTextBody || !mpTextBody->isEmpty() ) )
        {
            Reference < XText > xText( mxShape, UNO_QUERY );
            if ( xText.is() )   // not every shape is supporting an XText interface (e.g. GroupShape)
            {
                TextCharacterProperties aCharStyleProperties;
                if( const ShapeStyleRef* pFontRef = getShapeStyleRef( XML_fontRef ) )
                {
                    if( pFontRef->mnThemedIdx != 0 )
                    {
                        if( pTheme )
                            if( const TextCharacterProperties* pCharProps = pTheme->getFontStyle( pFontRef->mnThemedIdx ) )
                                aCharStyleProperties.assignUsed( *pCharProps );
                        SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "use font color");
                        aCharStyleProperties.maCharColor.assignIfUsed( pFontRef->maPhClr );
                    }
                }

                Reference < XTextCursor > xAt = xText->createTextCursor();
                getTextBody()->insertAt( rFilterBase, xText, xAt, aCharStyleProperties, mpMasterTextListStyle );
            }
        }
        else if (mbTextBox)
        {
            // No drawingML text, but WPS text is expected: save the theme
            // character color on the shape, then.
            if(const ShapeStyleRef* pFontRef = getShapeStyleRef(XML_fontRef))
            {
                sal_Int32 nCharColor = pFontRef->maPhClr.getColor(rGraphicHelper);
                aPropertySet.setAnyProperty(PROP_CharColor, uno::makeAny(nCharColor));
            }
        }
    }

    if( mxShape.is() )
        finalizeXShape( rFilterBase, rxShapes );

    return mxShape;
}

void Shape::keepDiagramCompatibilityInfo( XmlFilterBase& rFilterBase )
{
    try
    {
        if( !maDiagramDoms.hasElements() )
            return;

        Reference < XPropertySet > xSet( mxShape, UNO_QUERY_THROW );
        Reference < XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
        if ( !xSetInfo.is() )
            return;

        const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
        if( !xSetInfo->hasPropertyByName( aGrabBagPropName ) )
            return;

        Sequence < PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;

        // We keep the previous items, if present
        if ( aGrabBag.hasElements() )
        {
            sal_Int32 length = aGrabBag.getLength();
            aGrabBag.realloc( length+maDiagramDoms.getLength() );

            for( sal_Int32 i = 0; i < maDiagramDoms.getLength(); ++i )
                aGrabBag[length+i] = maDiagramDoms[i];

            xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
        } else
            xSet->setPropertyValue( aGrabBagPropName, Any( maDiagramDoms ) );

        xSet->setPropertyValue( "MoveProtect", Any( sal_True ) );
        xSet->setPropertyValue( "SizeProtect", Any( sal_True ) );

        // Replace existing shapes with a new Graphic Object rendered
        // from them
        Reference < XShape > xShape( renderDiagramToGraphic( rFilterBase ) );
        Reference < XShapes > xShapes( mxShape, UNO_QUERY_THROW );
        while( xShapes->hasElements() )
            xShapes->remove( Reference < XShape > ( xShapes->getByIndex( 0 ),  UNO_QUERY_THROW ) );
        xShapes->add( xShape );
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", OSL_THIS_FUNC << "Exception: " << e.Message );
    }
}

Reference < XShape > Shape::renderDiagramToGraphic( XmlFilterBase& rFilterBase )
{
    Reference< XShape > xShape;

    try
    {
        if( !maDiagramDoms.hasElements() )
            return xShape;

        // Stream in which to place the rendered shape
        SvMemoryStream mpTempStream;
        Reference < io::XStream > xStream( new utl::OStreamWrapper( mpTempStream ) );
        Reference < io::XOutputStream > xOutputStream( xStream->getOutputStream() );

        // Rendering format
        OUString sFormat( "SVM" );

        // Size of the rendering
        awt::Size aActualSize = mxShape->getSize();
        Size aResolution( Application::GetDefaultDevice()->LogicToPixel( Size( 100, 100 ), MAP_CM ) );
        double fPixelsPer100thmm = static_cast < double > ( aResolution.Width() ) / 100000.0;
        awt::Size aSize = awt::Size( static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Width ) + 0.5 ),
                                     static_cast < sal_Int32 > ( ( fPixelsPer100thmm * aActualSize.Height ) + 0.5 ) );

        Sequence< PropertyValue > aFilterData( 4 );
        aFilterData[ 0 ].Name = "PixelWidth";
        aFilterData[ 0 ].Value <<= aSize.Width;
        aFilterData[ 1 ].Name = "PixelHeight";
        aFilterData[ 1 ].Value <<= aSize.Height;
        aFilterData[ 2 ].Name = "LogicalWidth";
        aFilterData[ 2 ].Value <<= aActualSize.Width;
        aFilterData[ 3 ].Name = "LogicalHeight";
        aFilterData[ 3 ].Value <<= aActualSize.Height;

        Sequence < PropertyValue > aDescriptor( 3 );
        aDescriptor[ 0 ].Name = "OutputStream";
        aDescriptor[ 0 ].Value <<= xOutputStream;
        aDescriptor[ 1 ].Name = "FilterName";
        aDescriptor[ 1 ].Value <<= sFormat;
        aDescriptor[ 2 ].Name = "FilterData";
        aDescriptor[ 2 ].Value <<= aFilterData;

        Reference < lang::XComponent > xSourceDoc( mxShape, UNO_QUERY_THROW );
        Reference < XGraphicExportFilter > xGraphicExporter = GraphicExportFilter::create( rFilterBase.getComponentContext() );
        xGraphicExporter->setSourceDocument( xSourceDoc );
        xGraphicExporter->filter( aDescriptor );

        mpTempStream.Seek( STREAM_SEEK_TO_BEGIN );

        Graphic aGraphic;
        GraphicFilter aFilter( false );
        if ( aFilter.ImportGraphic( aGraphic, "", mpTempStream, GRFILTER_FORMAT_NOTFOUND, nullptr, GraphicFilterImportFlags::NONE, static_cast < Sequence < PropertyValue >* > ( nullptr ) ) != GRFILTER_OK )
        {
            SAL_WARN( "oox.drawingml", OSL_THIS_FUNC
                      << "Unable to import rendered stream into graphic object" );
            return xShape;
        }

        Reference < graphic::XGraphic > xGraphic( aGraphic.GetXGraphic() );
        Reference < lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
        xShape.set( xServiceFact->createInstance( "com.sun.star.drawing.GraphicObjectShape" ), UNO_QUERY_THROW );
        Reference < XPropertySet > xPropSet( xShape, UNO_QUERY_THROW );
        xPropSet->setPropertyValue(  "Graphic", Any( xGraphic ) );
        xPropSet->setPropertyValue(  "MoveProtect", Any( sal_True ) );
        xPropSet->setPropertyValue(  "SizeProtect", Any( sal_True ) );
        xPropSet->setPropertyValue(  "Name", Any( OUString( "RenderedShapes" ) ) );
    }
    catch( const Exception& e )
    {
        SAL_WARN( "oox.drawingml", OSL_THIS_FUNC << "Exception: " << e.Message );
    }

    return xShape;
}

void Shape::setTextBody(const TextBodyPtr & pTextBody)
{
    mpTextBody = pTextBody;
}

void Shape::setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle )
{
    SAL_INFO("oox.drawingml", OSL_THIS_FUNC << "set master text list style to shape id: " << msId);

    mpMasterTextListStyle = pMasterTextListStyle;
}

OUString Shape::finalizeServiceName( XmlFilterBase& rFilter, const OUString& rServiceName, const awt::Rectangle& rShapeRect )
{
    OUString aServiceName = rServiceName;
    switch( meFrameType )
    {
        case FRAMETYPE_OLEOBJECT:
        {
            awt::Size aOleSize( rShapeRect.Width, rShapeRect.Height );
            if( rFilter.getOleObjectHelper().importOleObject( maShapeProperties, *mxOleObjectInfo, aOleSize ) )
                aServiceName = "com.sun.star.drawing.OLE2Shape";

            // get the path to the representation graphic
            OUString aGraphicPath;
            if( !mxOleObjectInfo->maShapeId.isEmpty() )
                if( ::oox::vml::Drawing* pVmlDrawing = rFilter.getVmlDrawing() )
                    if( const ::oox::vml::ShapeBase* pVmlShape = pVmlDrawing->getShapes().getShapeById( mxOleObjectInfo->maShapeId, true ) )
                        aGraphicPath = pVmlShape->getGraphicPath();

            // import and store the graphic
            if( !aGraphicPath.isEmpty() )
            {
                // Transfer shape's width and height to graphicsfilter (can be used by WMF/EMF)
                WMF_EXTERNALHEADER aExtHeader;
                aExtHeader.mapMode = 8; // MM_ANISOTROPIC
                aExtHeader.xExt = rShapeRect.Width;
                aExtHeader.yExt = rShapeRect.Height;

                Reference< graphic::XGraphic > xGraphic = rFilter.getGraphicHelper().importEmbeddedGraphic( aGraphicPath, &aExtHeader );
                if( xGraphic.is() )
                    maShapeProperties.setProperty(PROP_Graphic, xGraphic);
            }
        }
        break;

        default:;
    }
    return aServiceName;
}

void Shape::finalizeXShape( XmlFilterBase& rFilter, const Reference< XShapes >& rxShapes )
{
    switch( meFrameType )
    {
        case FRAMETYPE_CHART:
        {
            OSL_ENSURE( !mxChartShapeInfo->maFragmentPath.isEmpty(), "Shape::finalizeXShape - missing chart fragment" );
            if( mxShape.is() && !mxChartShapeInfo->maFragmentPath.isEmpty() ) try
            {
                // set the chart2 OLE class ID at the OLE shape
                PropertySet aShapeProp( mxShape );
                aShapeProp.setProperty( PROP_CLSID, OUString( "12dcae26-281f-416f-a234-c3086127382e" ) );

                // get the XModel interface of the embedded object from the OLE shape
                Reference< frame::XModel > xDocModel;
                aShapeProp.getProperty( xDocModel, PROP_Model );
                Reference< chart2::XChartDocument > xChartDoc( xDocModel, UNO_QUERY_THROW );

                // load the chart data from the XML fragment
                bool bMSO2007Doc = rFilter.isMSO2007Document();
                chart::ChartSpaceModel aModel(bMSO2007Doc);
                chart::ChartSpaceFragment *pChartSpaceFragment = new chart::ChartSpaceFragment(
                        rFilter, mxChartShapeInfo->maFragmentPath, aModel );
                const OUString aThemeOverrideFragmentPath( pChartSpaceFragment->
                        getFragmentPathFromFirstTypeFromOfficeDoc("themeOverride") );
                rFilter.importFragment( pChartSpaceFragment );
                ::oox::ppt::PowerPointImport *pPowerPointImport =
                    dynamic_cast< ::oox::ppt::PowerPointImport* >(&rFilter);
                if (!aThemeOverrideFragmentPath.isEmpty() && pPowerPointImport)
                {
                    uno::Reference< xml::sax::XFastSAXSerializable > xDoc(
                            rFilter.importFragment(aThemeOverrideFragmentPath), uno::UNO_QUERY_THROW);
                    ThemePtr pTheme = pPowerPointImport->getActualSlidePersist()->getTheme();
                    rFilter.importFragment(new ThemeOverrideFragmentHandler(
                                rFilter, aThemeOverrideFragmentPath, *pTheme), xDoc);
                    pPowerPointImport->getActualSlidePersist()->setTheme(pTheme);
                }

                // convert imported chart model to chart document
                Reference< drawing::XShapes > xExternalPage;
                if( !mxChartShapeInfo->mbEmbedShapes )
                    xExternalPage = rxShapes;
                if( rFilter.getChartConverter() )
                {
                    rFilter.getChartConverter()->convertFromModel( rFilter, aModel, xChartDoc, xExternalPage, mxShape->getPosition(), mxShape->getSize() );
                    if( !xChartDoc->hasInternalDataProvider() )
                    {
                        Reference< chart2::data::XDataReceiver > xDataRec( xChartDoc, UNO_QUERY );
                        Reference< chart2::data::XDataSource > xData( xDataRec->getUsedData(), UNO_QUERY );
                        if( xData->getDataSequences().getLength() <= 0 || !xData->getDataSequences()[0]->getValues().is() ||
                                xData->getDataSequences()[0]->getValues()->getData().getLength() <= 0 )
                        {
                            rFilter.useInternalChartDataTable( true );
                            rFilter.getChartConverter()->convertFromModel( rFilter, aModel, xChartDoc, xExternalPage, mxShape->getPosition(), mxShape->getSize() );
                            rFilter.useInternalChartDataTable( false );
                        }
                    }

                }
            }
            catch( Exception& )
            {
            }
        }
        break;

        default:;
    }
}

void Shape::putPropertyToGrabBag( const OUString& sPropertyName, const Any& aPropertyValue )
{
    PropertyValue pNewProperty;
    pNewProperty.Name = sPropertyName;
    pNewProperty.Value = aPropertyValue;
    putPropertyToGrabBag( pNewProperty );
}

void Shape::putPropertyToGrabBag( const PropertyValue& pProperty )
{
    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
    const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if( mxShape.is() && xSet.is() && xSetInfo.is() && xSetInfo->hasPropertyByName( aGrabBagPropName ) )
    {
        Sequence< PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;

        sal_Int32 length = aGrabBag.getLength();
        aGrabBag.realloc( length + 1 );
        aGrabBag[length] = pProperty;

        xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
    }
}

void Shape::putPropertiesToGrabBag( const Sequence< PropertyValue >& aProperties )
{
    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
    const OUString aGrabBagPropName = UNO_NAME_MISC_OBJ_INTEROPGRABBAG;
    if( mxShape.is() && xSet.is() && xSetInfo.is() && xSetInfo->hasPropertyByName( aGrabBagPropName ) )
    {
        // get existing grab bag
        Sequence< PropertyValue > aGrabBag;
        xSet->getPropertyValue( aGrabBagPropName ) >>= aGrabBag;
        sal_Int32 length = aGrabBag.getLength();

        // update grab bag size to contain the new items
        aGrabBag.realloc( length + aProperties.getLength() );

        // put the new items
        for( sal_Int32 i=0; i < aProperties.getLength(); ++i )
        {
            aGrabBag[length + i].Name = aProperties[i].Name;
            aGrabBag[length + i].Value = aProperties[i].Value;
        }

        // put it back to the shape
        xSet->setPropertyValue( aGrabBagPropName, Any( aGrabBag ) );
    }
}

uno::Sequence< uno::Sequence< uno::Any > >  Shape::resolveRelationshipsOfTypeFromOfficeDoc(core::XmlFilterBase& rFilter, const OUString& sFragment, const OUString& sType )
{
    uno::Sequence< uno::Sequence< uno::Any > > xRelListTemp;
    sal_Int32 counter = 0;

    core::RelationsRef xRels = rFilter.importRelations( sFragment );
    if ( xRels )
    {
        core::RelationsRef xImageRels = xRels->getRelationsFromTypeFromOfficeDoc( sType );
        if ( xImageRels )
        {
            xRelListTemp.realloc( xImageRels->size() );
            for( ::std::map< OUString, core::Relation >::const_iterator aIt = xImageRels->begin(), aEnd = xImageRels->end(); aIt != aEnd; ++aIt )
            {
                uno::Sequence< uno::Any > diagramRelTuple (3);
                // [0] => RID, [1] => InputStream [2] => extension
                OUString sRelId = aIt->second.maId;

                diagramRelTuple[0] = uno::makeAny ( sRelId );
                OUString sTarget = xImageRels->getFragmentPathFromRelId( sRelId );

                uno::Reference< io::XInputStream > xImageInputStrm( rFilter.openInputStream( sTarget ), uno::UNO_SET_THROW );
                StreamDataSequence dataSeq;
                if ( rFilter.importBinaryData( dataSeq, sTarget ) )
                {
                    diagramRelTuple[1] = uno::makeAny( dataSeq );
                }

                diagramRelTuple[2] = uno::makeAny( sTarget.copy( sTarget.lastIndexOf(".") ) );

                xRelListTemp[counter] = diagramRelTuple;
                ++counter;
            }
            xRelListTemp.realloc(counter);

        }
    }
    return xRelListTemp;
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
