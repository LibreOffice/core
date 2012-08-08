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


#ifndef _DOTRANSFERABLE_HXX_
#define _DOTRANSFERABLE_HXX_

#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <cppuhelper/implbase2.hxx>
#include "DataFmtTransl.hxx"
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XSystemTransferable.hpp>

#include <systools/win32/comtools.hxx>

// forward
class CDTransObjFactory;
class CFormatEtc;

class CDOTransferable : public ::cppu::WeakImplHelper2< ::com::sun::star::datatransfer::XTransferable,
                        ::com::sun::star::datatransfer::XSystemTransferable>
{
public:
    typedef com::sun::star::uno::Sequence< sal_Int8 > ByteSequence_t;

    //------------------------------------------------------------------------
    // XTransferable
    //------------------------------------------------------------------------

    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
        throw( ::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException );

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& aFlavor )
        throw( ::com::sun::star::uno::RuntimeException );
    //------------------------------------------------------------------------
    // XSystemTransferable
    //------------------------------------------------------------------------
    virtual ::com::sun::star::uno::Any SAL_CALL getData( const com::sun::star::uno::Sequence<sal_Int8>& aProcessId  ) throw
    (::com::sun::star::uno::RuntimeException);


private:
    // should be created only by CDTransObjFactory
    explicit CDOTransferable(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ServiceManager,
        IDataObjectPtr rDataObject );

    //------------------------------------------------------------------------
    // some helper functions
    //------------------------------------------------------------------------

    void SAL_CALL initFlavorList( );

    void SAL_CALL addSupportedFlavor( const com::sun::star::datatransfer::DataFlavor& aFlavor );
    com::sun::star::datatransfer::DataFlavor SAL_CALL formatEtcToDataFlavor( const FORMATETC& aFormatEtc );

    ByteSequence_t SAL_CALL getClipboardData( CFormatEtc& aFormatEtc );
    rtl::OUString  SAL_CALL synthesizeUnicodeText( );

    void SAL_CALL clipDataToByteStream( CLIPFORMAT cf, STGMEDIUM stgmedium, ByteSequence_t& aByteSequence );

    ::com::sun::star::uno::Any SAL_CALL byteStreamToAny( ByteSequence_t& aByteStream, const com::sun::star::uno::Type& aRequestedDataType );
    rtl::OUString              SAL_CALL byteStreamToOUString( ByteSequence_t& aByteStream );

    LCID SAL_CALL getLocaleFromClipboard( );

    sal_Bool SAL_CALL compareDataFlavors( const com::sun::star::datatransfer::DataFlavor& lhs,
                                          const com::sun::star::datatransfer::DataFlavor& rhs );

    sal_Bool SAL_CALL cmpFullMediaType( const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType >& xLhs,
                                        const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType >& xRhs ) const;

    sal_Bool SAL_CALL cmpAllContentTypeParameter( const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType >& xLhs,
                                        const com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentType >& xRhs ) const;

private:
    IDataObjectPtr                                                                          m_rDataObject;
    com::sun::star::uno::Sequence< com::sun::star::datatransfer::DataFlavor >               m_FlavorList;
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >  m_SrvMgr;
    CDataFormatTranslator                                                                   m_DataFormatTranslator;
    com::sun::star::uno::Reference< com::sun::star::datatransfer::XMimeContentTypeFactory > m_rXMimeCntFactory;
    ::osl::Mutex                                                                            m_aMutex;
    sal_Bool                                                                                m_bUnicodeRegistered;
    CLIPFORMAT                                                                              m_TxtFormatOnClipboard;

// non supported operations
private:
    CDOTransferable( const CDOTransferable& );
    CDOTransferable& operator=( const CDOTransferable& );

    friend class CDTransObjFactory;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
