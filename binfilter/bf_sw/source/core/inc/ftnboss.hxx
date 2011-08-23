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
#ifndef _FTNBOSS_HXX
#define _FTNBOSS_HXX

#include "layfrm.hxx"
namespace binfilter {

class SwFtnBossFrm;
class SwFtnContFrm;
class SwFtnFrm;
class SwTxtFtn;


//Setzen des maximalen Fussnotenbereiches. Restaurieren des alten Wertes im DTor.
//Implementierung im ftnfrm.cxx
class SwSaveFtnHeight
{
    SwFtnBossFrm *pBoss;
    const SwTwips nOldHeight;
    SwTwips nNewHeight;
public:
    SwSaveFtnHeight( SwFtnBossFrm *pBs, const SwTwips nDeadLine );
    ~SwSaveFtnHeight();
};

#define NA_ONLY_ADJUST 0
#define NA_GROW_SHRINK 1
#define NA_GROW_ADJUST 2
#define NA_ADJUST_GROW 3

class SwFtnBossFrm: public SwLayoutFrm
{
    //Fuer die privaten Fussnotenoperationen
    friend class SwFrm;
    friend class SwSaveFtnHeight;
    friend class SwPageFrm; // fuer das Setzen der MaxFtnHeight

    //Maximale Hoehe des Fussnotencontainers fuer diese Seite.
    SwTwips nMaxFtnHeight;

    SwFtnContFrm *MakeFtnCont();
    SwFtnFrm	 *FindFirstFtn();
    BYTE _NeighbourhoodAdjustment( const SwFrm* pFrm ) const;
protected:

    void		  InsertFtn( SwFtnFrm * );
    static void	  ResetFtn( const SwFtnFrm *pAssumed );
public:
    inline SwFtnBossFrm( SwFrmFmt* pFmt) : SwLayoutFrm( pFmt ) {}

                 SwLayoutFrm *FindBodyCont();
    inline const SwLayoutFrm *FindBodyCont() const;
    inline void SetMaxFtnHeight( const SwTwips nNewMax ) { nMaxFtnHeight = nNewMax; }

    //Fussnotenschnittstelle
    void AppendFtn( SwCntntFrm *, SwTxtFtn * );
    void RemoveFtn( const SwCntntFrm *, const SwTxtFtn *, BOOL bPrep = TRUE );
    static		 SwFtnFrm 	  *FindFtn( const SwCntntFrm *, const SwTxtFtn * );
                 SwFtnContFrm *FindFtnCont();
    inline const SwFtnContFrm *FindFtnCont() const;
           const SwFtnFrm	  *FindFirstFtn( SwCntntFrm* ) const;
                 SwFtnContFrm *FindNearestFtnCont( BOOL bDontLeave = FALSE );

    void RearrangeFtns( const SwTwips nDeadLine, const BOOL bLock = FALSE,
                        const SwTxtFtn *pAttr = 0 );

    //SS damit der Textformatierer Temporaer die Fussnotenhoehe begrenzen
    //kann. DeadLine in Dokumentkoordinaten.
    void	SetFtnDeadLine( const SwTwips nDeadLine );
    SwTwips GetMaxFtnHeight() const { return nMaxFtnHeight; }

    //Liefert den Wert, der noch uebrig ist, bis der Body seine minimale
    //Hoehe erreicht hat.
    SwTwips GetVarSpace() const;

        //Layoutseitig benoetigte Methoden
    /// OD 03.04.2003 #108446# - add parameters <_bCollectOnlyPreviousFtns> and
    /// <_pRefFtnBossFrm> in order to control, if only footnotes, which are positioned
    /// before the given reference footnote boss frame have to be collected.
    /// Note: if parameter <_bCollectOnlyPreviousFtns> is true, then parameter
    /// <_pRefFtnBossFrm> have to be referenced to an object.
    /// OD 03.04.2003 #108446# - add parameter <_bCollectOnlyPreviousFtns> in
    /// order to control, if only footnotes, which are positioned before the
    /// footnote boss frame <this> have to be collected.
    // Sollte AdjustNeighbourhood gerufen werden (oder Grow/Shrink)?
    BYTE NeighbourhoodAdjustment( const SwFrm* pFrm ) const
        { return IsPageFrm() ? NA_ONLY_ADJUST : _NeighbourhoodAdjustment( pFrm ); }
};

inline const SwLayoutFrm *SwFtnBossFrm::FindBodyCont() const
{
    return ((SwFtnBossFrm*)this)->FindBodyCont();
}
inline const SwFtnContFrm *SwFtnBossFrm::FindFtnCont() const
{
    return ((SwFtnBossFrm*)this)->FindFtnCont();
}

} //namespace binfilter
#endif	//_FTNBOSS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
