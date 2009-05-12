/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DOTransferable.hxx,v $
 * $Revision: 1.10 $
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


#ifndef _DOTRANSFERABLE_HXX_
#define _DOTRANSFERABLE_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <com/sun/star/datatransfer/XTransferable.hpp>

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#include "DataFmtTransl.hxx"
#include <com/sun/star/datatransfer/XMimeContentTypeFactory.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/XSystemTransferable.hpp>

#include <systools/win32/comtools.hxx>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

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
