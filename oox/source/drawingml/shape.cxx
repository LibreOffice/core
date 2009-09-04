/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: shape.cxx,v $
 * $Revision: 1.8.6.1 $
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

#include "oox/drawingml/shape.hxx"
#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/fillproperties.hxx"
#include "oox/drawingml/lineproperties.hxx"
#include "oox/drawingml/textbody.hxx"
#include "oox/drawingml/table/tableproperties.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/helper/propertyset.hxx"
#include "properties.hxx"
#include "tokens.hxx"

#include <tools/solar.h>        // for the F_PI180 define
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/text/XText.hpp>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;

namespace oox { namespace drawingml {

// ============================================================================

CreateShapeCallback::CreateShapeCallback( XmlFilterBase& rFilter ) :
    mrFilter( rFilter )
{
}

CreateShapeCallback::~CreateShapeCallback()
{
}

OUString CreateShapeCallback::onCreateXShape( const OUString& rServiceName, const Rectangle& )
{
    return rServiceName;
}

void CreateShapeCallback::onXShapeCreated( const Reference< XShape >& ) const
{
}

// ============================================================================

Shape::Shape( const sal_Char* pServiceName )
: mpLinePropertiesPtr( new LineProperties )
, mpFillPropertiesPtr( new FillProperties )
, mpGraphicPropertiesPtr( new GraphicProperties )
, mpCustomShapePropertiesPtr( new CustomShapeProperties )
, mpMasterTextListStyle( new TextListStyle )
, mnSubType( 0 )
, mnIndex( 0 )
, mnRotation( 0 )
, mbFlipH( false )
, mbFlipV( false )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
    setDefaults();
}
Shape::~Shape()
{
}

table::TablePropertiesPtr Shape::getTableProperties()
{
    if ( !mpTablePropertiesPtr.get() )
        mpTablePropertiesPtr.reset( new table::TableProperties() );
    return mpTablePropertiesPtr;
}

void Shape::setDefaults()
{
    maShapeProperties[ PROP_TextAutoGrowHeight ] <<= false;
    maShapeProperties[ PROP_TextWordWrap ] <<= true;
    maShapeProperties[ PROP_TextLeftDistance ]  <<= static_cast< sal_Int32 >( 250 );
    maShapeProperties[ PROP_TextUpperDistance ] <<= static_cast< sal_Int32 >( 125 );
    maShapeProperties[ PROP_TextRightDistance ] <<= static_cast< sal_Int32 >( 250 );
    maShapeProperties[ PROP_TextLowerDistance ] <<= static_cast< sal_Int32 >( 125 );
    maShapeProperties[ PROP_CharHeight ] <<= static_cast< float >( 18.0 );
}

void Shape::setServiceName( const sal_Char* pServiceName )
{
    if ( pServiceName )
        msServiceName = OUString::createFromAscii( pServiceName );
}


const ShapeStyleRef* Shape::getShapeStyleRef( sal_Int32 nRefType ) const
{
    ShapeStyleRefMap::const_iterator aIt = maShapeStyleRefs.find( nRefType );
    return (aIt == maShapeStyleRefs.end()) ? 0 : &aIt->second;
}

void Shape::addShape(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const ThemePtr& rxTheme,
        const Reference< XShapes >& rxShapes,
        const awt::Rectangle* pShapeRect,
        ShapeIdMap* pShapeMap )
{
    try
    {
        rtl::OUString sServiceName( msServiceName );
        if( sServiceName.getLength() )
        {
            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, rxTheme, rxShapes, pShapeRect, sal_False ) );

            if( pShapeMap && msId.getLength() )
            {
                (*pShapeMap)[ msId ] = shared_from_this();
            }

            // if this is a group shape, we have to add also each child shape
            Reference< XShapes > xShapes( xShape, UNO_QUERY );
            if ( xShapes.is() )
                addChildren( rFilterBase, *this, rxTheme, xShapes, pShapeRect ? *pShapeRect : awt::Rectangle( maPosition.X, maPosition.Y, maSize.Width, maSize.Height ), pShapeMap );
        }
    }
    catch( const Exception&  )
    {
    }
}

void Shape::applyShapeReference( const Shape& rReferencedShape )
{
    mpTextBody = TextBodyPtr( new TextBody( *rReferencedShape.mpTextBody.get() ) );
    maShapeProperties = rReferencedShape.maShapeProperties;
    mpLinePropertiesPtr = LinePropertiesPtr( new LineProperties( *rReferencedShape.mpLinePropertiesPtr.get() ) );
    mpFillPropertiesPtr = FillPropertiesPtr( new FillProperties( *rReferencedShape.mpFillPropertiesPtr.get() ) );
    mpCustomShapePropertiesPtr = CustomShapePropertiesPtr( new CustomShapeProperties( *rReferencedShape.mpCustomShapePropertiesPtr.get() ) );
    mpTablePropertiesPtr = table::TablePropertiesPtr( rReferencedShape.mpTablePropertiesPtr.get() ? new table::TableProperties( *rReferencedShape.mpTablePropertiesPtr.get() ) : NULL );
    mpMasterTextListStyle = TextListStylePtr( new TextListStyle( *rReferencedShape.mpMasterTextListStyle.get() ) );
    maShapeStyleRefs = rReferencedShape.maShapeStyleRefs;
    maSize = rReferencedShape.maSize;
    maPosition = rReferencedShape.maPosition;
    mnRotation = rReferencedShape.mnRotation;
    mbFlipH = rReferencedShape.mbFlipH;
    mbFlipV = rReferencedShape.mbFlipV;
}

// for group shapes, the following method is also adding each child
void Shape::addChildren(
        const ::oox::core::XmlFilterBase& rFilterBase,
        Shape& rMaster,
        const ThemePtr& rxTheme,
        const Reference< XShapes >& rxShapes,
        const awt::Rectangle& rClientRect,
        ShapeIdMap* pShapeMap )
{
    // first the global child union needs to be calculated
    sal_Int32 nGlobalLeft  = SAL_MAX_INT32;
    sal_Int32 nGlobalRight = SAL_MIN_INT32;
    sal_Int32 nGlobalTop   = SAL_MAX_INT32;
    sal_Int32 nGlobalBottom= SAL_MIN_INT32;
    std::vector< ShapePtr >::iterator aIter( rMaster.maChildren.begin() );
    while( aIter != rMaster.maChildren.end() )
    {
        sal_Int32 l = (*aIter)->maPosition.X;
        sal_Int32 t = (*aIter)->maPosition.Y;
        sal_Int32 r = l + (*aIter)->maSize.Width;
        sal_Int32 b = t + (*aIter)->maSize.Height;
        if ( nGlobalLeft > l )
            nGlobalLeft = l;
        if ( nGlobalRight < r )
            nGlobalRight = r;
        if ( nGlobalTop > t )
            nGlobalTop = t;
        if ( nGlobalBottom < b )
            nGlobalBottom = b;
        aIter++;
    }
    aIter = rMaster.maChildren.begin();
    while( aIter != rMaster.maChildren.end() )
    {
        Rectangle aShapeRect;
        Rectangle* pShapeRect = 0;
        if ( ( nGlobalLeft != SAL_MAX_INT32 ) && ( nGlobalRight != SAL_MIN_INT32 ) && ( nGlobalTop != SAL_MAX_INT32 ) && ( nGlobalBottom != SAL_MIN_INT32 ) )
        {
            sal_Int32 nGlobalWidth = nGlobalRight - nGlobalLeft;
            sal_Int32 nGlobalHeight = nGlobalBottom - nGlobalTop;
            if ( nGlobalWidth && nGlobalHeight )
            {
                double fWidth = (*aIter)->maSize.Width;
                double fHeight= (*aIter)->maSize.Height;
                double fXScale = (double)rClientRect.Width / (double)nGlobalWidth;
                double fYScale = (double)rClientRect.Height / (double)nGlobalHeight;
                aShapeRect.X = static_cast< sal_Int32 >( ( ( (*aIter)->maPosition.X - nGlobalLeft ) * fXScale ) + rClientRect.X );
                aShapeRect.Y = static_cast< sal_Int32 >( ( ( (*aIter)->maPosition.Y - nGlobalTop  ) * fYScale ) + rClientRect.Y );
                fWidth *= fXScale;
                fHeight *= fYScale;
                aShapeRect.Width = static_cast< sal_Int32 >( fWidth );
                aShapeRect.Height = static_cast< sal_Int32 >( fHeight );
                pShapeRect = &aShapeRect;
            }
        }
        (*aIter++)->addShape( rFilterBase, rxTheme, rxShapes, pShapeRect, pShapeMap );
    }
}

Reference< XShape > Shape::createAndInsert(
        const ::oox::core::XmlFilterBase& rFilterBase,
        const rtl::OUString& rServiceName,
        const ThemePtr& rxTheme,
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
        const awt::Rectangle* pShapeRect,
        sal_Bool bClearText )
{
    awt::Size aSize( pShapeRect ? awt::Size( pShapeRect->Width, pShapeRect->Height ) : maSize );
    awt::Point aPosition( pShapeRect ? awt::Point( pShapeRect->X, pShapeRect->Y ) : maPosition );

    OUString aServiceName = rServiceName;
    if( mxCreateCallback.get() )
        aServiceName = mxCreateCallback->onCreateXShape( aServiceName, awt::Rectangle( aPosition.X / 360, aPosition.Y / 360, aSize.Width / 360, aSize.Height / 360 ) );

    basegfx::B2DHomMatrix aTransformation;
    if( aSize.Width != 1 || aSize.Height != 1)
    {
        // take care there are no zeros used by error
        aTransformation.scale(
            aSize.Width ? aSize.Width / 360.0 : 1.0,
            aSize.Height ? aSize.Height / 360.0 : 1.0 );
    }

    if( mbFlipH || mbFlipV || mnRotation != 0)
    {
        // calculate object's center
        basegfx::B2DPoint aCenter(0.5, 0.5);
        aCenter *= aTransformation;

        // center object at origin
        aTransformation.translate( -aCenter.getX(), -aCenter.getY() );

        if( mbFlipH || mbFlipV)
        {
            // mirror around object's center
            aTransformation.scale( mbFlipH ? -1.0 : 1.0, mbFlipV ? -1.0 : 1.0 );
        }

        if( mnRotation != 0 )
        {
            // rotate around object's center
            aTransformation.rotate( F_PI180 * ( (double)mnRotation / 60000.0 ) );
        }

        // move object back from center
        aTransformation.translate( aCenter.getX(), aCenter.getY() );
    }

    if( aPosition.X != 0 || aPosition.Y != 0)
    {
        // if global position is used, add it to transformation
        aTransformation.translate( aPosition.X / 360.0, aPosition.Y / 360.0 );
    }

    // special for lineshape
    if ( aServiceName == OUString::createFromAscii( "com.sun.star.drawing.LineShape" ) )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        // now creating the corresponding PolyPolygon
        sal_Int32 i, nNumPoints = aPoly.count();
        uno::Sequence< awt::Point > aPointSequence( nNumPoints );
        awt::Point* pPoints = aPointSequence.getArray();
        for( i = 0; i < nNumPoints; ++i )
        {
            const ::basegfx::B2DPoint aPoint( aPoly.getB2DPoint( i ) );
            pPoints[ i ] = awt::Point( static_cast< sal_Int32 >( aPoint.getX() ), static_cast< sal_Int32 >( aPoint.getY() ) );
        }
        uno::Sequence< uno::Sequence< awt::Point > > aPolyPolySequence( 1 );
        aPolyPolySequence.getArray()[ 0 ] = aPointSequence;

        maShapeProperties[ PROP_PolyPolygon ] <<= aPolyPolySequence;
    }
    else if ( aServiceName == OUString::createFromAscii( "com.sun.star.drawing.ConnectorShape" ) )
    {
        ::basegfx::B2DPolygon aPoly;
        aPoly.insert( 0, ::basegfx::B2DPoint( 0, 0 ) );
        aPoly.insert( 1, ::basegfx::B2DPoint( maSize.Width ? 1 : 0, maSize.Height ? 1 : 0 ) );
        aPoly.transform( aTransformation );

        basegfx::B2DPoint aStartPosition( aPoly.getB2DPoint( 0 ) );
        basegfx::B2DPoint aEndPosition( aPoly.getB2DPoint( 1 ) );
        awt::Point aAWTStartPosition( static_cast< sal_Int32 >( aStartPosition.getX() ), static_cast< sal_Int32 >( aStartPosition.getY() ) );
        awt::Point aAWTEndPosition( static_cast< sal_Int32 >( aEndPosition.getX() ), static_cast< sal_Int32 >( aEndPosition.getY() ) );

        maShapeProperties[ PROP_StartPosition ] <<= aAWTStartPosition;
        maShapeProperties[ PROP_EndPosition ] <<= aAWTEndPosition;
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

        maShapeProperties[ PROP_Transformation ] <<= aMatrix;
    }
    Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
    if ( !mxShape.is() )
        mxShape = Reference< drawing::XShape >( xServiceFact->createInstance( aServiceName ), UNO_QUERY_THROW );

    Reference< XPropertySet > xSet( mxShape, UNO_QUERY );
    if( mxShape.is() && xSet.is() )
    {
        if( msName.getLength() )
        {
            Reference< container::XNamed > xNamed( mxShape, UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( msName );
        }
        rxShapes->add( mxShape );

        // sj: removing default text of placeholder objects such as SlideNumberShape or HeaderShape
        if ( bClearText )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            if ( xText.is() )
            {
                OUString aEmpty;
                xText->setString( aEmpty );
            }
        }

        LineProperties aLineProperties;
        aLineProperties.maLineFill.moFillType = XML_noFill;
        sal_Int32 nLinePhClr = -1;
        FillProperties aFillProperties;
        aFillProperties.moFillType = XML_noFill;
        sal_Int32 nFillPhClr = -1;

        if( rxTheme.get() )
        {
            if( const ShapeStyleRef* pLineRef = getShapeStyleRef( XML_lnRef ) )
            {
                if( const LineProperties* pLineProps = rxTheme->getLineStyle( pLineRef->mnThemedIdx ) )
                    aLineProperties.assignUsed( *pLineProps );
                nLinePhClr = pLineRef->maPhClr.getColor( rFilterBase );
            }
            if( const ShapeStyleRef* pFillRef = getShapeStyleRef( XML_fillRef ) )
            {
                if( const FillProperties* pFillProps = rxTheme->getFillStyle( pFillRef->mnThemedIdx ) )
                    aFillProperties.assignUsed( *pFillProps );
                nFillPhClr = pFillRef->maPhClr.getColor( rFilterBase );
            }
//            if( const ShapeStyleRef* pEffectRef = getShapeStyleRef( XML_fillRef ) )
//            {
//                if( const EffectProperties* pEffectProps = rxTheme->getEffectStyle( pEffectRef->mnThemedIdx ) )
//                    aEffectProperties.assignUsed( *pEffectProps );
//                nEffectPhClr = pEffectRef->maPhClr.getColor( rFilterBase );
//            }
        }

        aLineProperties.assignUsed( getLineProperties() );
        aFillProperties.assignUsed( getFillProperties() );

        PropertyMap aShapeProperties;
        aShapeProperties.insert( getShapeProperties().begin(), getShapeProperties().end() );
        if( mxCreateCallback.get() )
            aShapeProperties.insert( mxCreateCallback->getShapeProperties().begin(), mxCreateCallback->getShapeProperties().end() );

        // add properties from textbody to shape properties
        if( mpTextBody.get() )
            aShapeProperties.insert( mpTextBody->getTextProperties().maPropertyMap.begin(), mpTextBody->getTextProperties().maPropertyMap.end() );

        // applying properties
        PropertySet aPropSet( xSet );
        if ( aServiceName == OUString::createFromAscii( "com.sun.star.drawing.GraphicObjectShape" ) )
            mpGraphicPropertiesPtr->pushToPropSet( aPropSet, rFilterBase, -1 );
        if ( mpTablePropertiesPtr.get() && ( aServiceName == OUString::createFromAscii( "com.sun.star.drawing.TableShape" ) ) )
            mpTablePropertiesPtr->pushToPropSet( rFilterBase, xSet, mpMasterTextListStyle );
        aFillProperties.pushToPropSet( aPropSet, FillProperties::DEFAULT_IDS, rFilterBase, rFilterBase.getModelObjectHelper(), mnRotation, nFillPhClr );
        aLineProperties.pushToPropSet( aPropSet, LineProperties::DEFAULT_IDS, rFilterBase, rFilterBase.getModelObjectHelper(), nLinePhClr );

        // applying autogrowheight property before setting shape size, because
        // the shape size might be changed if currently autogrowheight is true
        // we must also check that the PropertySet supports the property.
        Reference< XPropertySetInfo > xSetInfo( xSet->getPropertySetInfo() );
        const OUString& rPropName = PropertyMap::getPropertyName( PROP_TextAutoGrowHeight );
        if( xSetInfo.is() && xSetInfo->hasPropertyByName( rPropName ) )
            if( /*const Any* pAutoGrowHeight =*/ aShapeProperties.getProperty( PROP_TextAutoGrowHeight ) )
                xSet->setPropertyValue( rPropName, Any( false ) );

        // do not set properties at a group shape (this causes assertions from svx)
        if( aServiceName != OUString::createFromAscii( "com.sun.star.drawing.GroupShape" ) )
            aPropSet.setProperties( aShapeProperties );

        if( aServiceName == OUString::createFromAscii( "com.sun.star.drawing.CustomShape" ) )
            mpCustomShapePropertiesPtr->pushToPropSet( rFilterBase, xSet, mxShape );

        // in some cases, we don't have any text body.
        if( getTextBody() )
        {
            Reference < XText > xText( mxShape, UNO_QUERY );
            if ( xText.is() )   // not every shape is supporting an XText interface (e.g. GroupShape)
            {
                TextCharacterProperties aCharStyleProperties;
                if( const ShapeStyleRef* pFontRef = getShapeStyleRef( XML_fontRef ) )
                {
                    if( rxTheme.get() )
                        if( const TextCharacterProperties* pCharProps = rxTheme->getFontStyle( pFontRef->mnThemedIdx ) )
                            aCharStyleProperties.assignUsed( *pCharProps );
                    aCharStyleProperties.maCharColor.assignIfUsed( pFontRef->maPhClr );
                }

                Reference < XTextCursor > xAt = xText->createTextCursor();
                getTextBody()->insertAt( rFilterBase, xText, xAt, aCharStyleProperties, mpMasterTextListStyle );
            }
        }
    }

    // use a callback for further processing on the XShape (e.g. charts)
    if( mxShape.is() && mxCreateCallback.get() )
        mxCreateCallback->onXShapeCreated( mxShape );

    return mxShape;
}

// the properties of rSource which are not part of rDest are being put into rDest
void addMissingProperties( const PropertyMap& rSource, PropertyMap& rDest )
{
    PropertyMap::const_iterator aSourceIter( rSource.begin() );
    while( aSourceIter != rSource.end() )
    {
        if ( rDest.find( (*aSourceIter ).first ) == rDest.end() )
            rDest[ (*aSourceIter).first ] <<= (*aSourceIter).second;
        aSourceIter++;
    }
}

void Shape::setTextBody(const TextBodyPtr & pTextBody)
{
    mpTextBody = pTextBody;
}


TextBodyPtr Shape::getTextBody()
{
    return mpTextBody;
}

void Shape::setMasterTextListStyle( const TextListStylePtr& pMasterTextListStyle )
{
    mpMasterTextListStyle = pMasterTextListStyle;
}


} }
