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

#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <com/sun/star/datatransfer/clipboard/ClipboardEvent.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/svapp.hxx>
#include <svdata.hxx>
#include <salinst.hxx>

#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#include "XNotifyingDataObject.hxx"

#include <systools/win32/comtools.hxx>
#include "DtObjFactory.hxx"
#include "APNDataObject.hxx"
#include "DOTransferable.hxx"
#include "WinClipboard.hxx"

#if !defined WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <ole2.h>
#include <objidl.h>

using namespace com::sun::star;

// definition of static members
CWinClipboard* CWinClipboard::s_pCWinClipbImpl = nullptr;
osl::Mutex CWinClipboard::s_aMutex;

/*XEventListener,*/
CWinClipboard::CWinClipboard(const uno::Reference<uno::XComponentContext>& rxContext,
                             const OUString& aClipboardName)
    : WeakComponentImplHelper<XSystemClipboard, XFlushableClipboard, XServiceInfo>(
          m_aCbListenerMutex)
    , m_xContext(rxContext)
    , m_itsName(aClipboardName)
    , m_pCurrentClipContent(nullptr)
{
    // necessary to reassociate from
    // the static callback function
    s_pCWinClipbImpl = this;
    registerClipboardViewer();
}

CWinClipboard::~CWinClipboard()
{
    {
        osl::MutexGuard aGuard(s_aMutex);
        s_pCWinClipbImpl = nullptr;
    }

    unregisterClipboardViewer();
}

// XClipboard

// to avoid unnecessary traffic we check first if there is a clipboard
// content which was set via setContent, in this case we don't need
// to query the content from the clipboard, create a new wrapper object
// and so on, we simply return the original XTransferable instead of our
// DOTransferable

uno::Reference<datatransfer::XTransferable> SAL_CALL CWinClipboard::getContents()
{
    osl::MutexGuard aGuard(m_aMutex);

    if (rBHelper.bDisposed)
        throw lang::DisposedException("object is already disposed",
                                      static_cast<XClipboardEx*>(this));

    // use the shortcut or create a transferable from
    // system clipboard
    {
        osl::MutexGuard aGuard2(m_ClipContentMutex);

        if (nullptr != m_pCurrentClipContent)
            return m_pCurrentClipContent->m_XTransferable;

        // Content cached?
        if (m_foreignContent.is())
            return m_foreignContent;

        // release the mutex, so that the variable may be
        // changed by other threads
    }

    uno::Reference<datatransfer::XTransferable> rClipContent;

    // get the current dataobject from clipboard
    IDataObjectPtr pIDataObject;
    HRESULT hr = m_MtaOleClipboard.getClipboard(&pIDataObject);

    if (SUCCEEDED(hr))
    {
        // create an apartment neutral dataobject and initialize it with a
        // com smart pointer to the IDataObject from clipboard
        IDataObjectPtr pIDo(new CAPNDataObject(pIDataObject));

        // remember pIDo destroys itself due to the smart pointer
        rClipContent = CDOTransferable::create(m_xContext, pIDo);

        osl::MutexGuard aGuard2(m_ClipContentMutex);
        m_foreignContent = rClipContent;
    }

    return rClipContent;
}

void SAL_CALL CWinClipboard::setContents(
    const uno::Reference<datatransfer::XTransferable>& xTransferable,
    const uno::Reference<datatransfer::clipboard::XClipboardOwner>& xClipboardOwner)
{
    osl::MutexGuard aGuard(m_aMutex);

    if (rBHelper.bDisposed)
        throw lang::DisposedException("object is already disposed",
                                      static_cast<XClipboardEx*>(this));

    IDataObjectPtr pIDataObj;

    if (xTransferable.is())
    {
        {
            osl::MutexGuard aGuard2(m_ClipContentMutex);

            m_foreignContent.clear();

            m_pCurrentClipContent = new CXNotifyingDataObject(
                CDTransObjFactory::createDataObjFromTransferable(m_xContext, xTransferable),
                xTransferable, xClipboardOwner, this);
        }

        pIDataObj = IDataObjectPtr(m_pCurrentClipContent);
    }

    m_MtaOleClipboard.setClipboard(pIDataObj.get());
}

OUString SAL_CALL CWinClipboard::getName()
{
    if (rBHelper.bDisposed)
        throw lang::DisposedException("object is already disposed",
                                      static_cast<XClipboardEx*>(this));

    return m_itsName;
}

// XFlushableClipboard

void SAL_CALL CWinClipboard::flushClipboard()
{
    osl::MutexGuard aGuard(m_aMutex);

    if (rBHelper.bDisposed)
        throw lang::DisposedException("object is already disposed",
                                      static_cast<XClipboardEx*>(this));

    // actually it should be ClearableMutexGuard aGuard( m_ClipContentMutex );
    // but it does not work since FlushClipboard does a callback and frees DataObject
    // which results in a deadlock in onReleaseDataObject.
    // FlushClipboard had to be synchron in order to prevent shutdown until all
    // clipboard-formats are rendered.
    // The request is needed to prevent flushing if we are not clipboard owner (it is
    // not known what happens if we flush but aren't clipboard owner).
    // It may be possible to move the request to the clipboard STA thread by saving the
    // DataObject and call OleIsCurrentClipboard before flushing.

    if (nullptr != m_pCurrentClipContent)
        m_MtaOleClipboard.flushClipboard();
}

// XClipboardEx

sal_Int8 SAL_CALL CWinClipboard::getRenderingCapabilities()
{
    if (rBHelper.bDisposed)
        throw lang::DisposedException("object is already disposed",
                                      static_cast<XClipboardEx*>(this));

    using namespace datatransfer::clipboard::RenderingCapabilities;
    return (Delayed | Persistant);
}

// XClipboardNotifier

void SAL_CALL CWinClipboard::addClipboardListener(
    const uno::Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    if (rBHelper.bDisposed)
        throw lang::DisposedException("object is already disposed",
                                      static_cast<XClipboardEx*>(this));

    // check input parameter
    if (!listener.is())
        throw lang::IllegalArgumentException("empty reference", static_cast<XClipboardEx*>(this),
                                             1);

    rBHelper.aLC.addInterface(cppu::UnoType<decltype(listener)>::get(), listener);
}

void SAL_CALL CWinClipboard::removeClipboardListener(
    const uno::Reference<datatransfer::clipboard::XClipboardListener>& listener)
{
    if (rBHelper.bDisposed)
        throw lang::DisposedException("object is already disposed",
                                      static_cast<XClipboardEx*>(this));

    // check input parameter
    if (!listener.is())
        throw lang::IllegalArgumentException("empty reference", static_cast<XClipboardEx*>(this),
                                             1);

    rBHelper.aLC.removeInterface(cppu::UnoType<decltype(listener)>::get(), listener);
}

void CWinClipboard::notifyAllClipboardListener()
{
    if (rBHelper.bDisposed)
        return;

    osl::ClearableMutexGuard aGuard(rBHelper.rMutex);
    if (rBHelper.bDisposed)
        return;
    aGuard.clear();

    cppu::OInterfaceContainerHelper* pICHelper = rBHelper.aLC.getContainer(
        cppu::UnoType<datatransfer::clipboard::XClipboardListener>::get());
    if (!pICHelper)
        return;

    try
    {
        cppu::OInterfaceIteratorHelper iter(*pICHelper);
        uno::Reference<datatransfer::XTransferable> rXTransf(getContents());
        datatransfer::clipboard::ClipboardEvent aClipbEvent(static_cast<XClipboard*>(this),
                                                            rXTransf);

        while (iter.hasMoreElements())
        {
            try
            {
                uno::Reference<datatransfer::clipboard::XClipboardListener> xCBListener(
                    iter.next(), uno::UNO_QUERY);
                if (xCBListener.is())
                    xCBListener->changedContents(aClipbEvent);
            }
            catch (uno::RuntimeException&)
            {
                TOOLS_WARN_EXCEPTION("vcl", "");
            }
        }
    }
    catch (const lang::DisposedException&)
    {
        OSL_FAIL("Service Manager disposed");

        // no further clipboard changed notifications
        unregisterClipboardViewer();
    }
}

// XServiceInfo

OUString SAL_CALL CWinClipboard::getImplementationName()
{
    return "com.sun.star.datatransfer.clipboard.ClipboardW32";
}

sal_Bool SAL_CALL CWinClipboard::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<OUString> SAL_CALL CWinClipboard::getSupportedServiceNames()
{
    return { "com.sun.star.datatransfer.clipboard.SystemClipboard" };
}

// We run unit tests in parallel, which is a problem when touching a shared resource
// the system clipboard, so rather use the dummy GenericClipboard.
static const bool bRunningUnitTest = getenv("LO_TESTNAME");

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
dtrans_CWinClipboard_get_implementation(css::uno::XComponentContext* context,
                                        css::uno::Sequence<css::uno::Any> const& args)
{
    if (bRunningUnitTest)
    {
        SolarMutexGuard aGuard;
        auto xClipboard = ImplGetSVData()->mpDefInst->CreateClipboard(args);
        if (xClipboard.is())
            xClipboard->acquire();
        return xClipboard.get();
    }
    else
    {
        return cppu::acquire(static_cast<cppu::OWeakObject*>(new CWinClipboard(context, "")));
    }
}

void CWinClipboard::onReleaseDataObject(CXNotifyingDataObject* theCaller)
{
    OSL_ASSERT(nullptr != theCaller);

    if (theCaller)
        theCaller->lostOwnership();

    // if the current caller is the one we currently hold, then set it to NULL
    // because an external source must be the clipboardowner now
    osl::MutexGuard aGuard(m_ClipContentMutex);

    if (m_pCurrentClipContent == theCaller)
        m_pCurrentClipContent = nullptr;
}

void CWinClipboard::registerClipboardViewer()
{
    m_MtaOleClipboard.registerClipViewer(CWinClipboard::onClipboardContentChanged);
}

void CWinClipboard::unregisterClipboardViewer() { m_MtaOleClipboard.registerClipViewer(nullptr); }

void WINAPI CWinClipboard::onClipboardContentChanged()
{
    osl::MutexGuard aGuard(s_aMutex);

    // reassociation to instance through static member
    if (nullptr != s_pCWinClipbImpl)
    {
        s_pCWinClipbImpl->m_foreignContent.clear();
        s_pCWinClipbImpl->notifyAllClipboardListener();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
