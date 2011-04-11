/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright IBM Corporation 2009.
 * Copyright 2009 by Sun Microsystems, Inc.
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
#ifndef SCDPOUTPUTIMPL_HXX
#define SCDPOUTPUTIMPL_HXX

#include "document.hxx"

#define SC_DP_FRAME_INNER_BOLD      20
#define SC_DP_FRAME_OUTER_BOLD      40

#define SC_DP_FRAME_COLOR           Color(0,0,0) //( 0x20, 0x40, 0x68 )

class OutputImpl
{
    ScDocument*         mpDoc;
    sal_uInt16              mnTab;
    ::std::vector< bool > mbNeedLineCols;
    ::std::vector< SCCOL > mnCols;

    ::std::vector< bool > mbNeedLineRows;
    ::std::vector< SCROW > mnRows;

    SCCOL   mnTabStartCol;
    SCROW   mnTabStartRow;
    SCCOL   mnMemberStartCol;
    SCROW   mnMemberStartRow;

    SCCOL   mnDataStartCol;
    SCROW   mnDataStartRow;
    SCCOL   mnTabEndCol;
    SCROW   mnTabEndRow;

public:
    OutputImpl( ScDocument* pDoc, sal_uInt16 nTab,
        SCCOL   nTabStartCol,
        SCROW   nTabStartRow,
        SCCOL   nMemberStartCol,
        SCROW   nMemberStartRow,
        SCCOL nDataStartCol,
        SCROW nDataStartRow,
        SCCOL nTabEndCol,
        SCROW nTabEndRow );
    sal_Bool AddRow( SCROW nRow );
    sal_Bool AddCol( SCCOL nCol );

    void OutputDataArea();
    void OutputBlockFrame ( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, sal_Bool bHori = false );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
