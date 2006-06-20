/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XNotifyingDataObject.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:06:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#ifndef _XNOTIFYINGDATAOBJECT_HXX_
#define _XNOTIFYINGDATAOBJECT_HXX_


//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDOWNER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#endif

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <objidl.h>
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <systools/win32/comtools.hxx>

/*--------------------------------------------------------------------------
    To implement the lostOwnership mechanism cleanly we need this wrapper
    object
----------------------------------------------------------------------------*/

// forward
class CWinClipbImpl;

class CXNotifyingDataObject : public IDataObject
{
public:
    CXNotifyingDataObject(
        const IDataObjectPtr& aIDataObject,
        const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >& aXTransferable,
        const com::sun::star::uno::Reference< com::sun::star::datatransfer::clipboard::XClipboardOwner >& aXClipOwner,
        CWinClipbImpl* theWinClipImpl );

    virtual ~CXNotifyingDataObject() {}

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
    void SAL_CALL lostOwnership( );

private:
    sal_Int32                                                                                           m_nRefCnt;
    IDataObjectPtr                                                                                      m_aIDataObject;
    const com::sun::star::uno::Reference< com::sun::star::datatransfer::XTransferable >                 m_XTransferable;
    const com::sun::star::uno::Reference< com::sun::star::datatransfer::clipboard::XClipboardOwner >    m_XClipboardOwner;
    CWinClipbImpl*                                                                                      m_pWinClipImpl;

    friend class CWinClipbImpl;
};

#endif
