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
#include "oox/ppt/pptimport.hxx"


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
: FragmentHandler2( rFilter, rFragmentPath )
, mpSlidePersistPtr( pPersistPtr )
, meShapeLocation( eShapeLocation )
{
    OUString aVMLDrawingFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "vmlDrawing" ) );
    if( !aVMLDrawingFragmentPath.isEmpty() )
        getFilter().importFragment( new oox::vml::DrawingFragment(
            getFilter(), aVMLDrawingFragmentPath, *pPersistPtr->getDrawing() ) );
}

SlideFragmentHandler::~SlideFragmentHandler() throw()
{
    // convert and insert all VML shapes (mostly form controls)
    mpSlidePersistPtr->getDrawing()->convertAndInsert();
}

::oox::core::ContextHandlerRef SlideFragmentHandler::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case PPT_TOKEN( sldMaster ):        // CT_SlideMaster
    case PPT_TOKEN( handoutMaster ):    // CT_HandoutMaster
    case PPT_TOKEN( sld ):              // CT_CommonSlideData
    {
        Reference< XDrawPage > xSlide( mpSlidePersistPtr->getPage() );
        PropertyMap aPropMap;
        PropertySet aSlideProp( xSlide );

        aPropMap[ PROP_Visible ] = Any( rAttribs.getBool( XML_show, sal_True ) );
        aSlideProp.setProperties( aPropMap );

        return this;
    }
    case PPT_TOKEN( notes ):            // CT_NotesSlide
    {
        // Import notesMaster
        PowerPointImport& rFilter = dynamic_cast< PowerPointImport& >( getFilter() );
        OUString aNotesFragmentPath = getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "notesMaster" ) );

        std::vector< SlidePersistPtr >& rMasterPages( rFilter.getMasterPages() );
        std::vector< SlidePersistPtr >::iterator aIter( rMasterPages.begin() );
        while( aIter != rMasterPages.end() )
        {
            if( (*aIter)->getPath() == aNotesFragmentPath )
            {
                if( !mpSlidePersistPtr->getMasterPersist() )
                    mpSlidePersistPtr->setMasterPersist( *aIter );
                break;
            }
            ++aIter;
        }
        if( aIter == rMasterPages.end() && !mpSlidePersistPtr->getMasterPersist() )
        {
            TextListStylePtr pTextListStyle(new TextListStyle);
            SlidePersistPtr pMasterPersistPtr = SlidePersistPtr( new SlidePersist( rFilter, sal_True, sal_True, mpSlidePersistPtr->getPage(),
                                ShapePtr( new PPTShape( Master, "com.sun.star.drawing.GroupShape" ) ), mpSlidePersistPtr->getNotesTextStyle() ) );
            //pMasterPersistPtr->setLayoutPath( aLayoutFragmentPath );
            pMasterPersistPtr->setPath( aNotesFragmentPath );
            rFilter.getMasterPages().push_back( pMasterPersistPtr );
            FragmentHandlerRef xMasterFragmentHandler( new SlideFragmentHandler( rFilter, aNotesFragmentPath, pMasterPersistPtr, Master ) );
            rFilter.importFragment( xMasterFragmentHandler );
            //pMasterPersistPtr->createBackground( rFilter );
            //pMasterPersistPtr->createXShapes( rFilter );
            mpSlidePersistPtr->setMasterPersist( pMasterPersistPtr );
        }
        return this;
    }
    case PPT_TOKEN( notesMaster ):      // CT_NotesMaster
        return this;
    case PPT_TOKEN( cSld ):             // CT_CommonSlideData
        maSlideName = rAttribs.getString(XML_name, OUString());
        return this;

    case PPT_TOKEN( spTree ):           // CT_GroupShape
        {
            // TODO Convert this to FragmentHandler2
            return new PPTShapeGroupContext(
                *this, mpSlidePersistPtr, meShapeLocation, mpSlidePersistPtr->getShapes(),
                oox::drawingml::ShapePtr( new PPTShape( meShapeLocation, "com.sun.star.drawing.GroupShape" ) ) );
        }
        break;

    case PPT_TOKEN( controls ):
        return this;
    case PPT_TOKEN( control ):
        {
            ::oox::vml::ControlInfo aInfo;
            aInfo.setShapeId( rAttribs.getInteger( XML_spid, 0 ) );
            aInfo.maFragmentPath = getFragmentPathFromRelId( rAttribs.getString( R_TOKEN( id ), OUString() ) );
            aInfo.maName = rAttribs.getXString( XML_name, OUString() );
            mpSlidePersistPtr->getDrawing()->registerControl( aInfo );
        }
        return this;

    case PPT_TOKEN( timing ): // CT_SlideTiming
        return new SlideTimingContext( *this, mpSlidePersistPtr->getTimeNodeList() );
    case PPT_TOKEN( transition ): // CT_SlideTransition
        return new SlideTransitionContext( *this, rAttribs, maSlideProperties );
    case PPT_TOKEN( hf ):
        return new HeaderFooterContext( *this, rAttribs, mpSlidePersistPtr->getHeaderFooter() );

    // BackgroundGroup
    case PPT_TOKEN( bg ):
        return this;
    case PPT_TOKEN( bgPr ):             // CT_BackgroundProperties
        {
            FillPropertiesPtr pFillPropertiesPtr( new FillProperties );
            mpSlidePersistPtr->setBackgroundProperties( pFillPropertiesPtr );
            return new BackgroundPropertiesContext( *this, *pFillPropertiesPtr );
        }
        break;

    case PPT_TOKEN( bgRef ):            // a:CT_StyleMatrixReference
        {
            const FillProperties *pFillProperties = NULL;
            if( mpSlidePersistPtr->getTheme() )
                pFillProperties = mpSlidePersistPtr->getTheme()->getFillStyle( rAttribs.getInteger( XML_idx, -1 ) );
            FillPropertiesPtr pFillPropertiesPtr( pFillProperties ? new FillProperties( *pFillProperties ) : new FillProperties() );
            ContextHandlerRef ret = new ColorContext( *this, mpSlidePersistPtr->getBackgroundColor() );
            mpSlidePersistPtr->setBackgroundProperties( pFillPropertiesPtr );
            return ret;
        }
        break;

    case A_TOKEN( overrideClrMapping ):
    case PPT_TOKEN( clrMap ):           // CT_ColorMapping
        {
            oox::drawingml::ClrMapPtr pClrMapPtr( aElementToken == PPT_TOKEN( clrMap ) ? new oox::drawingml::ClrMap() : new oox::drawingml::ClrMap( *mpSlidePersistPtr->getClrMap() ) );
            ContextHandlerRef ret = new oox::drawingml::clrMapContext( *this, rAttribs.getFastAttributeList(), *pClrMapPtr );
            mpSlidePersistPtr->setClrMap( pClrMapPtr );
            return ret;
        }
        break;
    case PPT_TOKEN( clrMapOvr ):        // CT_ColorMappingOverride
    case PPT_TOKEN( sldLayoutIdLst ):   // CT_SlideLayoutIdList
        return this;
    case PPT_TOKEN( txStyles ):         // CT_SlideMasterTextStyles
        return new SlideMasterTextStylesContext( *this, mpSlidePersistPtr );
        break;
    case PPT_TOKEN( custDataLst ):      // CT_CustomerDataList
    case PPT_TOKEN( tagLst ):           // CT_TagList
        return this;
    }

    return this;
}

void SlideFragmentHandler::finalizeImport()
{
    try
    {
        Reference< XDrawPage > xSlide( mpSlidePersistPtr->getPage() );
        PropertySet aSlideProp( xSlide );
        aSlideProp.setProperties( maSlideProperties );
        if ( !maSlideName.isEmpty() )
        {
            Reference< XNamed > xNamed( xSlide, UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( maSlideName );
        }
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( (rtl::OString("oox::ppt::SlideFragmentHandler::EndElement(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
