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


#include <lineaction.hxx>
#include <outdevstate.hxx>

#include <com/sun/star/rendering/XCanvas.hpp>

#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <canvas/canvastools.hxx>

#include <cppcanvas/canvas.hxx>

#include <mtftools.hxx>


using namespace ::com::sun::star;

namespace cppcanvas
{
    namespace internal
    {
        namespace
        {
            class LineAction : public Action
            {
            public:
                LineAction( const ::basegfx::B2DPoint&,
                            const ::basegfx::B2DPoint&,
                            const CanvasSharedPtr&,
                            const OutDevState& );

                LineAction(const LineAction&) = delete;
                const LineAction& operator=(const LineAction&) = delete;

                virtual bool render( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual ::basegfx::B2DRange getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                ::basegfx::B2DPoint     maStartPoint;
                ::basegfx::B2DPoint     maEndPoint;
                CanvasSharedPtr         mpCanvas;
                rendering::RenderState  maState;
            };

            LineAction::LineAction( const ::basegfx::B2DPoint& rStartPoint,
                                    const ::basegfx::B2DPoint& rEndPoint,
                                    const CanvasSharedPtr&     rCanvas,
                                    const OutDevState&         rState ) :
                maStartPoint( rStartPoint ),
                maEndPoint( rEndPoint ),
                mpCanvas( rCanvas ),
                maState()
            {
                tools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            bool LineAction::render( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::LineAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::internal::LineAction: 0x" << std::hex << this );

                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                mpCanvas->getUNOCanvas()->drawLine( ::basegfx::unotools::point2DFromB2DPoint(maStartPoint),
                                                    ::basegfx::unotools::point2DFromB2DPoint(maEndPoint),
                                                    mpCanvas->getViewState(),
                                                    aLocalState );

                return true;
            }

            bool LineAction::renderSubset( const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const
            {
                // line only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return render( rTransformation );
            }

            ::basegfx::B2DRange LineAction::getBounds( const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                rendering::RenderState aLocalState( maState );
                ::canvas::tools::prependToRenderState(aLocalState, rTransformation);

                return tools::calcDevicePixelBounds( ::basegfx::B2DRange( maStartPoint,
                                                                          maEndPoint ),
                                                     mpCanvas->getViewState(),
                                                     aLocalState );
            }

            ::basegfx::B2DRange LineAction::getBounds( const ::basegfx::B2DHomMatrix&   rTransformation,
                                                       const Subset&                    rSubset ) const
            {
                // line only contains a single action, empty bounds
                // if subset requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return ::basegfx::B2DRange();

                return getBounds( rTransformation );
            }

            sal_Int32 LineAction::getActionCount() const
            {
                return 1;
            }
        }

        std::shared_ptr<Action> LineActionFactory::createLineAction( const ::basegfx::B2DPoint& rStartPoint,
                                                             const ::basegfx::B2DPoint& rEndPoint,
                                                             const CanvasSharedPtr&     rCanvas,
                                                             const OutDevState&         rState  )
        {
            return std::shared_ptr<Action>( new LineAction( rStartPoint,
                                                    rEndPoint,
                                                    rCanvas,
                                                    rState) );
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
