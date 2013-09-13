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
#include <tools/multisel.hxx>

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/sax/XFastSAXSerializable.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/ppt/pptshape.hxx"
#include "oox/ppt/presentationfragmenthandler.hxx"
#include "oox/ppt/slidefragmenthandler.hxx"
#include "oox/ppt/layoutfragmenthandler.hxx"
#include "oox/ppt/pptimport.hxx"

#include <com/sun/star/office/XAnnotation.hpp>
#include <com/sun/star/office/XAnnotationAccess.hpp>

using namespace ::com::sun::star;
using namespace ::oox::core;
using namespace ::oox::drawingml;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::presentation;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

PresentationFragmentHandler::PresentationFragmentHandler( XmlFilterBase& rFilter, const OUString& rFragmentPath ) throw()
: FragmentHandler2( rFilter, rFragmentPath )
, mpTextListStyle( new TextListStyle )
, mbCommentAuthorsRead(false)
{
    TextParagraphPropertiesVector& rParagraphDefaulsVector( mpTextListStyle->getListStyle() );
    TextParagraphPropertiesVector::iterator aParagraphDefaultIter( rParagraphDefaulsVector.begin() );
    while( aParagraphDefaultIter != rParagraphDefaulsVector.end() )
    {
        // ppt is having zero bottom margin per default, whereas OOo is 0,5cm,
        // so this attribute needs to be set always
        (*aParagraphDefaultIter++)->getParaBottomMargin() = TextSpacing( 0 );
    }
}

PresentationFragmentHandler::~PresentationFragmentHandler() throw()
{
}

void ResolveTextFields( XmlFilterBase& rFilter )
{
    const oox::core::TextFieldStack& rTextFields = rFilter.getTextFieldStack();
    if ( rTextFields.size() )
    {
        Reference< frame::XModel > xModel( rFilter.getModel() );
        oox::core::TextFieldStack::const_iterator aIter( rTextFields.begin() );
        while( aIter != rTextFields.end() )
        {
            const OUString sURL = "URL";
            Reference< drawing::XDrawPagesSupplier > xDPS( xModel, uno::UNO_QUERY_THROW );
            Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY_THROW );

            const oox::core::TextField& rTextField( *aIter++ );
            Reference< XPropertySet > xPropSet( rTextField.xTextField, UNO_QUERY );
            Reference< XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
            if ( xPropSetInfo->hasPropertyByName( sURL ) )
            {
                OUString aURL;
                if ( xPropSet->getPropertyValue( sURL ) >>= aURL )
                {
                    const OUString sSlide = "#Slide ";
                    const OUString sNotes = "#Notes ";
                    sal_Bool bNotes = sal_False;
                    sal_Int32 nPageNumber = 0;
                    if ( aURL.match( sSlide ) )
                        nPageNumber = aURL.copy( sSlide.getLength() ).toInt32();
                    else if ( aURL.match( sNotes ) )
                    {
                        nPageNumber = aURL.copy( sNotes.getLength() ).toInt32();
                        bNotes = sal_True;
                    }
                    if ( nPageNumber )
                    {
                        try
                        {
                            Reference< XDrawPage > xDrawPage;
                            xDrawPages->getByIndex( nPageNumber - 1 ) >>= xDrawPage;
                            if ( bNotes )
                            {
                                Reference< ::com::sun::star::presentation::XPresentationPage > xPresentationPage( xDrawPage, UNO_QUERY_THROW );
                                xDrawPage = xPresentationPage->getNotesPage();
                            }
                            Reference< container::XNamed > xNamed( xDrawPage, UNO_QUERY_THROW );
                            aURL = "#" + xNamed->getName();
                            xPropSet->setPropertyValue( sURL, Any( aURL ) );
                            Reference< text::XTextContent > xContent( rTextField.xTextField, UNO_QUERY);
                            Reference< text::XTextRange > xTextRange( rTextField.xTextCursor, UNO_QUERY );
                            rTextField.xText->insertTextContent( xTextRange, xContent, sal_True );
                        }
                        catch( uno::Exception& )
                        {
                        }
                    }
                }
            }
        }
    }
}

void PresentationFragmentHandler::importSlide(sal_uInt32 nSlide, sal_Bool bFirstPage, sal_Bool bImportNotesPage)
{
    PowerPointImport& rFilter = dynamic_cast< PowerPointImport& >( getFilter() );

    Reference< frame::XModel > xModel( rFilter.getModel() );
    Reference< drawing::XDrawPage > xSlide;

    // importing slide pages and its corresponding notes page
    Reference< drawing::XDrawPagesSupplier > xDPS( xModel, uno::UNO_QUERY_THROW );
    Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY_THROW );

    try {

        if( bFirstPage )
            xDrawPages->getByIndex( 0 ) >>= xSlide;
        else
            xSlide = xDrawPages->insertNewByIndex( xDrawPages->getCount() );

        OUString aSlideFragmentPath = getFragmentPathFromRelId( maSlidesVector[ nSlide ] );
        if( !aSlideFragmentPath.isEmpty() )
        {
            SlidePersistPtr pMasterPersistPtr;
            SlidePersistPtr pSlidePersistPtr( new SlidePersist( rFilter, sal_False, sal_False, xSlide,
                                ShapePtr( new PPTShape( Slide, "com.sun.star.drawing.GroupShape" ) ), mpTextListStyle ) );

            FragmentHandlerRef xSlideFragmentHandler( new SlideFragmentHandler( rFilter, aSlideFragmentPath, pSlidePersistPtr, Slide ) );

            // importing the corresponding masterpage/layout
            OUString aLayoutFragmentPath = xSlideFragmentHandler->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "slideLayout" ) );
            OUString aCommentFragmentPath = xSlideFragmentHandler->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "comments" ) );
            if ( !aLayoutFragmentPath.isEmpty() )
            {
                // importing layout
                RelationsRef xLayoutRelations = rFilter.importRelations( aLayoutFragmentPath );
                OUString aMasterFragmentPath = xLayoutRelations->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "slideMaster" ) );
                if( !aMasterFragmentPath.isEmpty() )
                {
                    // check if the corresponding masterpage+layout has already been imported
                    std::vector< SlidePersistPtr >& rMasterPages( rFilter.getMasterPages() );
                    std::vector< SlidePersistPtr >::iterator aIter( rMasterPages.begin() );
                    while( aIter != rMasterPages.end() )
                    {
                        if ( ( (*aIter)->getPath() == aMasterFragmentPath ) && ( (*aIter)->getLayoutPath() == aLayoutFragmentPath ) )
                        {
                            pMasterPersistPtr = *aIter;
                            break;
                        }
                        ++aIter;
                    }

                    if ( !pMasterPersistPtr.get() )
                    {   // masterpersist not found, we have to load it
                        Reference< drawing::XDrawPage > xMasterPage;
                        Reference< drawing::XMasterPagesSupplier > xMPS( xModel, uno::UNO_QUERY_THROW );
                        Reference< drawing::XDrawPages > xMasterPages( xMPS->getMasterPages(), uno::UNO_QUERY_THROW );

                        if( !(rFilter.getMasterPages().size() ))
                            xMasterPages->getByIndex( 0 ) >>= xMasterPage;
                        else
                            xMasterPage = xMasterPages->insertNewByIndex( xMasterPages->getCount() );

                        pMasterPersistPtr = SlidePersistPtr( new SlidePersist( rFilter, sal_True, sal_False, xMasterPage,
                            ShapePtr( new PPTShape( Master, "com.sun.star.drawing.GroupShape" ) ), mpTextListStyle ) );
                        pMasterPersistPtr->setLayoutPath( aLayoutFragmentPath );
                        rFilter.getMasterPages().push_back( pMasterPersistPtr );
                        rFilter.setActualSlidePersist( pMasterPersistPtr );
                        FragmentHandlerRef xMasterFragmentHandler( new SlideFragmentHandler( rFilter, aMasterFragmentPath, pMasterPersistPtr, Master ) );

                        // set the correct theme
                        OUString aThemeFragmentPath = xMasterFragmentHandler->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "theme" ) );
                        if( !aThemeFragmentPath.isEmpty() )
                        {
                            std::map< OUString, oox::drawingml::ThemePtr >& rThemes( rFilter.getThemes() );
                            std::map< OUString, oox::drawingml::ThemePtr >::iterator aIter2( rThemes.find( aThemeFragmentPath ) );
                            if( aIter2 == rThemes.end() )
                            {
                                oox::drawingml::ThemePtr pThemePtr( new oox::drawingml::Theme() );
                                pMasterPersistPtr->setTheme( pThemePtr );
                                Reference<xml::dom::XDocument> xDoc=
                                    rFilter.importFragment(aThemeFragmentPath);

                                rFilter.importFragment(
                                    new ThemeFragmentHandler(
                                        rFilter, aThemeFragmentPath, *pThemePtr ),
                                    Reference<xml::sax::XFastSAXSerializable>(
                                        xDoc,
                                        UNO_QUERY_THROW));
                                rThemes[ aThemeFragmentPath ] = pThemePtr;
                                pThemePtr->setFragment(xDoc);
                            }
                            else
                            {
                                pMasterPersistPtr->setTheme( (*aIter2).second );
                            }
                        }
                        importSlide( xMasterFragmentHandler, pMasterPersistPtr );
                        rFilter.importFragment( new LayoutFragmentHandler( rFilter, aLayoutFragmentPath, pMasterPersistPtr ) );
                        pMasterPersistPtr->createBackground( rFilter );
                        pMasterPersistPtr->createXShapes( rFilter );
                    }
                }
            }

            // importing slide page
            if (pMasterPersistPtr.get()) {
                pSlidePersistPtr->setMasterPersist( pMasterPersistPtr );
                pSlidePersistPtr->setTheme( pMasterPersistPtr->getTheme() );
                Reference< drawing::XMasterPageTarget > xMasterPageTarget( pSlidePersistPtr->getPage(), UNO_QUERY );
                if( xMasterPageTarget.is() )
                    xMasterPageTarget->setMasterPage( pMasterPersistPtr->getPage() );
            }
            rFilter.getDrawPages().push_back( pSlidePersistPtr );
            rFilter.setActualSlidePersist( pSlidePersistPtr );
            importSlide( xSlideFragmentHandler, pSlidePersistPtr );
            pSlidePersistPtr->createBackground( rFilter );
            pSlidePersistPtr->createXShapes( rFilter );

            if(bImportNotesPage) {

                // now importing the notes page
                OUString aNotesFragmentPath = xSlideFragmentHandler->getFragmentPathFromFirstType( CREATE_OFFICEDOC_RELATION_TYPE( "notesSlide" ) );
                if( !aNotesFragmentPath.isEmpty() )
                {
                    Reference< XPresentationPage > xPresentationPage( xSlide, UNO_QUERY );
                    if ( xPresentationPage.is() )
                    {
                        Reference< XDrawPage > xNotesPage( xPresentationPage->getNotesPage() );
                        if ( xNotesPage.is() )
                        {
                            SlidePersistPtr pNotesPersistPtr( new SlidePersist( rFilter, sal_False, sal_True, xNotesPage,
                                ShapePtr( new PPTShape( Slide, "com.sun.star.drawing.GroupShape" ) ), mpTextListStyle ) );
                            FragmentHandlerRef xNotesFragmentHandler( new SlideFragmentHandler( getFilter(), aNotesFragmentPath, pNotesPersistPtr, Slide ) );
                            rFilter.getNotesPages().push_back( pNotesPersistPtr );
                            rFilter.setActualSlidePersist( pNotesPersistPtr );
                            importSlide( xNotesFragmentHandler, pNotesPersistPtr );
                            pNotesPersistPtr->createBackground( rFilter );
                            pNotesPersistPtr->createXShapes( rFilter );
                        }
                    }
                }
            }

            if( !mbCommentAuthorsRead && !aCommentFragmentPath.isEmpty() )
            {
                // Comments are present and commentAuthors.xml has still not been read
                mbCommentAuthorsRead = true;
                OUString aCommentAuthorsFragmentPath = "ppt/commentAuthors.xml";
                Reference< XPresentationPage > xPresentationPage( xSlide, UNO_QUERY );
                Reference< XDrawPage > xCommentAuthorsPage( xPresentationPage->getNotesPage() );
                SlidePersistPtr pCommentAuthorsPersistPtr(
                    new SlidePersist( rFilter, sal_False, sal_True, xCommentAuthorsPage,
                                      ShapePtr(
                                          new PPTShape(
                                              Slide, "com.sun.star.drawing.GroupShape" ) ),
                                      mpTextListStyle ) );
                FragmentHandlerRef xCommentAuthorsFragmentHandler(
                    new SlideFragmentHandler( getFilter(),
                                              aCommentAuthorsFragmentPath,
                                              pCommentAuthorsPersistPtr,
                                              Slide ) );

                importSlide( xCommentAuthorsFragmentHandler, pCommentAuthorsPersistPtr );
                maAuthorList.setValues( pCommentAuthorsPersistPtr->getCommentAuthors() );
            }
            if( !aCommentFragmentPath.isEmpty() )
            {
                Reference< XPresentationPage > xPresentationPage( xSlide, UNO_QUERY );
                Reference< XDrawPage > xCommentsPage( xPresentationPage->getNotesPage() );
                SlidePersistPtr pCommentsPersistPtr(
                    new SlidePersist(
                        rFilter, sal_False, sal_True, xCommentsPage,
                        ShapePtr(
                            new PPTShape(
                                Slide, "com.sun.star.drawing.GroupShape" ) ),
                        mpTextListStyle ) );

                FragmentHandlerRef xCommentsFragmentHandler(
                    new SlideFragmentHandler(
                        getFilter(),
                        aCommentFragmentPath,
                        pCommentsPersistPtr,
                        Slide ) );
                pCommentsPersistPtr->getCommentsList().cmLst.clear();
                importSlide( xCommentsFragmentHandler, pCommentsPersistPtr );

                //set comment chars for last comment on slide
                SlideFragmentHandler* comment_handler =
                    dynamic_cast<SlideFragmentHandler*>(xCommentsFragmentHandler.get());
                // some comments have no text -> set empty string as text to avoid
                // crash (back() on empty vector is undefined) and losing other
                // comment data that might be there (author, position, timestamp etc.)
                pCommentsPersistPtr->getCommentsList().cmLst.back().setText(
                        comment_handler->getCharVector().empty() ? "" :
                        comment_handler->getCharVector().back() );
                pCommentsPersistPtr->getCommentAuthors().setValues(maAuthorList);

                //insert all comments from commentsList
                for(int i=0; i<pCommentsPersistPtr->getCommentsList().getSize(); i++)
                {
                    try {
                        Comment aComment = pCommentsPersistPtr->getCommentsList().getCommentAtIndex(i);
                        uno::Reference< office::XAnnotationAccess > xAnnotationAccess( xSlide, UNO_QUERY_THROW );
                        uno::Reference< office::XAnnotation > xAnnotation( xAnnotationAccess->createAndInsertAnnotation() );
                        int nPosX = aComment.getIntX();
                        int nPosY = aComment.getIntY();
                        xAnnotation->setPosition(
                            geometry::RealPoint2D(
                                ::oox::drawingml::convertEmuToHmm( nPosX ) * 15.87,
                                ::oox::drawingml::convertEmuToHmm( nPosY ) * 15.87 ) );
                        xAnnotation->setAuthor( aComment.getAuthor(maAuthorList) );
                        xAnnotation->setDateTime( aComment.getDateTime() );
                        uno::Reference< text::XText > xText( xAnnotation->getTextRange() );
                        xText->setString( aComment.get_text());
                    } catch( css::lang::IllegalArgumentException& ) {}
                }
            }
        }
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( OString("oox::ppt::PresentationFragmentHandler::EndDocument(), "
                    "exception caught: " +
            OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );

    }
}

void PresentationFragmentHandler::finalizeImport()
{
    PowerPointImport& rFilter = dynamic_cast< PowerPointImport& >( getFilter() );

    sal_Int32 nPageCount = maSlidesVector.size();

    // we will take the FilterData property "PageRange" if available, otherwise full range is used
    comphelper::SequenceAsHashMap& rFilterData = rFilter.getFilterData();

    // writing back the original PageCount of this document, it can be accessed from the XModel
    // via getArgs after the import.
    rFilterData["OriginalPageCount"] = makeAny(nPageCount);
    sal_Bool bImportNotesPages = rFilterData.getUnpackedValueOrDefault("ImportNotesPages", sal_True);
    OUString aPageRange = rFilterData.getUnpackedValueOrDefault("PageRange", OUString());

    if( !aPageRange.getLength() )
    {
        aPageRange = OUStringBuffer()
            .append( static_cast< sal_Int32 >( 1 ) )
            .append( static_cast< sal_Unicode >( '-' ) )
            .append( nPageCount ).makeStringAndClear();
    }

    StringRangeEnumerator aRangeEnumerator( aPageRange, 0, nPageCount - 1 );
    StringRangeEnumerator::Iterator aIter = aRangeEnumerator.begin();
    StringRangeEnumerator::Iterator aEnd  = aRangeEnumerator.end();
    if(aIter!=aEnd) {

        // todo: localized progress bar text
        const Reference< task::XStatusIndicator >& rxStatusIndicator( getFilter().getStatusIndicator() );
        if ( rxStatusIndicator.is() )
            rxStatusIndicator->start( OUString(), 10000 );

        try
        {
            int nPagesImported = 0;
            while (aIter!=aEnd)
            {
                if ( rxStatusIndicator.is() )
                    rxStatusIndicator->setValue((nPagesImported * 10000) / aRangeEnumerator.size());

                importSlide(*aIter, !nPagesImported, bImportNotesPages);
                nPagesImported++;
                ++aIter;
            }
            ResolveTextFields( rFilter );
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( OString("oox::ppt::PresentationFragmentHandler::finalizeImport(), "
                        "exception caught: " +
                OUStringToOString(
                    comphelper::anyToString( cppu::getCaughtException() ),
                    RTL_TEXTENCODING_UTF8 )).getStr() );
        }
        // todo error handling;
        if ( rxStatusIndicator.is() )
            rxStatusIndicator->end();
    }
}

// CT_Presentation
::oox::core::ContextHandlerRef PresentationFragmentHandler::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
{
    switch( aElementToken )
    {
    case PPT_TOKEN( presentation ):
    case PPT_TOKEN( sldMasterIdLst ):
    case PPT_TOKEN( notesMasterIdLst ):
    case PPT_TOKEN( sldIdLst ):
        return this;
    case PPT_TOKEN( sldMasterId ):
        maSlideMasterVector.push_back( rAttribs.getString( R_TOKEN( id ), OUString() ) );
        return this;
    case PPT_TOKEN( sldId ):
        maSlidesVector.push_back( rAttribs.getString( R_TOKEN( id ), OUString() ) );
        return this;
    case PPT_TOKEN( notesMasterId ):
        maNotesMasterVector.push_back( rAttribs.getString( R_TOKEN( id ), OUString() ) );
        return this;
    case PPT_TOKEN( sldSz ):
        maSlideSize = GetSize2D( rAttribs.getFastAttributeList() );
        return this;
    case PPT_TOKEN( notesSz ):
        maNotesSize = GetSize2D( rAttribs.getFastAttributeList() );
        return this;
    case PPT_TOKEN( custShowLst ):
        return new CustomShowListContext( *this, maCustomShowList );
    case PPT_TOKEN( defaultTextStyle ):
        return new TextListStyleContext( *this, *mpTextListStyle );
    }
    return this;
}

bool PresentationFragmentHandler::importSlide( const FragmentHandlerRef& rxSlideFragmentHandler,
        const SlidePersistPtr pSlidePersistPtr )
{
    Reference< drawing::XDrawPage > xSlide( pSlidePersistPtr->getPage() );
    SlidePersistPtr pMasterPersistPtr( pSlidePersistPtr->getMasterPersist() );
    if ( pMasterPersistPtr.get() )
    {
        const OUString sLayout = "Layout";
        uno::Reference< beans::XPropertySet > xSet( xSlide, uno::UNO_QUERY_THROW );
        xSet->setPropertyValue( sLayout, Any( pMasterPersistPtr->getLayoutFromValueToken() ) );
    }
    while( xSlide->getCount() )
    {
        Reference< drawing::XShape > xShape;
        xSlide->getByIndex(0) >>= xShape;
        xSlide->remove( xShape );
    }

    Reference< XPropertySet > xPropertySet( xSlide, UNO_QUERY );
    if ( xPropertySet.is() )
    {
        awt::Size& rPageSize( pSlidePersistPtr->isNotesPage() ? maNotesSize : maSlideSize );
        xPropertySet->setPropertyValue( "Width", Any( rPageSize.Width ) );
        xPropertySet->setPropertyValue( "Height", Any( rPageSize.Height ) );

        oox::ppt::HeaderFooter aHeaderFooter( pSlidePersistPtr->getHeaderFooter() );
        if ( !pSlidePersistPtr->isMasterPage() )
            aHeaderFooter.mbSlideNumber = aHeaderFooter.mbHeader = aHeaderFooter.mbFooter = aHeaderFooter.mbDateTime = sal_False;
        try
        {
            if ( pSlidePersistPtr->isNotesPage() )
                xPropertySet->setPropertyValue( "IsHeaderVisible", Any( aHeaderFooter.mbHeader ) );
            xPropertySet->setPropertyValue( "IsFooterVisible", Any( aHeaderFooter.mbFooter ) );
            xPropertySet->setPropertyValue( "IsDateTimeVisible", Any( aHeaderFooter.mbDateTime ) );
            xPropertySet->setPropertyValue( "IsPageNumberVisible", Any( aHeaderFooter.mbSlideNumber ) );
        }
        catch( uno::Exception& )
        {
        }
    }
    pSlidePersistPtr->setPath( rxSlideFragmentHandler->getFragmentPath() );
    return getFilter().importFragment( rxSlideFragmentHandler );
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
