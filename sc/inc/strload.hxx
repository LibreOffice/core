/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strload.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:58:55 $
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

#ifndef SC_STRLOAD_HXX
#define SC_STRLOAD_HXX

#ifndef SC_SCRESID_HXX
#include "scresid.hxx"
#endif

#ifndef _TOOLS_RC_HXX //autogen
#include <tools/rc.hxx>
#endif

class ScRscStrLoader : public Resource
{
public:
    ScRscStrLoader( USHORT nRsc, USHORT nStrId ) :
        Resource( ScResId( nRsc ) ), theStr( ScResId( nStrId ) )
    {
        FreeResource();
    }

    const String& GetString() const { return theStr; }

private:
    String theStr;
};


#endif // SC_STRLOAD_HXX

