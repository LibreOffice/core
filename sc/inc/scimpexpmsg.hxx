/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scimpexpmsg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:54:06 $
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

#ifndef SC_SCIMPEXPMSG_HXX
#define SC_SCIMPEXPMSG_HXX


#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

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


