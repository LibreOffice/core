/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: implrenderer.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 13:39:25 $
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

            virtual bool                draw() const;
            virtual bool                drawSubset( sal_Int32   nStartIndex,
                                                    sal_Int32   nEndIndex ) const;
            virtual ::basegfx::B2DRange getSubsetArea( sal_Int32    nStartIndex,
                                                       sal_Int32    nEndIndex ) const;


            // element of the Renderer's action vector. Need to be
            // public, since some functors need it, too.
            struct MtfAction
            {
                MtfAction( const ActionSharedPtr&   rAction,
                           sal_Int32                nOrigIndex ) :
                    mpAction( rAction ),
                    mnOrigIndex( nOrigIndex )
                {
                }

                ActionSharedPtr mpAction;
                sal_Int32       mnOrigIndex;
            };

            // prefetched and prepared canvas actions
            // (externally not visible)
            typedef ::std::vector< MtfAction >      ActionVector;


        private:
            // default: disabled copy/assignment
            ImplRenderer(const ImplRenderer&);
            ImplRenderer& operator=( const ImplRenderer& );

            void updateClipping( VectorOfOutDevStates&              rStates,
                                 const ::basegfx::B2DPolyPolygon&   rClipPoly,
                                 const CanvasSharedPtr&             rCanvas,
                                 bool                               bIntersect );

            void updateClipping( VectorOfOutDevStates&  rStates,
                                 const ::Rectangle&     rClipRect,
                                 const CanvasSharedPtr& rCanvas,
                                 bool                   bIntersect );

            ::com::sun::star::uno::Reference<
                ::com::sun::star::rendering::XCanvasFont > createFont( double&                  o_rFontRotation,
                                                                       const ::Font&            rFont,
                                                                       const CanvasSharedPtr&   rCanvas,
                                                                       const ::VirtualDevice&   rVDev,
                                                                       const Parameters&        rParms ) const;
            bool            createActions( const CanvasSharedPtr&   rCanvas,
                                           VirtualDevice&           rVDev,
                                           GDIMetaFile&             rMtf,
                                           VectorOfOutDevStates&    rStates,
                                           const Parameters&        rParms,
                                           bool                     bSubsettableActions,
                                           sal_Int32&               io_rCurrActionIndex );
            bool            createFillAndStroke( const ::PolyPolygon&           rPolyPoly,
                                                 const CanvasSharedPtr&         rCanvas,
                                                 sal_Int32&                     rActionIndex,
                                                 const VectorOfOutDevStates&    rStates );
            void            skipContent( GDIMetaFile& rMtf,
                                         const char*  pCommentString,
                                         sal_Int32&   io_rCurrActionIndex ) const;

            bool            isActionContained( GDIMetaFile& rMtf,
                                               const char*  pCommentString,
                                               USHORT       nType ) const;

            void            createGradientAction( const ::PolyPolygon&      rPoly,
                                                  const ::Gradient&         rGradient,
                                                  ::VirtualDevice&          rVDev,
                                                  const CanvasSharedPtr&    rCanvas,
                                                  VectorOfOutDevStates&     rStates,
                                                  const Parameters&         rParms,
                                                  sal_Int32&                io_rCurrActionIndex,
                                                  bool                      bIsPolygonRectangle,
                                                  bool                      bSubsettableActions );

            void            createTextAction( const ::Point&                rStartPoint,
                                              const String                  rString,
                                              int                           nIndex,
                                              int                           nLength,
                                              const sal_Int32*              pCharWidths,
                                              ::VirtualDevice&              rVDev,
                                              const CanvasSharedPtr&        rCanvas,
                                              VectorOfOutDevStates&         rStates,
                                              const Parameters&             rParms,
                                              bool                          bSubsettable,
                                              sal_Int32&                    io_rCurrActionIndex );

            bool            getSubsetIndices( sal_Int32&                        io_rStartIndex,
                                              sal_Int32&                        io_rEndIndex,
                                              ActionVector::const_iterator&     o_rRangeBegin,
                                              ActionVector::const_iterator&     o_rRangeEnd ) const;


            ActionVector                            maActions;
        };
    }
}

#endif /* _CPPCANVAS_IMPLRENDERER_HXX */
