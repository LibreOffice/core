/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _TABCOL_HXX
#define _TABCOL_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_LONGS
#define _SVSTDARR_BOOLS
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
    sal_Bool    bHidden;    //Fuer jeden Eintrag ein Flag, Hidden oder nicht.
                        //Wenn das Flag Hidden sal_True ist liegt der Spalten-
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
    SwTabCols( sal_uInt16 nSize = 0 );
    SwTabCols( const SwTabCols& );
    SwTabCols &operator=( const SwTabCols& );
    sal_Bool operator==( const SwTabCols& rCmp ) const;
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
