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

#include "oox/ppt/soundactioncontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;


namespace oox { namespace ppt {


    SoundActionContext::SoundActionContext( FragmentHandler2& rParent, PropertyMap & aProperties ) throw()
    : FragmentHandler2( rParent )
    , maSlideProperties( aProperties )
    , mbHasStartSound( false )
    , mbLoopSound( false )
    , mbStopSound( false )
    {
    }


    SoundActionContext::~SoundActionContext() throw()
    {
    }


    void SoundActionContext::onEndElement()
    {
        if ( isCurrentElement( PPT_TOKEN( sndAc ) ) )
        {
            if( mbHasStartSound )
            {
                OUString url;
                // TODO this is very wrong
                if ( msSndName.getLength() != 0 )
                {
                    // try the builtIn version
                    url = msSndName;
                }
#if 0 // OOo does not support embedded data yet
                else if ( msEmbedded.getLength() != 0 )
                {
                    RelationsRef xRel = getHandler()->getRelations();
                    url =   xRel->getRelationById( msEmbedded )->msTarget;
                }
                else if ( msLink.getLength() != 0 )
                {
                    url = msLink;
                }
#endif
                if ( url.getLength() != 0 )
                {
                    maSlideProperties[ PROP_Sound ] <<= url;
                    maSlideProperties[ PROP_SoundOn ] <<= sal_True;
                }
            }
//          else if( mbStopSound )
//          {
//              maSlideProperties[ CREATE_OUSTRING( "" ) ] = Any( sal_True );
//          }
        }
    }


    ::oox::core::ContextHandlerRef SoundActionContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( snd ):
            if( mbHasStartSound )
            {
                drawingml::EmbeddedWAVAudioFile aAudio;
                drawingml::getEmbeddedWAVAudioFile( getRelations(), rAttribs.getFastAttributeList(), aAudio);

                msSndName = ( aAudio.mbBuiltIn ? aAudio.msName : aAudio.msEmbed );
            }
            return this;
        case PPT_TOKEN( endSnd ):
            // CT_Empty
            mbStopSound = true;
            return this;
        case PPT_TOKEN( stSnd ):
            mbHasStartSound = true;
            mbLoopSound = rAttribs.getBool( XML_loop, false );
            return this;
        default:
            break;
        }

        return this;
    }



} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
