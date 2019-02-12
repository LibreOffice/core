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

#include <oox/ppt/soundactioncontext.hxx>

#include <cppuhelper/exc_hlp.hxx>

#include <oox/helper/attributelist.hxx>
#include <oox/helper/propertymap.hxx>
#include <drawingml/embeddedwavaudiofile.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/properties.hxx>
#include <oox/token/tokens.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <avmedia/mediaitem.hxx>

using namespace ::oox::core;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::uno;

namespace oox { namespace ppt {

    SoundActionContext::SoundActionContext( FragmentHandler2 const & rParent, PropertyMap & aProperties ) throw()
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
#if HAVE_FEATURE_AVMEDIA
                if ( !msSndName.isEmpty() )
                {
                    Reference<css::io::XInputStream>
                        xInputStream = getFilter().openInputStream(msSndName);
                    if (xInputStream.is())
                    {
                        ::avmedia::EmbedMedia(getFilter().getModel(), msSndName, url, xInputStream);
                        xInputStream->closeInput();
                    }
                }
#endif
                if ( !url.isEmpty() )
                {
                    maSlideProperties.setProperty( PROP_Sound, url);
                    maSlideProperties.setProperty( PROP_SoundOn, true);
                }
            }
        }
    }

    ::oox::core::ContextHandlerRef SoundActionContext::onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs )
    {
        switch( aElementToken )
        {
        case PPT_TOKEN( snd ):
            if( mbHasStartSound )
            {
                msSndName = drawingml::getEmbeddedWAVAudioFile( getRelations(), rAttribs );
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
