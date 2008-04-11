/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svcompat.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_SVCOMPAT_HXX
#define _SV_SVCOMPAT_HXX

#include <sv.h>

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
