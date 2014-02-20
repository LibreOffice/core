/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_TABCOL_HXX
#define INCLUDED_SW_INC_TABCOL_HXX

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

#endif // INCLUDED_SW_INC_TABCOL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
