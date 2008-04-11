/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: iipaobj.cxx,v $
 * $Revision: 1.5 $
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

#include "iipaobj.hxx"
#include "embeddoc.hxx"



CIIAObj::CIIAObj(DocumentHolder* pDocHolder)
    : m_refCount( 0L ),
      m_rDocHolder( pDocHolder )
{
}


CIIAObj::~CIIAObj()
{
    return;
}

/* IUnknown methods */

STDMETHODIMP CIIAObj::QueryInterface(REFIID riid, LPVOID FAR *ppv)
{
    *ppv=NULL;

    if(IID_IUnknown==riid ||
       IID_IOleWindow==riid ||
       IID_IOleInPlaceActiveObject==riid)
        *ppv=this;

    //AddRef any interface we'll return.
    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CIIAObj::AddRef(void)
{
    return osl_incrementInterlockedCount( &m_refCount);
}

STDMETHODIMP_(ULONG) CIIAObj::Release(void)
{
    sal_Int32 nCount = osl_decrementInterlockedCount( &m_refCount);
    if ( nCount == 0 )
        delete this;

    return nCount;
}

/* IOleInPlaceActiveObject methods*/

STDMETHODIMP CIIAObj::GetWindow(HWND *)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::ContextSensitiveHelp(BOOL)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::TranslateAccelerator(LPMSG)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::OnFrameWindowActivate(BOOL)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::OnDocWindowActivate(BOOL)
{
    return NOERROR;
}

STDMETHODIMP CIIAObj::ResizeBorder(
    LPCRECT pRect,LPOLEINPLACEUIWINDOW,BOOL bFrame)
{
    if(!bFrame) return NOERROR;

    if ( !m_rDocHolder.is() )
        return E_FAIL;

    return m_rDocHolder->SetContRects(pRect);
}


STDMETHODIMP CIIAObj::EnableModeless(BOOL)
{
    return NOERROR;
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif
