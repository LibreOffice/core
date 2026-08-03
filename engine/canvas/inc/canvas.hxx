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

#pragma once

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XUpdatable.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <canvas/vclcanvasdllapi.h>

#include "./base/basemutexhelper.hxx"
#include "./base/bitmapcanvasbase.hxx"
#include "./base/graphicdevicebase.hxx"

#include "canvashelper.hxx"
#include "impltools.hxx"
#include "devicehelper.hxx"
#include "XGraphicDevice.hxx"
#include "Texture.hxx"

class OutputDevice;

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper< vclcanvas::XGraphicDevice,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet >    GraphicDeviceBase_Base;
    typedef ::canvas::GraphicDeviceBase< ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >,
                                           DeviceHelper,
                                           vclcanvastools::LocalGuard,
                                           ::cppu::OWeakObject >    CanvasBase_Base;
    typedef canvas::BitmapCanvasBase<
            CanvasBase_Base,
            CanvasHelper,
            vclcanvastools::LocalGuard,
            ::cppu::OWeakObject> CanvasBaseT;

    /** Product of this component's factory.

        The Canvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class Canvas : public CanvasBaseT
    {
    public:
        VCLCANVAS_DLLPUBLIC Canvas( OutputDevice* pOutDev );

        /// For resource tracking
        virtual ~Canvas() override;

        /// Dispose all internal references
        virtual void disposeThis() override;

        // Forwarding the XComponent implementation to the
        // cppu::ImplHelper templated base
        virtual void acquire() noexcept override { GraphicDeviceBase_Base::acquire(); }   \
        virtual void release() noexcept override { GraphicDeviceBase_Base::release(); }   \
        virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override \
            { return GraphicDeviceBase_Base::queryInterface(_rType); }                               \
        virtual void dispose() override \
        {                                                                               \
            ::cppu::WeakComponentImplHelperBase::dispose();                                                      \
        }                                                                               \
        virtual void addEventListener(                                         \
            css::uno::Reference< css::lang::XEventListener > const & xListener ) override \
        {                                                                               \
            ::cppu::WeakComponentImplHelperBase::addEventListener(xListener);                                        \
        }                                                                               \
        virtual void removeEventListener(                                      \
            css::uno::Reference< css::lang::XEventListener > const & xListener ) override \
        {                                                                               \
            ::cppu::WeakComponentImplHelperBase::removeEventListener(xListener);                                 \
        }

        bool repaint( const GraphicObjectSharedPtr&                 rGrf,
                              const ::vclcanvas::ViewState&              viewState,
                              const ::vclcanvas::RenderState&            renderState,
                              const ::Point&                                rPt,
                              const ::Size&                                 rSz,
                              const GraphicAttr&                            rAttr ) const;

        void drawLine(const css::geometry::RealPoint2D&  aStartPoint,
                                       const css::geometry::RealPoint2D&  aEndPoint,
                                       const ::vclcanvas::ViewState&   viewState,
                                       const ::vclcanvas::RenderState& renderState)
        {
            canvastools::verifyArgs(aStartPoint, aEndPoint, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawLine( this, aStartPoint, aEndPoint, viewState, renderState );
        }

        rtl::Reference< vclcanvas::CachedBitmap >
            drawBitmap( const Bitmap&                                                   rBitmap,
                        const ::vclcanvas::ViewState&                                   viewState,
                        const ::vclcanvas::RenderState&                                 renderState )
        {
            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.drawBitmap( this, rBitmap, viewState, renderState );
        }

        void
            strokePolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >&   xPolyPolygon,
                              const ::vclcanvas::ViewState&                               viewState,
                              const ::vclcanvas::RenderState&                             renderState,
                              const css::rendering::StrokeAttributes&                        strokeAttributes)
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState, strokeAttributes,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.strokePolyPolygon( this, xPolyPolygon, viewState, renderState, strokeAttributes );
        }

        rtl::Reference< vclcanvas::CachedBitmap >
            fillPolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >&               xPolyPolygon,
                             const ::vclcanvas::ViewState&                                          viewState,
                             const ::vclcanvas::RenderState&                                        renderState)
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.fillPolyPolygon( this, xPolyPolygon, viewState, renderState );
        }

        rtl::Reference< vclcanvas::CachedBitmap >
            fillTexturedPolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                                    const ::vclcanvas::ViewState&                             viewState,
                                    const ::vclcanvas::RenderState&                           renderState,
                                    const std::vector< vclcanvas::Texture >&           textures)
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState, textures,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            return maCanvasHelper.fillTexturedPolyPolygon( this, xPolyPolygon, viewState, renderState, textures );
        }

        rtl::Reference< vclcanvas::CanvasFont >
            createFont( const css::rendering::FontRequest&                                     fontRequest,
                        FontEmphasisMark                                                       eMark,
                        const css::geometry::Matrix2D&                                         fontMatrix )
        {
            canvastools::verifyArgs(fontRequest,
                              // dummy, to keep argPos in sync
                              fontRequest,
                              fontMatrix,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            return maCanvasHelper.createFont( this, fontRequest, eMark, fontMatrix );
        }


        void
            drawText(const css::rendering::StringContext&                                     text,
                     const rtl::Reference< vclcanvas::CanvasFont >&                xFont,
                     const ::vclcanvas::ViewState&                                         viewState,
                     const ::vclcanvas::RenderState&                                       renderState,
                     sal_Int8                                                                 textDirection)
        {
            canvastools::verifyArgs(xFont, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));
            canvastools::verifyRange( textDirection,
                                css::rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                                css::rendering::TextDirection::STRONG_RIGHT_TO_LEFT );

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawText( this, text, xFont, viewState, renderState, textDirection );
        }


        void
            drawTextLayout(const rtl::Reference< vclcanvas::TextLayout >&               laidOutText,
                            const ::vclcanvas::ViewState&                                       viewState,
                            const ::vclcanvas::RenderState&                                     renderState)
        {
            canvastools::verifyArgs(laidOutText, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawTextLayout( this, laidOutText, viewState, renderState );
        }

        void
            drawPolyPolygon(const css::uno::Reference< css::rendering::XPolyPolygon2D >& xPolyPolygon,
                            const ::vclcanvas::ViewState&                             viewState,
                            const ::vclcanvas::RenderState&                           renderState)
        {
            canvastools::verifyArgs(xPolyPolygon, viewState, renderState,
                              __func__,
                              static_cast< UnambiguousBaseType* >(this));

            MutexType aGuard( BaseType::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawPolyPolygon( this, xPolyPolygon, viewState, renderState );
        }
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
