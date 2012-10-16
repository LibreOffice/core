/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _FRAMEWORK_TITLEHELPER_HXX_
#define _FRAMEWORK_TITLEHELPER_HXX_

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/frame/XUntitledNumbers.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/document/XEventListener.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <rtl/ustrbuf.hxx>

#include <boost/unordered_map.hpp>
#include <framework/fwedllapi.h>


namespace css = ::com::sun::star;

namespace framework{


/** @short  can be used as implementation helper of interface css.frame.XTitle

    @threadsafe
 */
class FWE_DLLPUBLIC TitleHelper : private ::cppu::BaseMutex
                  , public  ::cppu::WeakImplHelper5< css::frame::XTitle                 ,
                                                     css::frame::XTitleChangeBroadcaster,
                                                     css::frame::XTitleChangeListener   ,
                                                     css::frame::XFrameActionListener   ,
                                                     css::document::XEventListener      >
{
    //-------------------------------------------
    // interface
    public:

        //---------------------------------------
        /** @short  lightweight constructor.
         */
        TitleHelper(const css::uno::Reference< css::uno::XComponentContext >& rxContext);

        //---------------------------------------
        /** @short  free all internaly used resources.
         */
        virtual ~TitleHelper();

        //---------------------------------------
        /** set an outside component which uses this container and must be set
            as source of all broadcasted messages, exceptions.

            It's holded weak only so we do not need any complex dispose sessions.

            Note: Passing NULL as parameter will be alloed. It will reset the internal
            member reference only.

            @param  xOwner
                    the new owner of this collection.
         */
        void setOwner (const css::uno::Reference< css::uno::XInterface >& xOwner);

        //---------------------------------------
        /** set an outside component which provides the righht string and number for
            an untitled component.

            It's holded weak only so we do not need any complex dispose sessions.

            Note: Passing NULL as parameter will be alloed. It will reset the internal
            member reference only.

            @param  xNumbers
                    the right numbered collection for this helper.
         */
        void connectWithUntitledNumbers (const css::uno::Reference< css::frame::XUntitledNumbers >& xNumbers);

        //---------------------------------------
        /** @see XTitle */
        virtual ::rtl::OUString SAL_CALL getTitle()
            throw (css::uno::RuntimeException);

        //---------------------------------------
        /** @see XTitle */
        virtual void SAL_CALL setTitle(const ::rtl::OUString& sTitle)
            throw (css::uno::RuntimeException);

        //---------------------------------------
        /** @see XTitleChangeBroadcaster */
        virtual void SAL_CALL addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
            throw (css::uno::RuntimeException);

        //---------------------------------------
        /** @see XTitleChangeBroadcaster */
        virtual void SAL_CALL removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener)
            throw (css::uno::RuntimeException);

        //---------------------------------------
        /** @see XTitleChangeListener */
        virtual void SAL_CALL titleChanged(const css::frame::TitleChangedEvent& aEvent)
            throw (css::uno::RuntimeException);

        //---------------------------------------
        /** @see css.document.XEventListener */
        virtual void SAL_CALL notifyEvent(const css::document::EventObject& aEvent)
            throw (css::uno::RuntimeException);

        //---------------------------------------
        /** @see css.lang.XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw (css::uno::RuntimeException);

        //---------------------------------------
        /** @see css.frame.XFrameActionListener */
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent)
            throw(css::uno::RuntimeException);

    //-------------------------------------------
    // internal
    private:

        void impl_sendTitleChangedEvent ();

        void impl_updateTitle (bool init = false);
        void impl_updateTitleForModel (const css::uno::Reference< css::frame::XModel >& xModel, bool init);
        void impl_updateTitleForController (const css::uno::Reference< css::frame::XController >& xController, bool init);
        void impl_updateTitleForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame, bool init);

        void impl_startListeningForModel (const css::uno::Reference< css::frame::XModel >& xModel);
        void impl_startListeningForController (const css::uno::Reference< css::frame::XController >& xController);
        void impl_startListeningForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame);
        void impl_updateListeningForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame);

        void impl_appendComponentTitle (      ::rtl::OUStringBuffer&                       sTitle    ,
                                        const css::uno::Reference< css::uno::XInterface >& xComponent);
        void impl_appendProductName (::rtl::OUStringBuffer& sTitle);
        void impl_appendProductExtension (::rtl::OUStringBuffer& sTitle);
        void impl_appendModuleName (::rtl::OUStringBuffer& sTitle);
        void impl_appendDebugVersion (::rtl::OUStringBuffer& sTitle);

        void impl_setSubTitle (const css::uno::Reference< css::frame::XTitle >& xSubTitle);

        ::rtl::OUString impl_convertURL2Title(const ::rtl::OUString& sURL);

    //-------------------------------------------
    // member
    private:

        /** points to the global uno service manager. */
        css::uno::Reference< css::uno::XComponentContext> m_xContext;

        /** reference to the outside UNO class using this helper. */
        css::uno::WeakReference< css::uno::XInterface > m_xOwner;

        /** used to know how an "Untitled X" string can be created right :-) */
        css::uno::WeakReference< css::frame::XUntitledNumbers > m_xUntitledNumbers;

        /** provides parts of our own title and we listen there for changes too. */
        css::uno::WeakReference< css::frame::XTitle > m_xSubTitle;

        /** if it's set to sal_True the member m_sTitle has not to be changed internaly.
            It was set from outside and so outside code has to make sure it will be
            updated.
         */
        ::sal_Bool m_bExternalTitle;

        /** the actual title value */
        ::rtl::OUString m_sTitle;

        /** knows the leased number which must be used for untitled components. */
        ::sal_Int32 m_nLeasedNumber;

        /** contains all title change listener */
        ::cppu::OMultiTypeInterfaceContainerHelper m_aListener;
};

} // namespace framework

#endif // _FRAMEWORK_TITLEHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
