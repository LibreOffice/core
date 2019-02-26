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


#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <osl/diagnose.h>

#include "implcustomsprite.hxx"
#include "implcanvas.hxx"

using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        ImplCustomSprite::ImplCustomSprite( const uno::Reference< rendering::XSpriteCanvas >&       rParentCanvas,
                                            const uno::Reference< rendering::XCustomSprite >&       rSprite,
                                            const ImplSpriteCanvas::TransformationArbiterSharedPtr& rTransformArbiter ) :
            ImplSprite( rParentCanvas,
                        uno::Reference< rendering::XSprite >(rSprite,
                                                             uno::UNO_QUERY),
                        rTransformArbiter ),
            mpLastCanvas(),
            mxCustomSprite( rSprite )
        {
            OSL_ENSURE( rParentCanvas.is(), "ImplCustomSprite::ImplCustomSprite(): Invalid canvas" );
            OSL_ENSURE( mxCustomSprite.is(), "ImplCustomSprite::ImplCustomSprite(): Invalid sprite" );
        }

        ImplCustomSprite::~ImplCustomSprite()
        {
        }

        CanvasSharedPtr ImplCustomSprite::getContentCanvas() const
        {
            OSL_ENSURE( mxCustomSprite.is(), "ImplCustomSprite::getContentCanvas(): Invalid sprite" );

            if( !mxCustomSprite.is() )
                return CanvasSharedPtr();

            uno::Reference< rendering::XCanvas > xCanvas( mxCustomSprite->getContentCanvas() );

            if( !xCanvas.is() )
                return CanvasSharedPtr();

            // cache content canvas C++ wrapper
            if( mpLastCanvas.get() == nullptr ||
                mpLastCanvas->getUNOCanvas() != xCanvas )
            {
                mpLastCanvas = CanvasSharedPtr( new ImplCanvas( xCanvas ) );
            }

            return mpLastCanvas;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
