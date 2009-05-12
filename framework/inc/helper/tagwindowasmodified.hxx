/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tagwindowasmodified.hxx,v $
 * $Revision: 1.3 $
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

#ifndef __FRAMEWORK_HELPER_TAGWINDOWASMODIFIED_HXX_
#define __FRAMEWORK_HELPER_TAGWINDOWASMODIFIED_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <threadhelp/threadhelpbase.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <general.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/weak.hxx>

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
    @short          listen for modify events on model and tag frame container window so it can react accordingly
    @descr          Used e.g. by our MAC port where such state is shown seperately on some controls of the
                    title bar.

    @base           ThreadHelpBase
                        guarantee right initialized lock member during startup of instances of this class.

    @base           OWeakObject
                        implements ref counting for this class.

    @devstatus      draft
    @threadsafe     yes
    @modified       as96863
*//*-*************************************************************************************************************/
class TagWindowAsModified :   // interfaces
                              public css::lang::XTypeProvider,
                              public css::lang::XInitialization,
                              public css::frame::XFrameActionListener, // => XEventListener
                              public css::util::XModifyListener,       // => XEventListener
                              // baseclasses (order neccessary for right initialization!)
                              private ThreadHelpBase,
                              public  ::cppu::OWeakObject
{
    //________________________________
    // member

    private:

        /// may we need an uno service manager to create own services
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /// reference to the frame, where we listen for new loaded documents for updating our own xModel reference
        css::uno::WeakReference< css::frame::XFrame > m_xFrame;

        /// reference to the frame container window, where we must set the tag
        css::uno::WeakReference< css::awt::XWindow > m_xWindow;

        /// we list on the model for modify events
        css::uno::WeakReference< css::frame::XModel > m_xModel;

    //________________________________
    // interface

    public:

        //____________________________
        // ctor/dtor
                 TagWindowAsModified(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
        virtual ~TagWindowAsModified(                                                                   );

        //____________________________
        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //____________________________
        // XInitialization
        virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any >& lArguments)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        //____________________________
        // XModifyListener
        virtual void SAL_CALL modified(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

        //____________________________
        // XFrameActionListener
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent)
            throw(css::uno::RuntimeException);

        //____________________________
        // XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

    private:

        //____________________________
        // @todo document me
        void impl_update(const css::uno::Reference< css::frame::XFrame >& xFrame);

}; // class TagWindowAsModified

} // namespace framework

#endif // #ifndef __FRAMEWORK_HELPER_TAGWINDOWASMODIFIED_HXX_
