/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: progind.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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

