/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>

#include "tokens.hxx"
#include "properties.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/core/namespaces.hxx"
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
    OUString aVMLDrawingFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATIONSTYPE( "vmlDrawing" ) );
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
    case NMSP_PPT|XML_sldMaster:		// CT_SlideMaster
    case NMSP_PPT|XML_handoutMaster:	// CT_HandoutMaster
    case NMSP_PPT|XML_sld:				// CT_CommonSlideData
    {
        AttributeList attribs( xAttribs );

        Reference< XDrawPage > xSlide( mpSlidePersistPtr->getPage() );
        PropertyMap aPropMap;
        PropertySet aSlideProp( xSlide );

        aPropMap[ PROP_Visible ] = Any( attribs.getBool( XML_show, sal_True ) );
        aSlideProp.setProperties( aPropMap );

        break;
    }
    case NMSP_PPT|XML_notes:			// CT_NotesSlide
    case NMSP_PPT|XML_notesMaster:		// CT_NotesMaster
        break;
    case NMSP_PPT|XML_cSld:				// CT_CommonSlideData
        maSlideName = xAttribs->getOptionalValue(XML_name);
        break;

    case NMSP_PPT|XML_spTree:			// CT_GroupShape
        {
            xRet.set( new PPTShapeGroupContext(
                *this, mpSlidePersistPtr, meShapeLocation, mpSlidePersistPtr->getShapes(),
                oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GroupShape" ) ) ) );
        }
        break;

    case NMSP_PPT|XML_controls:
        xRet = getFastContextHandler();
        break;
    case NMSP_PPT|XML_control:
        {
            ::oox::vml::ControlInfo aInfo;
            aInfo.setShapeId( aAttribs.getInteger( XML_spid, 0 ) );
            aInfo.maFragmentPath = getFragmentPathFromRelId( aAttribs.getString( R_TOKEN( id ), OUString() ) );
            aInfo.maName = aAttribs.getXString( XML_name, OUString() );
            mpSlidePersistPtr->getDrawing()->registerControl( aInfo );
        }
        return xRet;

    case NMSP_PPT|XML_timing: // CT_SlideTiming
        xRet.set( new SlideTimingContext( *this, mpSlidePersistPtr->getTimeNodeList() ) );
        break;
    case NMSP_PPT|XML_transition: // CT_SlideTransition
        xRet.set( new SlideTransitionContext( *this, xAttribs, maSlideProperties ) );
        break;
    case NMSP_PPT|XML_hf:
        xRet.set( new HeaderFooterContext( *this, xAttribs, mpSlidePersistPtr->getHeaderFooter() ) );
        break;

    // BackgroundGroup
    case NMSP_PPT|XML_bgPr:				// CT_BackgroundProperties
        {
            FillPropertiesPtr pFillPropertiesPtr( new FillProperties );
            xRet.set( new BackgroundPropertiesContext( *this, *pFillPropertiesPtr ) );
            mpSlidePersistPtr->setBackgroundProperties( pFillPropertiesPtr );
        }
        break;
    case NMSP_PPT|XML_bgRef:			// a:CT_StyleMatrixReference
        break;

    case NMSP_PPT|XML_clrMap:			// CT_ColorMapping
        {
            oox::drawingml::ClrMapPtr pClrMapPtr( new oox::drawingml::ClrMap() );
            xRet.set( new oox::drawingml::clrMapContext( *this, xAttribs, *pClrMapPtr ) );
            mpSlidePersistPtr->setClrMap( pClrMapPtr );
        }
        break;
    case NMSP_PPT|XML_clrMapOvr:		// CT_ColorMappingOverride
    case NMSP_PPT|XML_sldLayoutIdLst:	// CT_SlideLayoutIdList
        break;
    case NMSP_PPT|XML_txStyles:			// CT_SlideMasterTextStyles
        xRet.set( new SlideMasterTextStylesContext( *this, mpSlidePersistPtr ) );
        break;
    case NMSP_PPT|XML_custDataLst:		// CT_CustomerDataList
    case NMSP_PPT|XML_tagLst:			// CT_TagList
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
