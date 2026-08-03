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
#include <com/sun/star/rendering/TextDirection.hpp>
#include <com/sun/star/util/XUpdatable.hpp>

#include <cppuhelper/compbase.hxx>
#include <comphelper/uno3.hxx>
#include <canvas/vclcanvasdllapi.h>
#include <sal/types.h>

#include "./base/basemutexhelper.hxx"

#include "canvasfont.hxx"
#include "canvashelper.hxx"
#include "impltools.hxx"
#include "devicehelper.hxx"
#include "XGraphicDevice.hxx"
#include "Texture.hxx"
#include "propertysethelper.hxx"
#include "parametricpolypolygon.hxx"
#include "verifyinput.hxx"

class OutputDevice;

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper< vclcanvas::XGraphicDevice,
                                             css::util::XUpdatable,
                                             css::beans::XPropertySet >    GraphicDeviceBase_Base;

    /** Product of this component's factory.

        The Canvas object combines the actual Window canvas with
        the XGraphicDevice interface. This is because there's a
        one-to-one relation between them, anyway, since each window
        can have exactly one canvas and one associated
        XGraphicDevice. And to avoid messing around with circular
        references, this is implemented as one single object.
     */
    class SAL_DLLPUBLIC_RTTI Canvas : public ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >
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

        // XGraphicDevice

        virtual css::uno::Reference< css::rendering::XLinePolyPolygon2D > createCompatibleLinePolyPolygon( const cpo::uno::Sequence< cpo::uno::Sequence< css::geometry::RealPoint2D > >& points ) override
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

            return maDeviceHelper.createCompatibleLinePolyPolygon( this, points );
        }

        virtual ::css::uno::Reference< ::css::rendering::XParametricPolyPolygon2D > createParametricPolyPolygon( const ::rtl::OUString& GradientService, const ::cpo::uno::Sequence< ::cpo::uno::Sequence< double > >& colors, const ::cpo::uno::Sequence< double >& stops, double aspectRatio ) override
        {
            return css::uno::Reference< css::rendering::XParametricPolyPolygon2D >(
                canvas::ParametricPolyPolygon::create(this,
                                              GradientService,
                                              colors, stops, aspectRatio));
        }

        // XUpdatable
        virtual void update() override
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

            if( mbDumpScreenContent )
                maDeviceHelper.dumpScreenContent();
        }


        // XPropertySet
        virtual css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo() override
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );
            return maPropHelper.getPropertySetInfo();
        }

        virtual void setPropertyValue( const OUString&                   aPropertyName,
                                                const cpo::uno::Any& aValue ) override
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );
            maPropHelper.setPropertyValue( aPropertyName, aValue );
        }

        virtual cpo::uno::Any getPropertyValue( const OUString& aPropertyName ) override
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );
            return maPropHelper.getPropertyValue( aPropertyName );
        }

        virtual void addPropertyChangeListener( const OUString& aPropertyName,
                                                         const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );
            maPropHelper.addPropertyChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void removePropertyChangeListener( const OUString& ,
                                                            const css::uno::Reference< css::beans::XPropertyChangeListener >& ) override
        {
        }

        virtual void addVetoableChangeListener( const OUString& aPropertyName,
                                                         const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener ) override
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );
            maPropHelper.addVetoableChangeListener( aPropertyName,
                                                    xListener );
        }

        virtual void removeVetoableChangeListener( const OUString& ,
                                                            const css::uno::Reference< css::beans::XVetoableChangeListener >& ) override
        {
        }

        void clear()
        {
            vclcanvastools::LocalGuard aGuard( m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.clear();
        }

        void drawPoint(const css::geometry::RealPoint2D&     aPoint,
                                        const ::vclcanvas::ViewState&      viewState,
                                        const ::vclcanvas::RenderState&    renderState)
        {
            canvastools::verifyArgs(aPoint, viewState, renderState,
                              __func__,
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( m_aMutex );

            mbSurfaceDirty = true;
        }

        css::uno::Reference< vclcanvas::XGraphicDevice > getDevice()
        {
            vclcanvastools::LocalGuard aGuard( m_aMutex );

            return maCanvasHelper.getDevice();
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
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawLine( this, aStartPoint, aEndPoint, viewState, renderState );
        }

        rtl::Reference< vclcanvas::CachedBitmap >
            drawBitmap( const Bitmap&                                                   rBitmap,
                        const ::vclcanvas::ViewState&                                   viewState,
                        const ::vclcanvas::RenderState&                                 renderState )
        {
            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

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
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

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
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

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
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

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
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

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
                              static_cast< ::cppu::OWeakObject* >(this));
            canvastools::verifyRange( textDirection,
                                css::rendering::TextDirection::WEAK_LEFT_TO_RIGHT,
                                css::rendering::TextDirection::STRONG_RIGHT_TO_LEFT );

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

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
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

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
                              static_cast< ::cppu::OWeakObject* >(this));

            vclcanvastools::LocalGuard aGuard( ::canvas::BaseMutexHelper< GraphicDeviceBase_Base >::m_aMutex );

            mbSurfaceDirty = true;

            maCanvasHelper.drawPolyPolygon( this, xPolyPolygon, viewState, renderState );
        }

    private:
        cpo::uno::Any getDumpScreenContent() const
        {
            return cpo::uno::Any( mbDumpScreenContent );
        }

        void setDumpScreenContent( const cpo::uno::Any& rAny )
        {
            // TODO(Q1): this was mbDumpScreenContent =
            // rAny.get<bool>(), only that gcc3.3 wouldn't eat it
            rAny >>= mbDumpScreenContent;
        }

        css::geometry::IntegerSize2D getSize(  )
        {
            vclcanvastools::LocalGuard aGuard( m_aMutex );

            return maCanvasHelper.getSize();
        }

        bool hasAlpha(  )
        {
            return true;
        }

        DeviceHelper      maDeviceHelper;
        canvas::PropertySetHelper maPropHelper;
        bool              mbDumpScreenContent;
        CanvasHelper        maCanvasHelper;
        mutable bool        mbSurfaceDirty;
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
