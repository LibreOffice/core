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

#include "timetargetelementcontext.hxx"

#include "comphelper/anytostring.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include <osl/diagnose.h>

#include <com/sun/star/uno/Any.hxx>

#include "oox/helper/attributelist.hxx"
#include "oox/core/namespaces.hxx"
#include "oox/drawingml/embeddedwavaudiofile.hxx"
#include "tokens.hxx"

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
                case NMSP_PPT|XML_bg:
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_bg;
                    break;
                case NMSP_PPT|XML_txEl:
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_txEl;
                    break;
                case NMSP_PPT|XML_subSp:
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_subSp;
                    maShapeTarget.msSubShapeId = xAttribs->getOptionalValue( XML_spid );
                    break;
                case NMSP_PPT|XML_graphicEl:
                case NMSP_PPT|XML_oleChartEl:
                    bTargetSet = true;
                    // TODO
                    break;
                case NMSP_PPT|XML_charRg:
                case NMSP_PPT|XML_pRg:
                    if( bTargetSet && maShapeTarget.mnType == XML_txEl )
                    {
                        maShapeTarget.mnRangeType = getToken( aElementToken );
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
        case NMSP_PPT|XML_inkTgt:
        {
            mpTarget->mnType = XML_inkTgt;
            OUString aId = xAttribs->getOptionalValue( XML_spid );
            if( aId.getLength() )
            {
                mpTarget->msValue = aId;
            }
            break;
        }
        case NMSP_PPT|XML_sldTgt:
            mpTarget->mnType = XML_sldTgt;
            break;
        case NMSP_PPT|XML_sndTgt:
        {
            mpTarget->mnType = XML_sndTgt;
            drawingml::EmbeddedWAVAudioFile aAudio;
            drawingml::getEmbeddedWAVAudioFile( getRelations(), xAttribs, aAudio);

            OUString sSndName = ( aAudio.mbBuiltIn ? aAudio.msName : aAudio.msEmbed );
            mpTarget->msValue = sSndName;
            break;
        }
        case NMSP_PPT|XML_spTgt:
        {
            mpTarget->mnType = XML_spTgt;
            OUString aId = xAttribs->getOptionalValue( XML_spid );
            mpTarget->msValue = aId;
            xRet.set( new ShapeTargetElementContext( *this, mpTarget->maShapeTarget ) );
            break;
        }
        default:
            OSL_TRACE( "OOX: unhandled tag %ld in TL_TimeTargetElement.", getToken( aElementToken ) );
            break;
        }

        if( !xRet.is() )
            xRet.set( this );

        return xRet;
    }


} }
