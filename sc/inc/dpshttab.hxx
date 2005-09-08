/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dpshttab.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:36:03 $
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

#ifndef SC_DPSHTTAB_HXX
#define SC_DPSHTTAB_HXX

#ifndef SC_DPTABDAT_HXX
#include "dptabdat.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif


// --------------------------------------------------------------------
//
//  implementation of ScDPTableData with sheet data
//

struct ScSheetSourceDesc
{
    ScRange         aSourceRange;
    ScQueryParam    aQueryParam;

    BOOL operator== ( const ScSheetSourceDesc& rOther ) const
        { return aSourceRange == rOther.aSourceRange &&
                 aQueryParam  == rOther.aQueryParam; }
};

class ScSheetDPData_Impl;

class ScSheetDPData : public ScDPTableData
{
private:
    ScSheetDPData_Impl* pImpl;
    BOOL* pSpecial;

public:
                    ScSheetDPData( ScDocument* pD, const ScSheetSourceDesc& rDesc );
    virtual         ~ScSheetDPData();

    virtual long                    GetColumnCount();
    virtual const TypedStrCollection&   GetColumnEntries(long nColumn);
    virtual String                  getDimensionName(long nColumn);
    virtual BOOL                    getIsDataLayoutDimension(long nColumn);
    virtual BOOL                    IsDateDimension(long nDim);
    virtual UINT32                  GetNumberFormat(long nDim);
    virtual void                    DisposeData();
    virtual void                    SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty );

    virtual void                    ResetIterator();
    virtual BOOL                    GetNextRow( const ScDPTableIteratorParam& rParam );
};



#endif

