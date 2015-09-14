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

#ifndef INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASFONT_HXX
#define INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASFONT_HXX

#include <cppuhelper/compbase.hxx>
#include <comphelper/broadcasthelper.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XCanvasFont.hpp>

#include <rtl/ref.hxx>

#include <memory>
#include <boost/noncopyable.hpp>

#include "dx_winstuff.hxx"
#include "dx_gdiplususer.hxx"


/* Definition of CanvasFont class */

namespace dxcanvas
{
    typedef std::shared_ptr< Gdiplus::Font >        FontSharedPtr;
    typedef std::shared_ptr< Gdiplus::FontFamily >  FontFamilySharedPtr;

    typedef ::cppu::WeakComponentImplHelper< css::rendering::XCanvasFont,
                                             css::lang::XServiceInfo > CanvasFont_Base;

    class CanvasFont : public ::comphelper::OBaseMutex,
                       public CanvasFont_Base,
                       private ::boost::noncopyable
    {
    public:
        typedef rtl::Reference<CanvasFont> ImplRef;

        CanvasFont( const css::rendering::FontRequest&                                     fontRequest,
                    const css::uno::Sequence< css::beans::PropertyValue >&    extraFontProperties,
                    const css::geometry::Matrix2D&                                         fontMatrix );

        /// Dispose all internal references
        virtual void SAL_CALL disposing();

        // XCanvasFont
        virtual css::uno::Reference< css::rendering::XTextLayout > SAL_CALL createTextLayout( const css::rendering::StringContext& aText, sal_Int8 nDirection, sal_Int64 nRandomSeed ) throw (css::uno::RuntimeException);
        virtual css::rendering::FontRequest SAL_CALL getFontRequest(  ) throw (css::uno::RuntimeException);
        virtual css::rendering::FontMetrics SAL_CALL getFontMetrics(  ) throw (css::uno::RuntimeException);
        virtual css::uno::Sequence< double > SAL_CALL getAvailableSizes(  ) throw (css::uno::RuntimeException);
        virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getExtraFontProperties(  ) throw (css::uno::RuntimeException);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
        virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()  throw( css::uno::RuntimeException );

        double              getCellAscent() const;
        double              getEmHeight() const;
        FontSharedPtr       getFont() const;
        const css::geometry::Matrix2D& getFontMatrix() const;

    private:
        GDIPlusUserSharedPtr                        mpGdiPlusUser;
        FontFamilySharedPtr                         mpFontFamily;
        FontSharedPtr                               mpFont;
        css::rendering::FontRequest    maFontRequest;
        css::geometry::Matrix2D        maFontMatrix;
    };

}

#endif // INCLUDED_CANVAS_SOURCE_DIRECTX_DX_CANVASFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
