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

#include <com/sun/star/awt/XProgressBar.hpp>

#include <tools/color.hxx>

#include <basecontrol.hxx>

namespace unocontrols {

#define PROGRESSBAR_FREESPACE               4
#define PROGRESSBAR_DEFAULT_HORIZONTAL      true
#define PROGRESSBAR_DEFAULT_BLOCKDIMENSION  Size(1,1)
#define PROGRESSBAR_DEFAULT_BACKGROUNDCOLOR Color( 0xC0, 0xC0, 0xC0 ) // lightgray
#define PROGRESSBAR_DEFAULT_FOREGROUNDCOLOR Color( 0x00, 0x00, 0x80 ) // blue
#define PROGRESSBAR_DEFAULT_MINRANGE        INT_MIN
#define PROGRESSBAR_DEFAULT_MAXRANGE        INT_MAX
#define PROGRESSBAR_DEFAULT_BLOCKVALUE      1
#define PROGRESSBAR_DEFAULT_VALUE           PROGRESSBAR_DEFAULT_MINRANGE
#define PROGRESSBAR_LINECOLOR_BRIGHT        sal_Int32(Color( 0xFF, 0xFF, 0xFF )) // white
#define PROGRESSBAR_LINECOLOR_SHADOW        sal_Int32(Color( 0x00, 0x00, 0x00 )) // black

class ProgressBar final : public css::awt::XControlModel
                    , public css::awt::XProgressBar
                    , public BaseControl
{
public:

    ProgressBar( const css::uno::Reference< css::uno::XComponentContext >& rxContext );

    virtual ~ProgressBar() override;

    //  XInterface

    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;

    /**
        @short      increment refcount
        @seealso    XInterface
        @seealso    release()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() noexcept override;

    /**
        @short      decrement refcount
        @seealso    XInterface
        @seealso    acquire()
        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() noexcept override;

    //  XTypeProvider

    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes() override;

    //  XAggregation

    css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& aType ) override;

    //  XProgressBar

    virtual void SAL_CALL setForegroundColor( sal_Int32 nColor ) override;

    virtual void SAL_CALL setBackgroundColor( sal_Int32 nColor ) override;

    virtual void SAL_CALL setValue( sal_Int32 nValue ) override;

    virtual void SAL_CALL setRange(
        sal_Int32   nMin    ,
        sal_Int32   nMax
    ) override;

    virtual sal_Int32 SAL_CALL getValue() override;

    //  XWindow

    virtual void SAL_CALL setPosSize(
        sal_Int32   nX      ,
        sal_Int32   nY      ,
        sal_Int32   nWidth  ,
        sal_Int32   nHeight ,
        sal_Int16   nFlags
    ) override;

    //  XControl

    virtual sal_Bool SAL_CALL setModel(
        const css::uno::Reference< css::awt::XControlModel >& xModel
    ) override;

    virtual css::uno::Reference< css::awt::XControlModel > SAL_CALL getModel() override;

private:
    virtual void impl_paint(
        sal_Int32 nX ,
        sal_Int32 nY ,
        const css::uno::Reference< css::awt::XGraphics >& xGraphics
    ) override;

    void impl_recalcRange();

    bool            m_bHorizontal;   // orientation for steps            [true=horizontal/false=vertical]
    css::awt::Size  m_aBlockSize;   // width and height of a block      [>=0,0]
    Color           m_nForegroundColor;   //                                  (alpha,r,g,b)
    Color           m_nBackgroundColor;   //                                  (alpha,r,g,b)
    sal_Int32       m_nMinRange;   // lowest value  =   0%             [long, <_nMaxRange]
    sal_Int32       m_nMaxRange;   // highest value = 100%             [long, >_nMinRange]
    double          m_nBlockValue;   // value for one block              [long, >0]
    sal_Int32       m_nValue;   // value for progress               [long]

};

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
