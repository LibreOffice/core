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

#ifndef INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_TEXTACTION_HXX
#define INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_TEXTACTION_HXX

#include <action.hxx>
#include <cppcanvas/canvas.hxx>
#include <cppcanvas/renderer.hxx>

class VirtualDevice;
class Point;
class Size;
class Color;


/* Definition of internal::TextActionFactory */

namespace cppcanvas
{
    namespace internal
    {
        struct OutDevState;

        /** Creates encapsulated converters between GDIMetaFile and
            XCanvas. The Canvas argument is deliberately placed at the
            constructor, to force reconstruction of this object for a
            new canvas. This considerably eases internal state
            handling, since a lot of the internal state (e.g. fonts,
            text layout) is Canvas-dependent.
         */
        namespace TextActionFactory
        {
            /** Create text action, optionally shadow/relief effect

                Note that this method accepts all coordinates in
                logical coordinates.

                @param pDXArray
                Pointer to array of logical character offsets (or NULL)

                @param bSubsettable
                When this parameter is set to true, the generated
                action might consume slightly more memory, but is
                subsettable (Action::render( Subset ) works on
                characters)
             */
            ActionSharedPtr createTextAction( const ::Point&                 rStartPoint,
                                                     const ::Size&                  rReliefOffset,
                                                     const ::Color&                 rReliefColor,
                                                     const ::Size&                  rShadowOffset,
                                                     const ::Color&                 rShadowColor,
                                                     const OUString&                rText,
                                                     sal_Int32                      nStartPos,
                                                     sal_Int32                      nLen,
                                                     const long*                    pDXArray,
                                                     VirtualDevice&                 rVDev,
                                                     const CanvasSharedPtr&         rCanvas,
                                                     const OutDevState&             rState,
                                                     const Renderer::Parameters&    rParms,
                                                     bool                           bSubsettable );
        }
    }
}

#endif // INCLUDED_CPPCANVAS_SOURCE_MTFRENDERER_TEXTACTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
