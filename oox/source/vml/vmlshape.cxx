/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmlshape.cxx,v $
 * $Revision: 1.5 $
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

#include "oox/vml/vmlshape.hxx"
#include <rtl/math.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include "properties.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/ole/axcontrol.hxx"
#include "oox/ole/axcontrolfragment.hxx"
#include "oox/ole/oleobjecthelper.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmlshapecontainer.hxx"

using ::rtl::OUString;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;
using ::com::sun::star::uno::UNO_SET_THROW;
using ::com::sun::star::lang::XMultiServiceFactory;
using ::com::sun::star::awt::Point;
using ::com::sun::star::awt::Rectangle;
using ::com::sun::star::awt::Size;
using ::com::sun::star::awt::XControlModel;
using ::com::sun::star::graphic::XGraphic;
using ::com::sun::star::drawing::PointSequenceSequence;
using ::com::sun::star::drawing::XControlShape;
using ::com::sun::star::drawing::XEnhancedCustomShapeDefaulter;
using ::com::sun::star::drawing::XShape;
using ::com::sun::star::drawing::XShapes;
using ::oox::core::XmlFilterBase;

namespace oox {
namespace vml {

// ============================================================================

namespace {

sal_Int32 lclGetMeasure( const XmlFilterBase& /*rFilter*/, const OUString& rValue, sal_Int32 nRefValue )
{
    // default for missing values is 0
    if( rValue.getLength() == 0 )
        return 0;

    // TODO: according to spec, value may contain "auto"
    if( rValue.equalsAscii( "auto" ) )
        return nRefValue;

    // extract the double value and find start position of unit characters
    rtl_math_ConversionStatus eConvStatus = rtl_math_ConversionStatus_Ok;
    sal_Int32 nEndPos = 0;
    double fValue = ::rtl::math::stringToDouble( rValue, '.', '\0', &eConvStatus, &nEndPos );
    if( (eConvStatus != rtl_math_ConversionStatus_Ok) || (fValue == 0.0) )
        return 0;

    // process trailing unit, convert to 1/100 mm
    static const OUString saPx = CREATE_OUSTRING( "px" );
    OUString aUnit = ((0 < nEndPos) && (nEndPos < rValue.getLength())) ? rValue.copy( nEndPos ) : saPx;
    if( aUnit.getLength() == 2 )
    {
        sal_Unicode cChar1 = aUnit[ 0 ];
        sal_Unicode cChar2 = aUnit[ 1 ];
        if( (cChar1 == 'i') && (cChar2 == 'n') )        // 1 inch = 2540 1/100mm
            fValue *= 2540.0;
        else if( (cChar1 == 'c') && (cChar2 == 'm') )   // 1 cm = 1000 1/100mm
            fValue *= 1000.0;
        else if( (cChar1 == 'm') && (cChar2 == 'm') )   // 1 mm = 100 1/100mm
            fValue *= 100.0;
        else if( (cChar1 == 'p') && (cChar2 == 't') )   // 1 point = 1/72 inch
            fValue *= 2540.0 / 72.0;
        else if( (cChar1 == 'p') && (cChar2 == 'c') )   // 1 pica = 1/6 inch
            fValue *= 2540.0 / 6.0;
        else if( (cChar1 == 'e') && (cChar2 == 'm') )   // relative to refvalue
            fValue *= nRefValue;
        else if( (cChar1 == 'p') && (cChar2 == 'x') )   // 1 pixel, dependent on output device
            fValue *= 1.0;
    }
    else if( (aUnit.getLength() == 1) && (aUnit[ 0 ] == '%') )
    {
        fValue *= nRefValue / 100.0;
    }
    else
    {
        OSL_ENSURE( false, "lclGetMeasure - unknown measure unit" );
        fValue = nRefValue;
    }
    return static_cast< sal_Int32 >( fValue + 0.5 );
}

Point lclGetAbsPoint( const Point& rRelPoint, const Rectangle& rShapeRect, const Rectangle& rCoordSys )
{
    double fWidthRatio = static_cast< double >( rShapeRect.Width ) / rCoordSys.Width;
    double fHeightRatio = static_cast< double >( rShapeRect.Height ) / rCoordSys.Height;
    Point aAbsPoint;
    aAbsPoint.X = static_cast< sal_Int32 >( rShapeRect.X + fWidthRatio * (rRelPoint.X - rCoordSys.X) + 0.5 );
    aAbsPoint.Y = static_cast< sal_Int32 >( rShapeRect.Y + fHeightRatio * (rRelPoint.Y - rCoordSys.Y) + 0.5 );
    return aAbsPoint;
}

Rectangle lclGetAbsRect( const Rectangle& rRelRect, const Rectangle& rShapeRect, const Rectangle& rCoordSys )
{
    double fWidthRatio = static_cast< double >( rShapeRect.Width ) / rCoordSys.Width;
    double fHeightRatio = static_cast< double >( rShapeRect.Height ) / rCoordSys.Height;
    Rectangle aAbsRect;
    aAbsRect.X = static_cast< sal_Int32 >( rShapeRect.X + fWidthRatio * (rRelRect.X - rCoordSys.X) + 0.5 );
    aAbsRect.Y = static_cast< sal_Int32 >( rShapeRect.Y + fHeightRatio * (rRelRect.Y - rCoordSys.Y) + 0.5 );
    aAbsRect.Width = static_cast< sal_Int32 >( fWidthRatio * rRelRect.Width + 0.5 );
    aAbsRect.Height = static_cast< sal_Int32 >( fHeightRatio * rRelRect.Height + 0.5 );
    return aAbsRect;
}

Reference< XShape > lclCreateXShape( const XmlFilterBase& rFilter, const OUString& rService )
{
    OSL_ENSURE( rService.getLength() > 0, "lclCreateXShape - missing UNO shape service name" );
    Reference< XShape > xShape;
    try
    {
        Reference< XMultiServiceFactory > xFactory( rFilter.getModel(), UNO_QUERY_THROW );
        xShape.set( xFactory->createInstance( rService ), UNO_QUERY_THROW );
    }
    catch( Exception& )
    {
    }
    OSL_ENSURE( xShape.is(), "lclCreateXShape - cannot instanciate shape object" );
    return xShape;
}

void lclInsertXShape( const Reference< XShapes >& rxShapes, const Reference< XShape >& rxShape, const Rectangle& rShapeRect )
{
    OSL_ENSURE( rxShapes.is(), "lclInsertXShape - missing XShapes container" );
    OSL_ENSURE( rxShape.is(), "lclInsertXShape - missing XShape" );
    if( rxShapes.is() && rxShape.is() ) try
    {
        // insert shape into passed shape collection (maybe drawpage or group shape)
        rxShapes->add( rxShape );
        // set position/size
        rxShape->setPosition( Point( rShapeRect.X, rShapeRect.Y ) );
        rxShape->setSize( Size( rShapeRect.Width, rShapeRect.Height ) );
    }
    catch( Exception& )
    {
    }
}

Reference< XShape > lclCreateAndInsertXShape( const XmlFilterBase& rFilter,
        const Reference< XShapes >& rxShapes, const OUString& rService, const Rectangle& rShapeRect )
{
    Reference< XShape > xShape = lclCreateXShape( rFilter, rService );
    lclInsertXShape( rxShapes, xShape, rShapeRect );
    return xShape;
}

} // namespace

// ============================================================================

ShapeTypeModel::ShapeTypeModel()
{
}

void ShapeTypeModel::assignUsed( const ShapeTypeModel& rSource )
{
    monShapeType.assignIfUsed( rSource.monShapeType );
    monCoordLeft.assignIfUsed( rSource.monCoordLeft );
    monCoordTop.assignIfUsed( rSource.monCoordTop );
    monCoordWidth.assignIfUsed( rSource.monCoordWidth );
    monCoordHeight.assignIfUsed( rSource.monCoordHeight );
    /*  The style properties position, left, top, width, height, margin-left,
        margin-top are not derived from shape template to shape. */
    mobStroked.assignIfUsed( rSource.mobStroked );
    moStrokeColor.assignIfUsed( rSource.moStrokeColor );
    mobFilled.assignIfUsed( rSource.mobFilled );
    moFillColor.assignIfUsed( rSource.moFillColor );
    moGraphicPath.assignIfUsed( rSource.moGraphicPath );
    moGraphicTitle.assignIfUsed( rSource.moGraphicTitle );
}

// ----------------------------------------------------------------------------

ShapeType::ShapeType( const Drawing& rDrawing ) :
    mrDrawing( rDrawing )
{
}

ShapeType::~ShapeType()
{
}

OUString ShapeType::getGraphicPath() const
{
    return maTypeModel.moGraphicPath.get( OUString() );
}

Rectangle ShapeType::getCoordSystem() const
{
    return Rectangle(
        maTypeModel.monCoordLeft.get( 0 ),
        maTypeModel.monCoordTop.get( 0 ),
        maTypeModel.monCoordWidth.get( 1000 ),
        maTypeModel.monCoordHeight.get( 1000 ) );
}

Rectangle ShapeType::getRectangle( const ShapeParentAnchor* pParentAnchor ) const
{
    return pParentAnchor ?
        lclGetAbsRect( getRelRectangle(), pParentAnchor->maShapeRect, pParentAnchor->maCoordSys ) :
        getAbsRectangle();
}

Rectangle ShapeType::getAbsRectangle() const
{
    const XmlFilterBase& rFilter = mrDrawing.getFilter();
    return Rectangle(
        lclGetMeasure( rFilter, maTypeModel.maLeft, 0 ) + lclGetMeasure( rFilter, maTypeModel.maMarginLeft, 0 ),
        lclGetMeasure( rFilter, maTypeModel.maTop, 0 ) + lclGetMeasure( rFilter, maTypeModel.maMarginTop, 0 ),
        lclGetMeasure( rFilter, maTypeModel.maWidth, 0 ),
        lclGetMeasure( rFilter, maTypeModel.maHeight, 0 ) );
}

Rectangle ShapeType::getRelRectangle() const
{
    return Rectangle(
        maTypeModel.maLeft.toInt32(),
        maTypeModel.maTop.toInt32(),
        maTypeModel.maWidth.toInt32(),
        maTypeModel.maHeight.toInt32() );
}

// ============================================================================

ShapeClientData::ShapeClientData() :
    mnObjType( XML_TOKEN_INVALID ),
    mbPrintObject( true )
{
}

// ----------------------------------------------------------------------------

ShapeModel::ShapeModel()
{
}

ShapeClientData& ShapeModel::createClientData()
{
    mxClientData.reset( new ShapeClientData );
    return *mxClientData;
}

// ----------------------------------------------------------------------------

ShapeBase::ShapeBase( const Drawing& rDrawing ) :
    ShapeType( rDrawing )
{
}

void ShapeBase::finalizeFragmentImport()
{
    // resolve shape template reference
    if( (maShapeModel.maType.getLength() > 1) && (maShapeModel.maType[ 0 ] == '#') )
        if( const ShapeType* pShapeType = mrDrawing.getShapes().getShapeTypeById( maShapeModel.maType.copy( 1 ), true ) )
            maTypeModel.assignUsed( pShapeType->getTypeModel() );
}

const ShapeType* ShapeBase::getChildTypeById( const OUString& ) const
{
    return 0;
}

const ShapeBase* ShapeBase::getChildById( const OUString& ) const
{
    return 0;
}

Reference< XShape > ShapeBase::convertAndInsert( const Reference< XShapes >& rxShapes, const ShapeParentAnchor* pParentAnchor ) const
{
    Reference< XShape > xShape;
    /*  Calculate shape rectangle. Applications may do something special
        according to some imported shape client data (e.g. Excel cell anchor). */
    Rectangle aShapeRect;
    if( !maShapeModel.mxClientData.get() || !mrDrawing.convertShapeClientAnchor( aShapeRect, maShapeModel.mxClientData->maAnchor ) )
        aShapeRect = getRectangle( pParentAnchor );
    // convert the shape, if the calculated rectangle is not empty
    if( (aShapeRect.Width > 0) || (aShapeRect.Height > 0) && rxShapes.is() )
        xShape = implConvertAndInsert( rxShapes, aShapeRect );
    return xShape;
}

// protected ------------------------------------------------------------------

void ShapeBase::convertShapeProperties( const Reference< XShape >& rxShape ) const
{
    // shape properties
    PropertySet aPropSet( rxShape );

    // fill style
    bool bFilled = maTypeModel.mobFilled.get( true );
    aPropSet.setProperty( PROP_FillStyle, bFilled ? ::com::sun::star::drawing::FillStyle_SOLID : ::com::sun::star::drawing::FillStyle_NONE );
}

// ============================================================================

SimpleShape::SimpleShape( const Drawing& rDrawing, const OUString& rService ) :
    ShapeBase( rDrawing ),
    maService( rService )
{
}

Reference< XShape > SimpleShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xShape = lclCreateAndInsertXShape( mrDrawing.getFilter(), rxShapes, maService, rShapeRect );
    convertShapeProperties( xShape );
    return xShape;
}

// ============================================================================

RectangleShape::RectangleShape( const Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.RectangleShape" ) )
{
}

// ============================================================================

EllipseShape::EllipseShape( const Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.EllipseShape" ) )
{
}

// ============================================================================

PolyLineShape::PolyLineShape( const Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.PolyLineShape" ) )
{
}

Reference< XShape > PolyLineShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );
    // polygon path
    Rectangle aCoordSys = getCoordSystem();
    if( !maShapeModel.maPoints.empty() && (aCoordSys.Width > 0) && (aCoordSys.Height > 0) )
    {
        ::std::vector< Point > aAbsPoints;
        for( ShapeModel::PointVector::const_iterator aIt = maShapeModel.maPoints.begin(), aEnd = maShapeModel.maPoints.end(); aIt != aEnd; ++aIt )
            aAbsPoints.push_back( lclGetAbsPoint( *aIt, rShapeRect, aCoordSys ) );
        PointSequenceSequence aPointSeq( 1 );
        aPointSeq[ 0 ] = ContainerHelper::vectorToSequence( aAbsPoints );
        PropertySet aPropSet( xShape );
        aPropSet.setProperty( PROP_PolyPolygon, aPointSeq );
    }
    return xShape;
}

// ============================================================================

CustomShape::CustomShape( const Drawing& rDrawing ) :
    SimpleShape( rDrawing, CREATE_OUSTRING( "com.sun.star.drawing.CustomShape" ) )
{
}

Reference< XShape > CustomShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    // try to create a custom shape
    Reference< XShape > xShape = SimpleShape::implConvertAndInsert( rxShapes, rShapeRect );
    if( xShape.is() ) try
    {
        // create the custom shape geometry
        Reference< XEnhancedCustomShapeDefaulter > xDefaulter( xShape, UNO_QUERY_THROW );
        xDefaulter->createCustomShapeDefaults( OUString::valueOf( maTypeModel.monShapeType.get( 0 ) ) );
        // convert common properties
        convertShapeProperties( xShape );
    }
    catch( Exception& )
    {
    }
    return xShape;
}

// ============================================================================

ComplexShape::ComplexShape( const Drawing& rDrawing ) :
    CustomShape( rDrawing )
{
}

Reference< XShape > ComplexShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    XmlFilterBase& rFilter = mrDrawing.getFilter();
    OUString aGraphicPath = getGraphicPath();

    // try to find registered OLE object info
    if( const OleObjectInfo* pOleObjectInfo = mrDrawing.getOleObjectInfo( maTypeModel.maShapeId ) )
    {
        // if OLE object is embedded into a DrawingML shape (PPTX), do not create it here
        if( pOleObjectInfo->mbDmlShape )
            return Reference< XShape >();

        PropertyMap aOleProps;
        Size aOleSize( rShapeRect.Width, rShapeRect.Height );
        if( rFilter.getOleObjectHelper().importOleObject( aOleProps, *pOleObjectInfo, aOleSize ) )
        {
            Reference< XShape > xShape = lclCreateAndInsertXShape( rFilter, rxShapes, CREATE_OUSTRING( "com.sun.star.drawing.OLE2Shape" ), rShapeRect );
            if( xShape.is() )
            {
                // set the replacement graphic
                if( aGraphicPath.getLength() > 0 )
                {
                    Reference< XGraphic > xGraphic = rFilter.importEmbeddedGraphic( aGraphicPath );
                    if( xGraphic.is() )
                        aOleProps[ PROP_Graphic ] <<= xGraphic;
                }

                PropertySet aPropSet( xShape );
                aPropSet.setProperties( aOleProps );

                return xShape;
            }
        }
    }

    // try to find registered form control info
    const ControlInfo* pControlInfo = mrDrawing.getControlInfo( maTypeModel.maShapeId );
    if( pControlInfo && (pControlInfo->maFragmentPath.getLength() > 0) && (maTypeModel.maName.getLength() > 0) )
    {
        OSL_ENSURE( maTypeModel.maName == pControlInfo->maName, "ComplexShape::implConvertAndInsert - control name mismatch" );
        ::oox::ole::AxControl aControl( maTypeModel.maName );
        // load the control properties from fragment
        if( rFilter.importFragment( new ::oox::ole::AxControlFragment( rFilter, pControlInfo->maFragmentPath, aControl ) ) ) try
        {
            // create control model and insert it into the form of the draw page
            Reference< XControlModel > xCtrlModel( aControl.convertAndInsert( mrDrawing.getControlHelper() ), UNO_SET_THROW );
            if( maShapeModel.mxClientData.get() )
                mrDrawing.convertControlClientData( xCtrlModel, *maShapeModel.mxClientData );

            // create the control shape, set control model at the shape
            Reference< XShape > xShape = lclCreateAndInsertXShape(
                rFilter, rxShapes, CREATE_OUSTRING( "com.sun.star.drawing.ControlShape" ), rShapeRect );
            Reference< XControlShape > xCtrlShape( xShape, UNO_QUERY ); // do not throw, but always return the shape
            if( xCtrlShape.is() )
                xCtrlShape->setControl( xCtrlModel );
            return xShape;
        }
        catch( Exception& )
        {
        }
        // on error, proceed and try to create picture from replacement image
    }

    // try to create a picture object
    if( aGraphicPath.getLength() > 0 )
    {
        Reference< XShape > xShape = lclCreateAndInsertXShape( rFilter, rxShapes, CREATE_OUSTRING( "com.sun.star.drawing.GraphicObjectShape" ), rShapeRect );
        if( xShape.is() )
        {
            OUString aGraphicUrl = rFilter.importEmbeddedGraphicObject( aGraphicPath );
            if( aGraphicUrl.getLength() > 0 )
            {
                PropertySet aPropSet( xShape );
                aPropSet.setProperty( PROP_GraphicURL, aGraphicUrl );
            }
        }
        return xShape;
    }

    // default: try to create a custom shape
    return CustomShape::implConvertAndInsert( rxShapes, rShapeRect );
}

// ============================================================================

GroupShape::GroupShape( const Drawing& rDrawing ) :
    ShapeBase( rDrawing ),
    mxChildren( new ShapeContainer( rDrawing ) )
{
}

GroupShape::~GroupShape()
{
}

void GroupShape::finalizeFragmentImport()
{
    // basic shape processing
    ShapeBase::finalizeFragmentImport();
    // finalize all child shapes
    mxChildren->finalizeFragmentImport();
}

const ShapeType* GroupShape::getChildTypeById( const OUString& rShapeId ) const
{
    return mxChildren->getShapeTypeById( rShapeId, true );
}

const ShapeBase* GroupShape::getChildById( const OUString& rShapeId ) const
{
    return mxChildren->getShapeById( rShapeId, true );
}

Reference< XShape > GroupShape::implConvertAndInsert( const Reference< XShapes >& rxShapes, const Rectangle& rShapeRect ) const
{
    Reference< XShape > xShape;
    // check that this shape contains children and a valid coordinate system
    ShapeParentAnchor aParentAnchor;
    aParentAnchor.maShapeRect = rShapeRect;
    aParentAnchor.maCoordSys = getCoordSystem();
    if( !mxChildren->empty() && (aParentAnchor.maCoordSys.Width > 0) && (aParentAnchor.maCoordSys.Height > 0) ) try
    {
        xShape = lclCreateAndInsertXShape( mrDrawing.getFilter(), rxShapes, CREATE_OUSTRING( "com.sun.star.drawing.GroupShape" ), rShapeRect );
        Reference< XShapes > xShapes( xShape, UNO_QUERY_THROW );
        mxChildren->convertAndInsert( xShapes, &aParentAnchor );
    }
    catch( Exception& )
    {
    }
    return xShape;
}

// ============================================================================

} // namespace vml
} // namespace oox

