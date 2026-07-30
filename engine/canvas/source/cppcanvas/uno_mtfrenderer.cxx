/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vclfactory.hxx"
#include <o3tl/any.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XMtfRenderer.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <comphelper/compbase.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/canvastools.hxx>
#include <canvas.hxx>
#include <canvasbitmap.hxx>

using namespace ::com::sun::star;

typedef comphelper::WeakComponentImplHelper<css::rendering::XMtfRenderer, css::lang::XServiceInfo> MtfRendererBase;

namespace {

class MtfRenderer : public MtfRendererBase
{
public:
    MtfRenderer (cpo::uno::Sequence<cpo::uno::Any> const& args,
                 css::uno::Reference<css::uno::XComponentContext> const&);

    // XMtfRenderer iface
    sal_Int64 SAL_CALL draw (sal_Int64 pOutputDevice, sal_Int64 pMeta, double fScaleX, double fScaleY) override;

    // XServiceIfno
    virtual ::rtl::OUString getImplementationName() override { return u"com.sun.star.comp.rendering.MtfRenderer"_ustr; }
    virtual bool supportsService( const ::rtl::OUString& rServiceName ) override
    {  return cppu::supportsService(this, rServiceName); }
    virtual ::cpo::uno::Sequence< ::rtl::OUString > getSupportedServiceNames() override
    {
        return { u"com.sun.star.rendering.MtfRenderer"_ustr };
    }
};

sal_Int64 MtfRenderer::draw (sal_Int64 pOutputDevice, sal_Int64 pMeta, double fScaleX, double fScaleY)
{
    rtl::Reference<vclcanvas::Canvas> xCanvas(new vclcanvas::Canvas(reinterpret_cast<OutputDevice*>(pOutputDevice)));
    comphelper::ScopeGuard aCanvasScopeGuard([&xCanvas] {
        comphelper::disposeComponent(xCanvas);
    });

    Size aSize (fScaleX + 1, fScaleY + 1);
    rtl::Reference<vclcanvas::CanvasBitmap> xBitmapCanvas = xCanvas->getDevice ()->createCompatibleAlphaBitmap (vcl::unotools::integerSize2DFromSize( aSize));
    if( !xBitmapCanvas )
        return 0;

    xBitmapCanvas->clear();

    GDIMetaFile* pMetafile = reinterpret_cast<GDIMetaFile*>(pMeta);
    ::basegfx::B2DHomMatrix aMatrix;
    aMatrix.scale( fScaleX, fScaleY );
    cppcanvas::RendererSharedPtr renderer = cppcanvas::VCLFactory::createRenderer (xBitmapCanvas, aMatrix, *pMetafile);
    renderer->draw ();

    Bitmap aBitmap = vcl::unotools::bitmapFromXBitmap(xBitmapCanvas);
    return reinterpret_cast<sal_Int64>(new Bitmap(aBitmap));
}

MtfRenderer::MtfRenderer (cpo::uno::Sequence<cpo::uno::Any> const&, uno::Reference<uno::XComponentContext> const&)
{
}

} // namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_rendering_MtfRenderer_get_implementation(
    css::uno::XComponentContext* context, cpo::uno::Sequence<cpo::uno::Any> const& args)
{
    return cppu::acquire(new MtfRenderer(args, context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
