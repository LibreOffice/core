/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidepersist.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:49:57 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <boost/bind.hpp>
#include "oox/ppt/timenode.hxx"
#include "oox/ppt/pptshape.hxx"
#include "oox/ppt/slidepersist.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/animations/XAnimationNodeSupplier.hpp>

using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::animations;

namespace oox { namespace ppt {

SlidePersist::SlidePersist( sal_Bool bMaster, sal_Bool bNotes,
    const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxPage,
        oox::drawingml::ShapePtr pShapesPtr, const drawingml::TextListStylePtr & pDefaultTextStyle )
: mpDrawingPtr( new oox::vml::Drawing )
, mxPage( rxPage )
, maShapesPtr( pShapesPtr )
, mnLayoutValueToken( 0 )
, mbMaster( bMaster )
, mbNotes ( bNotes )
, maDefaultTextStylePtr( pDefaultTextStyle )
, maTitleTextStylePtr( new oox::drawingml::TextListStyle )
, maBodyTextStylePtr( new oox::drawingml::TextListStyle )
, maNotesTextStylePtr( new oox::drawingml::TextListStyle )
, maOtherTextStylePtr( new oox::drawingml::TextListStyle )
{
}

SlidePersist::~SlidePersist()
{

}

sal_Int16 SlidePersist::getLayoutFromValueToken()
{
    sal_Int16 nLayout = 20;     // 20 == blanc (so many magic numbers :-( the description at com.sun.star.presentation.DrawPage.Layout does not help)
    switch( mnLayoutValueToken )
    {
        case XML_blank:             nLayout = 20; break;
        case XML_chart:             nLayout =  2; break;
        case XML_chartAndTx:        nLayout =  7; break;
        case XML_clipArtAndTx:      nLayout =  9; break;
        case XML_clipArtAndVertTx:  nLayout = 24; break;
        case XML_fourObj:           nLayout = 18; break;
        case XML_obj:               nLayout = 11; break;
        case XML_objAndTx:          nLayout = 13; break;
        case XML_objOverTx:         nLayout = 14; break;
        case XML_tbl:               nLayout =  8; break;
        case XML_title:             nLayout =  0; break;
        case XML_titleOnly:         nLayout = 19; break;
        case XML_twoObj:
        case XML_twoColTx:          nLayout =  3; break;
        case XML_twoObjAndTx:       nLayout = 15; break;
        case XML_twoObjOverTx:      nLayout = 16; break;
        case XML_tx:                nLayout =  1; break;
        case XML_txAndChart:        nLayout =  4; break;
        case XML_txAndClipArt:      nLayout =  6; break;
        case XML_txAndMedia:        nLayout =  6; break;
        case XML_txAndObj:          nLayout = 10; break;
        case XML_txAndTwoObj:       nLayout = 12; break;
        case XML_txOverObj:         nLayout = 17; break;
        case XML_vertTitleAndTx:    nLayout = 22; break;
        case XML_vertTitleAndTxOverChart: nLayout = 21; break;
        case XML_vertTx:            nLayout = 23; break;

        case XML_twoTxTwoObj:
        case XML_twoObjAndObj:
        case XML_objTx:
        case XML_picTx:
        case XML_secHead:
        case XML_objOnly:
        case XML_objAndTwoObj:
        case XML_mediaAndTx:
        case XML_dgm:
        case XML_cust:
        default:
            nLayout = 20;
    }
    return nLayout;
}

void SlidePersist::createXShapes( const oox::core::XmlFilterBase& rFilterBase )
{
    applyTextStyles( rFilterBase );

    Reference< XShapes > xShapes( getPage(), UNO_QUERY );

    std::vector< oox::drawingml::ShapePtr >& rShapes( maShapesPtr->getChilds() );
    std::vector< oox::drawingml::ShapePtr >::iterator aShapesIter( rShapes.begin() );
    while( aShapesIter != rShapes.end() )
    {
        std::vector< oox::drawingml::ShapePtr >& rChilds( (*aShapesIter++)->getChilds() );
        std::vector< oox::drawingml::ShapePtr >::iterator aChildIter( rChilds.begin() );
        while( aChildIter != rChilds.end() )
        {
            PPTShape* pPPTShape = dynamic_cast< PPTShape* >( (*aChildIter).get() );
            if ( pPPTShape )
                pPPTShape->addShape( rFilterBase, *this, getTheme(), xShapes, 0, &getShapeMap() );
            else
                (*aChildIter)->addShape( rFilterBase, getTheme(), xShapes, 0, &getShapeMap() );

            aChildIter++;
        }
    }

    Reference< XAnimationNodeSupplier > xNodeSupplier( getPage(), UNO_QUERY);
    if( xNodeSupplier.is() )
    {
        Reference< XAnimationNode > xNode( xNodeSupplier->getAnimationNode() );
        if( xNode.is() && !maTimeNodeList.empty() )
        {
            SlidePersistPtr pSlidePtr( shared_from_this() );
            TimeNodePtr pNode(maTimeNodeList.front());
            OSL_ENSURE( pNode, "pNode" );

            pNode->setNode( rFilterBase.getModel(), xNode, pSlidePtr );
        }
    }
}

void SlidePersist::createBackground( const oox::core::XmlFilterBase& rFilterBase )
{
    if ( mpBackgroundPropertiesPtr )
    {
        try
        {
            PropertyMap aPropMap;
            static const rtl::OUString sBackground( RTL_CONSTASCII_USTRINGPARAM( "Background" ) );
            uno::Reference< beans::XPropertySet > xPagePropSet( mxPage, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xPropertySet( aPropMap.makePropertySet() );
            mpBackgroundPropertiesPtr->pushToPropSet( rFilterBase, xPropertySet, 0 );
            xPagePropSet->setPropertyValue( sBackground, Any( xPropertySet ) );
        }
        catch( Exception )
        {
        }
    }
}

void setTextStyle( Reference< beans::XPropertySet >& rxPropSet,
    oox::drawingml::TextListStylePtr& pTextListStylePtr, int nLevel )
{
    ::oox::drawingml::TextParagraphPropertiesPtr pTextParagraphPropertiesPtr( pTextListStylePtr->getListStyle()[ nLevel ] );
    if( pTextParagraphPropertiesPtr == NULL )
    {
        // no properties. return
        return;
    }
    ::oox::drawingml::TextCharacterPropertiesPtr pTextCharacterPropertiesPtr( pTextParagraphPropertiesPtr->getTextCharacterProperties() );
    if( pTextCharacterPropertiesPtr == NULL )
    {
        // no properties. return
        return;
    }
    PropertyMap& rParagraphProperties( pTextParagraphPropertiesPtr->getTextParagraphPropertyMap() );
    PropertyMap& rCharacterProperties( pTextCharacterPropertiesPtr->getTextCharacterPropertyMap() );

    int i;
    Sequence< rtl::OUString > aNames;
    Sequence< Any > aValues;
    rParagraphProperties.makeSequence( aNames, aValues );
    for( i = 0; i < aNames.getLength(); i++ )
        rxPropSet->setPropertyValue( aNames[ i ], aValues[ i ] );
    rCharacterProperties.makeSequence( aNames, aValues );
    for( i = 0; i < aNames.getLength(); i++ )
        rxPropSet->setPropertyValue( aNames[ i ], aValues[ i ] );
}

void SlidePersist::applyTextStyles( const oox::core::XmlFilterBase& rFilterBase )
{
    if ( mbMaster )
    {
        try
        {
            Reference< style::XStyleFamiliesSupplier > aXStyleFamiliesSupplier( rFilterBase.getModel(), UNO_QUERY_THROW );
            Reference< container::XNameAccess > aXNameAccess( aXStyleFamiliesSupplier->getStyleFamilies() );
            Reference< container::XNamed > aXNamed( mxPage, UNO_QUERY_THROW );

            if ( aXNameAccess.is() && aXNamed.is() )
            {
                oox::drawingml::TextListStylePtr pTextListStylePtr;
                rtl::OUString aStyle;
                rtl::OUString aFamily;

                const rtl::OUString sOutline( RTL_CONSTASCII_USTRINGPARAM( "outline1" ) );
                const rtl::OUString sTitle( RTL_CONSTASCII_USTRINGPARAM( "title" ) );
                const rtl::OUString sStandard( RTL_CONSTASCII_USTRINGPARAM( "standard" ) );
                const rtl::OUString sSubtitle( RTL_CONSTASCII_USTRINGPARAM( "subtitle" ) );

                for( int i = 0; i < 4; i++ )    // todo: aggregation of bodystyle (subtitle)
                {
                    switch( i )
                    {
                        case 0 :    // title style
                        {
                            pTextListStylePtr = maTitleTextStylePtr;
                            aStyle = sTitle;
                            aFamily= aXNamed->getName();
                            break;
                        }
                        case 1 :    // body style
                        {
                            pTextListStylePtr = maBodyTextStylePtr;
                            aStyle = sOutline;
                            aFamily= aXNamed->getName();
                            break;
                        }
                        case 3 :    // notes style
                        {
                            pTextListStylePtr = maNotesTextStylePtr;
                            aStyle = sTitle;
                            aFamily= aXNamed->getName();
                            break;
                        }
                        case 4 :    // standard style
                        {
                            pTextListStylePtr = maOtherTextStylePtr;
                            aStyle = sStandard;
                            aFamily = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "graphics" ) );
                            break;
                        }
                        case 5 :    // subtitle
                        {
                            pTextListStylePtr = maBodyTextStylePtr;
                            aStyle = sSubtitle;
                            aFamily = aXNamed->getName();
                            break;
                        }
                    }
                    Reference< container::XNameAccess > xFamilies;
                    if ( aXNameAccess->hasByName( aFamily ) )
                    {
                        if( aXNameAccess->getByName( aFamily ) >>= xFamilies )
                        {
                            if ( xFamilies->hasByName( aStyle ) )
                            {
                                Reference< style::XStyle > aXStyle;
                                if ( xFamilies->getByName( aStyle ) >>= aXStyle )
                                {
                                    Reference< beans::XPropertySet > xPropSet( aXStyle, UNO_QUERY_THROW );
                                    setTextStyle( xPropSet, maDefaultTextStylePtr, 0 );
                                    setTextStyle( xPropSet, pTextListStylePtr, 0 );
                                    for ( int nLevel = 1; nLevel < 5; nLevel++ )
                                    {
                                        if ( i == 1 /* BodyStyle */ )
                                        {
                                            sal_Char pOutline[ 9 ] = "outline1";
                                            pOutline[ 7 ] = static_cast< sal_Char >( '1' + i );
                                            rtl::OUString sOutlineStyle( rtl::OUString::createFromAscii( pOutline ) );
                                            if ( xFamilies->hasByName( sOutlineStyle ) )
                                            {
                                                xFamilies->getByName( sOutlineStyle ) >>= aXStyle;
                                                if( aXStyle.is() )
                                                    xPropSet = Reference< beans::XPropertySet >( aXStyle, UNO_QUERY_THROW );
                                            }
                                        }
                                        setTextStyle( xPropSet, maDefaultTextStylePtr, nLevel );
                                        setTextStyle( xPropSet, pTextListStylePtr, nLevel );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        catch( Exception& )
        {
        }
    }
}

} }

