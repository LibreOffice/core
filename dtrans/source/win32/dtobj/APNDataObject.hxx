/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: APNDataObject.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:03:43 $
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


#ifndef _APNDATAOBJECT_HXX_
#define _APNDATAOBJECT_HXX_

#include <systools/win32/comtools.hxx>

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

/*
    an APartment Neutral dataobject wrapper; this wrapper of a IDataObject
    pointer can be used from any apartment without RPC_E_WRONG_THREAD
    which normally occurs if an apartment tries to use an interface
    pointer of another apartment; we use containment to hold the original
    DataObject
*/
class CAPNDataObject : public IDataObject
{
public:
    CAPNDataObject( IDataObjectPtr rIDataObject );
    virtual ~CAPNDataObject( );

    //-----------------------------------------------------------------
    //IUnknown interface methods
    //-----------------------------------------------------------------

    STDMETHODIMP           QueryInterface(REFIID iid, LPVOID* ppvObject);
    STDMETHODIMP_( ULONG ) AddRef( );
    STDMETHODIMP_( ULONG ) Release( );

    //-----------------------------------------------------------------
    // IDataObject interface methods
    //-----------------------------------------------------------------

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
    HRESULT MarshalIDataObjectIntoCurrentApartment( IDataObject** ppIDataObj );

private:
    IDataObjectPtr  m_rIDataObjectOrg;
    HGLOBAL         m_hGlobal;
    LONG            m_nRefCnt;

// prevent copy and assignment
private:
    CAPNDataObject( const CAPNDataObject& theOther );
    CAPNDataObject& operator=( const CAPNDataObject& theOther );
};

#endif
