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

#include <comphelper/compbase.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/StringContext.hpp>
#include <com/sun/star/rendering/XTextLayout.hpp>

#include "canvasfont.hxx"
#include <impltools.hxx>

/* Definition of TextLayout class */

namespace vclcanvas
{
    typedef ::comphelper::WeakComponentImplHelper< css::rendering::XTextLayout,
                                             css::lang::XServiceInfo > TextLayout_Base;

    class TextLayout : public TextLayout_Base
    {
    public:
        /// make noncopyable
        TextLayout(const TextLayout&) = delete;
        const TextLayout& operator=(const TextLayout&) = delete;

        TextLayout( css::rendering::StringContext                 aText,
                    sal_Int8                                      nDirection,
                    CanvasFont::Reference                         rFont,
                    css::uno::Reference<vclcanvas::XGraphicDevice> xDevice,
                    OutDevProviderSharedPtr                       xOutDev );

        /// Dispose all internal references
        virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

        // XTextLayout
        virtual cpo::uno::Sequence< double > queryLogicalAdvancements(  ) override;
        virtual void applyLogicalAdvancements( const cpo::uno::Sequence< double >& aAdvancements ) override;
        virtual cpo::uno::Sequence< bool > queryKashidaPositions(  ) override;
        virtual void applyKashidaPositions( const cpo::uno::Sequence< bool >& aPositions ) override;
        virtual css::geometry::RealRectangle2D queryTextBounds(  ) override;
        virtual sal_Int8 getMainTextDirection(  ) override;
        virtual css::uno::Reference< css::rendering::XCanvasFont > getFont(  ) override;
        virtual css::rendering::StringContext getText(  ) override;

        // XServiceInfo
        virtual OUString getImplementationName() override;
        virtual bool supportsService( const OUString& ServiceName ) override;
        virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

        void draw( OutputDevice&                                   rOutDev,
                   const Point&                                    rOutpos,
                   const ::vclcanvas::ViewState&                viewState,
                   const css::rendering::RenderState&              renderState ) const;

    private:
        KernArray setupTextOffsets(
                               const cpo::uno::Sequence< double >& inputOffsets,
                               const ::vclcanvas::ViewState&    viewState,
                               const css::rendering::RenderState&  renderState     ) const;

        css::rendering::StringContext                    maText;
        cpo::uno::Sequence< double >                     maLogicalAdvancements;
        cpo::uno::Sequence< bool >                   maKashidaPositions;
        CanvasFont::Reference                            mpFont;
        css::uno::Reference< vclcanvas::XGraphicDevice> mxDevice;
        OutDevProviderSharedPtr                          mpOutDevProvider;
        sal_Int8                                         mnTextDirection;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
