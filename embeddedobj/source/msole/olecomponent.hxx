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

#pragma once

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

class OleWrapperClientSite;
class OleWrapperAdviseSink;
class OleEmbeddedObject;
struct OleComponentNative_Impl;

class OleComponent : public ::cppu::WeakImplHelper< css::util::XCloseable, css::lang::XComponent,
                                                    css::lang::XUnoTunnel, css::util::XModifiable,
                                                    css::datatransfer::XTransferable >
{
    ::osl::Mutex m_aMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    bool m_bDisposed;
    bool m_bModified;
    OleComponentNative_Impl* m_pNativeImpl;

    OleEmbeddedObject* m_pUnoOleObject;
    OleWrapperClientSite* m_pOleWrapClientSite;
    OleWrapperAdviseSink* m_pImplAdviseSink;

    sal_Int32 m_nOLEMiscFlags;
    sal_Int32 m_nAdvConn;

    css::uno::Sequence< css::embed::VerbDescriptor > m_aVerbList;
    css::uno::Sequence< css::datatransfer::DataFlavor > m_aDataFlavors;

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    bool m_bOleInitialized;

    // specifies whether the workaround for some rare embedded objects is activated ( f.e. AcrobatReader 7.0.8 object )
    // such objects report the dirty state wrongly sometimes and do not allow to store them any time
    bool m_bWorkaroundActive;

    bool InitializeObject_Impl();

    void CreateNewIStorage_Impl();
    void RetrieveObjectDataFlavors_Impl();
    void Dispose();


public:
    OleComponent( const css::uno::Reference< css::uno::XComponentContext >& xContext,
                  OleEmbeddedObject* pOleObj );

    virtual ~OleComponent() override;

    OleComponent* createEmbeddedCopyOfLink();

    void disconnectEmbeddedObject();

    static css::awt::Size CalculateWithFactor( const css::awt::Size& aSize,
                                                            const css::awt::Size& aMultiplier,
                                                            const css::awt::Size& aDivisor );

    css::awt::Size CalculateTheRealSize( const css::awt::Size& aContSize, bool bUpdate );

    // ==== Initialization ==================================================
    void LoadEmbeddedObject( const OUString& aTempURL );
    void CreateObjectFromClipboard();
    void CreateNewEmbeddedObject( const css::uno::Sequence< sal_Int8 >& aSeqCLSID );
    static void CreateObjectFromData(
                        const css::uno::Reference< css::datatransfer::XTransferable >& xTransfer );
    void CreateObjectFromFile( const OUString& aFileName );
    void CreateLinkFromFile( const OUString& aFileName );
    void InitEmbeddedCopyOfLink( OleComponent const * pOleLinkComponent );


    void RunObject(); // switch OLE object to running state
    void CloseObject(); // switch OLE object to loaded state

    css::uno::Sequence< css::embed::VerbDescriptor > GetVerbList();

    void ExecuteVerb( sal_Int32 nVerbID );
    void SetHostName( const OUString& aContName, const OUString& aEmbDocName );
    void SetExtent( const css::awt::Size& aVisAreaSize, sal_Int64 nAspect );

    css::awt::Size GetExtent( sal_Int64 nAspect );
    css::awt::Size GetCachedExtent( sal_Int64 nAspect );
    css::awt::Size GetRecommendedExtent( sal_Int64 nAspect );

    sal_Int64 GetMiscStatus( sal_Int64 nAspect );

    css::uno::Sequence< sal_Int8 > GetCLSID();

    bool IsWorkaroundActive() { return m_bWorkaroundActive; }
    bool IsDirty();

    void StoreOwnTmpIfNecessary();

    bool SaveObject_Impl();
    bool OnShowWindow_Impl( bool bShow );
    void OnViewChange_Impl( sal_uInt32 dwAspect );
    void OnClose_Impl();

    // XCloseable
    virtual void SAL_CALL close( sal_Bool DeliverOwnership ) override;
    virtual void SAL_CALL addCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) override;
    virtual void SAL_CALL removeCloseListener( const css::uno::Reference< css::util::XCloseListener >& Listener ) override;

    // XTransferable
    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor ) override;
    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor ) override;

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener(const css::uno::Reference < css::lang::XEventListener >& aListener) override;
    virtual void SAL_CALL removeEventListener(const css::uno::Reference < css::lang::XEventListener >& aListener) override;

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // XModifiable
    virtual sal_Bool SAL_CALL isModified() override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) override;
    virtual void SAL_CALL addModifyListener( const css::uno::Reference < css::util::XModifyListener >& xListener ) override;
    virtual void SAL_CALL removeModifyListener( const css::uno::Reference < css::util::XModifyListener >& xListener) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
