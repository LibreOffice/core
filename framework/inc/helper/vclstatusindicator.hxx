/*************************************************************************
 *
 *  $RCSfile: vclstatusindicator.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 14:29:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_
#define __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_

//-----------------------------------------------
// includes of own modules

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//-----------------------------------------------
// includes of interfaces

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

//-----------------------------------------------
// includes of external modules

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _SV_STATUS_HXX
#include <vcl/status.hxx>
#endif

//-----------------------------------------------
// namespace

namespace framework {

//-----------------------------------------------
// declaration

class VCLStatusIndicator : public  css::task::XStatusIndicator
                         , private ThreadHelpBase // must be the first real base class!
                         , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // member

    private:

        /** can be used to create own needed uno resources. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** points to the parent window of this progress and
            hold it alive. */
        css::uno::Reference< css::awt::XWindow > m_xParentWindow;

        /** shows the progress.

            @attention  This member isnt synchronized using our own mutex!
                        Its guarded by the solarmutex only. Otherwhise
                        we have to lock two of them, which can force a deadlock ...
            */
        StatusBar* m_pStatusBar;

        /** knows the current info text of the progress. */
        ::rtl::OUString m_sText;

        /** knows the current range of the progress. */
        sal_Int32 m_nRange;

        /** knows the current value of the progress. */
        sal_Int32 m_nValue;

    //-------------------------------------------
    // interface

    public:

        DECLARE_XINTERFACE

        /// ctor
        VCLStatusIndicator(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR        ,
                           const css::uno::Reference< css::awt::XWindow >&               xParentWindow);

        /// dtor
        virtual ~VCLStatusIndicator();

        /// XStatusIndicator
        virtual void SAL_CALL start(const ::rtl::OUString& sText ,
                                          sal_Int32        nRange)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL reset()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL end()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setText(const ::rtl::OUString& sText)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL setValue(sal_Int32 nValue)
            throw(css::uno::RuntimeException);

    //-------------------------------------------
    // helper

    private:

        static void impl_recalcLayout(Window* pStatusBar   ,
                                      Window* pParentWindow);
};

} // namespace framework

#endif // __FRAMEWORK_HELPER_VCLSTATUSINDICATOR_HXX_