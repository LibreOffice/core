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

#ifndef INCLUDED_DTRANS_SOURCE_WIN32_DTOBJ_DOTRANSFERABLE_HXX
#define INCLUDED_DTRANS_SOURCE_WIN32_DTOBJ_DOTRANSFERABLE_HXX

#include <com/sun/star/datatransfer/XTransferable.hpp>

#include <cppuhelper/implbase.hxx>
#include "DataFmtTransl.hxx"
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XSystemTransferable.hpp>

#include <systools/win32/comtools.hxx>

// forward
class CDTransObjFactory;
class CFormatEtc;

class CDOTransferable : public ::cppu::WeakImplHelper<
                            css::datatransfer::XTransferable,
                            css::datatransfer::XSystemTransferable>
{
public:
    typedef css::uno::Sequence< sal_Int8 > ByteSequence_t;

    // XTransferable

    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& aFlavor )
        throw( css::datatransfer::UnsupportedFlavorException, css::io::IOException, css::uno::RuntimeException );

    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors(  )
        throw( css::uno::RuntimeException );

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& aFlavor )
        throw( css::uno::RuntimeException );

    // XSystemTransferable

    virtual css::uno::Any SAL_CALL getData( const css::uno::Sequence<sal_Int8>& aProcessId  ) throw
    (css::uno::RuntimeException);

private:
    // should be created only by CDTransObjFactory
    explicit CDOTransferable(
        const css::uno::Reference< css::uno::XComponentContext >& rxContext,
        IDataObjectPtr rDataObject );

    // some helper functions

    void SAL_CALL initFlavorList( );

    void SAL_CALL addSupportedFlavor( const css::datatransfer::DataFlavor& aFlavor );
    css::datatransfer::DataFlavor SAL_CALL formatEtcToDataFlavor( const FORMATETC& aFormatEtc );

    ByteSequence_t SAL_CALL getClipboardData( CFormatEtc& aFormatEtc );
    OUString  SAL_CALL synthesizeUnicodeText( );

    void SAL_CALL clipDataToByteStream( CLIPFORMAT cf, STGMEDIUM stgmedium, ByteSequence_t& aByteSequence );

    css::uno::Any SAL_CALL byteStreamToAny( ByteSequence_t& aByteStream, const css::uno::Type& aRequestedDataType );
    OUString              SAL_CALL byteStreamToOUString( ByteSequence_t& aByteStream );

    LCID SAL_CALL getLocaleFromClipboard( );

    sal_Bool SAL_CALL compareDataFlavors( const css::datatransfer::DataFlavor& lhs,
                                          const css::datatransfer::DataFlavor& rhs );

    sal_Bool SAL_CALL cmpFullMediaType( const css::uno::Reference< css::datatransfer::XMimeContentType >& xLhs,
                                        const css::uno::Reference< css::datatransfer::XMimeContentType >& xRhs ) const;

    sal_Bool SAL_CALL cmpAllContentTypeParameter( const css::uno::Reference< css::datatransfer::XMimeContentType >& xLhs,
                                        const css::uno::Reference< css::datatransfer::XMimeContentType >& xRhs ) const;

private:
    IDataObjectPtr                                                                          m_rDataObject;
    css::uno::Sequence< css::datatransfer::DataFlavor >               m_FlavorList;
    const css::uno::Reference< css::uno::XComponentContext >          m_xContext;
    CDataFormatTranslator                                             m_DataFormatTranslator;
    css::uno::Reference< css::datatransfer::XMimeContentTypeFactory > m_rXMimeCntFactory;
    ::osl::Mutex                                                      m_aMutex;
    sal_Bool                                                          m_bUnicodeRegistered;
    CLIPFORMAT                                                        m_TxtFormatOnClipboard;

// non supported operations
private:
    CDOTransferable( const CDOTransferable& );
    CDOTransferable& operator=( const CDOTransferable& );

    friend class CDTransObjFactory;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
