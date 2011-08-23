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
#ifndef _SPLCSTK_HXX
#define _SPLCSTK_HXX

#include <tools/solar.h>
namespace binfilter {

class SwIntrnlCrsr;
class SwCrsrShell;
struct SwPosition;

class SwSpellCrsrStk
{
    SwPosition* pErrorPos;
    SwIntrnlCrsr * pCurCrsr;		// der aktuelle Cursor
    SwIntrnlCrsr * pCrsrStk;		// der Cursor-Stack
    SwCrsrShell & rCrsrShell;		// die akt. CrsrShell

public:
    SwSpellCrsrStk( SwCrsrShell& rShell );
    ~SwSpellCrsrStk();

    void SetCurCrsr();          // akt. Cursor in der CrsrShell mit akt. setzen
    BOOL SetNextCrsr();			// gehe zum naechsten Cursor im Ring (falls am
                                // Ende returne FALSE;
    void RestoreCrsrs();		// alle Cursor aus dem CursorStack wieder
                                // in der Shell setzen
    void StoreErrPos();			// speicher die akt. Start-Position als Error,
                                // an dieser wird spaeter wieder aufgesetzt
    void SetErrPos();			// akt. Cursor auf die Error Position setzen

#if 0
                            // ??? wird das ueberhaupt benoetigt ???
                                // fuer die EditShell, damit sie die Cursor
                                // aus den Loeschbereichen verschieben kann
    SwPaM* GetCurCrsr() const;
    SwPosition* GetErrorPos() const { return pErrorPos; }
#endif
};


} //namespace binfilter
#endif	// _SPLCSTK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
