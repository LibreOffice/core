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

HyperLinkContext::HyperLinkContext( ContextHandler2Helper& rParent,
        const AttributeList& rAttribs, PropertyMap& aProperties )
    : ContextHandler2( rParent )
    , maProperties(aProperties)
{
    OUString sURL, sHref;
    OUString aRelId = rAttribs.getString( R_TOKEN( id ) ).get();
    if ( !aRelId.isEmpty() )
    {
        OSL_TRACE("OOX: URI rId %s", OUStringToOString (aRelId, RTL_TEXTENCODING_UTF8).pData->buffer);
        sHref = getRelations().getExternalTargetFromRelId( aRelId );
        if( !sHref.isEmpty() )
        {
            OSL_TRACE("OOX: URI href %s", OUStringToOString (sHref, RTL_TEXTENCODING_UTF8).pData->buffer);
            sURL = getFilter().getAbsoluteUrl( sHref );
        }
        else
        {
            // not sure if we also need to set sHref to the internal target
            sURL = getRelations().getInternalTargetFromRelId( aRelId );
        }
    }
    OUString sTooltip = rAttribs.getString( R_TOKEN( tooltip ) ).get();
    if ( !sTooltip.isEmpty() )
        maProperties[ PROP_Representation ] <<= sTooltip;
    OUString sFrame = rAttribs.getString( R_TOKEN( tgtFrame ) ).get();
    if( !sFrame.isEmpty() )
        maProperties[ PROP_TargetFrame ] <<= sFrame;
    OUString aAction = rAttribs.getString( XML_action ).get();
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

        const OUString sPPAction( "ppaction://" );
        if ( aAction.matchIgnoreAsciiCase( sPPAction, 0 ) )
        {
            OUString aPPAct( aAction.copy( sPPAction.getLength() ) );
            sal_Int32 nIndex = aPPAct.indexOf( '?', 0 );
            OUString aPPAction( nIndex > 0 ? aPPAct.copy( 0, nIndex ) : aPPAct );

            const OUString sHlinkshowjump( "hlinkshowjump" );
            const OUString sHlinksldjump( "hlinksldjump" );
            if ( aPPAction.match( sHlinkshowjump ) )
            {
                const OUString sJump( "jump=" );
                if ( aPPAct.match( sJump, nIndex + 1 ) )
                {
                    OUString aDestination( aPPAct.copy( nIndex + 1 + sJump.getLength() ) );
                    sURL = sURL.concat( "#action?jump=" );
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
                        const OUString sSlide( "slide" );
                        const OUString sNotesSlide( "notesSlide" );
                        const OUString aSlideType( sHref.copy( 0, nIndex2 ) );
                        if ( aSlideType.match( sSlide ) )
                            sURL = OUString( "#Slide " ).concat( OUString::number( nPageNumber ) );
                        else if ( aSlideType.match( sNotesSlide ) )
                            sURL = OUString( "#Notes " ).concat( OUString::number( nPageNumber ) );
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

ContextHandlerRef HyperLinkContext::onCreateContext(
        ::sal_Int32 aElement, const AttributeList& rAttribs )
{
    switch( aElement )
    {
    case A_TOKEN( extLst ):
        return 0;
    case A_TOKEN( snd ):
        EmbeddedWAVAudioFile aAudio;
        getEmbeddedWAVAudioFile( getRelations(), rAttribs.getFastAttributeList(), aAudio );
        break;
    }

    return this;
}

} // namespace drawingml
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
