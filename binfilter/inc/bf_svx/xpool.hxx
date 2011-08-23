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

#ifndef _XPOOL_HXX
#define _XPOOL_HXX

#include <bf_svtools/itempool.hxx>
#include <bf_svx/xdef.hxx>
namespace binfilter {

/*************************************************************************
|*
|* Klassendeklaration
|*
\************************************************************************/

class XOutdevItemPool: public SfxItemPool
{
protected:
    SfxPoolItem**   ppPoolDefaults;
    SfxItemInfo*    pItemInfos;
/*    USHORT*         pVersion1Map;
    USHORT*         pVersion2Map;
    USHORT*         pVersion3Map;
    USHORT*         pVersion4Map;*/
    USHORT          nStart, nEnd;

private:
    void Ctor(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd);

public:
    // Kontstruktion des Pools als MasterPool
    XOutdevItemPool(USHORT nAttrStart = XATTR_START,
                    USHORT nAttrEnd = XATTR_END,
                    FASTBOOL bLoadRefCounts = TRUE);

    // Damit meine SetItems mit dem MasterPool konstruiert werden koennen.
    // Der SdrItemPool wird dabei automatisch als Secondary an den
    // uebergebenen MasterPool bzw. an's Ende einer bestehenden Pool-Kette
    // angehaengt.
    XOutdevItemPool(SfxItemPool* pMaster,
                    USHORT nAttrStart = XATTR_START,
                    USHORT nAttrEnd = XATTR_END,
                    FASTBOOL bLoadRefCounts = TRUE);

    XOutdevItemPool(const XOutdevItemPool& rPool);

    virtual SfxItemPool* Clone() const;

    ~XOutdevItemPool();
};

}//end of namespace binfilter
#endif      // _XPOOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
