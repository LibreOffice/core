/*************************************************************************
 *
 *  $RCSfile: APNDataObject.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-07-26 11:47:57 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _APNDATAOBJECT_HXX_
#include "APNDataObject.hxx"
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FREE_HGLOB_ON_RELEASE   TRUE
#define KEEP_HGLOB_ON_RELEASE   FALSE

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CAPNDataObject::CAPNDataObject( IDataObjectPtr rIDataObject ) :
    m_rIDataObjectOrg( rIDataObject ),
    m_hGlobal( NULL ),
    m_nRefCnt( 0 )
{
    try
    {
        OSL_ENSURE( m_rIDataObjectOrg.GetInterfacePtr( ), "constructing CAPNDataObject with empty data object" );

        // we marshal the IDataObject interface pointer here so
        // that it can be unmarshaled multiple times when this
        // class will be used from another apartment
        IStreamPtr pStm;
        HRESULT hr = CreateStreamOnHGlobal( 0, KEEP_HGLOB_ON_RELEASE, &pStm );

        OSL_ENSURE( E_INVALIDARG != hr, "invalid args passed to CreateStreamOnHGlobal" );

        if ( SUCCEEDED( hr ) )
        {
            HRESULT hr_marshal = CoMarshalInterface(
                pStm,
                __uuidof( IDataObject ),
                m_rIDataObjectOrg,
                MSHCTX_LOCAL,
                NULL,
                MSHLFLAGS_TABLEWEAK );

            OSL_ENSURE( CO_E_NOTINITIALIZED != hr_marshal, "COM is not initialized" );

            // marshalling may fail if COM is not initialized
            // for the calling thread which is a program time
            // error or because of stream errors which are runtime
            // errors for instance E_OUTOFMEMORY etc.

            hr = GetHGlobalFromStream( pStm, &m_hGlobal );

            OSL_ENSURE( E_INVALIDARG != hr, "invalid stream passed to GetHGlobalFromStream" );

            // if the marshalling failed we free the
            // global memory again and set m_hGlobal
            // to a defined value
            if ( FAILED( hr_marshal ) )
            {
                OSL_ENSURE( sal_False, "marshalling failed" );

                HGLOBAL hGlobal = GlobalFree( m_hGlobal );

                OSL_ENSURE( NULL == hGlobal, "GlobalFree failed" );

                m_hGlobal = NULL;
            }
        }
    }
    catch( _com_error& )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
    }
}

//------------------------------------------------------------------------
// dtor
//------------------------------------------------------------------------

CAPNDataObject::~CAPNDataObject( )
{
    try
    {
        if ( m_hGlobal )
        {
            IStreamPtr pStm;
            HRESULT  hr = CreateStreamOnHGlobal( m_hGlobal, FREE_HGLOB_ON_RELEASE, &pStm );

            OSL_ENSURE( E_INVALIDARG != hr, "invalid args passed to CreateStreamOnHGlobal" );

            if ( SUCCEEDED( hr ) )
            {
                hr = CoReleaseMarshalData( pStm );

                OSL_ENSURE( SUCCEEDED( hr ), "CoReleaseMarshalData failed" );
            }
        }
    }
    catch( _com_error& )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
    }
}

//------------------------------------------------------------------------
// IUnknown->QueryInterface
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::QueryInterface( REFIID iid, LPVOID* ppvObject )
{
    OSL_ASSERT( NULL != ppvObject );

    if ( NULL == ppvObject )
        return E_INVALIDARG;

    HRESULT hr = E_NOINTERFACE;
    *ppvObject = NULL;

    if ( ( __uuidof( IUnknown ) == iid ) || ( __uuidof( IDataObject ) == iid ) )
    {
        *ppvObject = static_cast< IUnknown* >( this );
        ( (LPUNKNOWN)*ppvObject )->AddRef( );
        hr = S_OK;
    }

    return hr;
}

//------------------------------------------------------------------------
// IUnknown->AddRef
//------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAPNDataObject::AddRef( )
{
    return static_cast< ULONG >( InterlockedIncrement( &m_nRefCnt ) );
}

//------------------------------------------------------------------------
// IUnknown->Release
//------------------------------------------------------------------------

STDMETHODIMP_(ULONG) CAPNDataObject::Release( )
{
    // we need a helper variable because it's not allowed to access
    // a member variable after an object is destroyed
    ULONG nRefCnt = static_cast< ULONG >( InterlockedDecrement( &m_nRefCnt ) );

    if ( 0 == nRefCnt )
        delete this;

    return nRefCnt;
}

//------------------------------------------------------------------------
// IDataObject->GetData
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::GetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    HRESULT hr;

    try
    {
        hr = m_rIDataObjectOrg->GetData( pFormatetc, pmedium );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->GetData( pFormatetc, pmedium );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->EnumFormatEtc
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::EnumFormatEtc( DWORD dwDirection, IEnumFORMATETC** ppenumFormatetc )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->EnumFormatEtc( dwDirection, ppenumFormatetc );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->EnumFormatEtc( dwDirection, ppenumFormatetc );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->QueryGetData
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::QueryGetData( LPFORMATETC pFormatetc )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->QueryGetData( pFormatetc );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->QueryGetData( pFormatetc );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->GetDataHere
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::GetDataHere( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->GetDataHere( pFormatetc, pmedium );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->GetDataHere( pFormatetc, pmedium );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->GetCanonicalFormatEtc
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::GetCanonicalFormatEtc( LPFORMATETC pFormatectIn, LPFORMATETC pFormatetcOut )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->GetCanonicalFormatEtc( pFormatectIn, pFormatetcOut );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->GetCanonicalFormatEtc( pFormatectIn, pFormatetcOut );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->SetData
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::SetData( LPFORMATETC pFormatetc, LPSTGMEDIUM pmedium, BOOL fRelease )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->SetData( pFormatetc, pmedium, fRelease );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->SetData( pFormatetc, pmedium, fRelease );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->DAdvise
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::DAdvise( LPFORMATETC pFormatetc, DWORD advf, LPADVISESINK pAdvSink, DWORD * pdwConnection )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->DAdvise( pFormatetc, advf, pAdvSink, pdwConnection );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->DAdvise( pFormatetc, advf, pAdvSink, pdwConnection );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->DUnadvise
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::DUnadvise( DWORD dwConnection )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->DUnadvise( dwConnection );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->DUnadvise( dwConnection );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// IDataObject->EnumDAdvise
//------------------------------------------------------------------------

STDMETHODIMP CAPNDataObject::EnumDAdvise( LPENUMSTATDATA * ppenumAdvise )
{
    HRESULT hr = E_FAIL;

    try
    {
        hr = m_rIDataObjectOrg->EnumDAdvise( ppenumAdvise );

        if ( RPC_E_WRONG_THREAD == hr )
        {
            IDataObjectPtr pIDOTmp;
            hr = MarshalIDataObjectIntoCurrentApartment( &pIDOTmp );

            if ( SUCCEEDED( hr ) )
                hr = pIDOTmp->EnumDAdvise( ppenumAdvise );
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}

//------------------------------------------------------------------------
// for our convenience
//------------------------------------------------------------------------

CAPNDataObject::operator IDataObject*( )
{
    return static_cast< IDataObject* >( this );
}

//------------------------------------------------------------------------
// helper function
//------------------------------------------------------------------------

HRESULT CAPNDataObject::MarshalIDataObjectIntoCurrentApartment( IDataObject** ppIDataObj )
{
    OSL_ASSERT( NULL != ppIDataObj );

    *ppIDataObj = NULL;

    HRESULT hr = E_FAIL;

    try
    {
        if ( m_hGlobal )
        {
            IStreamPtr pStm;
            hr = CreateStreamOnHGlobal( m_hGlobal, KEEP_HGLOB_ON_RELEASE, &pStm );

            OSL_ENSURE( E_INVALIDARG != hr, "CreateStreamOnHGlobal with invalid args called" );

            if ( SUCCEEDED( hr ) )
            {
                hr = CoUnmarshalInterface(
                    pStm, __uuidof( IDataObject ), (void**)ppIDataObj );

                OSL_ENSURE( CO_E_NOTINITIALIZED != hr, "COM is not initialized" );
            }
        }
    }
    catch( _com_error& ex )
    {
        OSL_ENSURE( sal_False, "com exception caught" );
        hr = ex.Error( );
    }

    return hr;
}
