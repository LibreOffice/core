/*************************************************************************
 *
 *  $RCSfile: implrenderer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 20:53:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _CPPCANVAS_IMPLRENDERER_HXX
#define _CPPCANVAS_IMPLRENDERER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _CPPCANVAS_RENDERER_HXX
#include <cppcanvas/renderer.hxx>
#endif
#ifndef _CPPCANVAS_CANVAS_HXX
#include <cppcanvas/canvas.hxx>
#endif

#include <canvasgraphichelper.hxx>
#include <action.hxx>

#include <vector>

class GDIMetaFile;
class BitmapEx;
class VirtualDevice;
class Gradient;

namespace cppcanvas
{

    namespace internal
    {
        struct OutDevState;

        // state stack of OutputDevice, to correctly handle
        // push/pop actions
        typedef ::std::vector< OutDevState >    VectorOfOutDevStates;

        class ImplRenderer : public virtual Renderer, protected CanvasGraphicHelper
        {
        public:
            ImplRenderer( const CanvasSharedPtr&    rCanvas,
                          const GDIMetaFile&        rMtf,
                          const Parameters&         rParms );
            ImplRenderer( const CanvasSharedPtr&    rCanvas,
                          const BitmapEx&           rBmpEx,
                          const Parameters&         rParms );

            virtual ~ImplRenderer();

            virtual bool draw() const;
            virtual bool drawSubset( int    nStartIndex,
                                     int    nEndIndex ) const;

            // element of the Renderer's action vector. Need to be
            // public, since some functors need it, too.
            struct MtfAction
            {
                MtfAction( const ActionSharedPtr&   rAction,
                           int                      nOrigIndex ) :
                    mpAction( rAction ),
                    mnOrigIndex( nOrigIndex )
                {
                }

                ActionSharedPtr mpAction;
                int             mnOrigIndex;
            };

        private:
            // default: disabled copy/assignment
            ImplRenderer(const ImplRenderer&);
            ImplRenderer& operator=( const ImplRenderer& );

            void updateClipping( VectorOfOutDevStates&              rStates,
                                 const ::basegfx::B2DPolyPolygon&   rClipPoly,
                                 const CanvasSharedPtr&             rCanvas,
                                 bool                               bIntersect );

            ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::rendering::XCanvasFont > createFont( ::basegfx::B2DHomMatrix&     o_rFontMatrix,
                                                                               const ::Font&                rFont,
                                                                               const CanvasSharedPtr&       rCanvas,
                                                                               const ::VirtualDevice&       rVDev,
                                                                               const Parameters&            rParms ) const;
            bool            createActions( const CanvasSharedPtr&   rCanvas,
                                           VirtualDevice&           rVDev,
                                           GDIMetaFile&             rMtf,
                                           VectorOfOutDevStates&    rStates,
                                           const Parameters&        rParms,
                                           int&                     io_rCurrActionIndex );
            bool            createFillAndStroke( const ::PolyPolygon&       rPolyPoly,
                                                 const CanvasSharedPtr&     rCanvas,
                                                 int                        rActionIndex,
                                                 VectorOfOutDevStates&      rStates );
            void            skipContent( GDIMetaFile&   rMtf,
                                         const char&    rCommentString ) const;

            void            createGradientAction( const ::PolyPolygon&      rPoly,
                                                  const ::Gradient&         rGradient,
                                                  ::VirtualDevice&          rVDev,
                                                  const CanvasSharedPtr&    rCanvas,
                                                  VectorOfOutDevStates&     rStates,
                                                  const Parameters&         rParms,
                                                  int&                      io_rCurrActionIndex,
                                                  bool                      bIsPolygonRectangle );

            // create text effects such as shadow/relief/embossed
            void            createTextWithEffectsAction(
                                                const Point&            rStartPoint,
                                                const String            rString,
                                                int                     nIndex,
                                                int                     nLength,
                                                const long*             pCharWidths,
                                                VirtualDevice&          rVDev,
                                                const CanvasSharedPtr&  rCanvas,
                                                VectorOfOutDevStates&   rStates,
                                                const Parameters&       rParms,
                                                int                     nCurrActionIndex );

            // create text draw actions and add text lines
            void            createTextWithLinesAction(
                                                const Point&            rStartPoint,
                                                const String            rString,
                                                int                     nIndex,
                                                int                     nLength,
                                                const long*             pCharWidths,
                                                VirtualDevice&          rVDev,
                                                const CanvasSharedPtr&  rCanvas,
                                                VectorOfOutDevStates&   rStates,
                                                const Parameters&       rParms,
                                                int                     nCurrActionIndex );

            // create text lines such as underline and strikeout
            void            createJustTextLinesAction(
                                                const Point&            rStartPoint,
                                                long                    nLineWidth,
                                                VirtualDevice&          rVDev,
                                                const CanvasSharedPtr&  rCanvas,
                                                VectorOfOutDevStates&   rStates,
                                                const Parameters&       rParms,
                                                int                     nCurrActionIndex );

            // prefetched and prepared canvas actions
            // (externally not visible)
            typedef ::std::vector< MtfAction >      ActionVector;
            ActionVector                            maActions;
        };
    }
}

#endif /* _CPPCANVAS_IMPLRENDERER_HXX */
