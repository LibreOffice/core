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

#ifndef _TOOLKIT_AWT_VCLXTOOLKIT_HXX_
#define _TOOLKIT_AWT_VCLXTOOLKIT_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XToolkitExperimental.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#include <cppuhelper/compbase2.hxx>
#include "cppuhelper/interfacecontainer.hxx"
#include <osl/mutex.hxx>
#include <osl/module.h>
#include <tools/link.hxx>
#include <tools/wintypes.hxx>

#include <toolkit/dllapi.h>

TOOLKIT_DLLPUBLIC WinBits ImplGetWinBits( sal_uInt32 nComponentAttribs, sal_uInt16 nCompType );

class Window;
class VCLXWindow;
class VclSimpleEvent;

namespace com {
namespace sun {
namespace star {
namespace lang {
    struct EventObject;
}
namespace awt {
    struct WindowDescriptor;
} } } }

extern "C" {
    typedef Window* (SAL_CALL *FN_SvtCreateWindow)( VCLXWindow** ppNewComp, const ::com::sun::star::awt::WindowDescriptor* pDescriptor, Window* pParent, WinBits nWinBits );
}


//  ----------------------------------------------------
//  class VCLXTOOLKIT
//  ----------------------------------------------------

class VCLXToolkit_Impl
{
protected:
    ::osl::Mutex    maMutex;
};

class VCLXToolkit : public VCLXToolkit_Impl,
                    public cppu::WeakComponentImplHelper2<
                    ::com::sun::star::awt::XToolkitExperimental,
                    ::com::sun::star::lang::XServiceInfo >
{
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > mxClipboard;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > mxSelection;

    oslModule           hSvToolsLib;
    FN_SvtCreateWindow  fnSvtCreateWindow;

    ::cppu::OInterfaceContainerHelper m_aTopWindowListeners;
    ::cppu::OInterfaceContainerHelper m_aKeyHandlers;
    ::cppu::OInterfaceContainerHelper m_aFocusListeners;
    ::Link m_aEventListenerLink;
    ::Link m_aKeyListenerLink;
    bool m_bEventListener;
    bool m_bKeyListener;

    DECL_LINK(eventListenerHandler, ::VclSimpleEvent const *);

    DECL_LINK(keyListenerHandler, ::VclSimpleEvent const *);

    void callTopWindowListeners(
        ::VclSimpleEvent const * pEvent,
        void (SAL_CALL ::com::sun::star::awt::XTopWindowListener::* pFn)(
            ::com::sun::star::lang::EventObject const &));

    long callKeyHandlers(::VclSimpleEvent const * pEvent, bool bPressed);

    void callFocusListeners(::VclSimpleEvent const * pEvent, bool bGained);

protected:
    ::osl::Mutex&   GetMutex() { return maMutex; }

    virtual void SAL_CALL disposing();

    Window* ImplCreateWindow( VCLXWindow** ppNewComp, const ::com::sun::star::awt::WindowDescriptor& rDescriptor, Window* pParent, WinBits nWinBits );
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > ImplCreateWindow( const ::com::sun::star::awt::WindowDescriptor& Descriptor, WinBits nWinBits );

public:

    VCLXToolkit( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & );
    ~VCLXToolkit();

    // ::com::sun::star::awt::XToolkitExperimental
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >      SAL_CALL createScreenCompatibleDeviceUsingBuffer( sal_Int32 Width, sal_Int32 Height, sal_Int32 ScaleNumerator, sal_Int32 ScaleDenominator, sal_Int32 XOffset, sal_Int32 YOffset, sal_Int64 AddressOfMemoryBufferForSharedArrayWrapper ) throw
(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XToolkit
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  SAL_CALL getDesktopWindow(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Rectangle                                        SAL_CALL getWorkArea(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  SAL_CALL createWindow( const ::com::sun::star::awt::WindowDescriptor& Descriptor ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > > SAL_CALL createWindows( const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::WindowDescriptor >& Descriptors ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >      SAL_CALL createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) throw
(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XRegion >      SAL_CALL createRegion(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XSystemChildFactory
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > SAL_CALL createSystemChild( const ::com::sun::star::uno::Any& Parent, const ::com::sun::star::uno::Sequence< sal_Int8 >& ProcessId, sal_Int16 SystemType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XMessageBoxFactory
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMessageBox > SAL_CALL createMessageBox( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& aParent, const ::com::sun::star::awt::Rectangle& aPosSize, const ::rtl::OUString& aType, ::sal_Int32 aButtons, const ::rtl::OUString& aTitle, const ::rtl::OUString& aMessage ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDataTransfer
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > SAL_CALL getDragGestureRecognizer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& window ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource > SAL_CALL getDragSource( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& window ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget > SAL_CALL getDropTarget( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& window ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > SAL_CALL getClipboard( const ::rtl::OUString& clipboardName ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XExtendedToolkit:

    virtual ::sal_Int32 SAL_CALL getTopWindowCount()
        throw (::com::sun::star::uno::RuntimeException);

    virtual
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow >
    SAL_CALL getTopWindow(::sal_Int32 nIndex)
        throw (::com::sun::star::uno::RuntimeException);

    virtual
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindow >
    SAL_CALL getActiveTopWindow()
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addTopWindowListener(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XTopWindowListener > const & rListener)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeTopWindowListener(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XTopWindowListener > const & rListener)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addKeyHandler(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XKeyHandler > const & rHandler)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeKeyHandler(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XKeyHandler > const & rHandler)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addFocusListener(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener > const & rListener)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeFocusListener(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::awt::XFocusListener > const & rListener)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL fireFocusGained(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > const & source)
        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL fireFocusLost(
        ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XInterface > const & source)
        throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XReschedule:
    virtual void SAL_CALL reschedule()
        throw (::com::sun::star::uno::RuntimeException);


};

#endif // _TOOLKIT_AWT_VCLXTOOLKIT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
