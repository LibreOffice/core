/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_INC_IDPOOL_HXX
#define INCLUDED_SFX2_INC_IDPOOL_HXX

#include <limits.h>
#include "bitset.hxx"

// class IdPool ----------------------------------------------------------

class IdPool: private BitSet
{
private:
    sal_uInt16 nNextFree;
    sal_uInt16 nRange;
    sal_uInt16 nOffset;
public:
    bool Lock( const BitSet& rLockSet );
    bool IsLocked( sal_uInt16 nId ) const;
    IdPool( sal_uInt16 nMin = 1, sal_uInt16 nMax = USHRT_MAX );
    sal_uInt16 Get();
    bool Put( sal_uInt16 nId );
    bool Lock( const Range& rRange );
    bool Lock( sal_uInt16 nId );

};



// returns sal_True if the id is locked

inline bool IdPool::IsLocked( sal_uInt16 nId ) const
{
    return ( this->Contains(nId-nOffset) );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
