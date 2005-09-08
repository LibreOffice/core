/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asynclink.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:09:13 $
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

#ifndef SVTOOLS_ASYNCLINK_HXX
#define  SVTOOLS_ASYNCLINK_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

class Timer;

namespace vos
{
    class OMutex;
}

namespace svtools {

class SVT_DLLPUBLIC AsynchronLink
{
    Link   _aLink;
    ULONG  _nEventId;
    Timer* _pTimer;
    BOOL   _bInCall;
    BOOL*  _pDeleted;
    void*  _pArg;
    vos::OMutex* _pMutex;

    DECL_DLLPRIVATE_STATIC_LINK( AsynchronLink, HandleCall, void* );
    SVT_DLLPRIVATE void Call_Impl( void* pArg );

public:
    AsynchronLink( const Link& rLink ) :
        _aLink( rLink ), _nEventId( 0 ), _pTimer( 0 ), _bInCall( FALSE ),
        _pDeleted( 0 ), _pMutex( 0 ){}
    AsynchronLink() : _nEventId( 0 ), _pTimer( 0 ), _bInCall( FALSE ),
            _pDeleted( 0 ), _pMutex( 0 ){}
    ~AsynchronLink();

    void CreateMutex();
    void operator=( const Link& rLink ) { _aLink = rLink; }
    void Call( void* pObj, BOOL bAllowDoubles = FALSE,
               BOOL bUseTimer = FALSE );
    void ForcePendingCall( );
    void ClearPendingCall( );
    BOOL IsSet() const { return _aLink.IsSet(); }
    Link GetLink() const { return _aLink; }
};

}

#endif
