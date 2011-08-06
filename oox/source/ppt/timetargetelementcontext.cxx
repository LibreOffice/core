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
        : public FragmentHandler2
    {
    public:
        ShapeTargetElementContext( FragmentHandler2& rParent, ShapeTargetElement & aValue )
            : FragmentHandler2( rParent )
                , bTargetSet(false)
                , maShapeTarget(aValue)
            {
            }
        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
            {
                switch( aElementToken )
                {
                case PPT_TOKEN( bg ):
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_bg;
                    return this;
                case PPT_TOKEN( txEl ):
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_txEl;
                    return this;
                case PPT_TOKEN( subSp ):
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_subSp;
                    maShapeTarget.msSubShapeId = rAttribs.getString( XML_spid, OUString() );
                    return this;
                case PPT_TOKEN( graphicEl ):
                case PPT_TOKEN( oleChartEl ):
                    bTargetSet = true;
                    // TODO
                    return this;
                case PPT_TOKEN( charRg ):
                case PPT_TOKEN( pRg ):
                    if( bTargetSet && maShapeTarget.mnType == XML_txEl )
                    {
                        maShapeTarget.mnRangeType = getBaseToken( aElementToken );
                        maShapeTarget.maRange = drawingml::GetIndexRange( rAttribs.getFastAttributeList() );
                    }
                    return this;
                default:
                    break;
                }
                return this;
            }

    private:
        bool bTargetSet;
        ShapeTargetElement & maShapeTarget;
    };



    TimeTargetElementContext::TimeTargetElementContext( FragmentHandler2& rParent, const AnimTargetElementPtr & pValue )
        : FragmentHandler2( rParent ),
            mpTarget( pValue )
    {
        OSL_ENSURE( mpTarget, "no valid target passed" );
    }


    TimeTargetElementContext::~TimeTargetElementContext( ) throw( )
    {
    }

    ::oox::core::ContextHandlerRef TimeTargetElementContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( inkTgt ):
        {
            mpTarget->mnType = XML_inkTgt;
            OUString aId = rAttribs.getString( XML_spid, OUString() );
            if( aId.getLength() )
            {
                mpTarget->msValue = aId;
            }
            return this;
        }
        case PPT_TOKEN( sldTgt ):
            mpTarget->mnType = XML_sldTgt;
            return this;
        case PPT_TOKEN( sndTgt ):
        {
            mpTarget->mnType = XML_sndTgt;
            drawingml::EmbeddedWAVAudioFile aAudio;
            drawingml::getEmbeddedWAVAudioFile( getRelations(), rAttribs.getFastAttributeList(), aAudio);

            OUString sSndName = ( aAudio.mbBuiltIn ? aAudio.msName : aAudio.msEmbed );
            mpTarget->msValue = sSndName;
            break;
        }
        case PPT_TOKEN( spTgt ):
        {
            mpTarget->mnType = XML_spTgt;
            OUString aId = rAttribs.getString( XML_spid, OUString() );
            mpTarget->msValue = aId;
            return new ShapeTargetElementContext( *this, mpTarget->maShapeTarget );
        }
        default:
            OSL_TRACE( "OOX: unhandled tag %ld in TL_TimeTargetElement.", getBaseToken( aElementToken ) );
            break;
        }

        return this;
    }


} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
