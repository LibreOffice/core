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

#ifndef _INC_OLECOMPONENT_HXX_
#define _INC_OLECOMPONENT_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/embed/XOptimizedStorage.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/util/XModifyListener.hpp>

#include <vector>


namespace com { namespace sun { namespace star {
}}}

namespace cppu {
    class OMultiTypeInterfaceContainerHelper;
}

class OleWrapperClientSite;
class OleWrapperAdviseSink;
class OleEmbeddedObject;
struct OleComponentNative_Impl;

class OleComponent : public ::cppu::WeakImplHelper5< ::com::sun::star::util::XCloseable, ::com::sun::star::lang::XComponent,
                                                     ::com::sun::star::lang::XUnoTunnel, ::com::sun::star::util::XModifiable,
                                                     ::com::sun::star::datatransfer::XTransferable >
{
    ::osl::Mutex m_aMutex;
    ::cppu::OMultiTypeInterfaceContainerHelper* m_pInterfaceContainer;

    sal_Bool m_bDisposed;
    sal_Bool m_bModified;
    OleComponentNative_Impl* m_pNativeImpl;

    OleEmbeddedObject* m_pUnoOleObject;
    OleWrapperClientSite* m_pOleWrapClientSite;
    OleWrapperAdviseSink* m_pImplAdviseSink;

    sal_Int32 m_nOLEMiscFlags;
    sal_Int32 m_nAdvConn;

    ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor > m_aVerbList;
    ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > m_aDataFlavors;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

    sal_Bool m_bOleInitialized;

    // specifies whether the workaround for some rare embedded objects is activated ( f.e. AcrobatReader 7.0.8 object )
    // such objects report the dirty state wrongly sometimes and do not allow to store them any time
    sal_Bool m_bWorkaroundActive;

    sal_Bool InitializeObject_Impl();

    void CreateNewIStorage_Impl();
    void RetrieveObjectDataFlavors_Impl();
    void Dispose();


public:
    OleComponent( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& m_xFactory,
                  OleEmbeddedObject* pOleObj );

    virtual ~OleComponent();

    OleComponent* createEmbeddedCopyOfLink();

    void disconnectEmbeddedObject();

    static ::com::sun::star::awt::Size CalculateWithFactor( const ::com::sun::star::awt::Size& aSize,
                                                            const ::com::sun::star::awt::Size& aMultiplier,
                                                            const ::com::sun::star::awt::Size& aDivisor );

    ::com::sun::star::awt::Size CalculateTheRealSize( const ::com::sun::star::awt::Size& aContSize, sal_Bool bUpdate );

    // ==== Initialization ==================================================
    void LoadEmbeddedObject( const OUString& aTempURL );
    void CreateObjectFromClipboard();
    void CreateNewEmbeddedObject( const ::com::sun::star::uno::Sequence< sal_Int8 >& aSeqCLSID );
    void CreateObjectFromData(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTransfer );
    void CreateObjectFromFile( const OUString& aFileName );
    void CreateLinkFromFile( const OUString& aFileName );
    void InitEmbeddedCopyOfLink( OleComponent* pOleLinkComponent );

    // ======================================================================
    void RunObject(); // switch OLE object to running state
    void CloseObject(); // switch OLE object to loaded state

    ::com::sun::star::uno::Sequence< ::com::sun::star::embed::VerbDescriptor > GetVerbList();

    void ExecuteVerb( sal_Int32 nVerbID );
    void SetHostName( const OUString& aContName, const OUString& aEmbDocName );
    void SetExtent( const ::com::sun::star::awt::Size& aVisAreaSize, sal_Int64 nAspect );

    ::com::sun::star::awt::Size GetExtent( sal_Int64 nAspect );
    ::com::sun::star::awt::Size GetCachedExtent( sal_Int64 nAspect );
    ::com::sun::star::awt::Size GetReccomendedExtent( sal_Int64 nAspect );

    sal_Int64 GetMiscStatus( sal_Int64 nAspect );

    ::com::sun::star::uno::Sequence< sal_Int8 > GetCLSID();

    sal_Bool IsWorkaroundActive() { return m_bWorkaroundActive; }
    sal_Bool IsDirty();

    void StoreOwnTmpIfNecessary();

    sal_Bool SaveObject_Impl();
    sal_Bool OnShowWindow_Impl( bool bShow );
    void OnViewChange_Impl( sal_uInt32 dwAspect );
    void OnClose_Impl();

    // XCloseable
    virtual void SAL_CALL close( sal_Bool DeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

    // XTransferable
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw (::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener(const com::sun::star::uno::Reference < com::sun::star::lang::XEventListener >& aListener) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener(const com::sun::star::uno::Reference < com::sun::star::lang::XEventListener >& aListener) throw (::com::sun::star::uno::RuntimeException);

    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException) ;

    // XModifiable
    virtual sal_Bool SAL_CALL isModified() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addModifyListener( const com::sun::star::uno::Reference < com::sun::star::util::XModifyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const com::sun::star::uno::Reference < com::sun::star::util::XModifyListener >& xListener) throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
