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
#ifndef _SFX_WHMAP_HXX
#define _SFX_WHMAP_HXX

#include <solar.h>

namespace binfilter
{

struct SfxWhichMapEntry
{
    USHORT nWhichId;
    USHORT nSlotId;
};

class SfxWhichMap
{
    const SfxWhichMapEntry *pMap;

public:
    inline				SfxWhichMap( const SfxWhichMapEntry *pMappings = 0 );
    virtual             ~SfxWhichMap();

    int 				IsNull() const
                        { return 0 == pMap; }
    virtual USHORT		GetWhich(USHORT nSlot) const;
    virtual USHORT		GetSlot(USHORT nWhich) const;
    virtual SfxWhichMap*Clone() const;
};

inline SfxWhichMap::SfxWhichMap( const SfxWhichMapEntry *pMappings ):
    pMap( pMappings )
{
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
