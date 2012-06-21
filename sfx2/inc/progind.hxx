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
#ifndef _SFX_PROGIND_HXX
#define _SFX_PROGIND_HXX

#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <vcl/window.hxx>
#include <vcl/fixed.hxx>
#include <svtools/prgsbar.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/implbase1.hxx>

#include <sfx2/sfxuno.hxx>

class SfxStatusInd_Impl : public cppu::WeakImplHelper1< ::com::sun::star::task::XStatusIndicator >
{
    long                        nValue;
    long                        nRange;
    sal_uInt16                  nProgressCount;

friend class SfxPopupStatusIndicator;
    SfxPopupStatusIndicator*    pWindow;

public:

    SfxStatusInd_Impl();

    // XStatusIndicator
    virtual void SAL_CALL start(const ::rtl::OUString& aText, sal_Int32 nRange) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL end(void) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setText(const ::rtl::OUString& aText) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setValue(sal_Int32 nValue) throw ( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL reset() throw ( ::com::sun::star::uno::RuntimeException );
};

class SfxPopupStatusIndicator : public Window
{
friend class SfxStatusInd_Impl;
    FixedText                   aTextBar;
    ProgressBar                 aProgressBar;
    SfxStatusInd_Impl*          pInterface;

public:
                                SfxPopupStatusIndicator( Window* pParent );
                                ~SfxPopupStatusIndicator();

    virtual void                MakeVisible( sal_Bool bVisible );
    virtual void                Resize();
    virtual void                Paint( const Rectangle& rRect );
    Size                        CalcWindowSizePixel();
    ::com::sun::star::task::XStatusIndicator*           GetInterface()
                                { return pInterface; }
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
