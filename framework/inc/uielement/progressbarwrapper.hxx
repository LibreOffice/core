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

#ifndef _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_
#define _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_

#include <helper/uielementwrapperbase.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/awt/XWindow.hpp>

#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>

namespace framework{

class ProgressBarWrapper : public UIElementWrapperBase
{
    public:
        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------
        ProgressBarWrapper();
        virtual ~ProgressBarWrapper();

        // public interfaces
        void setStatusBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& rStatusBar, sal_Bool bOwnsInstance = sal_False );
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > getStatusBar() const;

        // wrapped methods of ::com::sun::star::task::XStatusIndicator
        void start( const ::rtl::OUString& Text, ::sal_Int32 Range ) throw (::com::sun::star::uno::RuntimeException);
        void end() throw (::com::sun::star::uno::RuntimeException);
        void setText( const ::rtl::OUString& Text ) throw (::com::sun::star::uno::RuntimeException);
        void setValue( ::sal_Int32 Value ) throw (::com::sun::star::uno::RuntimeException);
        void reset() throw (::com::sun::star::uno::RuntimeException);

        // UNO interfaces
        // XComponent
        virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // XUpdatable
        virtual void SAL_CALL update() throw (::com::sun::star::uno::RuntimeException);

        // XUIElement
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getRealInterface() throw (::com::sun::star::uno::RuntimeException);

    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >         m_xStatusBar;    // Reference to our status bar XWindow
        ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface >  m_xProgressBarIfacWrapper;
        sal_Bool                                                                   m_bOwnsInstance; // Indicator that we are owner of the XWindow
        sal_Int32                                                                  m_nRange;
        sal_Int32                                                                  m_nValue;
        rtl::OUString                                                              m_aText;
};      //  class ProgressBarWrapper

}       //  namespace framework

#endif // _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
