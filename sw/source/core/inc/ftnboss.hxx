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

class SwFootnoteBossFrm;
class SwFootnoteContFrm;
class SwFootnoteFrm;
class SwTextFootnote;

// Set max. footnote area.
// Restoration of the old value in DTor. Implementation in ftnfrm.cxx
class SwSaveFootnoteHeight
{
    SwFootnoteBossFrm *pBoss;
    const SwTwips nOldHeight;
    SwTwips nNewHeight;
public:
    SwSaveFootnoteHeight( SwFootnoteBossFrm *pBs, const SwTwips nDeadLine );
    ~SwSaveFootnoteHeight();
};

#define NA_ONLY_ADJUST 0
#define NA_GROW_SHRINK 1
#define NA_GROW_ADJUST 2
#define NA_ADJUST_GROW 3

typedef std::vector<SwFootnoteFrm*> SwFootnoteFrms;

class SwFootnoteBossFrm: public SwLayoutFrm
{
    // for private footnote operations
    friend class SwFrm;
    friend class SwSaveFootnoteHeight;
    friend class SwPageFrm; // for setting of MaxFootnoteHeight

    // max. height of the footnote container on this page
    SwTwips nMaxFootnoteHeight;

    SwFootnoteContFrm *MakeFootnoteCont();
    SwFootnoteFrm     *FindFirstFootnote();
    sal_uInt8 _NeighbourhoodAdjustment( const SwFrm* pFrm ) const;

protected:
    void          InsertFootnote( SwFootnoteFrm * );
    static void   ResetFootnote( const SwFootnoteFrm *pAssumed );

public:
    inline SwFootnoteBossFrm( SwFrameFormat* pFormat, SwFrm* pSib )
        : SwLayoutFrm( pFormat, pSib )
        , nMaxFootnoteHeight(0)
        {}

                 SwLayoutFrm *FindBodyCont();
    inline const SwLayoutFrm *FindBodyCont() const;
    inline void SetMaxFootnoteHeight( const SwTwips nNewMax ) { nMaxFootnoteHeight = nNewMax; }

    // footnote interface
    void AppendFootnote( SwContentFrm *, SwTextFootnote * );
    void RemoveFootnote( const SwContentFrm *, const SwTextFootnote *, bool bPrep = true );
    static       SwFootnoteFrm     *FindFootnote( const SwContentFrm *, const SwTextFootnote * );
                 SwFootnoteContFrm *FindFootnoteCont();
    inline const SwFootnoteContFrm *FindFootnoteCont() const;
           const SwFootnoteFrm     *FindFirstFootnote( SwContentFrm* ) const;
                 SwFootnoteContFrm *FindNearestFootnoteCont( bool bDontLeave = false );

    static void ChangeFootnoteRef( const SwContentFrm *pOld, const SwTextFootnote *,
                       SwContentFrm *pNew );
    void RearrangeFootnotes( const SwTwips nDeadLine, const bool bLock = false,
                        const SwTextFootnote *pAttr = nullptr );

    // Set DeadLine (in document coordinates) so that the text formatter can
    // temporarily limit footnote height.
    void    SetFootnoteDeadLine( const SwTwips nDeadLine );
    SwTwips GetMaxFootnoteHeight() const { return nMaxFootnoteHeight; }

    // returns value for remaining space until the body reaches minimal height
    SwTwips GetVarSpace() const;

    // methods needed for layouting
    // The parameters <_bCollectOnlyPreviousFootnotes> and <_pRefFootnoteBossFrm> control
    // if only footnotes that are positioned before the given reference
    // footnote boss-frame have to be collected.
    // Note: if parameter <_bCollectOnlyPreviousFootnotes> is true, then parameter
    // <_pRefFootnoteBossFrm> has to be referenced by an object.
    static void _CollectFootnotes( const SwContentFrm*   _pRef,
                              SwFootnoteFrm*           _pFootnote,
                              SwFootnoteFrms&          _rFootnoteArr,
                              const bool      _bCollectOnlyPreviousFootnotes = false,
                              const SwFootnoteBossFrm* _pRefFootnoteBossFrm = nullptr);
    // The parameter <_bCollectOnlyPreviousFootnotes> controls if only footnotes
    // that are positioned before the footnote boss-frame <this> have to be
    // collected.
    void    CollectFootnotes( const SwContentFrm* _pRef,
                         SwFootnoteBossFrm*     _pOld,
                         SwFootnoteFrms&        _rFootnoteArr,
                         const bool    _bCollectOnlyPreviousFootnotes = false );
    void    _MoveFootnotes( SwFootnoteFrms &rFootnoteArr, bool bCalc = false );
    void    MoveFootnotes( const SwContentFrm *pSrc, SwContentFrm *pDest,
                      SwTextFootnote *pAttr );

    // should AdjustNeighbourhood be called (or Grow/Shrink)?
    sal_uInt8 NeighbourhoodAdjustment( const SwFrm* pFrm ) const
        { return IsPageFrm() ? NA_ONLY_ADJUST : _NeighbourhoodAdjustment( pFrm ); }
};

inline const SwLayoutFrm *SwFootnoteBossFrm::FindBodyCont() const
{
    return const_cast<SwFootnoteBossFrm*>(this)->FindBodyCont();
}

inline const SwFootnoteContFrm *SwFootnoteBossFrm::FindFootnoteCont() const
{
    return const_cast<SwFootnoteBossFrm*>(this)->FindFootnoteCont();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
