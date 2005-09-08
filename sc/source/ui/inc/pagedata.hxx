/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pagedata.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:42:47 $
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

#ifndef SC_PAGEDATA_HXX
#define SC_PAGEDATA_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

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
    BOOL        bTopDown;
    BOOL        bAutomatic;

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
    void            SetTopDown( BOOL bSet )     { bTopDown = bSet; }
    BOOL            IsTopDown() const           { return bTopDown; }
    void            SetAutomatic( BOOL bSet )   { bAutomatic = bSet; }
    BOOL            IsAutomatic() const         { return bAutomatic; }
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

    BOOL                IsEqual( const ScPageBreakData& rOther ) const;

    void                AddPages();
};



#endif

