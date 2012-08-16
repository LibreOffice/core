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

#ifndef SC_PNTLOCK_HXX
#define SC_PNTLOCK_HXX

#include "rangelst.hxx"

class ScPaintLockData
{
private:
    ScRangeListRef  xRangeList;
    sal_uInt16          nMode;
    sal_uInt16          nLevel;
    sal_uInt16          nDocLevel;
    sal_uInt16          nParts;
    sal_Bool            bModified;

public:
                    ScPaintLockData(sal_uInt16 nNewMode);
                    ~ScPaintLockData();

    void            AddRange( const ScRange& rRange, sal_uInt16 nP );

    void            SetModified()   { bModified = sal_True; }
    void            IncLevel(sal_Bool bDoc)
                        { if (bDoc) ++nDocLevel; else ++nLevel; }
    void            DecLevel(sal_Bool bDoc)
                        { if (bDoc) --nDocLevel; else --nLevel; }

    const ScRangeListRef&   GetRangeList() const            { return xRangeList; }
    sal_uInt16                  GetParts() const                { return nParts; }
    sal_uInt16                  GetLevel(sal_Bool bDoc) const
                                { return bDoc ? nDocLevel : nLevel; }
    sal_Bool                    GetModified() const             { return bModified; }

    /** for recovery after reset */
    void            SetLevel(sal_uInt16 nNew, sal_Bool bDoc)
                        { if (bDoc) nDocLevel = nNew; else nLevel = nNew; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
