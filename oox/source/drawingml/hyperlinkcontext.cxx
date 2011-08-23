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

#include "hyperlinkcontext.hxx"

#include <rtl/ustring.hxx>

#include <com/sun/star/xml/sax/XFastContextHandler.hpp>

#include "oox/helper/propertymap.hxx"
#include "oox/core/relations.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using ::rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;

namespace oox {
namespace drawingml {

HyperLinkContext::HyperLinkContext( ContextHandler& rParent,
        const Reference< XFastAttributeList >& xAttributes, PropertyMap& aProperties )
    : ContextHandler( rParent )
    , maProperties(aProperties)
{
    OUString sURL, sHref;
    OUString aRelId = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_id );
    if ( aRelId.getLength() )
    {
        OSL_TRACE("OOX: URI rId %s", ::rtl::OUStringToOString (aRelId, RTL_TEXTENCODING_UTF8).pData->buffer);
        sHref = getRelations().getExternalTargetFromRelId( aRelId );
        if( sHref.getLength() > 0 )
        {
            OSL_TRACE("OOX: URI href %s", ::rtl::OUStringToOString (sHref, RTL_TEXTENCODING_UTF8).pData->buffer);
            sURL = getFilter().getAbsoluteUrl( sHref );
        }
    }
    OUString sTooltip = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_tooltip );
    if ( sTooltip.getLength() )
        maProperties[ PROP_Representation ] <<= sTooltip;
    OUString sFrame = xAttributes->getOptionalValue( NMSP_RELATIONSHIPS|XML_tgtFrame );
    if( sFrame.getLength() )
        maProperties[ PROP_TargetFrame ] <<= sFrame;
    OUString aAction = xAttributes->getOptionalValue( XML_action );
    if ( aAction.getLength() )
    {
        // reserved values of the unrestricted string aAction:
        // ppaction://customshow?id=SHOW_ID				// custom presentation
        // ppaction://hlinkfile							// external file via r:id
        // ppaction://hlinkpres?slideindex=SLIDE_NUM	// external presentation via r:id
        // ppaction://hlinkshowjump?jump=endshow
        // ppaction://hlinkshowjump?jump=firstslide
        // ppaction://hlinkshowjump?jump=lastslide
        // ppaction://hlinkshowjump?jump=lastslideviewed
        // ppaction://hlinkshowjump?jump=nextslide
        // ppaction://hlinkshowjump?jump=previousslide
        // ppaction://hlinksldjump
        // ppaction://macro?name=MACRO_NAME
        // ppaction://program

        const OUString sPPAction( CREATE_OUSTRING( "ppaction://" ) );
        if ( aAction.matchIgnoreAsciiCase( sPPAction, 0 ) )
        {
            OUString aPPAct( aAction.copy( sPPAction.getLength() ) );
            sal_Int32 nIndex = aPPAct.indexOf( '?', 0 );
            OUString aPPAction( nIndex > 0 ? aPPAct.copy( 0, nIndex ) : aPPAct );

            const OUString sHlinkshowjump( CREATE_OUSTRING( "hlinkshowjump" ) );
            const OUString sHlinksldjump( CREATE_OUSTRING( "hlinksldjump" ) );
            if ( aPPAction.match( sHlinkshowjump ) )
            {
                const OUString sJump( CREATE_OUSTRING( "jump=" ) );
                if ( aPPAct.match( sJump, nIndex + 1 ) )
                {
                    OUString aDestination( aPPAct.copy( nIndex + 1 + sJump.getLength() ) );
                    sURL = sURL.concat( CREATE_OUSTRING( "#action?jump=" ) );
                    sURL = sURL.concat( aDestination );
                }
            }
            else if ( aPPAction.match( sHlinksldjump ) )
            {
                sURL = OUString();

                sal_Int32 nIndex2 = 0;
                while ( nIndex2 < sHref.getLength() )
                {
                    sal_Unicode nChar = sHref[ nIndex2 ];
                    if ( ( nChar >= '0' ) && ( nChar <= '9' ) )
                        break;
                    nIndex2++;
                }
                if ( nIndex2 && ( nIndex2 != sHref.getLength() ) )
                {
                    sal_Int32 nLength = 1;
                    while( ( nIndex2 + nLength ) < sHref.getLength() )
                    {
                        sal_Unicode nChar = sHref[ nIndex2 + nLength ];
                        if ( ( nChar < '0' ) || ( nChar > '9' ) )
                            break;
                        nLength++;
                    }
                    sal_Int32 nPageNumber = sHref.copy( nIndex2, nLength ).toInt32();
                    if ( nPageNumber )
                    {
                        const OUString sSlide( CREATE_OUSTRING( "slide" ) );
                        const OUString sNotesSlide( CREATE_OUSTRING( "notesSlide" ) );
                        const OUString aSlideType( sHref.copy( 0, nIndex2 ) );
                        if ( aSlideType.match( sSlide ) )
                            sURL = CREATE_OUSTRING( "#Slide " ).concat( rtl::OUString::valueOf( nPageNumber ) );
                        else if ( aSlideType.match( sNotesSlide ) )
                            sURL = CREATE_OUSTRING( "#Notes " ).concat( rtl::OUString::valueOf( nPageNumber ) );
//						else: todo for other types such as notesMaster or slideMaster as they can't be referenced easily			
                    }
                }
            }
        }
    }
    if ( sURL.getLength() )
        maProperties[ PROP_URL ] <<= sURL;

    // TODO unhandled
    // XML_invalidUrl
    // XML_history
    // XML_highlightClick
    // XML_endSnd
}

HyperLinkContext::~HyperLinkContext()
{
}

Reference< XFastContextHandler > HyperLinkContext::createFastChildContext(
        ::sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
{
    Reference< XFastContextHandler > xRet;
    switch( aElement )
    {
    case NMSP_DRAWINGML|XML_extLst:
        return xRet;
    case NMSP_DRAWINGML|XML_snd:
        EmbeddedWAVAudioFile aAudio;
        getEmbeddedWAVAudioFile( getRelations(), xAttribs, aAudio );
        break;
    }
    if ( !xRet.is() )
        xRet.set( this );
    return xRet;
}

} // namespace drawingml
} // namespace oox

