/*************************************************************************
 *
 *  $RCSfile: scimpexpmsg.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: gt $ $Date: 2001-02-14 09:26:29 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SC_SCIMPEXPMSG_HXX
#define SC_SCIMPEXPMSG_HXX


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


