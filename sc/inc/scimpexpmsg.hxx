/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scimpexpmsg.hxx,v $
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

#ifndef SC_SCIMPEXPMSG_HXX
#define SC_SCIMPEXPMSG_HXX


#include <tools/string.hxx>
#include <tools/solar.h>

enum ScImpExpMsg
{
    SC_IMPEXPMSG_UNKNOWN
};


class String;


class ScImpExpLogMsg
{
private:
    ScImpExpMsg             eId;
    String*                 pPos;
    String*                 pHint;
protected:
public:
                            ScImpExpLogMsg( ScImpExpMsg eId );
                            ScImpExpLogMsg( ScImpExpMsg eId, const String& rPosition );
                            ScImpExpLogMsg( ScImpExpMsg eId, const String& rPosition, const String& rAdditionalHint );
                            ScImpExpLogMsg( const ScImpExpLogMsg& rCpy );
    virtual                 ~ScImpExpLogMsg();

    ScImpExpLogMsg&         operator =( const ScImpExpLogMsg& rCpy );

    void                    Set( ScImpExpMsg eId, const String* pPos = NULL, const String* pHint = NULL );

    inline ScImpExpMsg      GetId( void ) const;
    inline const String*    GetPos( void ) const;
    inline const String*    GetHint( void ) const;

    static String           GetMsg( ScImpExpMsg eId );
    inline String           GetMsg( void ) const;
};




inline ScImpExpLogMsg& ScImpExpLogMsg::operator =( const ScImpExpLogMsg& r )
{
    Set( r.eId, r.pPos, r.pHint );
    return *this;
}


inline ScImpExpMsg ScImpExpLogMsg::GetId( void ) const
{
    return eId;
}


inline const String* ScImpExpLogMsg::GetPos( void ) const
{
    return pPos;
}


inline const String* ScImpExpLogMsg::GetHint( void ) const
{
    return pHint;
}


inline String ScImpExpLogMsg::GetMsg( void ) const
{
    return GetMsg( eId );
}


#endif


