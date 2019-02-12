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

#ifndef INCLUDED_FRAMEWORK_TITLEHELPER_HXX
#define INCLUDED_FRAMEWORK_TITLEHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/XTitleChangeBroadcaster.hpp>
#include <com/sun/star/frame/XFrameActionListener.hpp>
#include <com/sun/star/document/XDocumentEventListener.hpp>

#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/weakref.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <rtl/ustrbuf.hxx>

#include <framework/fwedllapi.h>

namespace com::sun::star::frame { class XController; }
namespace com::sun::star::frame { class XFrame; }
namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::frame { class XUntitledNumbers; }
namespace com::sun::star::uno { class XComponentContext; }
namespace com::sun::star::uno { class XInterface; }


namespace framework{


/** @short  can be used as implementation helper of interface css.frame.XTitle

    @threadsafe
 */
class FWE_DLLPUBLIC TitleHelper : private ::cppu::BaseMutex
                  , public  ::cppu::WeakImplHelper< css::frame::XTitle                 ,
                                                     css::frame::XTitleChangeBroadcaster,
                                                     css::frame::XTitleChangeListener   ,
                                                     css::frame::XFrameActionListener   ,
                                                     css::document::XDocumentEventListener >
{

    // interface
    public:


        /** @short  lightweight constructor.
         */
        TitleHelper(const css::uno::Reference< css::uno::XComponentContext >& rxContext);


        /** @short  free all internally used resources.
         */
        virtual ~TitleHelper() override;


        /** set an outside component which uses this container and must be set
            as source of all broadcasted messages, exceptions.

            It's holded weak only so we do not need any complex dispose sessions.

            Note: Passing NULL as parameter will be allowed. It will reset the internal
            member reference only.

            @param  xOwner
                    the new owner of this collection.
         */
        void setOwner (const css::uno::Reference< css::uno::XInterface >& xOwner);


        /** set an outside component which provides the right string and number for
            an untitled component.

            It's holded weak only so we do not need any complex dispose sessions.

            Note: Passing NULL as parameter will be allowed. It will reset the internal
            member reference only.

            @param  xNumbers
                    the right numbered collection for this helper.
         */
        void connectWithUntitledNumbers (const css::uno::Reference< css::frame::XUntitledNumbers >& xNumbers);


        /** @see XTitle */
        virtual OUString SAL_CALL getTitle() override;


        /** @see XTitle */
        virtual void SAL_CALL setTitle(const OUString& sTitle) override;


        /** @see XTitleChangeBroadcaster */
        virtual void SAL_CALL addTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener) override;


        /** @see XTitleChangeBroadcaster */
        virtual void SAL_CALL removeTitleChangeListener(const css::uno::Reference< css::frame::XTitleChangeListener >& xListener) override;


        /** @see XTitleChangeListener */
        virtual void SAL_CALL titleChanged(const css::frame::TitleChangedEvent& aEvent) override;


        /** @see css.document.XDocumentEventListener */
        virtual void SAL_CALL documentEventOccured(const css::document::DocumentEvent& aEvent) override;


        /** @see css.lang.XEventListener */
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;


        /** @see css.frame.XFrameActionListener */
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent) override;


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

        void impl_appendComponentTitle (      OUStringBuffer&                       sTitle    ,
                                        const css::uno::Reference< css::uno::XInterface >& xComponent);
        void impl_appendProductName (OUStringBuffer& sTitle);
        void impl_appendModuleName (OUStringBuffer& sTitle);
        void impl_appendDebugVersion (OUStringBuffer& sTitle);
        void impl_appendSafeMode (OUStringBuffer& sTitle);

        void impl_setSubTitle (const css::uno::Reference< css::frame::XTitle >& xSubTitle);

        OUString impl_convertURL2Title(const OUString& sURL);


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

        /** if it's set to sal_True the member m_sTitle has not to be changed internally.
            It was set from outside and so outside code has to make sure it will be
            updated.
         */
        bool m_bExternalTitle;

        /** the actual title value */
        OUString m_sTitle;

        /** knows the leased number which must be used for untitled components. */
        ::sal_Int32 m_nLeasedNumber;

        /** contains all title change listener */
        ::cppu::OMultiTypeInterfaceContainerHelper m_aListener;
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_TITLEHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
