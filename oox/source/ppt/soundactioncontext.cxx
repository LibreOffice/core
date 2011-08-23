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
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"
#include "properties.hxx"
#include "tokens.hxx"

using rtl::OUString;
using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;


namespace oox { namespace ppt {


    SoundActionContext::SoundActionContext( ContextHandler& rParent, PropertyMap & aProperties ) throw()
    : ContextHandler( rParent )
    , maSlideProperties( aProperties )
    , mbHasStartSound( false )
    , mbLoopSound( false )
    , mbStopSound( false )
    {
    }


    SoundActionContext::~SoundActionContext() throw()
    {
    }


    void SoundActionContext::endFastElement( sal_Int32 aElement ) throw (SAXException, RuntimeException)
    {
        if ( aElement == ( NMSP_PPT|XML_sndAc ) )
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
                    url =	xRel->getRelationById( msEmbedded )->msTarget;
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
//			else if( mbStopSound )
//			{
//				maSlideProperties[ CREATE_OUSTRING( "" ) ] = Any( sal_True );
//			}
        }
    }


    Reference< XFastContextHandler > SoundActionContext::createFastChildContext( ::sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
    {
        Reference< XFastContextHandler > xRet;
        AttributeList attribs(xAttribs);

        switch( aElement )
        {
        case NMSP_PPT|XML_snd:
            if( mbHasStartSound )
            {
                drawingml::EmbeddedWAVAudioFile aAudio;
                drawingml::getEmbeddedWAVAudioFile( getRelations(), xAttribs, aAudio);

                msSndName = ( aAudio.mbBuiltIn ? aAudio.msName : aAudio.msEmbed );
            }
            break;
        case NMSP_PPT|XML_endSnd:
            // CT_Empty
            mbStopSound = true;
            break;
        case NMSP_PPT|XML_stSnd:
            mbHasStartSound = true;
            mbLoopSound = attribs.getBool( XML_loop, false );
        default:
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }



} }
