/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
