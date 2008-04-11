/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: XTDataObject.hxx,v $
 * $Revision: 1.13 $
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


#ifndef _XTDATAOBJECT_HXX_
#define _XTDATAOBJECT_HXX_


//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>

#ifndef _DATAFORMATTRANSLATOR_HXX_
#include "DataFmtTransl.hxx"
#endif

#ifndef _FETCLIST_HXX_
#include "FEtcList.hxx"
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <ole2.h>
#include <objidl.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

/*--------------------------------------------------------------------------
    - the function principle of the windows clipboard:
      a data provider offers all formats he can deliver on the clipboard
      a clipboard client ask for the available formats on the clipboard
      and decides if there is a format he can use
      if there is one, he requests the data in this format

    - This class inherits from IDataObject an so can be placed on the
      OleClipboard. The class wrapps a transferable object which is the
      original DataSource
    - DataFlavors offerd by this transferable will be translated into
      appropriate clipboard formats
    - if the transferable contains text data always text and unicodetext
      will be offered or vice versa
    - text data will be automaticaly converted between text und unicode text
    - although the transferable may support text in different charsets
      (codepages) only text in one codepage can be offered by the clipboard

----------------------------------------------------------------------------*/

class CStgTransferHelper;

class CXTDataObject : public IDataObject
{
public:
    CXTDataObject( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& aServiceManager,
                   const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& aXTransferable );
    virtual ~CXTDataObject() {}

    //-----------------------------------------------------------------
    // ole interface implementation
    //-----------------------------------------------------------------

    //IUnknown interface methods
    STDMETHODIMP           QueryInterface(REFIID iid, LPVOID* ppvObject);
    STDMETHODIMP_( ULONG ) AddRef( );
    STDMETHODIMP_( ULONG ) Release( );

    // IDataObject interface methods
    STDMETHODIMP GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium );
    STDMETHODIMP GetDataHere( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium );
    STDMETHODIMP QueryGetData( LPFORMATETC pFormatetc );
    STDMETHODIMP GetCanonicalFormatEtc( LPFORMATETC pFormatectIn, LPFORMATETC pFormatetcOut );
    STDMETHODIMP SetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium, BOOL fRelease );
    STDMETHODIMP EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc );
    STDMETHODIMP DAdvise( LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD* pdwConnection );
    STDMETHODIMP DUnadvise( DWORD dwConnection );
    STDMETHODIMP EnumDAdvise( LPENUMSTATDATA* ppenumAdvise );

    operator IDataObject*( );

private:
    com::sun::star::datatransfer::DataFlavor SAL_CALL formatEtcToDataFlavor( const FORMATETC& aFormatEtc ) const;

    void SAL_CALL renderDataAndSetupStgMedium( const sal_Int8* lpStorage,
                                               const FORMATETC& fetc,
                                               sal_uInt32 nInitStgSize,
                                               sal_uInt32 nBytesToTransfer,
                                               STGMEDIUM& stgmedium );

    void SAL_CALL renderLocaleAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void SAL_CALL renderUnicodeAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void SAL_CALL renderAnyDataAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );

    HRESULT SAL_CALL renderSynthesizedFormatAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void    SAL_CALL renderSynthesizedUnicodeAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void    SAL_CALL renderSynthesizedTextAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void    SAL_CALL renderSynthesizedHtmlAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );

    void SAL_CALL setupStgMedium( const FORMATETC& fetc,
                                  CStgTransferHelper& stgTransHlp,
                                  STGMEDIUM& stgmedium );

    void validateFormatEtc( LPFORMATETC lpFormatEtc ) const;
    void SAL_CALL invalidateStgMedium( STGMEDIUM& stgmedium ) const;

    HRESULT SAL_CALL translateStgExceptionCode( HRESULT hr ) const;

    inline void SAL_CALL InitializeFormatEtcContainer( );

private:
    LONG m_nRefCnt;
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_SrvMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >      m_XTransferable;
    CFormatEtcContainer                                                                    m_FormatEtcContainer;
    sal_Bool                                                                               m_bFormatEtcContainerInitialized;
    CDataFormatTranslator                                                                  m_DataFormatTranslator;
    CFormatRegistrar                                                                       m_FormatRegistrar;
};

//------------------------------------------------------------------------
//
//------------------------------------------------------------------------

class CEnumFormatEtc : public IEnumFORMATETC
{
public:
    CEnumFormatEtc( LPUNKNOWN lpUnkOuter, const CFormatEtcContainer& aFormatEtcContainer );
    virtual ~CEnumFormatEtc() {}

    // IUnknown
    STDMETHODIMP           QueryInterface( REFIID iid, LPVOID* ppvObject );
    STDMETHODIMP_( ULONG ) AddRef( );
    STDMETHODIMP_( ULONG ) Release( );

    //IEnumFORMATETC
    STDMETHODIMP Next( ULONG nRequested, LPFORMATETC lpDest, ULONG* lpFetched );
    STDMETHODIMP Skip( ULONG celt );
    STDMETHODIMP Reset( );
    STDMETHODIMP Clone( IEnumFORMATETC** ppenum );

private:
    LONG                m_nRefCnt;
    LPUNKNOWN           m_lpUnkOuter;
    CFormatEtcContainer m_FormatEtcContainer;
};

typedef CEnumFormatEtc *PCEnumFormatEtc;

#endif
