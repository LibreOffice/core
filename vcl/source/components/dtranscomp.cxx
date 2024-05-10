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

#include <comphelper/compbase.hxx>
#include <cppuhelper/supportsservice.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace vcl
{
namespace {

// generic implementation to satisfy SalInstance
class GenericClipboard :
        public comphelper::WeakComponentImplHelper<
        datatransfer::clipboard::XSystemClipboard,
        XServiceInfo
        >
{
    Reference< css::datatransfer::XTransferable >                           m_aContents;
    Reference< css::datatransfer::clipboard::XClipboardOwner >              m_aOwner;
    std::vector< Reference< css::datatransfer::clipboard::XClipboardListener > > m_aListeners;

public:

    GenericClipboard()
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
    Sequence< OUString > aRet { u"com.sun.star.datatransfer.clipboard.SystemClipboard"_ustr };
    return aRet;
}

OUString GenericClipboard::getImplementationName()
{
    return u"com.sun.star.datatransfer.VCLGenericClipboard"_ustr;
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
    std::unique_lock aGuard( m_aMutex );
    Reference< datatransfer::clipboard::XClipboardOwner > xOldOwner( m_aOwner );
    Reference< datatransfer::XTransferable > xOldContents( m_aContents );
    m_aContents = xTrans;
    m_aOwner = xClipboardOwner;

    std::vector< Reference< datatransfer::clipboard::XClipboardListener > > aListeners( m_aListeners );
    datatransfer::clipboard::ClipboardEvent aEv;
    aEv.Contents = m_aContents;

    aGuard.unlock();

    if( xOldOwner.is() && xOldOwner != xClipboardOwner )
        xOldOwner->lostOwnership( this, xOldContents );
    for (auto const& listener : aListeners)
    {
        listener->changedContents( aEv );
    }
}

OUString GenericClipboard::getName()
{
    return u"CLIPBOARD"_ustr;
}

sal_Int8 GenericClipboard::getRenderingCapabilities()
{
    return 0;
}

void GenericClipboard::addClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
{
    std::unique_lock aGuard(m_aMutex);

    m_aListeners.push_back( listener );
}

void GenericClipboard::removeClipboardListener( const Reference< datatransfer::clipboard::XClipboardListener >& listener )
{
    std::unique_lock aGuard(m_aMutex);

    std::erase(m_aListeners, listener);
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
class GenericDragSource : public ::comphelper::WeakComponentImplHelper<
            datatransfer::dnd::XDragSource,
            XInitialization,
            css::lang::XServiceInfo
            >
{
    css::uno::Reference<css::datatransfer::XTransferable> m_xTrans;
public:
    GenericDragSource() {}

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
    { return u"com.sun.star.datatransfer.dnd.VclGenericDragSource"_ustr; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return getSupportedServiceNames_static(); }

    static Sequence< OUString > getSupportedServiceNames_static()
    {
       return { u"com.sun.star.datatransfer.dnd.GenericDragSource"_ustr };
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
    return { u"com.sun.star.datatransfer.dnd.X11DragSource"_ustr };
#else
    return { "com.sun.star.datatransfer.dnd.VclGenericDragSource" };
#endif
}

OUString DragSource_getImplementationName()
{
#if defined MACOSX
    return "com.sun.star.comp.datatransfer.dnd.OleDragSource_V1";
#elif defined UNX
    return u"com.sun.star.datatransfer.dnd.XdndSupport"_ustr;
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

class GenericDropTarget : public comphelper::WeakComponentImplHelper<
                                           datatransfer::dnd::XDropTarget,
                                           XInitialization,
                                           css::lang::XServiceInfo
                                           >
{
public:
    GenericDropTarget() {}

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
    { return u"com.sun.star.datatransfer.dnd.VclGenericDropTarget"_ustr; }

    sal_Bool SAL_CALL supportsService(OUString const & ServiceName) override
    { return cppu::supportsService(this, ServiceName); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    { return getSupportedServiceNames_static(); }

    static Sequence< OUString > getSupportedServiceNames_static()
    {
      return { u"com.sun.star.datatransfer.dnd.GenericDropTarget"_ustr };
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
    return { u"com.sun.star.datatransfer.dnd.X11DropTarget"_ustr };
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
    u"com.sun.star.datatransfer.dnd.XdndDropTarget"_ustr
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
            u"non-empty SalInstance::CreateClipboard arguments"_ustr, {}, -1);
    }
#ifdef IOS
    return getXWeak(new vcl::GenericClipboard());
#else
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
#endif
    DBG_TESTSOLARMUTEX();
    if (!m_clipboard.is()) {
        m_clipboard = getXWeak(new vcl::GenericClipboard());
    }
    return m_clipboard;
}

uno::Reference<uno::XInterface> SalInstance::ImplCreateDragSource(const SystemEnvData*)
{
    return css::uno::Reference<css::uno::XInterface>();
}

Reference< XInterface > SalInstance::CreateDragSource(const SystemEnvData* pSysEnv)
{
    // We run unit tests in parallel, which is a problem when touching a shared resource
    // the system clipboard, so rather use the dummy GenericClipboard.
    if (Application::IsHeadlessModeEnabled() || IsRunningUnitTest())
        return getXWeak(new vcl::GenericDragSource());
    return ImplCreateDragSource(pSysEnv);
}

uno::Reference<uno::XInterface> SalInstance::ImplCreateDropTarget(const SystemEnvData*)
{
    return css::uno::Reference<css::uno::XInterface>();
}

Reference< XInterface > SalInstance::CreateDropTarget(const SystemEnvData* pSysEnv)
{
    // see SalInstance::CreateDragSource
    if (Application::IsHeadlessModeEnabled() || IsRunningUnitTest())
        return getXWeak(new vcl::GenericDropTarget());
    return ImplCreateDropTarget(pSysEnv);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
