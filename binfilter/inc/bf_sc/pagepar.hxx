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

#ifndef SC_PAGEPAR_HXX
#define SC_PAGEPAR_HXX

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif
namespace binfilter {

//-----------------------------------------------------------------------

struct ScPageTableParam
{
    BOOL	bNotes;
    BOOL	bGrid;
    BOOL	bHeaders;
    BOOL	bCharts;
    BOOL	bObjects;
    BOOL	bDrawings;
    BOOL	bFormulas;
    BOOL	bNullVals;
    BOOL	bTopDown;
    BOOL	bLeftRight;
    BOOL	bSkipEmpty;
    BOOL	bScaleNone;
    BOOL	bScaleAll;
    BOOL	bScalePageNum;
    USHORT	nScaleAll;
    USHORT	nScalePageNum;
    USHORT	nFirstPageNo;

    ScPageTableParam();
    ScPageTableParam( const ScPageTableParam& r );
    ~ScPageTableParam();

    void				Reset		();
};

struct ScPageAreaParam
{
    BOOL	bPrintArea;
    BOOL	bRepeatRow;
    BOOL	bRepeatCol;
    ScRange	aPrintArea;
    ScRange	aRepeatRow;
    ScRange	aRepeatCol;

    ScPageAreaParam();
    ScPageAreaParam( const ScPageAreaParam& r );
    ~ScPageAreaParam();

    void				Reset		();
};


} //namespace binfilter
#endif


