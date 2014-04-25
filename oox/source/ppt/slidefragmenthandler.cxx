/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "oox/helper/propertyset.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "headerfootercontext.hxx"
#include "oox/ppt/backgroundproperties.hxx"
#include "oox/ppt/slidefragmenthandler.hxx"
#include "oox/ppt/slidetimingcontext.hxx"
#include "oox/ppt/slidetransitioncontext.hxx"
#include "oox/ppt/slidemastertextstylescontext.hxx"
#include "oox/ppt/pptshapegroupcontext.hxx"
#include "oox/ppt/pptshape.hxx"
#include "oox/vml/vmldrawing.hxx"
#include "oox/vml/vmldrawingfragment.hxx"
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
    OUString aVMLDrawingFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "vmlDrawing" ) );
    if( aVMLDrawingFragmentPath.getLength() > 0 )
        getFilter().importFragment( new oox::vml::DrawingFragment(
            getFilter(), aVMLDrawingFragmentPath, *pPersistPtr->getDrawing() ) );
}

SlideFragmentHandler::~SlideFragmentHandler() throw()
{
    // convert and insert all VML shapes (mostly form controls)
    mpSlidePersistPtr->getDrawing()->convertAndInsert();
}

Reference< XFastContextHandler > SlideFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    AttributeList aAttribs( xAttribs );

    switch( aElementToken )
    {
    case PPT_TOKEN( sld ):              // CT_Slide
    {
        OptValue< bool > aShowMasterSp = aAttribs.getBool( XML_showMasterSp );
        if( aShowMasterSp.has() && !aShowMasterSp.get() )
        {
            PropertyMap aPropMap;
            aPropMap[ PROP_IsBackgroundObjectsVisible ] = Any( false );

            Reference< XDrawPage > xSlide( mpSlidePersistPtr->getPage() );
            PropertySet aSlideProp( xSlide );
            aSlideProp.setProperties( aPropMap );
        }
    }
    case PPT_TOKEN( sldMaster ):        // CT_SlideMaster
    case PPT_TOKEN( handoutMaster ):    // CT_HandoutMaster
    {
        OptValue< bool > aShow = aAttribs.getBool( XML_show );
        if( aShow.has() && !aShow.get() )
        {
            PropertyMap aPropMap;
            aPropMap[ PROP_Visible ] = Any( false );

            Reference< XDrawPage > xSlide( mpSlidePersistPtr->getPage() );
            PropertySet aSlideProp( xSlide );
            aSlideProp.setProperties( aPropMap );
        }
        break;
    }
    case PPT_TOKEN( notes ):            // CT_NotesSlide
    case PPT_TOKEN( notesMaster ):      // CT_NotesMaster
        break;
    case PPT_TOKEN( cSld ):             // CT_CommonSlideData
        maSlideName = xAttribs->getOptionalValue(XML_name);
        break;

    case PPT_TOKEN( spTree ):           // CT_GroupShape
        {
            xRet.set( new PPTShapeGroupContext(
                *this, mpSlidePersistPtr, meShapeLocation, mpSlidePersistPtr->getShapes(),
                oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GroupShape" ) ) ) );
        }
        break;

    case PPT_TOKEN( controls ):
        xRet = getFastContextHandler();
        break;
    case PPT_TOKEN( control ):
        {
            ::oox::vml::ControlInfo aInfo;
            aInfo.setShapeId( aAttribs.getInteger( XML_spid, 0 ) );
            aInfo.maFragmentPath = getFragmentPathFromRelId( aAttribs.getString( R_TOKEN( id ), OUString() ) );
            aInfo.maName = aAttribs.getXString( XML_name, OUString() );
            mpSlidePersistPtr->getDrawing()->registerControl( aInfo );
        }
        return xRet;

    case PPT_TOKEN( timing ): // CT_SlideTiming
        xRet.set( new SlideTimingContext( *this, mpSlidePersistPtr->getTimeNodeList() ) );
        break;
    case PPT_TOKEN( transition ): // CT_SlideTransition
        xRet.set( new SlideTransitionContext( *this, xAttribs, maSlideProperties ) );
        break;
    case PPT_TOKEN( hf ):
        xRet.set( new HeaderFooterContext( *this, xAttribs, mpSlidePersistPtr->getHeaderFooter() ) );
        break;

    // BackgroundGroup
    case PPT_TOKEN( bgPr ):             // CT_BackgroundProperties
        {
            FillPropertiesPtr pFillPropertiesPtr( new FillProperties );
            xRet.set( new BackgroundPropertiesContext( *this, *pFillPropertiesPtr ) );
            mpSlidePersistPtr->setBackgroundProperties( pFillPropertiesPtr );
        }
        break;

    case PPT_TOKEN( bgRef ):            // a:CT_StyleMatrixReference
        {
            oox::drawingml::ThemePtr pTheme = mpSlidePersistPtr->getTheme();
            if (pTheme)
            {
                FillPropertiesPtr pFillPropertiesPtr( new FillProperties(
                    *pTheme->getFillStyle( xAttribs->getOptionalValue( XML_idx ).toInt32() ) ) );
                mpSlidePersistPtr->setBackgroundProperties( pFillPropertiesPtr );
            }
            xRet.set( new ColorContext( *this, mpSlidePersistPtr->getBackgroundColor() ) );

        }
        break;

    case PPT_TOKEN( clrMap ):           // CT_ColorMapping
        {
            oox::drawingml::ClrMapPtr pClrMapPtr( new oox::drawingml::ClrMap() );
            xRet.set( new oox::drawingml::clrMapContext( *this, xAttribs, *pClrMapPtr ) );
            mpSlidePersistPtr->setClrMap( pClrMapPtr );
        }
        break;
    case PPT_TOKEN( clrMapOvr ):        // CT_ColorMappingOverride
    case PPT_TOKEN( sldLayoutIdLst ):   // CT_SlideLayoutIdList
        break;
    case PPT_TOKEN( txStyles ):         // CT_SlideMasterTextStyles
        xRet.set( new SlideMasterTextStylesContext( *this, mpSlidePersistPtr ) );
        break;
    case PPT_TOKEN( custDataLst ):      // CT_CustomerDataList
    case PPT_TOKEN( tagLst ):           // CT_TagList
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
        PropertySet aSlideProp( xSlide );
        aSlideProp.setProperties( maSlideProperties );
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

