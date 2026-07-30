/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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


#include "lineaction.hxx"
#include "outdevstate.hxx"

#include <basegfx/range/b2drange.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/utils/canvastools.hxx>
#include <canvastools.hxx>
#include <sal/log.hxx>
#include <RenderState.hxx>

#include "canvas.hxx"
#include <utility>

#include "mtftools.hxx"
#include <XCanvas.hxx>

using namespace ::com::sun::star;

namespace cppcanvas
{
        namespace
        {
            class LineAction : public Action
            {
            public:
                LineAction( const ::basegfx::B2DPoint&,
                            const ::basegfx::B2DPoint&,
                            const OutDevState& );

                LineAction(const LineAction&) = delete;
                const LineAction& operator=(const LineAction&) = delete;

                virtual bool render( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                     const vclcanvas::ViewState& rViewState,
                                     const ::basegfx::B2DHomMatrix& rTransformation ) const override;
                virtual bool renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                           const vclcanvas::ViewState& rViewState,
                                           const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const override;

                virtual sal_Int32 getActionCount() const override;

            private:
                ::basegfx::B2DPoint     maStartPoint;
                ::basegfx::B2DPoint     maEndPoint;
                vclcanvas::RenderState  maState;
            };

            LineAction::LineAction( const ::basegfx::B2DPoint& rStartPoint,
                                    const ::basegfx::B2DPoint& rEndPoint,
                                    const OutDevState&         rState ) :
                maStartPoint( rStartPoint ),
                maEndPoint( rEndPoint )
            {
                cppcanvastools::initRenderState(maState,rState);
                maState.DeviceColor = rState.lineColor;
            }

            bool LineAction::render( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                     const vclcanvas::ViewState& rViewState,
                                     const ::basegfx::B2DHomMatrix& rTransformation ) const
            {
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::LineAction::render()" );
                SAL_INFO( "cppcanvas.emf", "::cppcanvas::LineAction: 0x" << std::hex << this );

                vclcanvas::RenderState aLocalState( maState );
                ::canvastools::prependToRenderState(aLocalState, rTransformation);

                rCanvas->drawLine( ::basegfx::unotools::point2DFromB2DPoint(maStartPoint),
                                    ::basegfx::unotools::point2DFromB2DPoint(maEndPoint),
                                    rViewState,
                                    aLocalState );

                return true;
            }

            bool LineAction::renderSubset( const css::uno::Reference<vclcanvas::XCanvas>& rCanvas,
                                           const vclcanvas::ViewState& rViewState,
                                           const ::basegfx::B2DHomMatrix& rTransformation,
                                           const Subset&                  rSubset ) const
            {
                // line only contains a single action, fail if subset
                // requests different range
                if( rSubset.mnSubsetBegin != 0 ||
                    rSubset.mnSubsetEnd != 1 )
                    return false;

                return render( rCanvas, rViewState, rTransformation );
            }

            sal_Int32 LineAction::getActionCount() const
            {
                return 1;
            }
        }

        std::shared_ptr<Action> LineActionFactory::createLineAction( const ::basegfx::B2DPoint& rStartPoint,
                                                             const ::basegfx::B2DPoint& rEndPoint,
                                                             const OutDevState&         rState  )
        {
            return std::make_shared<LineAction>( rStartPoint,
                                                 rEndPoint,
                                                 rState);
        }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
