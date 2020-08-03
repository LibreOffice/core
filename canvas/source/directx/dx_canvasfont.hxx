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

#pragma once

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

#include <rtl/ref.hxx>

#include <memory>

#include "dx_winstuff.hxx"
#include "dx_gdiplususer.hxx"


/* Definition of CanvasFont class */

namespace dxcanvas
{
    typedef std::shared_ptr< Gdiplus::Font >        FontSharedPtr;
    typedef std::shared_ptr< Gdiplus::FontFamily >  FontFamilySharedPtr;

    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCanvasFont,
                                             css::lang::XServiceInfo > CanvasFont_Base;

    class CanvasFont : public ::cppu::BaseMutex,
                       public CanvasFont_Base
    {
    public:
        typedef rtl::Reference<CanvasFont> ImplRef;
        /// make noncopyable
        CanvasFont(const CanvasFont&) = delete;
        const CanvasFont& operator=(const CanvasFont&) = delete;

        CanvasFont( const css::rendering::FontRequest&                                     fontRequest,
                    const css::uno::Sequence< css::beans::PropertyValue >&    extraFontProperties,
                    const css::geometry::Matrix2D&                                         fontMatrix );

        /// Dispose all internal references
        virtual void SAL_CALL disposing() override;

        // XCanvasFont
        virtual css::uno::Reference< css::rendering::XTextLayout > SAL_CALL createTextLayout( const css::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) override;
        virtual css::rendering::FontRequest SAL_CALL getFontRequest(  ) override;
        virtual css::rendering::FontMetrics SAL_CALL getFontMetrics(  ) override;
        virtual css::uno::Sequence< double > SAL_CALL getAvailableSizes(  ) override;
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getExtraFontProperties(  ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

        double              getCellAscent() const;
        double              getEmHeight() const;
        FontSharedPtr       getFont() const;
        const css::geometry::Matrix2D& getFontMatrix() const;
        sal_uInt32          getEmphasisMark() const { return mnEmphasisMark; }

    private:
        GDIPlusUserSharedPtr                        mpGdiPlusUser;
        FontFamilySharedPtr                         mpFontFamily;
        FontSharedPtr                               mpFont;
        css::rendering::FontRequest    maFontRequest;
        sal_uInt32                     mnEmphasisMark;
        css::geometry::Matrix2D        maFontMatrix;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
