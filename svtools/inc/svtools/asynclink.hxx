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
