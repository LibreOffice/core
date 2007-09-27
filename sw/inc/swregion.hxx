/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swregion.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:11:52 $
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
#ifndef _SWREGION_HXX
#define _SWREGION_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

#include "swrect.hxx"

SV_DECL_VARARR( SwRects, SwRect, 20, 8 )

class SwRegionRects : public SwRects
{
    SwRect aOrigin; // die Kopie des StartRects

    inline void InsertRect( const SwRect &rRect, const USHORT nPos, BOOL &rDel);

public:
    SwRegionRects( const SwRect& rStartRect, USHORT nInit = 20,
                                             USHORT nGrow = 8 );
    // Zum Ausstanzen aus aOrigin.
    void operator-=( const SwRect& rRect );

    // Aus Loechern werden Flaechen, aus Flaechen werden Loecher.
    void Invert();
    // Benachbarte Rechtecke zusammenfassen.
    void Compress( BOOL bFuzzy = TRUE );

    inline const SwRect &GetOrigin() const { return aOrigin; }
    inline void ChangeOrigin( const SwRect &rRect ) { aOrigin = rRect; }
};

#endif  //_SWREGION_HXX
