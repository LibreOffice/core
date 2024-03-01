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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXFONT_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXFONT_HXX

#include <config_options.h>
#include <memory>
#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XFont2.hpp>
#include <cppuhelper/implbase.hxx>
#include <mutex>
#include <vcl/font.hxx>

namespace com::sun::star::awt { class XDevice; }

class FontMetric;




class UNLESS_MERGELIBS_MORE(TOOLKIT_DLLPUBLIC) VCLXFont final :
                        public cppu::WeakImplHelper<
                            css::awt::XFont2>
{
    std::mutex    maMutex;
    css::uno::Reference< css::awt::XDevice> mxDevice;
    vcl::Font       maFont;
    std::unique_ptr<FontMetric>
                    mpFontMetric;

    bool            ImplAssertValidFontMetric();

public:
                    VCLXFont();
                    virtual ~VCLXFont() override;

    void            Init( css::awt::XDevice& rxDev, const vcl::Font& rFont );
    const vcl::Font&     GetFont() const { return maFont; }

    // css::lang::XFont
    css::awt::FontDescriptor           SAL_CALL getFontDescriptor(  ) override;
    css::awt::SimpleFontMetric         SAL_CALL getFontMetric(  ) override;
    sal_Int16                                       SAL_CALL getCharWidth( sal_Unicode c ) override;
    css::uno::Sequence< sal_Int16 >    SAL_CALL getCharWidths( sal_Unicode nFirst, sal_Unicode nLast ) override;
    sal_Int32                                       SAL_CALL getStringWidth( const OUString& str ) override;
    sal_Int32                                       SAL_CALL getStringWidthArray( const OUString& str, css::uno::Sequence< sal_Int32 >& rDXArray ) override;
    void                                            SAL_CALL getKernPairs( css::uno::Sequence< sal_Unicode >& rnChars1, css::uno::Sequence< sal_Unicode >& rnChars2, css::uno::Sequence< sal_Int16 >& rnKerns ) override;

    // css::lang::XFont2
    sal_Bool                                        SAL_CALL hasGlyphs( const OUString& aText ) override;
};


#endif // INCLUDED_TOOLKIT_AWT_VCLXFONT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
