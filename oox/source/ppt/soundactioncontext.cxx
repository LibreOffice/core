/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        if ( aElement == PPT_TOKEN( sndAc ) )
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


    Reference< XFastContextHandler > SoundActionContext::createFastChildContext( ::sal_Int32 aElement, const Reference< XFastAttributeList >& xAttribs ) throw (SAXException, RuntimeException)
    {
        Reference< XFastContextHandler > xRet;
        AttributeList attribs(xAttribs);

        switch( aElement )
        {
        case PPT_TOKEN( snd ):
            if( mbHasStartSound )
            {
                drawingml::EmbeddedWAVAudioFile aAudio;
                drawingml::getEmbeddedWAVAudioFile( getRelations(), xAttribs, aAudio);

                msSndName = ( aAudio.mbBuiltIn ? aAudio.msName : aAudio.msEmbed );
            }
            break;
        case PPT_TOKEN( endSnd ):
            // CT_Empty
            mbStopSound = true;
            break;
        case PPT_TOKEN( stSnd ):
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
