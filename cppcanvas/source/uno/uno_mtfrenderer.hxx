/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _UNO_MTF_RENDERER_HXX_
#define _UNO_MTF_RENDERER_HXX_
#include <com/sun/star/rendering/MtfRenderer.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/basemutex.hxx>
#include <vcl/gdimtf.hxx>

typedef cppu::WeakComponentImplHelper2<com::sun::star::rendering::XMtfRenderer, com::sun::star::beans::XFastPropertySet> MtfRendererBase;

class MtfRenderer : private cppu::BaseMutex, public MtfRendererBase
{
public:
    MtfRenderer (com::sun::star::uno::Sequence<com::sun::star::uno::Any> const& args,
                 com::sun::star::uno::Reference<com::sun::star::uno::XComponentContext> const&);

    // XMtfRenderer iface
    void SAL_CALL setMetafile (const ::com::sun::star::uno::Sequence< sal_Int8 >& rMtf) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    void SAL_CALL draw (double fScaleX, double fScaleY) throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XFastPropertySet
    // setFastPropertyValue (0, GDIMetaFile*) is used to speedup the rendering
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 /*nHandle*/)  throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE { return ::com::sun::star::uno::Any(); }
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any&)  throw (::com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    GDIMetaFile* mpMetafile;
    com::sun::star::uno::Reference<com::sun::star::rendering::XBitmapCanvas> mxCanvas;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
