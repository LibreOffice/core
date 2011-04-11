/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2010 Novell, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    void SAL_CALL setMetafile (const ::com::sun::star::uno::Sequence< sal_Int8 >& rMtf) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL draw (double fScaleX, double fScaleY) throw (::com::sun::star::uno::RuntimeException);

    // XFastPropertySet
    // setFastPropertyValue (0, GDIMetaFile*) is used to speedup the rendering
    virtual ::com::sun::star::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 /*nHandle*/)  throw (::com::sun::star::uno::RuntimeException) { return ::com::sun::star::uno::Any(); }
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const ::com::sun::star::uno::Any&)  throw (::com::sun::star::uno::RuntimeException);

private:
    GDIMetaFile* mpMetafile;
    com::sun::star::uno::Reference<com::sun::star::rendering::XBitmapCanvas> mxCanvas;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
