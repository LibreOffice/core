/*************************************************************************
 *
 *  $RCSfile: XNotifyingDataObject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-10-22 07:57:11 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _XNOTIFYINGDATAOBJECT_HXX_
#include "XNotifyingDataObject.hxx"
#endif

#ifndef _WINCLIPBIMPL_HXX_
#include "..\clipb\WinClipbImpl.hxx"
#endif

#ifndef _WINCLIPBOARD_HXX_
#include "..\clipb\WinClipboard.hxx"
#endif

#ifndef _DTOBJFACTORY_HXX_
#include "..\..\inc\DtObjFactory.hxx"
#endif


using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Reference;


CXNotifyingDataObject::CXNotifyingDataObject(
    const IDataObjectPtr& aIDataObject,
    const Reference< XTransferable >& aXTransferable,
    const Reference< XClipboardOwner >& aXClipOwner,
    CWinClipbImpl* theWinClipImpl ) :
    m_nRefCnt( 0 ),
    m_aIDataObject( aIDataObject ),
    m_XTransferable( aXTransferable ),
    m_XClipboardOwner( aXClipOwner ),
    m_pWinClipImpl( theWinClipImpl )
{
}

STDMETHODIMP CXNotifyingDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;

    *ppvObject = NULL;
    if ( ( __uuidof( IUnknown ) == iid ) ||
         ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        ( (LPUNKNOWN)*ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP_(ULONG) CXNotifyingDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

STDMETHODIMP_(ULONG) CXNotifyingDataObject::Release( )
{
    ULONG nRefCnt =
        static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
    {
        if ( m_pWinClipImpl )
            m_pWinClipImpl->onReleaseDataObject( this );

        delete this;
    }

    return nRefCnt;
}

STDMETHODIMP CXNotifyingDataObject::GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    return m_aIDataObject->GetData(pFormatetc, pmedium);
}

STDMETHODIMP CXNotifyingDataObject::EnumFormatEtc(
    DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    return m_aIDataObject->EnumFormatEtc(dwDirection, ppenumFormatetc);
}

STDMETHODIMP CXNotifyingDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    return m_aIDataObject->QueryGetData(pFormatetc);
}

STDMETHODIMP CXNotifyingDataObject::GetDataHere( LPFORMATETC lpFetc, LPSTGMEDIUM lpStgMedium )
{
    return m_aIDataObject->GetDataHere(lpFetc, lpStgMedium);
}

STDMETHODIMP CXNotifyingDataObject::GetCanonicalFormatEtc( LPFORMATETC lpFetc, LPFORMATETC lpCanonicalFetc )
{
    return m_aIDataObject->GetCanonicalFormatEtc(lpFetc, lpCanonicalFetc);
}

STDMETHODIMP CXNotifyingDataObject::SetData( LPFORMATETC lpFetc, LPSTGMEDIUM lpStgMedium, BOOL bRelease )
{
    return m_aIDataObject->SetData( lpFetc, lpStgMedium, bRelease );
}

STDMETHODIMP CXNotifyingDataObject::DAdvise(
    LPFORMATETC lpFetc, DWORD advf, LPADVISESINK lpAdvSink, DWORD* pdwConnection )
{
    return m_aIDataObject->DAdvise( lpFetc, advf, lpAdvSink, pdwConnection );
}

STDMETHODIMP CXNotifyingDataObject::DUnadvise( DWORD dwConnection )
{
    return m_aIDataObject->DUnadvise( dwConnection );
}

STDMETHODIMP CXNotifyingDataObject::EnumDAdvise( LPENUMSTATDATA * ppenumAdvise )
{
    return m_aIDataObject->EnumDAdvise( ppenumAdvise );
}

CXNotifyingDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

void SAL_CALL CXNotifyingDataObject::lostOwnership( )
{
    try
    {
        if (m_XClipboardOwner.is())
            m_XClipboardOwner->lostOwnership(
                static_cast<XClipboardEx*>(m_pWinClipImpl->m_pWinClipboard ), m_XTransferable);
    }
    catch(RuntimeException&)
    {
        OSL_ENSURE( sal_False, "RuntimeException caught" );
    }
}
