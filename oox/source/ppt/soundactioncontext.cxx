/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: soundactioncontext.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:51:09 $
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

#include "oox/ppt/soundactioncontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"

#include "oox/helper/attributelist.hxx"
#include "oox/helper/propertymap.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"
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
                    url =   xRel->getRelationById( msEmbedded )->msTarget;
                }
                else if ( msLink.getLength() != 0 )
                {
                    url = msLink;
                }
#endif
                if ( url.getLength() != 0 )
                {
                    maSlideProperties[ CREATE_OUSTRING( "Sound" ) ] = Any( url );
                    maSlideProperties[ CREATE_OUSTRING( "SoundOn" ) ] = Any( sal_True );
                }
            }
//          else if( mbStopSound )
//          {
//              maSlideProperties[ CREATE_OUSTRING( "" ) ] = Any( sal_True );
//          }
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

                msSndName = ( aAudio.mbBuiltIn ? aAudio.msName : aAudio.msLink );
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
