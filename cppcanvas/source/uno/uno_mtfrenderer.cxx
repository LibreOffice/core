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

#include "uno_mtfrenderer.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <comphelper/servicedecl.hxx>
#include <cppuhelper/factory.hxx>

using namespace ::com::sun::star;

void MtfRenderer::setMetafile (const uno::Sequence< sal_Int8 >& /*rMtf*/) throw (uno::RuntimeException)
{
        // printf ("MtfRenderer::setMetafile unimplemented, use fast property set or implement me\n");
}

void MtfRenderer::draw (double fScaleX, double fScaleY) throw (uno::RuntimeException)
{
    if (mpMetafile && mxCanvas.get()) {
        cppcanvas::VCLFactory& factory = cppcanvas::VCLFactory::getInstance();
        cppcanvas::BitmapCanvasSharedPtr canvas = factory.createCanvas (mxCanvas);
        cppcanvas::RendererSharedPtr renderer = factory.createRenderer (canvas, *mpMetafile, cppcanvas::Renderer::Parameters ());
        ::basegfx::B2DHomMatrix aMatrix;
        aMatrix.scale( fScaleX, fScaleY );
        canvas->setTransformation( aMatrix );
        renderer->draw ();
    }
}

void MtfRenderer::setFastPropertyValue( sal_Int32 nHandle, const uno::Any& aAny)  throw (uno::RuntimeException)
{
    if (nHandle == 0) {
        mpMetafile = (GDIMetaFile*) *reinterpret_cast<const sal_Int64*>(aAny.getValue());
    }
}

MtfRenderer::MtfRenderer (uno::Sequence<uno::Any> const& aArgs, uno::Reference<uno::XComponentContext> const&) : MtfRendererBase (m_aMutex), mpMetafile (NULL)
{
    if( aArgs.getLength() == 1 ) {
        aArgs[0] >>= mxCanvas;
    }
}

namespace sdecl = comphelper::service_decl;
#if defined (__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ <= 3)
 sdecl::class_<MtfRenderer, sdecl::with_args<true> > serviceImpl;
 const sdecl::ServiceDecl MtfRendererDecl(
     serviceImpl,
#else
 const sdecl::ServiceDecl MtfRendererDecl(
     sdecl::class_<MtfRenderer, sdecl::with_args<true> >(),
#endif
    "com.sun.star.comp.rendering.MtfRenderer",
    "com.sun.star.rendering.MtfRenderer" );

// The C shared lib entry points
COMPHELPER_SERVICEDECL_EXPORTS1(MtfRendererDecl)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
