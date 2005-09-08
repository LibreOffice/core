/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: progressbarwrapper.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:46:01 $
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

#ifndef _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_
#define _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_HELPER_UIELEMENTWRAPPERBASE_HXX_
#include <helper/uielementwrapperbase.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#include <macros/generic.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATOR_HPP_
#include <com/sun/star/task/XStatusIndicator.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

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
