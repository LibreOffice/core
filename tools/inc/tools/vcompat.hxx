/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vcompat.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:21:17 $
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
#ifndef _VCOMPAT_HXX
#define _VCOMPAT_HXX

#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

// -----------
// - Defines -
// -----------

#define COMPAT_FORMAT( char1, char2, char3, char4 ) \
    ((UINT32)((((UINT32)(char)(char1)))|                \
    (((UINT32)(char)(char2))<<8UL)|                 \
    (((UINT32)(char)(char3))<<16UL)|                    \
    ((UINT32)(char)(char4))<<24UL))

// --------------
// - ImplCompat -
// --------------

class SvStream;

class TOOLS_DLLPUBLIC VersionCompat
{
    SvStream*       mpRWStm;
    UINT32          mnCompatPos;
    UINT32          mnTotalSize;
    UINT16          mnStmMode;
    UINT16          mnVersion;

                    VersionCompat() {}
                    VersionCompat( const VersionCompat& ) {}
    VersionCompat&  operator=( const VersionCompat& ) { return *this; }
    BOOL            operator==( const VersionCompat& ) { return FALSE; }

public:

                    VersionCompat( SvStream& rStm, USHORT nStreamMode, UINT16 nVersion = 1 );
                    ~VersionCompat();

    UINT16          GetVersion() const { return mnVersion; }
};

#endif // _VCOMPAT_HXX
