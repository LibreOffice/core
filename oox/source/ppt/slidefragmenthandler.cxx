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

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/diagnose_ex.h>

#include <oox/helper/attributelist.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include "headerfootercontext.hxx"
#include <oox/ppt/backgroundproperties.hxx>
#include <oox/ppt/slidefragmenthandler.hxx>
#include <oox/ppt/slidetimingcontext.hxx>
#include <oox/ppt/slidetransitioncontext.hxx>
#include <oox/ppt/slidemastertextstylescontext.hxx>
#include <oox/ppt/pptshapegroupcontext.hxx>
#include <oox/ppt/pptshape.hxx>
#include <oox/vml/vmldrawing.hxx>
#include <oox/vml/vmldrawingfragment.hxx>
#include <drawingml/clrschemecontext.hxx>
#include <drawingml/textliststyle.hxx>
#include <oox/ppt/pptimport.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>

using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::container;

namespace oox { namespace ppt {

SlideFragmentHandler::SlideFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath, const SlidePersistPtr& pPersistPtr, const ShapeLocation eShapeLocation )
: FragmentHandler2( rFilter, rFragmentPath )
, mpSlidePersistPtr( pPersistPtr )
, meShapeLocation( eShapeLocation )
{
    OUString aVMLDrawingFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "vmlDrawing" );
    if( !aVMLDrawingFragmentPath.isEmpty() )
        getFilter().importFragment( new oox::vml::DrawingFragment(
            getFilter(), aVMLDrawingFragmentPath, *pPersistPtr->getDrawing() ) );
}

SlideFragmentHandler::~SlideFragmentHandler()
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

        aPropMap.setProperty( PROP_Visible, rAttribs.getBool( XML_show, true ));
        aSlideProp.setProperties( aPropMap );

        return this;
    }
    case PPT_TOKEN( notes ):            // CT_NotesSlide
    {
        // Import notesMaster
        PowerPointImport& rFilter = dynamic_cast< PowerPointImport& >( getFilter() );
        OUString aNotesFragmentPath = getFragmentPathFromFirstTypeFromOfficeDoc( "notesMaster" );

        std::vector< SlidePersistPtr >& rMasterPages( rFilter.getMasterPages() );
        bool bNotesFragmentPathFound = false;
        for (auto const& masterPage : rMasterPages)
        {
            if( masterPage->getPath() == aNotesFragmentPath )
            {
                if( !mpSlidePersistPtr->getMasterPersist() )
                    mpSlidePersistPtr->setMasterPersist(masterPage);
                bNotesFragmentPathFound=true;
                break;
            }
        }
        if( !bNotesFragmentPathFound && !mpSlidePersistPtr->getMasterPersist() )
        {
            TextListStylePtr pTextListStyle(new TextListStyle);
            SlidePersistPtr pMasterPersistPtr = std::make_shared<SlidePersist>( rFilter, true, true, mpSlidePersistPtr->getPage(),
                                ShapePtr( new PPTShape( Master, "com.sun.star.drawing.GroupShape" ) ), mpSlidePersistPtr->getNotesTextStyle() );
            pMasterPersistPtr->setPath( aNotesFragmentPath );
            rFilter.getMasterPages().push_back( pMasterPersistPtr );
            FragmentHandlerRef xMasterFragmentHandler( new SlideFragmentHandler( rFilter, aNotesFragmentPath, pMasterPersistPtr, Master ) );
            rFilter.importFragment( xMasterFragmentHandler );
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
            const FillProperties *pFillProperties = nullptr;
            if( mpSlidePersistPtr->getTheme() )
                pFillProperties = mpSlidePersistPtr->getTheme()->getFillStyle( rAttribs.getInteger( XML_idx, -1 ) );
            FillPropertiesPtr pFillPropertiesPtr( pFillProperties ? new FillProperties( *pFillProperties ) : new FillProperties );
            mpSlidePersistPtr->setBackgroundProperties( pFillPropertiesPtr );
            ContextHandlerRef ret = new ColorContext( *this, mpSlidePersistPtr->getBackgroundColor() );
            return ret;
        }
        break;

    case A_TOKEN( overrideClrMapping ):
    case PPT_TOKEN( clrMap ):           // CT_ColorMapping
        {
            oox::drawingml::ClrMapPtr pClrMapPtr( ( aElementToken == PPT_TOKEN( clrMap ) || !mpSlidePersistPtr.get() || !mpSlidePersistPtr->getClrMap().get() ) ? new oox::drawingml::ClrMap : new oox::drawingml::ClrMap( *mpSlidePersistPtr->getClrMap() ) );
            ContextHandlerRef ret = new oox::drawingml::clrMapContext( *this, rAttribs, *pClrMapPtr );
            mpSlidePersistPtr->setClrMap( pClrMapPtr );
            return ret;
        }
        break;
    case PPT_TOKEN( clrMapOvr ):        // CT_ColorMappingOverride
    case PPT_TOKEN( sldLayoutIdLst ):   // CT_SlideLayoutIdList
        return this;
    case PPT_TOKEN( txStyles ):         // CT_SlideMasterTextStyles
        return new SlideMasterTextStylesContext( *this, mpSlidePersistPtr );
    case PPT_TOKEN( custDataLst ):      // CT_CustomerDataList
    case PPT_TOKEN( tagLst ):           // CT_TagList
        return this;

    //for Comments
    case PPT_TOKEN( cmLst ):
        break;
    case PPT_TOKEN( cm ):
        if (!mpSlidePersistPtr->getCommentsList().cmLst.empty() && !getCharVector().empty())
        {
            // set comment text for earlier comment
            mpSlidePersistPtr->getCommentsList().cmLst.back().setText( getCharVector().back() );
        }
        // insert a new comment in vector commentsList
        mpSlidePersistPtr->getCommentsList().cmLst.emplace_back();
        mpSlidePersistPtr->getCommentsList().cmLst.back().setAuthorId(rAttribs.getString(XML_authorId, OUString()));
        mpSlidePersistPtr->getCommentsList().cmLst.back().setdt(rAttribs.getString(XML_dt, OUString()));
        mpSlidePersistPtr->getCommentsList().cmLst.back().setidx(rAttribs.getString(XML_idx, OUString()));
        break;

    case PPT_TOKEN( pos ):
        mpSlidePersistPtr->getCommentsList().cmLst.back().setPoint(
            rAttribs.getString(XML_x, OUString()),
            rAttribs.getString(XML_y, OUString()));
        break;

    case PPT_TOKEN( cmAuthor ):
        CommentAuthor _author;
        _author.clrIdx = rAttribs.getString(XML_clrIdx, OUString());
        _author.id = rAttribs.getString(XML_id, OUString());
        _author.initials = rAttribs.getString(XML_initials, OUString());
        _author.lastIdx = rAttribs.getString(XML_lastIdx, OUString());
        _author.name = rAttribs.getString(XML_name, OUString());
        mpSlidePersistPtr->getCommentAuthors().addAuthor(_author);
        break;
    }

    return this;
}
void SlideFragmentHandler::onCharacters( const OUString& rChars)
{
    maCharVector.push_back(rChars);
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
        SAL_WARN( "oox", "oox::ppt::SlideFragmentHandler::EndElement(), "
                    "exception caught: " << exceptionToString( cppu::getCaughtException() ) );
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
