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
#ifndef _TABCOL_HXX
#define _TABCOL_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_LONGS
#include <svl/svstdarr.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif
#include "swdllapi.h"

struct SwTabColsEntry
{
    long    nPos;
    long    nMin;
    long    nMax;
    BOOL    bHidden;    //Fuer jeden Eintrag ein Flag, Hidden oder nicht.
                        //Wenn das Flag Hidden TRUE ist liegt der Spalten-
                        //trenner nicht in der aktuellen Zeile; er muss
                        //mit gepflegt werden, darf aber nicht angezeigt
                        //werden.
};

typedef std::vector< SwTabColsEntry > SwTabColsEntries;

class SW_DLLPUBLIC SwTabCols
{
    long nLeftMin,      //Linker aeusserer Rand (Bezugspunkt) in
                        //Dokumentkordinaten.
                        //Alle anderen Werte relativ zu diesem Punkt!
            nLeft,      //Linker Rand der Tabelle.
           nRight,      //Rechter Rand der Tabelle.
           nRightMax;   //Maximaler rechter Rand der Tabelle.

    bool bLastRowAllowedToChange;       // if the last row of the table frame
                                        // is split across pages, it may not
                                        // change its size

    SwTabColsEntries aData;

    //fuer den CopyCTor
    const SwTabColsEntries& GetData() const { return aData; }

public:
    SwTabCols( USHORT nSize = 0 );
    SwTabCols( const SwTabCols& );
    SwTabCols &operator=( const SwTabCols& );
    BOOL operator==( const SwTabCols& rCmp ) const;
    long& operator[]( USHORT nPos ) { return aData[nPos].nPos; }
    long operator[]( USHORT nPos ) const { return aData[nPos].nPos; }
    USHORT Count() const { return sal::static_int_cast< USHORT >(aData.size()); }

    BOOL IsHidden( USHORT nPos ) const         { return aData[nPos].bHidden; }
    void SetHidden( USHORT nPos, BOOL bValue ) { aData[nPos].bHidden = bValue; }

    void Insert( long nValue, BOOL bValue, USHORT nPos );
    void Insert( long nValue, long nMin, long nMax, BOOL bValue, USHORT nPos );
    void Remove( USHORT nPos, USHORT nAnz = 1 );

    const SwTabColsEntry& GetEntry( USHORT nPos ) const { return aData[nPos]; }
          SwTabColsEntry& GetEntry( USHORT nPos )  { return aData[nPos]; }

    long GetLeftMin() const { return nLeftMin; }
    long GetLeft()  const { return nLeft;    }
    long GetRight() const { return nRight;   }
    long GetRightMax()const { return nRightMax;}

    void SetLeftMin ( long nNew )   { nLeftMin = nNew; }
    void SetLeft    ( long nNew )   { nLeft = nNew;    }
    void SetRight   ( long nNew )   { nRight = nNew;   }
    void SetRightMax( long nNew )   { nRightMax = nNew;}

    bool IsLastRowAllowedToChange() const { return bLastRowAllowedToChange; }
    void SetLastRowAllowedToChange( bool bNew ) { bLastRowAllowedToChange = bNew; }
};

#endif  //_TABCOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
