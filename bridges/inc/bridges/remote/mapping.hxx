/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: mapping.hxx,v $
 * $Revision: 1.4 $
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
