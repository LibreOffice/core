/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include "uno_mtfrenderer.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <comphelper/servicedecl.hxx>
#include <cppuhelper/factory.hxx>

using namespace ::com::sun::star;

void MtfRenderer::setMetafile (const uno::Sequence< sal_Int8 >& /*rMtf*/) throw (uno::RuntimeException)
{
        
}

void MtfRenderer::draw (double fScaleX, double fScaleY) throw (uno::RuntimeException)
{
    if (mpMetafile && mxCanvas.get()) {
        cppcanvas::VCLFactory& factory = cppcanvas::VCLFactory::getInstance();
        cppcanvas::BitmapCanvasSharedPtr canvas = factory.createBitmapCanvas (mxCanvas);
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
 const sdecl::ServiceDecl MtfRendererDecl(
     sdecl::class_<MtfRenderer, sdecl::with_args<true> >(),
    "com.sun.star.comp.rendering.MtfRenderer",
    "com.sun.star.rendering.MtfRenderer" );


COMPHELPER_SERVICEDECL_EXPORTS1(mtfrenderer, MtfRendererDecl)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
