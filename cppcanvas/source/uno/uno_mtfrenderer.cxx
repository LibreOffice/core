/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppcanvas/vclfactory.hxx>
#include <o3tl/any.hxx>
#include <com/sun/star/rendering/XMtfRenderer.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <vcl/gdimtf.hxx>

using namespace ::com::sun::star;

typedef cppu::WeakComponentImplHelper<css::rendering::XMtfRenderer, css::beans::XFastPropertySet> MtfRendererBase;

namespace {

class MtfRenderer : private cppu::BaseMutex, public MtfRendererBase
{
public:
    MtfRenderer (css::uno::Sequence<css::uno::Any> const& args,
                 css::uno::Reference<css::uno::XComponentContext> const&);

    // XMtfRenderer iface
    void SAL_CALL setMetafile (const css::uno::Sequence< sal_Int8 >& rMtf) override;
    void SAL_CALL draw (double fScaleX, double fScaleY) override;

    // XFastPropertySet
    // setFastPropertyValue (0, GDIMetaFile*) is used to speedup the rendering
    virtual css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 /*nHandle*/) override { return css::uno::Any(); }
    virtual void SAL_CALL setFastPropertyValue(sal_Int32 nHandle, const css::uno::Any&) override;

private:
    GDIMetaFile* mpMetafile;
    css::uno::Reference<css::rendering::XBitmapCanvas> mxCanvas;
};

void MtfRenderer::setMetafile (const uno::Sequence< sal_Int8 >& /*rMtf*/)
{
        // printf ("MtfRenderer::setMetafile unimplemented, use fast property set or implement me\n");
}

void MtfRenderer::draw (double fScaleX, double fScaleY)
{
    if (mpMetafile && mxCanvas) {
        cppcanvas::BitmapCanvasSharedPtr canvas = cppcanvas::VCLFactory::createBitmapCanvas (mxCanvas);
        cppcanvas::RendererSharedPtr renderer = cppcanvas::VCLFactory::createRenderer (canvas, *mpMetafile, cppcanvas::Renderer::Parameters ());
        ::basegfx::B2DHomMatrix aMatrix;
        aMatrix.scale( fScaleX, fScaleY );
        canvas->setTransformation( aMatrix );
        renderer->draw ();
    }
}

void MtfRenderer::setFastPropertyValue( sal_Int32 nHandle, const uno::Any& aAny)
{
    if (nHandle == 0) {
        mpMetafile = reinterpret_cast<GDIMetaFile*>( *o3tl::doAccess<sal_Int64>(aAny) );
    }
}

MtfRenderer::MtfRenderer (uno::Sequence<uno::Any> const& aArgs, uno::Reference<uno::XComponentContext> const&) : MtfRendererBase (m_aMutex), mpMetafile (nullptr)
{
    if( aArgs.getLength() == 1 ) {
        aArgs[0] >>= mxCanvas;
    }
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_rendering_MtfRenderer_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& args)
{
    return cppu::acquire(new MtfRenderer(args, context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
