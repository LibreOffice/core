/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progind.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:11:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SFX_PROGIND_HXX
#define _SFX_PROGIND_HXX

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif
#include <vcl/window.hxx>
#include <vcl/fixed.hxx>
#include <svtools/prgsbar.hxx>
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#include "sfxuno.hxx"

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

