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


#include <com/sun/star/rendering/XCanvas.hpp>

#include <sal/types.h>
#include <vcl/canvastools.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <canvas/canvastools.hxx>

#include "pointaction.hxx"
#include "outdevstate.hxx"
#include "cppcanvas/canvas.hxx"
#include "mtftools.hxx"


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        namespace
        {
            class PointAction : public Action
            {
            public:
                PointAction( const ::basegfx::B2DPoint&,
                             const CanvasSharedPtr&,
                             const OutDevState& );
                PointAction( const ::basegfx::B2DPoint&,
                             const CanvasSharedPtr&,
                             const OutDevState&,
                             const ::Color&     );

                PointAction(const PointAction&) = delete;
                const PointAction& operator=(const PointAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                ::basegfx::B2DPoint                         maPoint;
                CanvasSharedPtr                             mpCanvas;
                css::rendering::RenderState                 maState;
            };

            PointAction::PointAction( const ::basegfx::B2DPoint& rPoint,
                                      const CanvasSharedPtr&     rCanvas,
                                      const OutDevState&         rState ) :
                maPoint( rPoint ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            PointAction::PointAction( const ::basegfx::B2DPoint& rPoint,
                                      const CanvasSharedPtr&     rCanvas,
                                      const OutDevState&         rState,
                                      const ::Color&             rAltColor ) :
                maPoint( rPoint ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = vcl::unotools::colorToDoubleSequence(
                    rAltColor,
                    rCanvas->getUNOCanvas()->getDevice()->getDeviceColorSpace() );
            }

            bool PointAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PointAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::PointAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                mpCanvas->getUNOCanvas()->drawPoint( ::basegfx::unotools::point2DFromB2DPoint(maPoint),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );

                return true;
            }

            bool PointAction::renderSubset( const ::basegfx::B2DHomMatrix&    rTransformation,
                                            const Subset&                     rSubset ) const
            {
                // point only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return render( rTransformation );
            }

            ::basegfx::B2DRange PointAction::getBounds( const ::basegfx::B2DHomMatrix&  rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds( ::basegfx::B2DRange( maPoint.getX()-1,
                                                                          maPoint.getY()-1,
                                                                          maPoint.getX()+1,
                                                                          maPoint.getY()+1 ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            ::basegfx::B2DRange PointAction::getBounds( const ::basegfx::B2DHomMatrix&  rTransformation,
                                                        const Subset&                   rSubset ) const
            {
                // point only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
            }

            sal_Int32 PointAction::getActionCount() const
            {
                return 1;
            }
        }

        std::shared_ptr<Action> PointActionFactory::createPointAction( const ::basegfx::B2DPoint& rPoint,
                                                               const CanvasSharedPtr&     rCanvas,
                                                               const OutDevState&         rState )
        {
            return std::shared_ptr<Action>( new PointAction( rPoint, rCanvas, rState ) );
        }

        std::shared_ptr<Action> PointActionFactory::createPointAction( const ::basegfx::B2DPoint& rPoint,
                                                               const CanvasSharedPtr&     rCanvas,
                                                               const OutDevState&         rState,
                                                               const ::Color&             rColor    )
        {
            return std::shared_ptr<Action>( new PointAction( rPoint, rCanvas, rState, rColor ) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
