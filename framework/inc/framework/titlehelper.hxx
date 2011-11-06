/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _FRAMEWORK_TITLEHELPER_HXX_
#define _FRAMEWORK_TITLEHELPER_HXX_

//_______________________________________________
// includes

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
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

#include <hash_map>
#include <framework/fwedllapi.h>

//_______________________________________________
// namespace

namespace framework{

#ifdef css
    #error "Ambigious namespace definition of css."
#else
    #define css ::com::sun::star
#endif

//_______________________________________________
// definitions

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
        TitleHelper(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

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

        void impl_updateTitle ();
        void impl_updateTitleForModel (const css::uno::Reference< css::frame::XModel >& xModel);
        void impl_updateTitleForController (const css::uno::Reference< css::frame::XController >& xController);
        void impl_updateTitleForFrame (const css::uno::Reference< css::frame::XFrame >& xFrame);

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
        void impl_appendEvalVersion (::rtl::OUStringBuffer& sTitle);

        void impl_setSubTitle (const css::uno::Reference< css::frame::XTitle >& xSubTitle);
        ::rtl::OUString impl_getSubTitle ();

        ::rtl::OUString impl_convertURL2Title(const ::rtl::OUString& sURL);

    //-------------------------------------------
    // member
    private:

        /** points to the global uno service manager. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

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

#undef css

} // namespace framework

#endif // _FRAMEWORK_TITLEHELPER_HXX_
