/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textaction.hxx,v $
 * $Revision: 1.9 $
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

#ifndef _CPPCANVAS_TEXTACTION_HXX
#define _CPPCANVAS_TEXTACTION_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <action.hxx>
#include <cppcanvas/canvas.hxx>
#include <cppcanvas/renderer.hxx>
#include <tools/poly.hxx>
#include <tools/gen.hxx>

class VirtualDevice;
class Point;
class Color;
class String;


/* Definition of internal::TextActionFactory class */

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
        class TextActionFactory
        {
        public:
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
            static ActionSharedPtr createTextAction( const ::Point&                 rStartPoint,
                                                     const ::Size&                  rReliefOffset,
                                                     const ::Color&                 rReliefColor,
                                                     const ::Size&                  rShadowOffset,
                                                     const ::Color&                 rShadowColor,
                                                     const ::String&                rText,
                                                     sal_Int32                      nStartPos,
                                                     sal_Int32                      nLen,
                                                     const sal_Int32*               pDXArray,
                                                     VirtualDevice&                 rVDev,
                                                     const CanvasSharedPtr&         rCanvas,
                                                     const OutDevState&             rState,
                                                     const Renderer::Parameters&    rParms,
                                                     bool                           bSubsettable );

        private:
            // static factory, disable big four
            TextActionFactory();
            ~TextActionFactory();
            TextActionFactory(const TextActionFactory&);
            TextActionFactory& operator=( const TextActionFactory& );
        };
    }
}

#endif /* _CPPCANVAS_TEXTACTION_HXX */
