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

#ifndef SC_PFUNCACHE_HXX
#define SC_PFUNCACHE_HXX

#include "rangelst.hxx"
namespace binfilter {

class ScDocShell;
class ScMarkData;


/** Possible types of selection for print functions */

enum ScPrintSelectionMode
{
    SC_PRINTSEL_INVALID,
    SC_PRINTSEL_DOCUMENT,
    SC_PRINTSEL_CURSOR,
    SC_PRINTSEL_RANGE
};


/** Stores the selection in the ScPrintFuncCache so it is only used
    for the same selection again. */

class ScPrintSelectionStatus
{
    ScPrintSelectionMode	eMode;
    ScRangeList				aRanges;

public:
            ScPrintSelectionStatus() : eMode(SC_PRINTSEL_INVALID) {}
            ~ScPrintSelectionStatus() {}

    void	SetMode(ScPrintSelectionMode eNew)	{ eMode = eNew; }
    void	SetRanges(const ScRangeList& rNew)	{ aRanges = rNew; }

    BOOL	operator==(const ScPrintSelectionStatus& rOther) const
            { return eMode == rOther.eMode && aRanges == rOther.aRanges; }
};


/** Stores the data for printing that is needed from several sheets,
    so it doesn't have to be calculated for rendering each page. */

class ScPrintFuncCache
{
    ScPrintSelectionStatus	aSelection;
    ScDocShell*				pDocSh;
    long					nTotalPages;
    long					nPages[MAXTAB+1];
    long					nFirstAttr[MAXTAB+1];

public:
            ScPrintFuncCache( ScDocShell* pD, const ScMarkData& rMark,
                                const ScPrintSelectionStatus& rStatus );
            ~ScPrintFuncCache();

    BOOL	IsSameSelection( const ScPrintSelectionStatus& rStatus ) const;

    long	GetPageCount() const				{ return nTotalPages; }
    long	GetFirstAttr( USHORT nTab ) const	{ return nFirstAttr[nTab]; }
    USHORT	GetTabForPage( long nPage ) const;
    long	GetTabStart( USHORT nTab ) const;
    long	GetDisplayStart( USHORT nTab ) const;
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
