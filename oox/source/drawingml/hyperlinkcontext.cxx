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

#include <oox/helper/attributelist.hxx>
#include <oox/helper/propertymap.hxx>
#include <oox/core/relations.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <o3tl/string_view.hxx>
#include <ooxresid.hxx>
#include <strings.hrc>

using namespace ::oox::core;

namespace oox::drawingml {

HyperLinkContext::HyperLinkContext( ContextHandler2Helper const & rParent,
        const AttributeList& rAttribs, PropertyMap& aProperties )
    : ContextHandler2( rParent )
    , maProperties(aProperties)
{
    OUString sURL, sHref;
    OUString aRelId = rAttribs.getStringDefaulted( R_TOKEN( id ) );
    if ( !aRelId.isEmpty() )
    {
        sHref = getRelations().getExternalTargetFromRelId( aRelId );
        if( !sHref.isEmpty() )
        {
            sURL = getFilter().getAbsoluteUrl( sHref );
        }
        else
        {
            // not sure if we also need to set sHref to the internal target
            sURL = getRelations().getInternalTargetFromRelId( aRelId );
        }
    }
    OUString sTooltip = rAttribs.getStringDefaulted( R_TOKEN( tooltip ) );
    if ( !sTooltip.isEmpty() )
        maProperties.setProperty(PROP_Representation, sTooltip);
    OUString sFrame = rAttribs.getStringDefaulted( R_TOKEN( tgtFrame ) );
    if( !sFrame.isEmpty() )
        maProperties.setProperty(PROP_TargetFrame, sFrame);
    OUString aAction = rAttribs.getStringDefaulted( XML_action );
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

        static constexpr OUString sPPAction( u"ppaction://"_ustr );
        if ( aAction.matchIgnoreAsciiCase( sPPAction ) )
        {
            OUString aPPAct( aAction.copy( sPPAction.getLength() ) );
            sal_Int32 nIndex = aPPAct.indexOf( '?' );
            OUString aPPAction( nIndex > 0 ? aPPAct.copy( 0, nIndex ) : aPPAct );

            if ( aPPAction.match( "hlinkshowjump" ) )
            {
                static constexpr OUString sJump( u"jump="_ustr );
                if ( aPPAct.match( sJump, nIndex + 1 ) )
                {
                    std::u16string_view aDestination( aPPAct.subView( nIndex + 1 + sJump.getLength() ) );
                    sURL += OUString::Concat("#action?jump=") + aDestination;
                }
            }
            else if ( aPPAction.match( "hlinksldjump" ) )
            {
                sHref = sURL;

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
                    sal_Int32 nPageNumber = o3tl::toInt32(sHref.subView( nIndex2, nLength ));
                    if ( nPageNumber )
                    {
                        const OUString aSlideType( sHref.copy( 0, nIndex2 ) );
                        if ( aSlideType.match( "slide" ) )
                            sURL = "#" + URLResId(STR_SLIDE_NAME) + " " + OUString::number( nPageNumber );
                        else if ( aSlideType.match( "notesSlide" ) )
                            sURL = "#Notes " + OUString::number( nPageNumber );
//                      else: todo for other types such as notesMaster or slideMaster as they can't be referenced easily
                    }
                }
            }
        }
    }
    if ( !sURL.isEmpty() )
        maProperties.setProperty(PROP_URL, sURL);

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
        ::sal_Int32 aElement, const AttributeList&  )
{
    switch( aElement )
    {
    case A_TOKEN( extLst ):
        maProperties.setProperty(PROP_CharColor, XML_fillcolor);
        break;
    case A_TOKEN( snd ):
        // TODO use getEmbeddedWAVAudioFile() here
        break;
    }

    return this;
}

} // namespace oox::drawingml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
