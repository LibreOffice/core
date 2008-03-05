/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slidefragmenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:49:23 $
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

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "tokens.hxx"
#include "oox/core/namespaces.hxx"
#include <oox/ppt/backgroundproperties.hxx>
#include "oox/ppt/slidefragmenthandler.hxx"
#include "oox/ppt/slidetimingcontext.hxx"
#include "oox/ppt/slidetransitioncontext.hxx"
#include "oox/ppt/slidemastertextstylescontext.hxx"
#include "oox/ppt/pptshapegroupcontext.hxx"
#include "oox/ppt/pptshape.hxx"
#include "oox/drawingml/clrschemecontext.hxx"


using rtl::OUString;
using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {

SlideFragmentHandler::SlideFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, SlidePersistPtr pPersistPtr, const ShapeLocation eShapeLocation ) throw()
: FragmentHandler( rFilter, rFragmentPath )
, mpSlidePersistPtr( pPersistPtr )
, meShapeLocation( eShapeLocation )
{
    OUString aVMLDrawingFragmentPath = getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "vmlDrawing" ) );
    if( aVMLDrawingFragmentPath.getLength() > 0 )
    {
        getFilter().importFragment( new oox::vml::DrawingFragmentHandler(
            getFilter(), aVMLDrawingFragmentPath, pPersistPtr->getDrawing() ) );
    }
}

SlideFragmentHandler::~SlideFragmentHandler() throw()
{
}

Reference< XFastContextHandler > SlideFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;

    switch( aElementToken )
    {
    case NMSP_PPT|XML_sldMaster:        // CT_SlideMaster
    case NMSP_PPT|XML_handoutMaster:    // CT_HandoutMaster
    case NMSP_PPT|XML_sld:              // CT_CommonSlideData
    case NMSP_PPT|XML_notes:            // CT_NotesSlide
    case NMSP_PPT|XML_notesMaster:      // CT_NotesMaster
        break;
    case NMSP_PPT|XML_cSld:             // CT_CommonSlideData
        maSlideName = xAttribs->getOptionalValue(XML_name);
        break;

    case NMSP_PPT|XML_spTree:           // CT_GroupShape
        {
            xRet.set( new PPTShapeGroupContext(
                *this, mpSlidePersistPtr, meShapeLocation, mpSlidePersistPtr->getShapes(),
                oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GroupShape" ) ) ) );
        }
        break;

    case NMSP_PPT|XML_timing: // CT_SlideTiming
        xRet.set( new SlideTimingContext( *this, mpSlidePersistPtr->getTimeNodeList() ) );
        break;
    case NMSP_PPT|XML_transition: // CT_SlideTransition
        xRet.set( new SlideTransitionContext( *this, xAttribs, maSlideProperties ) );
        break;

    // BackgroundGroup
    case NMSP_PPT|XML_bgPr:             // CT_BackgroundProperties
        {
            FillPropertiesPtr pFillPropertiesPtr( new FillProperties() );
            xRet.set( new BackgroundPropertiesContext( *this, pFillPropertiesPtr ) );
            mpSlidePersistPtr->setBackgroundProperties( pFillPropertiesPtr );
        }
        break;
    case NMSP_PPT|XML_bgRef:            // a:CT_StyleMatrixReference
        break;

    case NMSP_PPT|XML_clrMap:           // CT_ColorMapping
        {
            oox::drawingml::ClrMapPtr pClrMapPtr( new oox::drawingml::ClrMap() );
            xRet.set( new oox::drawingml::clrMapContext( *this, xAttribs, *pClrMapPtr ) );
            mpSlidePersistPtr->setClrMap( pClrMapPtr );
        }
        break;
    case NMSP_PPT|XML_clrMapOvr:        // CT_ColorMappingOverride
    case NMSP_PPT|XML_sldLayoutIdLst:   // CT_SlideLayoutIdList
        break;
    case NMSP_PPT|XML_txStyles:         // CT_SlideMasterTextStyles
        xRet.set( new SlideMasterTextStylesContext( *this, mpSlidePersistPtr ) );
        break;
    case NMSP_PPT|XML_custDataLst:      // CT_CustomerDataList
    case NMSP_PPT|XML_tagLst:           // CT_TagList
        break;
    }

    if( !xRet.is() )
        xRet = getFastContextHandler();

    return xRet;
}

void SAL_CALL SlideFragmentHandler::endDocument(  ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException)
{
    try
    {
        Reference< XDrawPage > xSlide( mpSlidePersistPtr->getPage() );
        if( !maSlideProperties.empty() )
        {
            uno::Reference< beans::XMultiPropertySet > xMSet( xSlide, uno::UNO_QUERY );
            if( xMSet.is() )
            {
                uno::Sequence< OUString > aNames;
                uno::Sequence< uno::Any > aValues;
                maSlideProperties.makeSequence( aNames, aValues );
                xMSet->setPropertyValues( aNames,  aValues);
            }
            else
            {
                uno::Reference< beans::XPropertySet > xSet( xSlide, uno::UNO_QUERY_THROW );
                uno::Reference< beans::XPropertySetInfo > xInfo( xSet->getPropertySetInfo() );

                for( PropertyMap::const_iterator aIter( maSlideProperties.begin() ); aIter != maSlideProperties.end(); aIter++ )
                {
                    if ( xInfo->hasPropertyByName( (*aIter).first ) )
                        xSet->setPropertyValue( (*aIter).first, (*aIter).second );
                }
            }
        }
        if ( maSlideName.getLength() )
        {
            Reference< XNamed > xNamed( xSlide, UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( maSlideName );
        }
    }
    catch( uno::Exception& )
    {
        OSL_ENSURE( false,
            (rtl::OString("oox::ppt::SlideFragmentHandler::EndElement(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

} }

