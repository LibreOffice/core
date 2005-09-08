/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textaction.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 08:22:31 $
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

#ifndef _CPPCANVAS_TEXTACTION_HXX
#define _CPPCANVAS_TEXTACTION_HXX

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#include <action.hxx>
#include <cppcanvas/canvas.hxx>
#include <cppcanvas/renderer.hxx>

#ifndef _TL_POLY_HXX
#include <tools/poly.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

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
