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
#define _SVSTDARR_BOOLS
#include <bf_svtools/svstdarr.hxx>
#endif
namespace binfilter {

class SwTabCols : public SvLongs
{
    long nLeftMin,		//Linker aeusserer Rand (Bezugspunkt) in
                        //Dokumentkordinaten.
                        //Alle anderen Werte relativ zu diesem Punkt!
            nLeft,		//Linker Rand der Tabelle.
           nRight,		//Rechter Rand der Tabelle.
           nRightMax;	//Maximaler rechter Rand der Tabelle.



    SvBools aHidden;	//Fuer jeden Eintrag ein Flag, Hidden oder nicht.
                        //Wenn das Flag Hidden TRUE ist liegt der Spalten-
                        //trenner nicht in der aktuellen Zeile; er muss
                        //mit gepflegt werden, darf aber nicht angezeigt
                        //werden.

public:
        SwTabCols( USHORT nSize = 0 ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	SwTabCols( USHORT nSize = 0 );
        SwTabCols( const SwTabCols& ){DBG_BF_ASSERT(0, "STRIP");} ;//STRIP001 	SwTabCols( const SwTabCols& );

    BOOL IsHidden( USHORT nPos ) const 		   { return aHidden[nPos]; }
    void SetHidden( USHORT nPos, BOOL bValue ) { aHidden[nPos] = bValue; }
    inline void InsertHidden( USHORT nPos, BOOL bValue );
    inline void DeleteHidden( USHORT nPos, USHORT nAnz = 1 );

    //fuer den CopyCTor
    const SvBools& GetHidden() const { return aHidden; }

    long GetLeftMin() const { return nLeftMin; }
    long GetLeft()	const { return nLeft;	 }
    long GetRight()	const { return nRight;	 }
    long GetRightMax()const { return nRightMax;}

    void SetLeftMin ( long nNew )	{ nLeftMin = nNew; }
    void SetLeft	( long nNew )	{ nLeft = nNew;	   }
    void SetRight	( long nNew )	{ nRight = nNew;   }
    void SetRightMax( long nNew )	{ nRightMax = nNew;}
};

inline void SwTabCols::InsertHidden( USHORT nPos, BOOL bValue )
{
    aHidden.Insert( bValue, nPos );
}
inline void SwTabCols::DeleteHidden( USHORT nPos, USHORT nAnz )
{
    aHidden.Remove( nPos, nAnz );
}

} //namespace binfilter
#endif	//_TABCOL_HXX
