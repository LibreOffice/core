/*************************************************************************
 *
 *  $RCSfile: backingcomp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:50 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_SERVICES_BACKINGCOMP_HXX_
#define __FRAMEWORK_SERVICES_BACKINGCOMP_HXX_

//__________________________________________
// own includes

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//__________________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XKEYLISTENER_HPP_
#include <com/sun/star/awt/XKeyListener.hpp>
#endif

#ifndef _COM_SUN_STAR_FAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORFACTORY_HPP_
#include <com/sun/star/task/XStatusIndicatorFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_DND_XDROPTARGETELISTENER_HPP_
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

//__________________________________________
// other includes

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef SVTOOLS_ASYNCLINK_HXX
#include <svtools/asynclink.hxx>
#endif

//__________________________________________
// definition

namespace framework
{

//__________________________________________
/**
    implements the backing component.

    This component is a special one, which doesn't provide a controller
    nor a model. It supports the following features:
        - Drag & Drop
        - Key Accelerators
        - Simple Menu
        - Progress Bar
        - Background
 */
class BackingComp : public  css::lang::XTypeProvider
                  , public  css::lang::XServiceInfo
                  , public  css::lang::XInitialization
                  , public  css::task::XStatusIndicatorSupplier
                  , public  css::frame::XController  // => XComponent
                  , public  css::awt::XKeyListener // => XEventListener
                  // attention! Must be the first base class to guarentee right initialize lock ...
                  , private ThreadHelpBase
                  , public  ::cppu::OWeakObject
{
    //______________________________________
    // const

        /** static implementation name for instances of this class. */
        static const sal_Char* IMPLEMENTATIONNAME;

        /** static uno service name for instances of this class. */
        static const sal_Char* SERVICENAME;

    //______________________________________
    // member

    private:

        /** the global uno service manager.
            Must be used to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** reference to the component window. */
        css::uno::Reference< css::awt::XWindow > m_xWindow;

        /** the owner frame of this component. */
        css::uno::Reference< css::frame::XFrame > m_xFrame;

        /** used to show a progress for loading documents. */
        css::uno::Reference< css::task::XStatusIndicatorFactory > m_xStatus;

        /** helper for drag&drop. */
        css::uno::Reference< css::datatransfer::dnd::XDropTargetListener > m_xDropTargetListener;

        /** used for asynchronous callbacks within the main thread, to
            unbind e.g. key event or other broadcaster from destroying of the window. :-( */
        ::svtools::AsynchronLink m_aAsyncCallback;

        /** queue of outstanding asynchronous URLs for dispatch. */
        OUStringQueue m_lAsyncQueue;

    //______________________________________
    // interface

    public:

                 BackingComp( const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR );
        virtual ~BackingComp(                                                                    );

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw(css::uno::RuntimeException);
        virtual void          SAL_CALL acquire       (                             ) throw(                          );
        virtual void          SAL_CALL release       (                             ) throw(                          );

        // XTypeProvide
        virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes           () throw(css::uno::RuntimeException);
        virtual css::uno::Sequence< sal_Int8 >       SAL_CALL getImplementationId() throw(css::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString                       SAL_CALL getImplementationName   (                                     ) throw(css::uno::RuntimeException);
        virtual sal_Bool                              SAL_CALL supportsService         ( const ::rtl::OUString& sServiceName ) throw(css::uno::RuntimeException);
        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(                                     ) throw(css::uno::RuntimeException);

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArgs ) throw(css::uno::Exception, css::uno::RuntimeException);

        // XStatusIndicatorSupplier
        virtual css::uno::Reference< css::task::XStatusIndicator > SAL_CALL getStatusIndicator() throw(css::uno::RuntimeException);

        // XController
        virtual void                                      SAL_CALL attachFrame    ( const css::uno::Reference< css::frame::XFrame >& xFrame   ) throw(css::uno::RuntimeException);
        virtual sal_Bool                                  SAL_CALL attachModel    ( const css::uno::Reference< css::frame::XModel >& xModel   ) throw(css::uno::RuntimeException);
        virtual sal_Bool                                  SAL_CALL suspend        (       sal_Bool                                   bSuspend ) throw(css::uno::RuntimeException);
        virtual css::uno::Any                             SAL_CALL getViewData    (                                                           ) throw(css::uno::RuntimeException);
        virtual void                                      SAL_CALL restoreViewData( const css::uno::Any&                             aData    ) throw(css::uno::RuntimeException);
        virtual css::uno::Reference< css::frame::XModel > SAL_CALL getModel       (                                                           ) throw(css::uno::RuntimeException);
        virtual css::uno::Reference< css::frame::XFrame > SAL_CALL getFrame       (                                                           ) throw(css::uno::RuntimeException);

        // XKeyListener
        virtual void SAL_CALL keyPressed ( const css::awt::KeyEvent& aEvent ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL keyReleased( const css::awt::KeyEvent& aEvent ) throw(css::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent ) throw(css::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose            (                                                                   ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL addEventListener   ( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw(css::uno::RuntimeException);

    //______________________________________
    // helper

    public:

        static css::uno::Sequence< ::rtl::OUString >                   SAL_CALL impl_getStaticSupportedServiceNames(                                                                     );
        static ::rtl::OUString                                         SAL_CALL impl_getStaticImplementationName   (                                                                     );
        static css::uno::Reference< css::uno::XInterface >             SAL_CALL impl_createInstance                ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ) throw( css::uno::Exception );
        static css::uno::Reference< css::lang::XSingleServiceFactory > SAL_CALL impl_createFactory                 ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR );

    private:

        void impl_resizeStatusBar();
        DECL_LINK( impl_asyncCallback, void* );
};

} // namespace framework

#endif // __FRAMEWORK_SERVICES_BACKINGCOMP_HXX_
