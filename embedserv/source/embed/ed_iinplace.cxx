/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ed_iinplace.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-10-06 10:38:09 $
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

#include "embeddoc.hxx"

#ifndef _OSL_DIAGNOSE_H_
#include <ols/diagnose.h>
#endif


STDMETHODIMP EmbedDocument_Impl::GetWindow(HWND *hWnd)
{
    OSL_ENSURE(m_pDocHolder,"no document for inplace activation");

    *hWnd = m_pDocHolder->GetTopMostWinHandle();
    if(*hWnd != NULL)
        return NOERROR;
    else
        return ERROR;
}

STDMETHODIMP EmbedDocument_Impl::ContextSensitiveHelp(BOOL)
{
    return NOERROR;
}

STDMETHODIMP EmbedDocument_Impl::InPlaceDeactivate(void)
{
    // no locking is used since the OLE must use the same thread always
    if ( m_bIsInVerbHandling )
        return E_UNEXPECTED;

    BooleanGuard_Impl aGuard( m_bIsInVerbHandling );

    m_pDocHolder->InPlaceDeactivate();

    // the inplace object needs the notification after the storing ( on deactivating )
    // if it happens before the storing the replacement might not be updated
    notify();

    return NOERROR;
}

STDMETHODIMP EmbedDocument_Impl::UIDeactivate(void)
{
    // no locking is used since the OLE must use the same thread always
    if ( m_bIsInVerbHandling )
        return E_UNEXPECTED;

    BooleanGuard_Impl aGuard( m_bIsInVerbHandling );


    m_pDocHolder->UIDeactivate();

    // the inplace object needs the notification after the storing ( on deactivating )
    // if it happens before the storing the replacement might not be updated
    notify();

    return NOERROR;
}

STDMETHODIMP EmbedDocument_Impl::SetObjectRects(LPCRECT aRect, LPCRECT aClip)
{
    OSL_ENSURE(m_pDocHolder,"no document for inplace activation");

    return m_pDocHolder->SetObjectRects(aRect,aClip);
}

STDMETHODIMP EmbedDocument_Impl::ReactivateAndUndo(void)
{
    return E_NOTIMPL;
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif
