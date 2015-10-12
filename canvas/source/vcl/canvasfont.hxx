/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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

#ifndef INCLUDED_CANVAS_SOURCE_VCL_CANVASFONT_HXX
#define INCLUDED_CANVAS_SOURCE_VCL_CANVASFONT_HXX

#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/rendering/FontRequest.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>

#include <vcl/font.hxx>

#include <canvas/vclwrapper.hxx>

#include "spritecanvas.hxx"
#include "impltools.hxx"

#include <boost/noncopyable.hpp>


/* Definition of CanvasFont class */

namespace vclcanvas
{
    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCanvasFont,
                                             css::lang::XServiceInfo > CanvasFont_Base;

    class CanvasFont : public ::comphelper::OBaseMutex,
                       public CanvasFont_Base,
                       private ::boost::noncopyable
    {
    public:
        typedef rtl::Reference<CanvasFont> Reference;

        CanvasFont( const css::rendering::FontRequest&                                     fontRequest,
                    const css::uno::Sequence< css::beans::PropertyValue >&                 extraFontProperties,
                    const css::geometry::Matrix2D&                                         rFontMatrix,
                    css::rendering::XGraphicDevice&                                        rDevice,
                    const OutDevProviderSharedPtr&                                                      rOutDevProvider );

        /// Dispose all internal references
        virtual void SAL_CALL disposing() override;

        // XCanvasFont
        virtual css::uno::Reference< css::rendering::XTextLayout > SAL_CALL createTextLayout( const css::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::rendering::FontRequest SAL_CALL getFontRequest(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::rendering::FontMetrics SAL_CALL getFontMetrics(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< double > SAL_CALL getAvailableSizes(  ) throw (css::uno::RuntimeException, std::exception) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getExtraFontProperties(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw( css::uno::RuntimeException, std::exception ) override;

        vcl::Font getVCLFont() const;

    private:
        ::canvas::vcltools::VCLObject<vcl::Font>                          maFont;
        css::rendering::FontRequest                                       maFontRequest;
        css::uno::Reference< css::rendering::XGraphicDevice>              mpRefDevice;
        OutDevProviderSharedPtr                                           mpOutDevProvider;
    };

}

#endif // INCLUDED_CANVAS_SOURCE_VCL_CANVASFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
