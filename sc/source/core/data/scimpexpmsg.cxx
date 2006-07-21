/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scimpexpmsg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 11:08:05 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include "document.hxx"
#include "scimpexpmsg.hxx"

#include <tools/string.hxx>


void ScDocument::AddToImpExpLog( const ScImpExpLogMsg& r )
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

