/*************************************************************************
 *
 *  $RCSfile: XTDataObject.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-05 06:36:35 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _XTDATAOBJECT_HXX_
#define _XTDATAOBJECT_HXX_


//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDOWNER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#endif

#ifndef _DATAFORMATTRANSLATOR_HXX_
#include "DataFmtTransl.hxx"
#endif

#ifndef _FETCLIST_HXX_
#include "FEtcList.hxx"
#endif

#include <windows.h>
#include <ole2.h>
#include <objidl.h>

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

    sal_Int64 SAL_CALL QueryDataSize( );

    // retrieve the data from the source
    // necessary so that we have all data
    // when we flush the clipboard
    void SAL_CALL GetAllDataFromSource( );

private:
    void validateFormatEtc( LPFORMATETC lpFormatEtc ) const;
    com::sun::star::datatransfer::DataFlavor SAL_CALL formatEtcToDataFlavor( const FORMATETC& aFormatEtc ) const;
    sal_Bool SAL_CALL isSynthesizeableFormat( LPFORMATETC lpFormatEtc ) const;
    void SAL_CALL transferDataToStorageAndSetupStgMedium( const sal_Int8* lpStorage,
                                                          const FORMATETC& fetc,
                                                          sal_uInt32 nInitStgSize,
                                                          sal_uInt32 nBytesToTransfer,
                                                          STGMEDIUM& stgmedium );

    void SAL_CALL transferLocaleToClipbAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void SAL_CALL transferUnicodeToClipbAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );
    void SAL_CALL transferAnyDataToClipbAndSetupStgMedium( FORMATETC& fetc, STGMEDIUM& stgmedium );

    void SAL_CALL setupStgMedium( const FORMATETC& fetc,
                                  CStgTransferHelper& stgTransHlp,
                                  STGMEDIUM& stgmedium );

    void SAL_CALL invalidateStgMedium( STGMEDIUM& stgmedium ) const;

    sal_Bool SAL_CALL isOemOrAnsiTextFormat( const CLIPFORMAT& aClipformat ) const;
    HRESULT SAL_CALL translateStgExceptionCode( HRESULT hr ) const;

private:
    LONG m_nRefCnt;
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_SrvMgr;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >      m_XTransferable;
    CFormatEtcContainer                                                                    m_FormatEtcContainer;
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