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

#ifndef SC_DPSHTTAB_HXX
#define SC_DPSHTTAB_HXX

#include "dptabdat.hxx"

#include "global.hxx"
namespace binfilter {


// --------------------------------------------------------------------
//
//	implementation of ScDPTableData with sheet data
//

struct ScSheetSourceDesc
{
    ScRange			aSourceRange;
    ScQueryParam	aQueryParam;

    BOOL operator==	( const ScSheetSourceDesc& rOther ) const
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
    virtual			~ScSheetDPData();

    virtual long					GetColumnCount();
    virtual const TypedStrCollection&	GetColumnEntries(long nColumn);
    virtual String					getDimensionName(long nColumn);
    virtual BOOL					getIsDataLayoutDimension(long nColumn);
    virtual BOOL					IsDateDimension(long nDim);
    virtual UINT32					GetNumberFormat(long nDim);
    virtual void					DisposeData();
    virtual void					SetEmptyFlags( BOOL bIgnoreEmptyRows, BOOL bRepeatIfEmpty );

    virtual void					ResetIterator();
    virtual BOOL					GetNextRow( const ScDPTableIteratorParam& rParam );
};



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
