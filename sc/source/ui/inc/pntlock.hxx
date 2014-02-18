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
    sal_uInt16      nLevel;
    sal_uInt16      nDocLevel;
    sal_uInt16      nParts;
    bool            bModified;

public:
                    ScPaintLockData();
                    ~ScPaintLockData();

    void            AddRange( const ScRange& rRange, sal_uInt16 nP );

    void            SetModified()   { bModified = true; }
    void            IncLevel(bool bDoc)
                        { if (bDoc) ++nDocLevel; else ++nLevel; }
    void            DecLevel(bool bDoc)
                        { if (bDoc) --nDocLevel; else --nLevel; }

    const ScRangeListRef&   GetRangeList() const            { return xRangeList; }
    sal_uInt16              GetParts() const                { return nParts; }
    sal_uInt16              GetLevel(bool bDoc) const
                                { return bDoc ? nDocLevel : nLevel; }
    bool                    GetModified() const             { return bModified; }

    /** for recovery after reset */
    void            SetLevel(sal_uInt16 nNew, bool bDoc)
                        { if (bDoc) nDocLevel = nNew; else nLevel = nNew; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
