/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: txtflcnt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:22:05 $
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
#ifndef _TXTFLCNT_HXX
#define _TXTFLCNT_HXX

#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif

class SwFlyInCntFrm;
class SwFrm;
class SwTxtNode;
class SwDoc;

// SWTXT_FLYCNT ********************************
// Attribut fuer zeilengebundene Frames (ohne Endindex)

class SwTxtFlyCnt : public SwTxtAttr
{
    SwFlyInCntFrm  *_GetFlyFrm( const SwFrm *pCurrFrm );

public:
    SwTxtFlyCnt( const SwFmtFlyCnt& rAttr, xub_StrLen nStart );

    // Setzt den Anker im pFmt und
    void    SetAnchor( const SwTxtNode *pNode );

    inline        SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm );
    inline const  SwFlyInCntFrm  *GetFlyFrm( const SwFrm *pCurrFrm ) const;

    // erzeugt sich ein neues FlyFrameFormat
    void CopyFlyFmt( SwDoc* pDoc );
};

inline SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm )
{
    return _GetFlyFrm( pCurrFrm );
}

inline const SwFlyInCntFrm *SwTxtFlyCnt::GetFlyFrm( const SwFrm *pCurrFrm ) const
{
    return ((SwTxtFlyCnt*)this)->_GetFlyFrm( pCurrFrm );
}

#endif

