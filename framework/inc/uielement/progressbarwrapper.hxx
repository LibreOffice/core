/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: progressbarwrapper.hxx,v $
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

#ifndef _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_
#define _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <helper/uielementwrapperbase.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/debug.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/awt/XWindow.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakref.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

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
