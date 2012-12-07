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

#include "hyperlinkcontext.hxx"

#include <com/sun/star/xml/sax/XFastContextHandler.hpp>

#include "oox/helper/propertymap.hxx"
#include "oox/core/relations.hxx"
#include "oox/core/xmlfilterbase.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"

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
    OUString aRelId = xAttributes->getOptionalValue( R_TOKEN( id ) );
    if ( !aRelId.isEmpty() )
    {
        OSL_TRACE("OOX: URI rId %s", OUStringToOString (aRelId, RTL_TEXTENCODING_UTF8).pData->buffer);
        sHref = getRelations().getExternalTargetFromRelId( aRelId );
        if( !sHref.isEmpty() )
        {
            OSL_TRACE("OOX: URI href %s", OUStringToOString (sHref, RTL_TEXTENCODING_UTF8).pData->buffer);
            sURL = getFilter().getAbsoluteUrl( sHref );
        }
    }
    OUString sTooltip = xAttributes->getOptionalValue( R_TOKEN( tooltip ) );
    if ( !sTooltip.isEmpty() )
        maProperties[ PROP_Representation ] <<= sTooltip;
    OUString sFrame = xAttributes->getOptionalValue( R_TOKEN( tgtFrame ) );
    if( !sFrame.isEmpty() )
        maProperties[ PROP_TargetFrame ] <<= sFrame;
    OUString aAction = xAttributes->getOptionalValue( XML_action );
    if ( !aAction.isEmpty() )
    {
        // reserved values of the unrestricted string aAction:
        // ppaction://customshow?id=SHOW_ID             // custom presentation
        // ppaction://hlinkfile                         // external file via r:id
        // ppaction://hlinkpres?slideindex=SLIDE_NUM    // external presentation via r:id
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
                            sURL = CREATE_OUSTRING( "#Slide " ).concat( OUString::valueOf( nPageNumber ) );
                        else if ( aSlideType.match( sNotesSlide ) )
                            sURL = CREATE_OUSTRING( "#Notes " ).concat( OUString::valueOf( nPageNumber ) );
//                      else: todo for other types such as notesMaster or slideMaster as they can't be referenced easily
                    }
                }
            }
        }
    }
    if ( !sURL.isEmpty() )
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
    case A_TOKEN( extLst ):
        return xRet;
    case A_TOKEN( snd ):
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
