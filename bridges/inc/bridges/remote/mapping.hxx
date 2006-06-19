/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mapping.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:38:47 $
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
#ifndef _BRIDGES_REMOTE_MAPPING_HXX_
#define _BRIDGES_REMOTE_MAPPING_HXX_

#include <osl/interlck.h>
#include <rtl/ustring.hxx>

#include <typelib/typedescription.h>

#include <bridges/remote/remote.h>

typedef struct _uno_Environment uno_Environment;

namespace bridges_remote
{
    extern "C" typedef void SAL_CALL RemoteToUno(
        uno_Mapping *pMapping, void **ppOut, void *pInterface,
        typelib_InterfaceTypeDescription *pInterfaceTypeDescr );
    RemoteToUno remoteToUno;

    extern "C" typedef void SAL_CALL UnoToRemote(
        uno_Mapping *pMapping, void **ppOut, void *pInterface,
        typelib_InterfaceTypeDescription *pInterfaceTypeDescr );
    UnoToRemote unoToRemote;

    extern "C" typedef void SAL_CALL FreeRemoteMapping(uno_Mapping * mapping);
    FreeRemoteMapping freeRemoteMapping;

    class RemoteMapping :
        public remote_Mapping
    {
    public:
        RemoteMapping( uno_Environment *pEnvUno ,
                       uno_Environment *pEnvRemote,
                       uno_MapInterfaceFunc func ,
                       const ::rtl::OUString sPurpose);
        ~RemoteMapping();

        oslInterlockedCount m_nRef;
        ::rtl::OUString m_sPurpose;
    };

}
#endif
