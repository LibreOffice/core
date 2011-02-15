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

#ifndef SC_PAGEDATA_HXX
#define SC_PAGEDATA_HXX

#include "global.hxx"
#include "address.hxx"

class ScDocShell;

//============================================================================

class ScPrintRangeData
{
private:
    ScRange     aPrintRange;
    size_t      nPagesX;
    SCCOL*      pPageEndX;
    size_t      nPagesY;
    SCROW*      pPageEndY;
    long        nFirstPage;
    sal_Bool        bTopDown;
    sal_Bool        bAutomatic;

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
    void            SetTopDown( sal_Bool bSet )     { bTopDown = bSet; }
    sal_Bool            IsTopDown() const           { return bTopDown; }
    void            SetAutomatic( sal_Bool bSet )   { bAutomatic = bSet; }
    sal_Bool            IsAutomatic() const         { return bAutomatic; }
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

    sal_Bool                IsEqual( const ScPageBreakData& rOther ) const;

    void                AddPages();
};



#endif

