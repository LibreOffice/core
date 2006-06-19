/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: columninfo.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 14:12:43 $
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

#ifndef COLUMNINFO_HXX_INCLUDED
#define COLUMNINFO_HXX_INCLUDED

#if defined _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4917)
#endif
#include <shlobj.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif


class CColumnInfo : public IColumnProvider
{
public:
    CColumnInfo(long RefCnt = 1);
    virtual ~CColumnInfo();

    //-----------------------------
    // IUnknown methods
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid,
            void __RPC_FAR *__RPC_FAR *ppvObject);

    virtual ULONG STDMETHODCALLTYPE AddRef( void);

    virtual ULONG STDMETHODCALLTYPE Release( void);

    //-----------------------------
    // IColumnProvider
    //-----------------------------

    virtual HRESULT STDMETHODCALLTYPE Initialize(LPCSHCOLUMNINIT psci);

    virtual HRESULT STDMETHODCALLTYPE GetColumnInfo(DWORD dwIndex, SHCOLUMNINFO *psci);

    virtual HRESULT STDMETHODCALLTYPE GetItemData(
        LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData);

private:
    bool IsOOFileExtension(wchar_t* Extension) const;

private:
    long    m_RefCnt;
};

#endif
