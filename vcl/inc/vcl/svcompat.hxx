/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svcompat.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:02:02 $
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

#ifndef _SV_SVCOMPAT_HXX
#define _SV_SVCOMPAT_HXX

#ifndef _SV_SV_H
#include <sv.h>
#endif

// -----------
// - Defines -
// -----------

#define COMPAT_FORMAT( char1, char2, char3, char4 ) \
    ((ULONG)((((ULONG)(char)(char1)))|              \
    (((ULONG)(char)(char2))<<8UL)|                  \
    (((ULONG)(char)(char3))<<16UL)|                 \
    ((ULONG)(char)(char4))<<24UL))


class SvStream;

// --------------
// - ImplCompat -
// --------------

class ImplCompat
{
    SvStream*   mpRWStm;
    UINT32      mnCompatPos;
    UINT32      mnTotalSize;
    UINT16      mnStmMode;
    UINT16      mnVersion;

                ImplCompat() {}
                ImplCompat( const ImplCompat& rCompat ) {}
    ImplCompat& operator=( const ImplCompat& rCompat ) { return *this; }
    BOOL        operator==( const ImplCompat& rCompat ) { return FALSE; }

public:
                ImplCompat( SvStream& rStm, USHORT nStreamMode, UINT16 nVersion = 1 );
                ~ImplCompat();

    UINT16      GetVersion() const { return mnVersion; }
};

#endif // _SV_SVCOMPAT_HXX
