/*************************************************************************
 *
 *  $RCSfile: persistentwindowstate.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-12-03 14:04:13 $
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

#ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_
#define __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_DEBUG_HXX_
#include <macros/debug.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XFRAMEACTIONLISTENER_HPP_
#include <com/sun/star/frame/XFrameActionListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          listener for closing document frames to make her window state persistent
    @descr          It's a feature of our office. If a document window was created by ourself (and not from
                    any external process e.g. the office bean) we save and restore the window state of it
                    corresponding to the document service factory. That means: one instance of this class will be
                    a listener on one frame which container window was created by ourself.
                    We listen for frame action events and everytimes a component will deattached from a frame
                    we store its current position and size to the configuration. Everytimes a new component is
                    attached to a frame first time(!) we restore this informations again.

    @base           ThreadHelpBase
                        guarantee right initialized lock member during startup of instances of this class.

    @base           OWeakObject
                        implements ref counting for this class.

    @devstatus      ready
    @threadsafe     yes
    @modified       06.08.2004 08:41, as96863
*//*-*************************************************************************************************************/
class PersistentWindowState :   // interfaces
                                public css::lang::XTypeProvider,
                                public css::lang::XInitialization,
                                public css::frame::XFrameActionListener, // => XEventListener
                                // baseclasses (order neccessary for right initialization!)
                                private ThreadHelpBase,
                                public  ::cppu::OWeakObject
{
    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// reference to the frame which was created by the office himself
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// we call SetWindowState one times only for the same frame!
        sal_Bool m_bWindowStateAlreadySet;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 PersistentWindowState(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~PersistentWindowState(                                                                   );

        //____________________________
        // XInterface, XTypeProvider
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //____________________________
        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        //____________________________
        // XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent)
            throw(css::uno::RuntimeException);

        //____________________________
        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    //________________________________
    // helper

    private:
        //____________________________
        /** @short  identify the application module, which  is used behind the component
                    of our frame.

            @param  xSMGR
                    needed to create needed uno resources.

            @param  xFrame
                    contains the component, wich must be identified.

            @return [string]
                    a module identifier for the current frame component.
         */
        static ::rtl::OUString implst_identifyModule(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR ,
                                                     const css::uno::Reference< css::frame::XFrame >&              xFrame);

        //____________________________
        /** @short  retrieve the window state from the configuration.

            @param  xSMGR
                    needed to create the configuration access.

            @param  sModuleName
                    identifies the application module, where the
                    information should be getted for.

            @return [string]
                    contains the information about position and size.
         */
        static ::rtl::OUString implst_getWindowStateFromConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR      ,
                                                               const ::rtl::OUString&                                        sModuleName);

        //____________________________
        /** @short  retrieve the window state from the container window.

            @param  xWindow
                    must point to the container window of the frame.
                    We use it VCL part here - because the toolkit doesnt
                    provide the right functionality!

            @return [string]
                    contains the information about position and size.
         */
        static ::rtl::OUString implst_getWindowStateFromWindow(const css::uno::Reference< css::awt::XWindow >& xWindow);

        //____________________________
        /** @short  restore the position and size on the container window.

            @param  xSMGR
                    needed to create the configuration access.

            @param  sModuleName
                    identifies the application module, where the
                    information should be setted on.

            @param  sWindowState
                    contains the information about position and size.
         */
        static void implst_setWindowStateOnConfig(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR         ,
                                                  const ::rtl::OUString&                                        sModuleName   ,
                                                  const ::rtl::OUString&                                        sWindowState  );

        //____________________________
        /** @short  restore the position and size on the container window.

            @param  xWindow
                    must point to the container window of the frame.
                    We use it VCL part here - because the toolkit doesnt
                    provide the right functionality!

            @param  sWindowState
                    contains the information about position and size.
         */
        static void implst_setWindowStateOnWindow(const css::uno::Reference< css::awt::XWindow >& xWindow     ,
                                                  const ::rtl::OUString&                          sWindowState);

}; // class PersistentWindowState

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_
