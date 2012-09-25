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

#include <tools/solar.h>

#include <vector>
#include "swdllapi.h"

struct SwTabColsEntry
{
    long    nPos;
    long    nMin;
    long    nMax;
    sal_Bool    bHidden; // For each entry a flag, hidden or not.
                         // If the flag bHidden is sal_True column separator
                         // is not in current line. It must maintained
                         // but it may not be displayed.
};

typedef std::vector< SwTabColsEntry > SwTabColsEntries;

class SW_DLLPUBLIC SwTabCols
{
    long nLeftMin,      // Leftmost border (reference point) for
                        // document coordinates.
                        // All other values are relative to this point!
            nLeft,      // Left border of table.
           nRight,      // Right border of table.
           nRightMax;   // Maximum right border of table.

    bool bLastRowAllowedToChange;       // If the last row of the table frame
                                        // is split across pages, it may not
                                        // change its size.

    SwTabColsEntries aData;

    //For the CopyCTor.
    const SwTabColsEntries& GetData() const { return aData; }

public:
    SwTabCols( sal_uInt16 nSize = 0 );
    SwTabCols( const SwTabCols& );
    SwTabCols &operator=( const SwTabCols& );
    bool operator==( const SwTabCols& rCmp ) const;
    long& operator[]( sal_uInt16 nPos ) { return aData[nPos].nPos; }
    long operator[]( sal_uInt16 nPos ) const { return aData[nPos].nPos; }
    sal_uInt16 Count() const { return sal::static_int_cast< sal_uInt16 >(aData.size()); }

    sal_Bool IsHidden( sal_uInt16 nPos ) const         { return aData[nPos].bHidden; }
    void SetHidden( sal_uInt16 nPos, sal_Bool bValue ) { aData[nPos].bHidden = bValue; }

    void Insert( long nValue, sal_Bool bValue, sal_uInt16 nPos );
    void Insert( long nValue, long nMin, long nMax, sal_Bool bValue, sal_uInt16 nPos );
    void Remove( sal_uInt16 nPos, sal_uInt16 nAnz = 1 );

    const SwTabColsEntry& GetEntry( sal_uInt16 nPos ) const { return aData[nPos]; }
          SwTabColsEntry& GetEntry( sal_uInt16 nPos )  { return aData[nPos]; }

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
