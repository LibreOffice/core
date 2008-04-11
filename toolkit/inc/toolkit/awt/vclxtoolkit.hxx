/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxtoolkit.hxx,v $
 * $Revision: 1.20 $
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

#ifndef _TOOLKIT_AWT_VCLXTOOLKIT_HXX_
#define _TOOLKIT_AWT_VCLXTOOLKIT_HXX_

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XSystemChildFactory.hpp>
#include <com/sun/star/awt/XToolkit.hpp>
#include <com/sun/star/awt/XDataTransferProviderAccess.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/awt/XReschedule.hpp>
#include <com/sun/star/awt/XMessageBoxFactory.hpp>
#include <cppuhelper/compbase7.hxx>
#include "cppuhelper/interfacecontainer.hxx"
#include <osl/mutex.hxx>
#include <osl/module.h>
#include <tools/link.hxx>
#include <vcl/wintypes.hxx>

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
    class XDataTransfer;
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
                    public cppu::WeakComponentImplHelper7<
                    ::com::sun::star::awt::XToolkit,
                    ::com::sun::star::lang::XServiceInfo,
                    ::com::sun::star::awt::XSystemChildFactory,
                    ::com::sun::star::awt::XMessageBoxFactory,
                    ::com::sun::star::awt::XDataTransferProviderAccess,
                    ::com::sun::star::awt::XExtendedToolkit,
                    ::com::sun::star::awt::XReschedule >
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

    // ::com::sun::star::awt::XToolkit
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  SAL_CALL getDesktopWindow(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::awt::Rectangle                                        SAL_CALL getWorkArea(  ) throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >  SAL_CALL createWindow( const ::com::sun::star::awt::WindowDescriptor& Descriptor ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > > SAL_CALL createWindows( const ::com::sun::star::uno::Sequence< ::com::sun::star::awt::WindowDescriptor >& Descriptors ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XDevice >      SAL_CALL createScreenCompatibleDevice( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
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
