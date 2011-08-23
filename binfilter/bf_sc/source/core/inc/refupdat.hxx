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

#ifndef SC_REFUPDAT_HXX
#define SC_REFUPDAT_HXX

#include "global.hxx"
namespace binfilter {

class ScDocument;
class ScBigRange;
struct ComplRefData;

enum ScRefUpdateRes {
    UR_NOTHING	= 0,		// keine Anpassungen
    UR_UPDATED	= 1,		// Anpassungen erfolgt
    UR_INVALID	= 2			// Referenz wurde ungueltig
};

class ScRefUpdate
{
public:
    static ScRefUpdateRes Update
        ( ScDocument* pDoc, UpdateRefMode eUpdateRefMode,
                            USHORT nCol1, USHORT nRow1, USHORT nTab1,
                            USHORT nCol2, USHORT nRow2, USHORT nTab2,
                            short nDx, short nDy, short nDz,
                            USHORT& theCol1, USHORT& theRow1, USHORT& theTab1,
                            USHORT& theCol2, USHORT& theRow2, USHORT& theTab2 );

     static ScRefUpdateRes Update( UpdateRefMode eUpdateRefMode,
                                 const ScBigRange& rWhere,
                                 INT32 nDx, INT32 nDy, INT32 nDz,
                                 ScBigRange& rWhat );

     static ScRefUpdateRes Update( ScDocument* pDoc,
                                   UpdateRefMode eUpdateRefMode,
                                   const ScAddress& rPos, const ScRange& rRange,
                                   short nDx, short nDy, short nDz,
                                   ComplRefData& rRef );
     static ScRefUpdateRes Move( ScDocument* pDoc, const ScAddress& rPos,
                                 short nDx, short nDy, short nDz,
                                 ComplRefData& rRef, BOOL bWrap, BOOL bAbsolute );
    static void MoveRelWrap( ScDocument* pDoc, const ScAddress& rPos,
                                ComplRefData& rRef );
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
