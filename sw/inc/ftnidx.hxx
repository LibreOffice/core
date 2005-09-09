/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ftnidx.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:55:15 $
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
#ifndef _FTNIDX_HXX
#define _FTNIDX_HXX


#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>

class SwTxtFtn;
class SwNodeIndex;
class SwSectionNode;

// ueberall, wo der NodeIndex gebraucht wird, werden die hier fehlenden
// Headerfiles schon includes. Darum hier nur als define und nicht als
// inline Methode (spart Compile-Zeit)
#define _SwTxtFtn_GetIndex( pFIdx ) (pFIdx->GetTxtNode().GetIndex())


typedef SwTxtFtn* SwTxtFtnPtr;
SV_DECL_PTRARR_SORT( _SwFtnIdxs, SwTxtFtnPtr, 0, 10 )

class SwFtnIdxs : public _SwFtnIdxs
{
public:
    SwFtnIdxs() {}

    void UpdateFtn( const SwNodeIndex& rStt );      // ab Pos. alle Updaten
    void UpdateAllFtn();                    // alle Fussnoten updaten

    SwTxtFtn* SeekEntry( const SwNodeIndex& rIdx, USHORT* pPos = 0 ) const;
};


class SwUpdFtnEndNtAtEnd
{
    SvPtrarr aFtnSects, aEndSects;
    SvUShorts aFtnNums, aEndNums;

public:
    SwUpdFtnEndNtAtEnd() : aFtnSects( 0, 4 ), aEndSects( 0, 4 ),
                           aFtnNums( 0, 4 ), aEndNums( 0, 4 )
        {}

    static const SwSectionNode* FindSectNdWithEndAttr(
                                            const SwTxtFtn& rTxtFtn );

    USHORT GetNumber( const SwTxtFtn& rTxtFtn, const SwSectionNode& rNd );
    USHORT ChkNumber( const SwTxtFtn& rTxtFtn );
};



#endif // _FTNIDX_HXX

