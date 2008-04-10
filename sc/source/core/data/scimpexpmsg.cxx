/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scimpexpmsg.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "document.hxx"
#include "scimpexpmsg.hxx"

#include <tools/string.hxx>


void ScDocument::AddToImpExpLog( const ScImpExpLogMsg& /* r */ )
{
}


void ScDocument::AddToImpExpLog( ScImpExpLogMsg* p )
{
    delete p;
}




ScImpExpLogMsg::ScImpExpLogMsg( ScImpExpMsg e ) : eId( e ), pPos( NULL ), pHint( NULL )
{
}


ScImpExpLogMsg::ScImpExpLogMsg( ScImpExpMsg e, const String& r ) : eId( e ), pHint( NULL )
{
    pPos = new String( r );
}


ScImpExpLogMsg::ScImpExpLogMsg( ScImpExpMsg e, const String& rP, const String& rH ) : eId( e )
{
    pPos = new String( rP );
    pHint = new String( rH );
}

ScImpExpLogMsg::ScImpExpLogMsg( const ScImpExpLogMsg& r ) : eId( r.eId )
{
    if( r.pPos )
        pPos = new String( *r.pPos );
    else
        pPos = NULL;

    if( r.pHint )
        pHint = new String( *r.pHint );
    else
        pHint = NULL;
}


ScImpExpLogMsg::~ScImpExpLogMsg()
{
    if( pPos )
        delete pPos;

    if( pHint )
        delete pHint;
}


void ScImpExpLogMsg::Set( ScImpExpMsg e, const String* pP, const String* pH )
{
    eId = e;
    if( pPos )
        delete pPos;

    if( pHint )
        delete pHint;

    if( pP )
        pPos = new String( *pP );
    else
        pPos = NULL;

    if( pH )
        pHint = new String( *pH );
}


String ScImpExpLogMsg::GetMsg( ScImpExpMsg e )
{
    const sal_Char*     p;
    switch( e )
    {
        case SC_IMPEXPMSG_UNKNOWN:  p = "unknown log message";                  break;
        default:                    p = "Not specified type of log message";
    }

    String  aRet;
    aRet.AssignAscii( p );
    return aRet;
}

