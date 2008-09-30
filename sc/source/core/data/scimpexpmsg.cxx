/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scimpexpmsg.cxx,v $
 * $Revision: 1.5.32.3 $
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

//UNUSED2008-05  ScImpExpLogMsg::ScImpExpLogMsg( ScImpExpMsg e ) : eId( e ), pPos( NULL ), pHint( NULL )
//UNUSED2008-05  {
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  ScImpExpLogMsg::ScImpExpLogMsg( ScImpExpMsg e, const String& r ) : eId( e ), pHint( NULL )
//UNUSED2008-05  {
//UNUSED2008-05      pPos = new String( r );
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  ScImpExpLogMsg::ScImpExpLogMsg( ScImpExpMsg e, const String& rP, const String& rH ) : eId( e )
//UNUSED2008-05  {
//UNUSED2008-05      pPos = new String( rP );
//UNUSED2008-05      pHint = new String( rH );
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05  ScImpExpLogMsg::ScImpExpLogMsg( const ScImpExpLogMsg& r ) : eId( r.eId )
//UNUSED2008-05  {
//UNUSED2008-05      if( r.pPos )
//UNUSED2008-05          pPos = new String( *r.pPos );
//UNUSED2008-05      else
//UNUSED2008-05          pPos = NULL;
//UNUSED2008-05
//UNUSED2008-05      if( r.pHint )
//UNUSED2008-05          pHint = new String( *r.pHint );
//UNUSED2008-05      else
//UNUSED2008-05          pHint = NULL;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  ScImpExpLogMsg::~ScImpExpLogMsg()
//UNUSED2008-05  {
//UNUSED2008-05      if( pPos )
//UNUSED2008-05          delete pPos;
//UNUSED2008-05
//UNUSED2008-05      if( pHint )
//UNUSED2008-05          delete pHint;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  void ScImpExpLogMsg::Set( ScImpExpMsg e, const String* pP, const String* pH )
//UNUSED2008-05  {
//UNUSED2008-05      eId = e;
//UNUSED2008-05      if( pPos )
//UNUSED2008-05          delete pPos;
//UNUSED2008-05
//UNUSED2008-05      if( pHint )
//UNUSED2008-05          delete pHint;
//UNUSED2008-05
//UNUSED2008-05      if( pP )
//UNUSED2008-05          pPos = new String( *pP );
//UNUSED2008-05      else
//UNUSED2008-05          pPos = NULL;
//UNUSED2008-05
//UNUSED2008-05      if( pH )
//UNUSED2008-05          pHint = new String( *pH );
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  String ScImpExpLogMsg::GetMsg( ScImpExpMsg e )
//UNUSED2008-05  {
//UNUSED2008-05      const sal_Char*     p;
//UNUSED2008-05      switch( e )
//UNUSED2008-05      {
//UNUSED2008-05          case SC_IMPEXPMSG_UNKNOWN:  p = "unknown log message";                  break;
//UNUSED2008-05          default:                    p = "Not specified type of log message";
//UNUSED2008-05      }
//UNUSED2008-05
//UNUSED2008-05      String  aRet;
//UNUSED2008-05      aRet.AssignAscii( p );
//UNUSED2008-05      return aRet;
//UNUSED2008-05  }

