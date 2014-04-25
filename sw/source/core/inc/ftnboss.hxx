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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FTNBOSS_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FTNBOSS_HXX

#include "layfrm.hxx"

class SwFtnBossFrm;
class SwFtnContFrm;
class SwFtnFrm;
class SwTxtFtn;

// Set max. footnote area.
// Restoration of the old value in DTor. Implementation in ftnfrm.cxx
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

typedef std::vector<SwFtnFrm*> SwFtnFrms;

class SwFtnBossFrm: public SwLayoutFrm
{
    // for private footnote operations
    friend class SwFrm;
    friend class SwSaveFtnHeight;
    friend class SwPageFrm; // for setting of MaxFtnHeight

    // max. height of the footnote container on this page
    SwTwips nMaxFtnHeight;

    SwFtnContFrm *MakeFtnCont();
    SwFtnFrm     *FindFirstFtn();
    sal_uInt8 _NeighbourhoodAdjustment( const SwFrm* pFrm ) const;

protected:
    void          InsertFtn( SwFtnFrm * );
    static void   ResetFtn( const SwFtnFrm *pAssumed );

public:
    inline SwFtnBossFrm( SwFrmFmt* pFmt, SwFrm* pSib )
        : SwLayoutFrm( pFmt, pSib )
        , nMaxFtnHeight(0)
        {}

                 SwLayoutFrm *FindBodyCont();
    inline const SwLayoutFrm *FindBodyCont() const;
    inline void SetMaxFtnHeight( const SwTwips nNewMax ) { nMaxFtnHeight = nNewMax; }

    // footnote interface
    void AppendFtn( SwCntntFrm *, SwTxtFtn * );
    void RemoveFtn( const SwCntntFrm *, const SwTxtFtn *, bool bPrep = true );
    static       SwFtnFrm     *FindFtn( const SwCntntFrm *, const SwTxtFtn * );
                 SwFtnContFrm *FindFtnCont();
    inline const SwFtnContFrm *FindFtnCont() const;
           const SwFtnFrm     *FindFirstFtn( SwCntntFrm* ) const;
                 SwFtnContFrm *FindNearestFtnCont( bool bDontLeave = false );

    void ChangeFtnRef( const SwCntntFrm *pOld, const SwTxtFtn *,
                       SwCntntFrm *pNew );
    void RearrangeFtns( const SwTwips nDeadLine, const bool bLock = false,
                        const SwTxtFtn *pAttr = 0 );

    // Set DeadLine (in document coordinates) so that the text formatter can
    // temporarily limit footnote height.
    void    SetFtnDeadLine( const SwTwips nDeadLine );
    SwTwips GetMaxFtnHeight() const { return nMaxFtnHeight; }

    // returns value for remaining space until the body reaches minimal height
    SwTwips GetVarSpace() const;

    // methods needed for layouting
    // The parameters <_bCollectOnlyPreviousFtns> and <_pRefFtnBossFrm> control
    // if only footnotes that are positioned before the given reference
    // footnote boss-frame have to be collected.
    // Note: if parameter <_bCollectOnlyPreviousFtns> is true, then parameter
    // <_pRefFtnBossFrm> has to be referenced by an object.
    static void _CollectFtns( const SwCntntFrm*   _pRef,
                              SwFtnFrm*           _pFtn,
                              SwFtnFrms&          _rFtnArr,
                              const bool      _bCollectOnlyPreviousFtns = false,
                              const SwFtnBossFrm* _pRefFtnBossFrm = NULL);
    // The parameter <_bCollectOnlyPreviousFtns> controls if only footnotes
    // that are positioned before the footnote boss-frame <this> have to be
    // collected.
    void    CollectFtns( const SwCntntFrm* _pRef,
                         SwFtnBossFrm*     _pOld,
                         SwFtnFrms&        _rFtnArr,
                         const bool    _bCollectOnlyPreviousFtns = false );
    void    _MoveFtns( SwFtnFrms &rFtnArr, bool bCalc = false );
    void    MoveFtns( const SwCntntFrm *pSrc, SwCntntFrm *pDest,
                      SwTxtFtn *pAttr );

    // should AdjustNeighbourhood be called (or Grow/Shrink)?
    sal_uInt8 NeighbourhoodAdjustment( const SwFrm* pFrm ) const
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
