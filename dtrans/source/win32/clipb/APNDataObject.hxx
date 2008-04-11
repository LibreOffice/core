/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: APNDataObject.hxx,v $
 * $Revision: 1.7 $
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


#ifndef _APNDATAOBJECT_HXX_
#define _APNDATAOBJECT_HXX_

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
    ~CAPNDataObject( );

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
