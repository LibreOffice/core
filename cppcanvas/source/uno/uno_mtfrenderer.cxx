/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "uno_mtfrenderer.hxx"
#include <cppcanvas/vclfactory.hxx>
#include <comphelper/servicedecl.hxx>
#include <cppuhelper/factory.hxx>

using namespace ::com::sun::star;

void MtfRenderer::setMetafile (const uno::Sequence< sal_Int8 >& /*rMtf*/) throw (uno::RuntimeException, std::exception)
{
        // printf ("MtfRenderer::setMetafile unimplemented, use fast property set or implement me\n");
}

void MtfRenderer::draw (double fScaleX, double fScaleY) throw (uno::RuntimeException, std::exception)
{
    if (mpMetafile && mxCanvas.get()) {
        cppcanvas::BitmapCanvasSharedPtr canvas = cppcanvas::VCLFactory::createBitmapCanvas (mxCanvas);
        cppcanvas::RendererSharedPtr renderer = cppcanvas::VCLFactory::createRenderer (canvas, *mpMetafile, cppcanvas::Renderer::Parameters ());
        ::basegfx::B2DHomMatrix aMatrix;
        aMatrix.scale( fScaleX, fScaleY );
        canvas->setTransformation( aMatrix );
        renderer->draw ();
    }
}

void MtfRenderer::setFastPropertyValue( sal_Int32 nHandle, const uno::Any& aAny)  throw (uno::RuntimeException, std::exception)
{
    if (nHandle == 0) {
        mpMetafile = reinterpret_cast<GDIMetaFile*>( *static_cast<const sal_Int64*>(aAny.getValue()) );
    }
}

MtfRenderer::MtfRenderer (uno::Sequence<uno::Any> const& aArgs, uno::Reference<uno::XComponentContext> const&) : MtfRendererBase (m_aMutex), mpMetafile (nullptr)
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

// The C shared lib entry points
extern "C"
SAL_DLLPUBLIC_EXPORT void* SAL_CALL mtfrenderer_component_getFactory( sal_Char const* pImplName,
                                         void*, void* )
{
    return sdecl::component_getFactoryHelper( pImplName, {&MtfRendererDecl} );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
