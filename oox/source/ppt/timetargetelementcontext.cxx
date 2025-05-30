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

#include <config_features.h>

#include "timetargetelementcontext.hxx"

#include <osl/diagnose.h>
#include <sal/log.hxx>

#include <oox/helper/attributelist.hxx>
#include <drawingml/embeddedwavaudiofile.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/tokens.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <avmedia/mediaitem.hxx>
#include <sax/fastattribs.hxx>
#include <utility>

using namespace ::com::sun::star::uno;
using namespace ::oox::core;

namespace oox::ppt {

    namespace {

    // CT_TLShapeTargetElement
    class ShapeTargetElementContext
        : public FragmentHandler2
    {
    public:
        ShapeTargetElementContext( FragmentHandler2 const & rParent, ShapeTargetElement & aValue )
            : FragmentHandler2( rParent )
                , bTargetSet(false)
                , maShapeTarget(aValue)
            {
            }
        virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs ) override
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
                    maShapeTarget.msSubShapeId = rAttribs.getStringDefaulted( XML_spid);
                    return this;
                case PPT_TOKEN( graphicEl ):
                    return this; // needs a:dgm for the target
                case A_TOKEN( dgm ):
                    bTargetSet = true;
                    maShapeTarget.mnType = XML_dgm;
                    maShapeTarget.msSubShapeId = rAttribs.getStringDefaulted( XML_id);
                    return this;
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

    }

    TimeTargetElementContext::TimeTargetElementContext( FragmentHandler2 const & rParent, AnimTargetElementPtr pValue )
        : FragmentHandler2( rParent ),
            mpTarget(std::move( pValue ))
    {
        OSL_ENSURE( mpTarget, "no valid target passed" );
    }

    TimeTargetElementContext::~TimeTargetElementContext( ) noexcept
    {
    }

    ::oox::core::ContextHandlerRef TimeTargetElementContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( inkTgt ):
        {
            mpTarget->mnType = XML_inkTgt;
            OUString aId = rAttribs.getStringDefaulted( XML_spid);
            if( !aId.isEmpty() )
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

#if HAVE_FEATURE_AVMEDIA
            OUString srcFile = drawingml::getEmbeddedWAVAudioFile(getRelations(), rAttribs);
            Reference<css::io::XInputStream>
                xInputStream = getFilter().openInputStream(srcFile);

            if (xInputStream.is())
            {
                ::avmedia::EmbedMedia(getFilter().getModel(), srcFile, mpTarget->msValue, xInputStream);
                xInputStream->closeInput();
            }
#endif
            break;
        }
        case PPT_TOKEN( spTgt ):
        {
            mpTarget->mnType = XML_spTgt;
            mpTarget->msValue = rAttribs.getStringDefaulted(XML_spid);
            return new ShapeTargetElementContext( *this, mpTarget->maShapeTarget );
        }
        default:
            SAL_INFO(
                "oox.ppt",
                "unhandled tag " << getBaseToken(aElementToken)
                    << " in TL_TimeTargetElement");
            break;
        }

        return this;
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
