/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: proxy.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:39:10 $
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

#include <uno/environment.h>
#include <uno/dispatcher.h>
#include <uno/mapping.hxx>

#include <bridges/remote/remote.h>

namespace bridges_remote {

extern "C" typedef void SAL_CALL FreeRemote2UnoProxy(
    uno_ExtEnvironment * environment, void * proxy);
FreeRemote2UnoProxy freeRemote2UnoProxy;

// private:
extern "C" typedef void SAL_CALL AcquireRemote2UnoProxy(uno_Interface *);
AcquireRemote2UnoProxy acquireRemote2UnoProxy;

// private:
extern "C" typedef void SAL_CALL ReleaseRemote2UnoProxy(uno_Interface *);
ReleaseRemote2UnoProxy releaseRemote2UnoProxy;

// private:
extern "C" typedef void SAL_CALL DispatchRemote2UnoProxy(
    uno_Interface *, typelib_TypeDescription const *, void *, void **,
    uno_Any **);
DispatchRemote2UnoProxy dispatchRemote2UnoProxy;

extern "C" void SAL_CALL remote_release( void * );

class Remote2UnoProxy :
    public uno_Interface
{
public:
    Remote2UnoProxy(
        remote_Interface *pRemoteI,
        rtl_uString *pOid,
        typelib_InterfaceTypeDescription *pType ,
        uno_Environment *pEnvUno,
        uno_Environment *pEnvRemote
        );

    ~Remote2UnoProxy();

private:
    ::rtl::OUString m_sOid;
    typelib_InterfaceTypeDescription *m_pType;
    remote_Interface *m_pRemoteI;
    uno_Environment *m_pEnvUno;
    uno_Environment *m_pEnvRemote;
    ::com::sun::star::uno::Mapping m_mapRemote2Uno;
    ::com::sun::star::uno::Mapping m_mapUno2Remote;

    oslInterlockedCount m_nRef;

    friend void SAL_CALL acquireRemote2UnoProxy(uno_Interface *);

    friend void SAL_CALL releaseRemote2UnoProxy(uno_Interface *);

    friend void SAL_CALL dispatchRemote2UnoProxy(
        uno_Interface *, typelib_TypeDescription const *, void *, void **,
        uno_Any **);
};

}
