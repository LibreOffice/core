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

#ifndef SC_PAGEDATA_HXX
#define SC_PAGEDATA_HXX

#include "global.hxx"
#include "address.hxx"

class ScPrintRangeData
{
private:
    ScRange     aPrintRange;
    size_t      nPagesX;
    SCCOL*      pPageEndX;
    size_t      nPagesY;
    SCROW*      pPageEndY;
    long        nFirstPage;
    bool        bTopDown;
    bool        bAutomatic;

public:
                ScPrintRangeData();
                ~ScPrintRangeData();

    void            SetPrintRange( const ScRange& rNew )    { aPrintRange = rNew; }
    const ScRange&  GetPrintRange() const                   { return aPrintRange; }

    void            SetPagesX( size_t nCount, const SCCOL* pEnd );
    void            SetPagesY( size_t nCount, const SCROW* pEnd );

    size_t          GetPagesX() const       { return nPagesX;   }
    const SCCOL*    GetPageEndX() const     { return pPageEndX; }
    size_t          GetPagesY() const       { return nPagesY;   }
    const SCROW*    GetPageEndY() const     { return pPageEndY; }

    void            SetFirstPage( long nNew )   { nFirstPage = nNew; }
    long            GetFirstPage() const        { return nFirstPage; }
    void            SetTopDown( bool bSet )     { bTopDown = bSet; }
    bool            IsTopDown() const           { return bTopDown; }
    void            SetAutomatic( bool bSet )   { bAutomatic = bSet; }
    bool            IsAutomatic() const         { return bAutomatic; }
};

class ScPageBreakData
{
private:
    size_t              nAlloc;
    size_t              nUsed;
    ScPrintRangeData*   pData;          // Array

public:
                ScPageBreakData(size_t nMax);
                ~ScPageBreakData();

    size_t              GetCount() const            { return nUsed; }
    ScPrintRangeData&   GetData(size_t i);

    bool                operator==( const ScPageBreakData& rOther ) const;

    void                AddPages();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
