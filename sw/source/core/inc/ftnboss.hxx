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

class SwFootnoteBossFrame;
class SwFootnoteContFrame;
class SwFootnoteFrame;
class SwTextFootnote;

// Set max. footnote area.
// Restoration of the old value in DTor. Implementation in ftnfrm.cxx
class SwSaveFootnoteHeight
{
    SwFootnoteBossFrame *pBoss;
    const SwTwips nOldHeight;
    SwTwips nNewHeight;
public:
    SwSaveFootnoteHeight( SwFootnoteBossFrame *pBs, const SwTwips nDeadLine );
    ~SwSaveFootnoteHeight();
};

#define NA_ONLY_ADJUST 0
#define NA_GROW_SHRINK 1
#define NA_GROW_ADJUST 2
#define NA_ADJUST_GROW 3

typedef std::vector<SwFootnoteFrame*> SwFootnoteFrames;

class SwFootnoteBossFrame: public SwLayoutFrame
{
    // for private footnote operations
    friend class SwFrame;
    friend class SwSaveFootnoteHeight;
    friend class SwPageFrame; // for setting of MaxFootnoteHeight

    // max. height of the footnote container on this page
    SwTwips nMaxFootnoteHeight;

    SwFootnoteContFrame *MakeFootnoteCont();
    SwFootnoteFrame     *FindFirstFootnote();
    sal_uInt8 _NeighbourhoodAdjustment( const SwFrame* pFrame ) const;

protected:
    void          InsertFootnote( SwFootnoteFrame * );
    static void   ResetFootnote( const SwFootnoteFrame *pAssumed );

public:
    inline SwFootnoteBossFrame( SwFrameFormat* pFormat, SwFrame* pSib )
        : SwLayoutFrame( pFormat, pSib )
        , nMaxFootnoteHeight(0)
        {}

                 SwLayoutFrame *FindBodyCont();
    inline const SwLayoutFrame *FindBodyCont() const;
    inline void SetMaxFootnoteHeight( const SwTwips nNewMax ) { nMaxFootnoteHeight = nNewMax; }

    // footnote interface
    void AppendFootnote( SwContentFrame *, SwTextFootnote * );
    void RemoveFootnote( const SwContentFrame *, const SwTextFootnote *, bool bPrep = true );
    static       SwFootnoteFrame     *FindFootnote( const SwContentFrame *, const SwTextFootnote * );
                 SwFootnoteContFrame *FindFootnoteCont();
    inline const SwFootnoteContFrame *FindFootnoteCont() const;
           const SwFootnoteFrame     *FindFirstFootnote( SwContentFrame* ) const;
                 SwFootnoteContFrame *FindNearestFootnoteCont( bool bDontLeave = false );

    static void ChangeFootnoteRef( const SwContentFrame *pOld, const SwTextFootnote *,
                       SwContentFrame *pNew );
    void RearrangeFootnotes( const SwTwips nDeadLine, const bool bLock = false,
                        const SwTextFootnote *pAttr = nullptr );

    // Set DeadLine (in document coordinates) so that the text formatter can
    // temporarily limit footnote height.
    void    SetFootnoteDeadLine( const SwTwips nDeadLine );
    SwTwips GetMaxFootnoteHeight() const { return nMaxFootnoteHeight; }

    // returns value for remaining space until the body reaches minimal height
    SwTwips GetVarSpace() const;

    // methods needed for layouting
    // The parameters <_bCollectOnlyPreviousFootnotes> and <_pRefFootnoteBossFrame> control
    // if only footnotes that are positioned before the given reference
    // footnote boss-frame have to be collected.
    // Note: if parameter <_bCollectOnlyPreviousFootnotes> is true, then parameter
    // <_pRefFootnoteBossFrame> has to be referenced by an object.
    static void _CollectFootnotes( const SwContentFrame*   _pRef,
                              SwFootnoteFrame*           _pFootnote,
                              SwFootnoteFrames&          _rFootnoteArr,
                              const bool      _bCollectOnlyPreviousFootnotes = false,
                              const SwFootnoteBossFrame* _pRefFootnoteBossFrame = nullptr);
    // The parameter <_bCollectOnlyPreviousFootnotes> controls if only footnotes
    // that are positioned before the footnote boss-frame <this> have to be
    // collected.
    void    CollectFootnotes( const SwContentFrame* _pRef,
                         SwFootnoteBossFrame*     _pOld,
                         SwFootnoteFrames&        _rFootnoteArr,
                         const bool    _bCollectOnlyPreviousFootnotes = false );
    void    _MoveFootnotes( SwFootnoteFrames &rFootnoteArr, bool bCalc = false );
    void    MoveFootnotes( const SwContentFrame *pSrc, SwContentFrame *pDest,
                      SwTextFootnote *pAttr );

    // should AdjustNeighbourhood be called (or Grow/Shrink)?
    sal_uInt8 NeighbourhoodAdjustment( const SwFrame* pFrame ) const
        { return IsPageFrame() ? NA_ONLY_ADJUST : _NeighbourhoodAdjustment( pFrame ); }
};

inline const SwLayoutFrame *SwFootnoteBossFrame::FindBodyCont() const
{
    return const_cast<SwFootnoteBossFrame*>(this)->FindBodyCont();
}

inline const SwFootnoteContFrame *SwFootnoteBossFrame::FindFootnoteCont() const
{
    return const_cast<SwFootnoteBossFrame*>(this)->FindFootnoteCont();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
