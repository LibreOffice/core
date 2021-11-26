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

#include <sal/config.h>

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>

#include <factory.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/LokClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardListener.hpp>
#include <com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace vcl
{
namespace {

// generic implementation to satisfy SalInstance
class GenericClipboard :
        public cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        XServiceInfo
        >
{
    osl::Mutex                                                              m_aMutex;
    Reference< css::datatransfer::XTransferable >                           m_aContents;
    Reference< css::datatransfer::clipboard::XClipboardOwner >              m_aOwner;
    std::vector< Reference< css::datatransfer::clipboard::XClipboardListener > > m_aListeners;

public:

    GenericClipboard() : cppu::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        XServiceInfo
        >( m_aMutex )
    {}

    /*
     * XServiceInfo
     */

    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    static Sequence< OUString > getSupportedServiceNames_static();

    /*
     * XClipboard
     */

    virtual Reference< css::datatransfer::XTransferable > SAL_CALL getContents() override;

    virtual void SAL_CALL setContents(
        const Reference< css::datatransfer::XTransferable >& xTrans,
        const Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner ) override;

    virtual OUString SAL_CALL getName() override;

    /*
     * XClipboardEx
     */

    virtual sal_Int8 SAL_CALL getRenderingCapabilities() override;

    /*
     * XClipboardNotifier
     */
    virtual void SAL_CALL addClipboardListener(
        const Reference< css::datatransfer::clipboard::XClipboardListener >& listener ) override;

    virtual void SAL_CALL removeClipboardListener(
        const Reference< css::datatransfer::clipboard::XClipboardListener >& listener ) override;
};

}

Sequence< OUString > GenericClipboard::getSupportedServiceNames_static()
{
    Sequence< OUString > aRet { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
    return aRet;
}

OUString GenericClipboard::getImplementationName()
{
    return "com.sun.star.datatransfer.VCLGenericClipboard";
}

Sequence< OUString > GenericClipboard::getSupportedServiceNames()
{
    return getSupportedServiceNames_static();
}

sal_Bool GenericClipboard::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Reference< css::datatransfer::XTransferable > GenericClipboard::getContents()
{
    return m_aContents;
}

void GenericClipboard::setContents(
        const Reference< css::datatransfer::XTransferable >& xTrans,
        const Reference< css::datatransfer::clipboard::XClipboardOwner >& xClipboardOwner )
{
    osl::ClearableMutexGuard aGuard( m_aMutex );
    Reference< datatransfer::clipboard::XClipboardOwner > xOldOwner( m_aOwner );
    Reference< datatransfer::XTransferable > xOldContents( m_aContents );
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    std::vector< Reference< datatransfer::clipboard::XClipboardListener > > aListeners( m_aListeners );
    datatransfer::clipboard::ClipboardEvent aEv;
    aEv.Contents = m_aContents;

    aGuard.clear();

    if( xOldOwner.is() && xOldOwner != xClipboardOwner )
        xOldOwner->lostOwnership( this, xOldContents );
    for (auto const& listener : aListeners)
    {
        listener->changedContents( aEv );
    }
}

OUString GenericClipboard::getName()
{
    return "CLIPBOARD";
}

sal_Int8 GenericClipboard::getRenderingCapabilities()
{
    return 0;
}

void GenericClipboard::addClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
{
    osl::MutexGuard aGuard(m_aMutex);

    m_aListeners.push_back( listener );
}

void GenericClipboard::removeClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
{
    osl::MutexGuard aGuard(m_aMutex);

    m_aListeners.erase(std::remove(m_aListeners.begin(), m_aListeners.end(), listener), m_aListeners.end());
}



extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
vcl_SystemClipboard_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const& args)
{
    SolarMutexGuard aGuard;
    auto xClipboard = ImplGetSVData()->mpDefInst->CreateClipboard( args );
    if (xClipboard.is())
        xClipboard->acquire();
    return xClipboard.get();
}

namespace {

/*
*   generic DragSource dummy
*/
class GenericDragSource : public cppu::WeakComponentImplHelper<
            datatransfer::dnd::XDragSource,
            XInitialization,
            css::lang::XServiceInfo
            >
{
    osl::Mutex                          m_aMutex;
    css::uno::Reference<css::datatransfer::XTransferable> m_xTrans;
public:
    GenericDragSource() : WeakComponentImplHelper( m_aMutex ) {}

    // XDragSource
    virtual sal_Bool    SAL_CALL isDragImageSupported() override;
    virtual sal_Int32   SAL_CALL getDefaultCursor( sal_Int8 dragAction ) override;
    virtual void        SAL_CALL startDrag(
                                     const datatransfer::dnd::DragGestureEvent& trigger,
                                     sal_Int8 sourceActions, sal_Int32 cursor, sal_Int32 image,
                                     const Reference< datatransfer::XTransferable >& transferable,
                                     const Reference< datatransfer::dnd::XDragSourceListener >& listener
                                     ) override;

    // XInitialization
    virtual void        SAL_CALL initialize( const Sequence< Any >& arguments ) override;

    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.datatransfer.dnd.VclGenericDragSource"; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return getSupportedServiceNames_static(); }

    static Sequence< OUString > getSupportedServiceNames_static()
    {
       return { "com.sun.star.datatransfer.dnd.GenericDragSource" };
    }
};

}

sal_Bool GenericDragSource::isDragImageSupported()
{
    return false;
}

sal_Int32 GenericDragSource::getDefaultCursor( sal_Int8 )
{
    return 0;
}

void GenericDragSource::startDrag( const datatransfer::dnd::DragGestureEvent&,
                                   sal_Int8 /*sourceActions*/, sal_Int32 /*cursor*/, sal_Int32 /*image*/,
                                   const Reference< datatransfer::XTransferable >& rTrans,
                                   const Reference< datatransfer::dnd::XDragSourceListener >& listener
                                   )
{
    if (comphelper::LibreOfficeKit::isActive()) {
        m_xTrans = rTrans;
        return;
    }

    datatransfer::dnd::DragSourceDropEvent aEv;
    aEv.DropAction = datatransfer::dnd::DNDConstants::ACTION_COPY;
    aEv.DropSuccess = false;
    listener->dragDropEnd( aEv );
}

void GenericDragSource::initialize( const Sequence< Any >& )
{
}

Sequence< OUString > DragSource_getSupportedServiceNames()
{
#if defined MACOSX
    return { "com.sun.star.datatransfer.dnd.OleDragSource" };
#elif defined UNX
    return { "com.sun.star.datatransfer.dnd.X11DragSource" };
#else
    return { "com.sun.star.datatransfer.dnd.VclGenericDragSource" };
#endif
}

OUString DragSource_getImplementationName()
{
#if defined MACOSX
    return "com.sun.star.comp.datatransfer.dnd.OleDragSource_V1";
#elif defined UNX
    return "com.sun.star.datatransfer.dnd.XdndSupport";
#else
    return "com.sun.star.datatransfer.dnd.VclGenericDragSource";
#endif
}

Reference< XInterface > DragSource_createInstance( const Reference< XMultiServiceFactory >&  )
{
    SolarMutexGuard aGuard;
    Reference< XInterface > xResult = ImplGetSVData()->mpDefInst->CreateDragSource();
    return xResult;
}

/*
*   generic DragSource dummy
*/

namespace {

class GenericDropTarget : public cppu::WeakComponentImplHelper<
                                           datatransfer::dnd::XDropTarget,
                                           XInitialization,
                                           css::lang::XServiceInfo
                                           >
{
    osl::Mutex m_aMutex;
public:
    GenericDropTarget() : WeakComponentImplHelper( m_aMutex )
    {}

    // XInitialization
    virtual void        SAL_CALL initialize( const Sequence< Any >& args ) override;

    // XDropTarget
    virtual void        SAL_CALL addDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& ) override;
    virtual void        SAL_CALL removeDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& ) override;
    virtual sal_Bool    SAL_CALL isActive() override;
    virtual void        SAL_CALL setActive( sal_Bool active ) override;
    virtual sal_Int8    SAL_CALL getDefaultActions() override;
    virtual void        SAL_CALL setDefaultActions( sal_Int8 actions ) override;

    OUString SAL_CALL getImplementationName() override
    { return "com.sun.star.datatransfer.dnd.VclGenericDropTarget"; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return getSupportedServiceNames_static(); }

    static Sequence< OUString > getSupportedServiceNames_static()
    {
      return { "com.sun.star.datatransfer.dnd.GenericDropTarget" };
    }
};

}

void GenericDropTarget::initialize( const Sequence< Any >& )
{
}

void GenericDropTarget::addDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& )
{
}

void GenericDropTarget::removeDropTargetListener( const Reference< css::datatransfer::dnd::XDropTargetListener >& )
{
}

sal_Bool GenericDropTarget::isActive()
{
    return false;
}

void GenericDropTarget::setActive( sal_Bool )
{
}

sal_Int8 GenericDropTarget::getDefaultActions()
{
    return 0;
}

void GenericDropTarget::setDefaultActions( sal_Int8)
{
}

Sequence< OUString > DropTarget_getSupportedServiceNames()
{
#if defined MACOSX
    return {  "com.sun.star.datatransfer.dnd.OleDropTarget" };
#elif defined UNX
    return { "com.sun.star.datatransfer.dnd.X11DropTarget" };
#else
    return GenericDropTarget::getSupportedServiceNames_static();
#endif
}

OUString DropTarget_getImplementationName()
{
    return
    #if defined MACOSX
    "com.sun.star.comp.datatransfer.dnd.OleDropTarget_V1"
    #elif defined UNX
    "com.sun.star.datatransfer.dnd.XdndDropTarget"
    #else
    "com.sun.star.datatransfer.dnd.VclGenericDropTarget"
    #endif
                   ;
}

Reference< XInterface > DropTarget_createInstance( const Reference< XMultiServiceFactory >&  )
{
    SolarMutexGuard aGuard;
    Reference< XInterface > xResult = ImplGetSVData()->mpDefInst->CreateDropTarget();
    return xResult;
}

} // namespace vcl

/*
*   SalInstance generic
*/
Reference< XInterface > SalInstance::CreateClipboard( const Sequence< Any >& arguments )
{
    if (arguments.hasElements()) {
        throw css::lang::IllegalArgumentException(
            "non-empty SalInstance::CreateClipboard arguments", {}, -1);
    }
    if (comphelper::LibreOfficeKit::isActive()) {
        // In LOK, each document view shall have its own clipboard instance (whereas
        // in non-LOK below we keep handing out one single instance; see also
        // <https://lists.freedesktop.org/archives/libreoffice/2020-April/084824.html> "Re: Linux
        // SAL_USE_VCLPLUGIN=svp and the clipboard"):
        css::uno::Reference<css::datatransfer::clipboard::XClipboard> xClipboard =
            css::datatransfer::clipboard::LokClipboard::create(
                comphelper::getProcessComponentContext());
        return xClipboard;
    }
    DBG_TESTSOLARMUTEX();
    if (!m_clipboard.is()) {
        m_clipboard = static_cast<cppu::OWeakObject *>(new vcl::GenericClipboard());
    }
    return m_clipboard;
}

Reference< XInterface > SalInstance::CreateDragSource()
{
    return Reference< XInterface >( static_cast<cppu::OWeakObject *>(new vcl::GenericDragSource()) );
}

Reference< XInterface > SalInstance::CreateDropTarget()
{
    return Reference< XInterface >( static_cast<cppu::OWeakObject *>(new vcl::GenericDropTarget()) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
