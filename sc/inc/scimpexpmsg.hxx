/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scimpexpmsg.hxx,v $
 * $Revision: 1.4.32.2 $
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

#ifndef SC_SCIMPEXPMSG_HXX
#define SC_SCIMPEXPMSG_HXX


#include <tools/string.hxx>
#include <tools/solar.h>

//UNUSED2008-05  enum ScImpExpMsg
//UNUSED2008-05  {
//UNUSED2008-05      SC_IMPEXPMSG_UNKNOWN
//UNUSED2008-05  };
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  class String;
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  class ScImpExpLogMsg
//UNUSED2008-05  {
//UNUSED2008-05  private:
//UNUSED2008-05      ScImpExpMsg             eId;
//UNUSED2008-05      String*                 pPos;
//UNUSED2008-05      String*                 pHint;
//UNUSED2008-05  protected:
//UNUSED2008-05  public:
//UNUSED2008-05                              ScImpExpLogMsg( ScImpExpMsg eId );
//UNUSED2008-05                              ScImpExpLogMsg( ScImpExpMsg eId, const String& rPosition );
//UNUSED2008-05                              ScImpExpLogMsg( ScImpExpMsg eId, const String& rPosition, const String& rAdditionalHint );
//UNUSED2008-05                              ScImpExpLogMsg( const ScImpExpLogMsg& rCpy );
//UNUSED2008-05      virtual                 ~ScImpExpLogMsg();
//UNUSED2008-05
//UNUSED2008-05      ScImpExpLogMsg&         operator =( const ScImpExpLogMsg& rCpy );
//UNUSED2008-05
//UNUSED2008-05      void                    Set( ScImpExpMsg eId, const String* pPos = NULL, const String* pHint = NULL );
//UNUSED2008-05
//UNUSED2008-05      inline ScImpExpMsg      GetId( void ) const;
//UNUSED2008-05      inline const String*    GetPos( void ) const;
//UNUSED2008-05      inline const String*    GetHint( void ) const;
//UNUSED2008-05
//UNUSED2008-05      static String           GetMsg( ScImpExpMsg eId );
//UNUSED2008-05      inline String           GetMsg( void ) const;
//UNUSED2008-05  };
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  inline ScImpExpLogMsg& ScImpExpLogMsg::operator =( const ScImpExpLogMsg& r )
//UNUSED2008-05  {
//UNUSED2008-05      Set( r.eId, r.pPos, r.pHint );
//UNUSED2008-05      return *this;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  inline ScImpExpMsg ScImpExpLogMsg::GetId( void ) const
//UNUSED2008-05  {
//UNUSED2008-05      return eId;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  inline const String* ScImpExpLogMsg::GetPos( void ) const
//UNUSED2008-05  {
//UNUSED2008-05      return pPos;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  inline const String* ScImpExpLogMsg::GetHint( void ) const
//UNUSED2008-05  {
//UNUSED2008-05      return pHint;
//UNUSED2008-05  }
//UNUSED2008-05
//UNUSED2008-05
//UNUSED2008-05  inline String ScImpExpLogMsg::GetMsg( void ) const
//UNUSED2008-05  {
//UNUSED2008-05      return GetMsg( eId );
//UNUSED2008-05  }

#endif


