/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
    ((sal_uLong)((((sal_uLong)(char)(char1)))|              \
    (((sal_uLong)(char)(char2))<<8UL)|                  \
    (((sal_uLong)(char)(char3))<<16UL)|                 \
    ((sal_uLong)(char)(char4))<<24UL))


class SvStream;

// --------------
// - ImplCompat -
// --------------

class ImplCompat
{
    SvStream*   mpRWStm;
    sal_uInt32      mnCompatPos;
    sal_uInt32      mnTotalSize;
    sal_uInt16      mnStmMode;
    sal_uInt16      mnVersion;

                ImplCompat() {}
                ImplCompat( const ImplCompat& rCompat ) {}
    ImplCompat& operator=( const ImplCompat& rCompat ) { return *this; }
    sal_Bool        operator==( const ImplCompat& rCompat ) { return sal_False; }

public:
                ImplCompat( SvStream& rStm, sal_uInt16 nStreamMode, sal_uInt16 nVersion = 1 );
                ~ImplCompat();

    sal_uInt16      GetVersion() const { return mnVersion; }
};

#endif // _SV_SVCOMPAT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
