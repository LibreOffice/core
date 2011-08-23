/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#include "socont.h"


namespace binfilter {

/*
 * CImpIOleClientSite::CImpIOleClientSite
 * CImpIOleClientSite::~CImpIOleClientSite
 *
 * Parameters (Constructor):
 *  pSO_Cont         PCSO_Cont of the tenant we're in.
 *  pUnkOuter       LPUNKNOWN to which we delegate.
 */

CImpIOleClientSite::CImpIOleClientSite(PCSO_Cont pSO_Cont
    , LPUNKNOWN pUnkOuter)
    {
    m_cRef=0;
    m_pTen=pSO_Cont;
    m_pUnkOuter=pUnkOuter;
    return;
    }

CImpIOleClientSite::~CImpIOleClientSite(void)
    {
    return;
    }




/*
 * CImpIOleClientSite::QueryInterface
 * CImpIOleClientSite::AddRef
 * CImpIOleClientSite::Release
 *
 * Purpose:
 *  IUnknown members for CImpIOleClientSite object.
 */

STDMETHODIMP CImpIOleClientSite::QueryInterface(REFIID riid
    , PPVOID ppv)
    {
    return m_pUnkOuter->QueryInterface(riid, ppv);
    }


STDMETHODIMP_(ULONG) CImpIOleClientSite::AddRef(void)
    {
    ++m_cRef;
    return m_pUnkOuter->AddRef();
    }

STDMETHODIMP_(ULONG) CImpIOleClientSite::Release(void)
    {
    --m_cRef;
    return m_pUnkOuter->Release();
    }




/*
 * CImpIOleClientSite::SaveObject
 *
 * Purpose:
 *  Requests that the container call OleSave for the object that
 *  lives here.  Typically this happens on server shutdown.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  HRESULT         Standard.
 */

STDMETHODIMP CImpIOleClientSite::SaveObject(void)
    {
    //We're already set up with the tenant to save; this is trivial.
        vos::IMutex& mutex= Application::GetSolarMutex();
        sal_Bool bAquired= mutex.tryToAcquire();
    m_pTen->Update();
    if( bAquired)
        mutex.release();
    return NOERROR;
    }





/*
 * CImpIOleClientSite::GetMoniker
 *
 * Purpose:
 *  Retrieves the moniker for the site in which this object lives,
 *  either the moniker relative to the container or the full
 *  moniker.
 *
 * Parameters:
 *  dwAssign        DWORD specifying that the object wants moniker
 *                  assignment.  Yeah.  Right.  Got any bridges to
 *                  sell?
 *  dwWhich         DWORD identifying which moniker the object
 *                  wants, either the container's moniker, the
 *                  moniker relative to this client site, or the
 *                  full moniker.
 *
 * Return Value:
 *  HRESULT         Standard.
 */

STDMETHODIMP CImpIOleClientSite::GetMoniker(DWORD
    , DWORD, LPMONIKER *ppmk)
    {
    *ppmk=NULL;

    /*
     * We don't yet want to allow for linking to embedded objects
     * within us, so we just fail for now.  If you are only a simple
     * container you never have to implement this function.
     */
    return ResultFromScode(E_NOTIMPL);
    }






/*
 * CImpIOleClientSite::GetContainer
 *
 * Purpose:
 *  Returns a pointer to the document's IOleContainer interface.
 *
 * Parameters:
 *  ppContainer     LPOLECONTAINER * in which to return the
 *                  interface.
 *
 * Return Value:
 *  HRESULT         Standard.
 */

STDMETHODIMP CImpIOleClientSite::GetContainer(LPOLECONTAINER
    * ppContainer)
    {
    //Only necessary if you allow linking to embeddings
    *ppContainer=NULL;
    return ResultFromScode(E_NOTIMPL);
    }






/*
 * CImpIOleClientSite::ShowObject
 *
 * Purpose:
 *  Tells the container to bring the object fully into view as much
 *  as possible, that is, scroll the document.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  HRESULT         Standard.
 */

STDMETHODIMP CImpIOleClientSite::ShowObject(void)
    {
    /*
     * We let the tenant do this, since it can access the current
     * scroll position as a friend of CPages whereas we cannot.
     */
    m_pTen->ShowYourself();
    return NOERROR;
    }






/*
 * CImpIOleClientSite::OnShowWindow
 *
 * Purpose:
 *  Informs the container if the object is showing itself or
 *  hiding itself.  This is done only in the opening mode and allows
 *  the container to know when to shade or unshade the object.
 *
 * Parameters:
 *  fShow           BOOL indiciating that the object is being shown
 *                  (TRUE) or hidden (FALSE).
 * Return Value:
 *  HRESULT         Standard.
 */

STDMETHODIMP CImpIOleClientSite::OnShowWindow(WIN_BOOL fShow)
    {
    //All we have to do is tell the tenant of the open state change.
    m_pTen->ShowAsOpen(fShow);
    return NOERROR;
    }






/*
 * CImpIOleClientSite::RequestNewObjectLayout
 *
 * Purpose:
 *  Called when the object needs more room in the container.
 *
 * Parameters:
 *  None
 *
 * Return Value:
 *  HRESULT         Standard.
 */

STDMETHODIMP CImpIOleClientSite::RequestNewObjectLayout(void)
    {
    return ResultFromScode(E_NOTIMPL);
    }

}
