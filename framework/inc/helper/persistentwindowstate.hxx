/*************************************************************************
 *
 *  $RCSfile: persistentwindowstate.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2002-07-29 08:15:40 $
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

#ifndef _COM_SUN_STAR_UTIL_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_CLOSEVETOEXCEPTION_HPP_
#include <com/sun/star/util/CloseVetoException.hpp>
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
                    a listener on one frame which container window was created by ourself. In case we get
                    the callback "queryClosing()" we try to find out which application module is used for this document
                    and which state the container window has. During callback "notifyClosing()" we use this informations
                    then to make it persistent inside the configuration package "org.openoffice.Setup".
                    We are used for restoring such data too. The class TaskCreator inside the framework will
                    initialize us with the frame, on which we must listen till it dies.
                    So we can react for loading the first document in it and restore the data for this application module,
                    which is represented by the new document ... or can make it persistent for this module again
                    during closing the frame.

    @base           ThreadHelpBase
                        guarantee right initialized lock member during startup of instances of this class

    @devstatus      draft
    @threadsafe     yes
    @modified       02.07.2002 09:51, as96863
*//*-*************************************************************************************************************/
class PersistentWindowState :   // interfaces
                                public css::lang::XTypeProvider,
                                public css::lang::XInitialization,
                                public css::frame::XFrameActionListener, // => XEventListener
                                public css::util::XCloseListener,        // => XEventListener
                                // baseclasses (order neccessary for right initialization!)
                                private ThreadHelpBase,
                                public  ::cppu::OWeakObject
{
    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;
        /// reference to the frame which was created by the office himself
        css::uno::Reference< css::frame::XFrame > m_xFrame;
        /// we get the window state inside queryClosing() but save it inside notifyClosing(). This member hold it alive between these two calls.
        ::rtl::OUString m_sWindowState;
        /// to make the state persistent it's neccessary to know the document factory
        SvtModuleOptions::EFactory m_eFactory;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 PersistentWindowState( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~PersistentWindowState(                                                                        );

        //____________________________
        // XInterface, XTypeProvider
        DECLARE_XINTERFACE
        DECLARE_XTYPEPROVIDER

        //____________________________
        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& lArguments ) throw (css::uno::Exception       ,
                                                                                                         css::uno::RuntimeException);

        //____________________________
        // XFrameActionListener
        virtual void SAL_CALL frameAction( const css::frame::FrameActionEvent& aEvent ) throw (css::uno::RuntimeException);

        //____________________________
        // XCloseListener
        virtual void SAL_CALL queryClosing ( const css::lang::EventObject& aSource        ,
                                                   sal_Bool                bGetsOwnership ) throw (css::util::CloseVetoException,
                                                                                                   css::uno::RuntimeException   );
        virtual void SAL_CALL notifyClosing( const css::lang::EventObject& aSource        ) throw (css::uno::RuntimeException   );

        //____________________________
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& aSource ) throw (css::uno::RuntimeException);

    //________________________________
    // helper

    private:
        static sal_Bool        implst_getFrameProps ( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory         ,
                                                      const css::uno::Reference< css::frame::XFrame >&              xFrame           ,
                                                            SvtModuleOptions::EFactory*                             pModule          ,
                                                            css::uno::Reference< css::awt::XWindow >*               pContainerWindow );
        static ::rtl::OUString implst_getWindowState( const css::uno::Reference< css::awt::XWindow >&               xWindow          );
        static void            implst_setWindowState( const css::uno::Reference< css::awt::XWindow >&               xWindow          ,
                                                      const ::rtl::OUString&                                        sWindowState     );

}; // class PersistentWindowState

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_PERSISTENTWINDOWSTATE_HXX_
