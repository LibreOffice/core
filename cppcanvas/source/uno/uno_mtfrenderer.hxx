/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CPPCANVAS_SOURCE_UNO_UNO_MTFRENDERER_HXX
#define INCLUDED_CPPCANVAS_SOURCE_UNO_UNO_MTFRENDERER_HXX
#include <com/sun/star/rendering/MtfRenderer.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <vcl/gdimtf.hxx>

typedef cppu::WeakComponentImplHelper<css::rendering::XMtfRenderer, css::beans::XFastPropertySet> MtfRendererBase;

class MtfRenderer : private cppu::BaseMutex, public MtfRendererBase
{
public:
    MtfRenderer (css::uno::Sequence<css::uno::Any> const& args,
                 css::uno::Reference<css::uno::XComponentContext> const&);

    // XMtfRenderer iface
    void SAL_CALL setMetafile (const css::uno::Sequence< sal_Int8 >& rMtf) throw (css::uno::RuntimeException, std::exception) override;
    void SAL_CALL draw (double fScaleX, double fScaleY) throw (css::uno::RuntimeException, std::exception) override;

    // XFastPropertySet
    // setFastPropertyValue (0, GDIMetaFile*) is used to speedup the rendering
    virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 /*nHandle*/)  throw (css::uno::RuntimeException, std::exception) override { return css::uno::Any(); }
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const css::uno::Any&)  throw (css::uno::RuntimeException, std::exception) override;

private:
    GDIMetaFile* mpMetafile;
    css::uno::Reference<css::rendering::XBitmapCanvas> mxCanvas;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
