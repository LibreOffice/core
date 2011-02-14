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

#ifndef SVTOOLS_ASYNCLINK_HXX
#define  SVTOOLS_ASYNCLINK_HXX

#include "svtools/svtdllapi.h"
#include <tools/solar.h>
#include <tools/link.hxx>

class Timer;

namespace vos
{
    class OMutex;
}

namespace svtools {

class SVT_DLLPUBLIC AsynchronLink
{
    Link   _aLink;
    sal_uLong  _nEventId;
    Timer* _pTimer;
    sal_Bool   _bInCall;
    sal_Bool*  _pDeleted;
    void*  _pArg;
    vos::OMutex* _pMutex;

    DECL_DLLPRIVATE_STATIC_LINK( AsynchronLink, HandleCall, void* );
    SVT_DLLPRIVATE void Call_Impl( void* pArg );

public:
    AsynchronLink( const Link& rLink ) :
        _aLink( rLink ), _nEventId( 0 ), _pTimer( 0 ), _bInCall( sal_False ),
        _pDeleted( 0 ), _pMutex( 0 ){}
    AsynchronLink() : _nEventId( 0 ), _pTimer( 0 ), _bInCall( sal_False ),
            _pDeleted( 0 ), _pMutex( 0 ){}
    ~AsynchronLink();

    void CreateMutex();
    void operator=( const Link& rLink ) { _aLink = rLink; }
    void Call( void* pObj, sal_Bool bAllowDoubles = sal_False,
               sal_Bool bUseTimer = sal_False );
    void ForcePendingCall( );
    void ClearPendingCall( );
    sal_Bool IsSet() const { return _aLink.IsSet(); }
    Link GetLink() const { return _aLink; }
};

}

#endif
