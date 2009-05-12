/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ed_iinplace.cxx,v $
 * $Revision: 1.6 $
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

#include "embeddoc.hxx"
#include <osl/diagnose.h>


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
