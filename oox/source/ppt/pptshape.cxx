/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptshape.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:48:12 $
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

#include "oox/ppt/pptshape.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/text/XText.hpp>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include "oox/ppt/slidepersist.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::drawing;

namespace oox { namespace ppt {

PPTShape::PPTShape( const oox::ppt::ShapeLocation eShapeLocation, const sal_Char* pServiceName )
: Shape( pServiceName )
, meShapeLocation( eShapeLocation )
, mbReferenced( sal_False )
{
}

PPTShape::~PPTShape()
{
}

void PPTShape::addShape(
        const oox::core::XmlFilterBase& rFilterBase,
        const SlidePersist& rSlidePersist,
        const oox::drawingml::ThemePtr& rxTheme,
        const Reference< XShapes >& rxShapes,
        const awt::Rectangle* pShapeRect,
        ::oox::drawingml::ShapeIdMap* pShapeMap )
{
    // only placeholder from layout are being inserted
    if ( mnSubType && ( meShapeLocation == Master ) )
        return;
    try
    {
        rtl::OUString sServiceName( msServiceName );
        if( sServiceName.getLength() )
        {
            oox::drawingml::TextListStylePtr aMasterTextListStyle;
            Reference< lang::XMultiServiceFactory > xServiceFact( rFilterBase.getModel(), UNO_QUERY_THROW );
            switch( mnSubType )
            {
                case XML_ctrTitle :
                case XML_title :
                {
                    const rtl::OUString sTitleShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.TitleTextShape" ) );
                    sServiceName = sTitleShapeService;
                    aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getTitleTextStyle() : rSlidePersist.getTitleTextStyle();
                }
                break;
                case XML_obj :
                {
                    const rtl::OUString sOutlinerShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.OutlinerShape" ) );
                    sServiceName = sOutlinerShapeService;
                    aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                }
                break;
                case XML_body :
                {
                    const rtl::OUString sNotesShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.NotesShape" ) );
                    const rtl::OUString sOutlinerShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.OutlinerShape" ) );
                    if ( rSlidePersist.isNotesPage() )
                    {
                        sServiceName = sNotesShapeService;
                        aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getNotesTextStyle() : rSlidePersist.getNotesTextStyle();
                    }
                    else
                    {
                        sServiceName = sOutlinerShapeService;
                        aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getBodyTextStyle() : rSlidePersist.getBodyTextStyle();
                    }
                }
                break;
                case XML_dt :
                {
                    const rtl::OUString sDateTimeShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.DateTimeShape" ) );
                    sServiceName = sDateTimeShapeService;
                }
                break;
                case XML_hdr :
                {
                    const rtl::OUString sHeaderShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.HeaderShape" ) );
                    sServiceName = sHeaderShapeService;
                }
                break;
                case XML_ftr :
                {
                    const rtl::OUString sFooterShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.FooterShape" ) );
                    sServiceName = sFooterShapeService;
                }
                break;
                case XML_sldNum :
                {
                    const rtl::OUString sSlideNumberShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.SlideNumberShape" ) );
                    sServiceName = sSlideNumberShapeService;
                }
                break;
                case XML_sldImg :
                {
                    const rtl::OUString sPageShapeService( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.presentation.PageShape" ) );
                    sServiceName = sPageShapeService;
                }
                break;

                default:
                break;

            }
            if ( !aMasterTextListStyle.get() )
                aMasterTextListStyle = rSlidePersist.getMasterPersist().get() ? rSlidePersist.getMasterPersist()->getOtherTextStyle() : rSlidePersist.getOtherTextStyle();
            setMasterTextListStyle( aMasterTextListStyle );

            Reference< XShape > xShape( createAndInsert( rFilterBase, sServiceName, rxTheme, rxShapes, pShapeRect ) );

            if( pShapeMap && msId.getLength() )
            {
                (*pShapeMap)[ msId ] = shared_from_this();
            }

            // if this is a group shape, we have to add also each child shape
            Reference< XShapes > xShapes( xShape, UNO_QUERY );
            if ( xShapes.is() )
                addChilds( rFilterBase, *this, rxTheme, xShapes, pShapeRect ? *pShapeRect : awt::Rectangle( maPosition.X, maPosition.Y, maSize.Width, maSize.Height ), pShapeMap );
        }
    }
    catch( const Exception&  )
    {
    }
}

void PPTShape::applyShapeReference( const oox::drawingml::Shape& rReferencedShape )
{
    Shape::applyShapeReference( rReferencedShape );
}

} }
