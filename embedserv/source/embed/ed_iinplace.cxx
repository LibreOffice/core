/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <embeddoc.hxx>
#include <osl/diagnose.h>

COM_DECLSPEC_NOTHROW STDMETHODIMP EmbedDocument_Impl::GetWindow(HWND* hWnd)
{
    OSL_ENSURE(m_pDocHolder, "no document for inplace activation");

    *hWnd = m_pDocHolder->GetTopMostWinHandle();
    if (*hWnd != nullptr)
        return NOERROR;
    else
        return ERROR;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP EmbedDocument_Impl::ContextSensitiveHelp(BOOL) { return NOERROR; }

COM_DECLSPEC_NOTHROW STDMETHODIMP EmbedDocument_Impl::InPlaceDeactivate()
{
    // no locking is used since the OLE must use the same thread always
    if (m_bIsInVerbHandling)
        return E_UNEXPECTED;

    BooleanGuard_Impl aGuard(m_bIsInVerbHandling);

    m_pDocHolder->InPlaceDeactivate();

    // the inplace object needs the notification after the storing ( on deactivating )
    // if it happens before the storing the replacement might not be updated
    notify();

    return NOERROR;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP EmbedDocument_Impl::UIDeactivate()
{
    // no locking is used since the OLE must use the same thread always
    if (m_bIsInVerbHandling)
        return E_UNEXPECTED;

    BooleanGuard_Impl aGuard(m_bIsInVerbHandling);

    m_pDocHolder->UIDeactivate();

    // the inplace object needs the notification after the storing ( on deactivating )
    // if it happens before the storing the replacement might not be updated
    notify();

    return NOERROR;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP EmbedDocument_Impl::SetObjectRects(LPCRECT aRect, LPCRECT aClip)
{
    OSL_ENSURE(m_pDocHolder, "no document for inplace activation");

    return m_pDocHolder->SetObjectRects(aRect, aClip);
}

COM_DECLSPEC_NOTHROW STDMETHODIMP EmbedDocument_Impl::ReactivateAndUndo() { return E_NOTIMPL; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
