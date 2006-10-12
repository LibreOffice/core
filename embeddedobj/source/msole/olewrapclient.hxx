/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olewrapclient.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 11:23:42 $
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

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <platform.h>

class OleComponent;
class OleWrapperClientSite : public IOleClientSite
{
    osl::Mutex m_aMutex;
    oslInterlockedCount m_nRefCount;
    OleComponent* m_pOleComp;

public:
    OleWrapperClientSite( OleComponent* pOleComp );
    virtual ~OleWrapperClientSite(void);

    void disconnectOleComponent();

    STDMETHODIMP QueryInterface(REFIID, void**);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    STDMETHODIMP SaveObject(void);
    STDMETHODIMP GetMoniker(DWORD, DWORD, LPMONIKER *);
    STDMETHODIMP GetContainer(LPOLECONTAINER *);
    STDMETHODIMP ShowObject(void);
    STDMETHODIMP OnShowWindow(BOOL);
    STDMETHODIMP RequestNewObjectLayout(void);
};

