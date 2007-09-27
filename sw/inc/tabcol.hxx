/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tabcol.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:13:02 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _TABCOL_HXX
#define _TABCOL_HXX

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_LONGS
#define _SVSTDARR_BOOLS
#include <svtools/svstdarr.hxx>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

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
