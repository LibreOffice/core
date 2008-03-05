/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: presentationfragmenthandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:49:09 $
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

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/drawing/XMasterPageTarget.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/presentation/XPresentationPage.hpp>

#include "oox/drawingml/theme.hxx"
#include "oox/drawingml/drawingmltypes.hxx"
#include "oox/drawingml/themefragmenthandler.hxx"
#include "oox/drawingml/textliststylecontext.hxx"
#include "oox/ppt/pptshape.hxx"
#include "oox/ppt/presentationfragmenthandler.hxx"
#include "oox/ppt/slidefragmenthandler.hxx"
#include "oox/ppt/layoutfragmenthandler.hxx"
#include "oox/ppt/pptimport.hxx"
#include "oox/core/namespaces.hxx"
#include "tokens.hxx"

using rtl::OUString;
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
: FragmentHandler( rFilter, rFragmentPath )
, mpTextListStyle( new TextListStyle )
{
}

PresentationFragmentHandler::~PresentationFragmentHandler() throw()
{

}
void PresentationFragmentHandler::startDocument() throw (SAXException, RuntimeException)
{
}

void PresentationFragmentHandler::endDocument() throw (SAXException, RuntimeException)
{
    try
    {
        PowerPointImport& rFilter = dynamic_cast< PowerPointImport& >( getFilter() );

        Reference< frame::XModel > xModel( rFilter.getModel() );
        Reference< drawing::XDrawPage > xSlide;
        sal_uInt32 nSlide;

        // importing slide pages and its corresponding notes page
        Reference< drawing::XDrawPagesSupplier > xDPS( xModel, uno::UNO_QUERY_THROW );
        Reference< drawing::XDrawPages > xDrawPages( xDPS->getDrawPages(), uno::UNO_QUERY_THROW );

        for( nSlide = 0; nSlide < maSlidesVector.size(); nSlide++ )
        {
            if( nSlide == 0 )
                xDrawPages->getByIndex( 0 ) >>= xSlide;
            else
                xSlide = xDrawPages->insertNewByIndex( nSlide );

            OUString aSlideFragmentPath = getFragmentPathFromRelId( maSlidesVector[ nSlide ] );
            if( aSlideFragmentPath.getLength() > 0 )
            {
                SlidePersistPtr pMasterPersistPtr;
                SlidePersistPtr pSlidePersistPtr( new SlidePersist( sal_False, sal_False, xSlide,
                                    ShapePtr( new PPTShape( Slide, "com.sun.star.drawing.GroupShape" ) ), mpTextListStyle ) );

                FragmentHandlerRef xSlideFragmentHandler( new SlideFragmentHandler( rFilter, aSlideFragmentPath, pSlidePersistPtr, Slide ) );

                // importing the corresponding masterpage/layout
                OUString aLayoutFragmentPath = xSlideFragmentHandler->getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "slideLayout" ) );
                if ( aLayoutFragmentPath.getLength() > 0 )
                {
                    // importing layout
                    RelationsRef xLayoutRelations = rFilter.importRelations( aLayoutFragmentPath );
                    OUString aMasterFragmentPath = xLayoutRelations->getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "slideMaster" ) );
                    if( aMasterFragmentPath.getLength() )
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
                            aIter++;
                        }
                        if ( aIter == rMasterPages.end() )
                        {   // masterpersist not found, we have to load it
                            Reference< drawing::XDrawPage > xMasterPage;
                            Reference< drawing::XMasterPagesSupplier > xMPS( xModel, uno::UNO_QUERY_THROW );
                            Reference< drawing::XDrawPages > xMasterPages( xMPS->getMasterPages(), uno::UNO_QUERY_THROW );

                            if( !(rFilter.getMasterPages().size() ))
                                xMasterPages->getByIndex( 0 ) >>= xMasterPage;
                            else
                                xMasterPage = xMasterPages->insertNewByIndex( xMasterPages->getCount() );

                            pMasterPersistPtr = SlidePersistPtr( new SlidePersist( sal_True, sal_False, xMasterPage,
                                ShapePtr( new PPTShape( Master, "com.sun.star.drawing.GroupShape" ) ), mpTextListStyle ) );
                            pMasterPersistPtr->setLayoutPath( aLayoutFragmentPath );
                            rFilter.getMasterPages().push_back( pMasterPersistPtr );
                            rFilter.setActualSlidePersist( pMasterPersistPtr );
                            FragmentHandlerRef xMasterFragmentHandler( new SlideFragmentHandler( rFilter, aMasterFragmentPath, pMasterPersistPtr, Master ) );

                            // set the correct theme
                            OUString aThemeFragmentPath = xMasterFragmentHandler->getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "theme" ) );
                            if( aThemeFragmentPath.getLength() > 0 )
                            {
                                std::map< OUString, oox::drawingml::ThemePtr >& rThemes( rFilter.getThemes() );
                                std::map< OUString, oox::drawingml::ThemePtr >::iterator aIter2( rThemes.find( aThemeFragmentPath ) );
                                if( aIter2 == rThemes.end() )
                                {
                                    oox::drawingml::ThemePtr pThemePtr( new oox::drawingml::Theme() );
                                    pMasterPersistPtr->setTheme( pThemePtr );
                                    rFilter.importFragment( new ThemeFragmentHandler( rFilter, aThemeFragmentPath, *pThemePtr ) );
                                    rThemes[ aThemeFragmentPath ] = pThemePtr;
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
                pSlidePersistPtr->setMasterPersist( pMasterPersistPtr );
                pSlidePersistPtr->setTheme( pMasterPersistPtr->getTheme() );
                Reference< drawing::XMasterPageTarget > xMasterPageTarget( pSlidePersistPtr->getPage(), UNO_QUERY );
                if( xMasterPageTarget.is() )
                    xMasterPageTarget->setMasterPage( pMasterPersistPtr->getPage() );
                rFilter.getDrawPages().push_back( pSlidePersistPtr );
                rFilter.setActualSlidePersist( pSlidePersistPtr );
                importSlide( xSlideFragmentHandler, pSlidePersistPtr );
                pSlidePersistPtr->createBackground( rFilter );
                pSlidePersistPtr->createXShapes( rFilter );

                // now importing the notes page
                OUString aNotesFragmentPath = xSlideFragmentHandler->getFragmentPathFromType( CREATE_OFFICEDOC_RELATIONSTYPE( "notesSlide" ) );
                if( aNotesFragmentPath.getLength() > 0 )
                {
                    Reference< XPresentationPage > xPresentationPage( xSlide, UNO_QUERY );
                    if ( xPresentationPage.is() )
                    {
                        Reference< XDrawPage > xNotesPage( xPresentationPage->getNotesPage() );
                        if ( xNotesPage.is() )
                        {
                            SlidePersistPtr pNotesPersistPtr( new SlidePersist( sal_False, sal_True, xNotesPage,
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
        }
    }
    catch( uno::Exception& )
    {
        OSL_ENSURE( false,
            (rtl::OString("oox::ppt::PresentationFragmentHandler::EndDocument(), "
                    "exception caught: ") +
            rtl::OUStringToOString(
                comphelper::anyToString( cppu::getCaughtException() ),
                RTL_TEXTENCODING_UTF8 )).getStr() );

    }

    // todo error handling;
}

// CT_Presentation
Reference< XFastContextHandler > PresentationFragmentHandler::createFastChildContext( sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElementToken )
    {
    case NMSP_PPT|XML_presentation:
    case NMSP_PPT|XML_sldMasterIdLst:
    case NMSP_PPT|XML_notesMasterIdLst:
    case NMSP_PPT|XML_sldIdLst:
        break;
    case NMSP_PPT|XML_sldMasterId:
        maSlideMasterVector.push_back( xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_id ) );
        break;
    case NMSP_PPT|XML_sldId:
        maSlidesVector.push_back( xAttribs->getOptionalValue( NMSP_RELATIONSHIPS|XML_id ) );
        break;
    case NMSP_PPT|XML_notesMasterId:
        maNotesMasterVector.push_back( xAttribs->getOptionalValue(NMSP_RELATIONSHIPS|XML_id ) );
        break;
    case NMSP_PPT|XML_sldSz:
        maSlideSize = GetSize2D( xAttribs );
        break;
    case NMSP_PPT|XML_notesSz:
        maNotesSize = GetSize2D( xAttribs );
        break;
    case NMSP_PPT|XML_custShowLst:
        xRet.set( new CustomShowListContext( *this, maCustomShowList ) );
        break;
    case NMSP_PPT|XML_defaultTextStyle:
        xRet.set( new TextListStyleContext( *this, *mpTextListStyle ) );
        break;
    }
    if ( !xRet.is() )
        xRet = getFastContextHandler();
    return xRet;
}

bool PresentationFragmentHandler::importSlide( const FragmentHandlerRef& rxSlideFragmentHandler,
        const SlidePersistPtr pSlidePersistPtr )
{
    Reference< drawing::XDrawPage > xSlide( pSlidePersistPtr->getPage() );
    SlidePersistPtr pMasterPersistPtr( pSlidePersistPtr->getMasterPersist() );
    if ( pMasterPersistPtr.get() )
    {
        const OUString sLayout = CREATE_OUSTRING( "Layout" );
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
        static const OUString sWidth = CREATE_OUSTRING( "Width" );
        static const OUString sHeight = CREATE_OUSTRING( "Height" );
        awt::Size& rPageSize( pSlidePersistPtr->isNotesPage() ? maNotesSize : maSlideSize );
        xPropertySet->setPropertyValue( sWidth, Any( rPageSize.Width ) );
        xPropertySet->setPropertyValue( sHeight, Any( rPageSize.Height ) );
    }
    pSlidePersistPtr->setPath( rxSlideFragmentHandler->getFragmentPath() );
    return getFilter().importFragment( rxSlideFragmentHandler );
}

} }

