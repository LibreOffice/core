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



#include "timetargetelementcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::oox::core;

using ::rtl::OUString;

namespace oox { namespace ppt {



    // CT_TLShapeTargetElement
    class ShapeTargetElementContext
        : public ContextHandler
    {
    public:
        ShapeTargetElementContext( ContextHandler& rParent, ShapeTargetElement & aValue )
            : ContextHandler( rParent )
                , bTargetSet(false)
                , maShapeTarget(aValue)
            {
            }
        virtual Reference< XFastContextHandler > SAL_CALL createFastChildContext( ::sal_Int32 aElementToken,
                                                                                                                                                            const Reference< XFastAttributeList >& xAttribs )
            throw ( SAXException, RuntimeException )
            {
                Reference< XFastContextHandler > xRet;

                switch( aElementToken )
                {
                case PPT_TOKEN( bg ):
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_bg;
                    break;
                case PPT_TOKEN( txEl ):
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_txEl;
                    break;
                case PPT_TOKEN( subSp ):
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_subSp;
                    maShapeTarget.msSubShapeId = xAttribs->getOptionalValue( XML_spid );
                    break;
                case PPT_TOKEN( graphicEl ):
                case PPT_TOKEN( oleChartEl ):
                    bTargetSet = true;
                    // TODO
                    break;
                case PPT_TOKEN( charRg ):
                case PPT_TOKEN( pRg ):
                    if( bTargetSet && maShapeTarget.mnType == XML_txEl )
                    {
                        maShapeTarget.mnRangeType = getBaseToken( aElementToken );
                        maShapeTarget.maRange = drawingml::GetIndexRange( xAttribs );
                    }
                    break;
                default:
                    break;
                }
                if( !xRet.is() )
                    xRet.set( this );
                return xRet;
            }

    private:
        bool bTargetSet;
        ShapeTargetElement & maShapeTarget;
    };



    TimeTargetElementContext::TimeTargetElementContext( ContextHandler& rParent, const AnimTargetElementPtr & pValue )
        : ContextHandler( rParent ),
            mpTarget( pValue )
    {
        OSL_ENSURE( mpTarget, "no valid target passed" );
    }


    TimeTargetElementContext::~TimeTargetElementContext( ) throw( )
    {
    }

    void SAL_CALL TimeTargetElementContext::endFastElement( sal_Int32 /*aElement*/ ) throw ( SAXException, RuntimeException)
    {
    }

    Reference< XFastContextHandler > SAL_CALL TimeTargetElementContext::createFastChildContext( ::sal_Int32 aElementToken, const Reference< XFastAttributeList >& xAttribs ) throw ( SAXException, RuntimeException )
    {
        Reference< XFastContextHandler > xRet;

        switch( aElementToken )
        {
        case PPT_TOKEN( inkTgt ):
        {
            mpTarget->mnType = XML_inkTgt;
            OUString aId = xAttribs->getOptionalValue( XML_spid );
            if( aId.getLength() )
            {
                mpTarget->msValue = aId;
            }
            break;
        }
        case PPT_TOKEN( sldTgt ):
            mpTarget->mnType = XML_sldTgt;
            break;
        case PPT_TOKEN( sndTgt ):
        {
            mpTarget->mnType = XML_sndTgt;
            drawingml::EmbeddedWAVAudioFile aAudio;
            drawingml::getEmbeddedWAVAudioFile( getRelations(), xAttribs, aAudio);

            OUString sSndName = ( aAudio.mbBuiltIn ? aAudio.msName : aAudio.msEmbed );
            mpTarget->msValue = sSndName;
            break;
        }
        case PPT_TOKEN( spTgt ):
        {
            mpTarget->mnType = XML_spTgt;
            OUString aId = xAttribs->getOptionalValue( XML_spid );
            mpTarget->msValue = aId;
            xRet.set( new ShapeTargetElementContext( *this, mpTarget->maShapeTarget ) );
            break;
        }
        default:
            OSL_TRACE( "OOX: unhandled tag %ld in TL_TimeTargetElement.", getBaseToken( aElementToken ) );
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }


} }
