/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prnsave.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:49:17 $
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

#ifndef SC_PRNSAVE_HXX
#define SC_PRNSAVE_HXX

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include <vector>

class ScRange;

class ScPrintSaverTab
{
    typedef ::std::vector< ScRange > ScRangeVec;

    ScRangeVec  maPrintRanges;      // Array
    ScRange*    mpRepeatCol;        // einzeln
    ScRange*    mpRepeatRow;        // einzeln
    BOOL        mbEntireSheet;

public:
            ScPrintSaverTab();
            ~ScPrintSaverTab();

    void            SetAreas( const ScRangeVec& rRanges, BOOL bEntireSheet );
    void            SetRepeat( const ScRange* pCol, const ScRange* pRow );

    const ScRangeVec&   GetPrintRanges() const  { return maPrintRanges; }
    BOOL                IsEntireSheet() const   { return mbEntireSheet; }
    const ScRange*      GetRepeatCol() const    { return mpRepeatCol; }
    const ScRange*      GetRepeatRow() const    { return mpRepeatRow; }

    BOOL    operator==( const ScPrintSaverTab& rCmp ) const;
};

class ScPrintRangeSaver
{
    SCTAB               nTabCount;
    ScPrintSaverTab*    pData;      // Array

public:
            ScPrintRangeSaver( SCTAB nCount );
            ~ScPrintRangeSaver();

    SCTAB                   GetTabCount() const     { return nTabCount; }
    ScPrintSaverTab&        GetTabData(SCTAB nTab);
    const ScPrintSaverTab&  GetTabData(SCTAB nTab) const;

    BOOL    operator==( const ScPrintRangeSaver& rCmp ) const;
};


#endif


