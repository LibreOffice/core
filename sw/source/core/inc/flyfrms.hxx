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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_FLYFRMS_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FLYFRMS_HXX

#include "flyfrm.hxx"

// #i28701#
class SwFlyAtCntFrm;

// Base class for those Flys that can "move freely" or better that are not
// bound in Cntnt.
class SwFlyFreeFrm : public SwFlyFrm
{
    SwPageFrm *pPage;   // page where the Fly is registered

    // #i34753# - flag for at-page anchored Writer fly frames
    // to prevent a positioning - call of method <MakeObjPos()> -, if Writer
    // fly frame is already clipped during its format by the object formatter.
    bool mbNoMakePos;

    // #i37068# - flag to prevent move in method <CheckClip(..)>
    bool mbNoMoveOnCheckClip;

    SwRect maUnclippedFrm;

    void CheckClip( const SwFmtFrmSize &rSz );  //'Emergency' Clipping.

    /** determines, if direct environment of fly frame has 'auto' size

        #i17297#
        start with anchor frame and search for a header, footer, row or fly frame
        stopping at page frame.
        return <true>, if such a frame is found and it has 'auto' size.
        otherwise <false> is returned.

        @return boolean indicating, that direct environment has 'auto' size
    */
    bool HasEnvironmentAutoSize() const;

protected:
    // #i28701# - new friend class <SwFlyNotify> for access to
    // method <NotifyBackground>
    friend class SwFlyNotify;
    virtual void NotifyBackground( SwPageFrm *pPage,
                                   const SwRect& rRect, PrepareHint eHint);
    SwFlyFreeFrm( SwFlyFrmFmt*, SwFrm*, SwFrm *pAnchor );

public:
    // #i28701#
    TYPEINFO_OVERRIDE();

    virtual ~SwFlyFreeFrm();

    virtual void MakeAll();

    // #i37068# - accessors for member <mbNoMoveOnCheckClip>
    inline void SetNoMoveOnCheckClip( const bool _bNewNoMoveOnCheckClip )
    {
        mbNoMoveOnCheckClip = _bNewNoMoveOnCheckClip;
    }
    inline bool IsNoMoveOnCheckClip() const
    {
        return mbNoMoveOnCheckClip;
    }
    // #i34753# - accessors for member <mbNoMakePos>
    inline void SetNoMakePos( const bool _bNoMakePos )
    {
        if ( IsFlyLayFrm() )
        {
            mbNoMakePos = _bNoMakePos;
        }
    }
    inline bool IsNoMakePos() const
    {
        if ( IsFlyLayFrm() )
        {
            return mbNoMakePos;
        }
        else
        {
            return false;
        }
    }

    inline const SwRect& GetUnclippedFrm( ) const
    {
        if ( maUnclippedFrm.HasArea( ) )
            return maUnclippedFrm;
        else
            return Frm();
    }

    /** method to determine, if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwFlyFrm::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame isn't registered at a page frame
        and its anchor frame isn't inside another Writer fly frame.
    */
    virtual bool IsFormatPossible() const;
};

// Flys that are bound to LayoutFrms and not to Cntnt
class SwFlyLayFrm : public SwFlyFreeFrm
{
public:
    // #i28701#
    TYPEINFO_OVERRIDE();

    SwFlyLayFrm( SwFlyFrmFmt*, SwFrm*, SwFrm *pAnchor );
    SwFlyLayFrm( SwFlyLayFrm& );
protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );
};

// Flys that are bound to Cntnt but not in Cntnt
class SwFlyAtCntFrm : public SwFlyFreeFrm
{
protected:
    virtual void MakeAll();

    // #i28701#
    virtual bool _InvalidationAllowed( const InvalidationType _nInvalid ) const;

    /** method to assure that anchored object is registered at the correct
        page frame

        #i28701#
    */
    virtual void RegisterAtCorrectPage();
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    // #i28701#
    TYPEINFO_OVERRIDE();

    SwFlyAtCntFrm( SwFlyFrmFmt*, SwFrm*, SwFrm *pAnchor );

    void SetAbsPos( const Point &rNew );

    // #i26791#
    virtual void MakeObjPos();

    /** method to determine, if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwFlyFreeFrm::IsFormatPossible()> by:
        format isn't possible, if method <MakeAll()> is already in progress.
    */
    virtual bool IsFormatPossible() const;
};

// Flys that are bound to a character in Cntnt
class SwFlyInCntFrm : public SwFlyFrm
{
    Point aRef;  // relative to this point AbsPos is being calculated

    sal_Bool bInvalidLayout :1;
    sal_Bool bInvalidCntnt  :1;

protected:
    virtual void NotifyBackground( SwPageFrm *pPage,
                                   const SwRect& rRect, PrepareHint eHint);
    virtual void MakeAll();
    virtual void  Modify( const SfxPoolItem*, const SfxPoolItem* );

public:
    // #i28701#
    TYPEINFO_OVERRIDE();

    SwFlyInCntFrm( SwFlyFrmFmt*, SwFrm*, SwFrm *pAnchor );

    virtual ~SwFlyInCntFrm();
    virtual void  Format(  const SwBorderAttrs *pAttrs = 0 );

    void SetRefPoint( const Point& rPoint, const Point &rRelAttr,
        const Point &rRelPos );
    const Point &GetRefPoint() const { return aRef; }
    const Point GetRelPos() const;

    inline void InvalidateLayout() const;
    inline void InvalidateCntnt() const;
    inline void ValidateLayout() const;
    inline void ValidateCntnt() const;
    sal_Bool IsInvalid() const { return (bInvalidLayout || bInvalidCntnt); }
    sal_Bool IsInvalidLayout() const { return bInvalidLayout; }
    sal_Bool IsInvalidCntnt() const { return bInvalidCntnt; }

    // (26.11.93, see tabfrm.hxx, but might also be valid for others)
    // For creation of a Fly after a FlyCnt was created _and_ inserted.
    // Must be called by creator because can be pasted only after creation.
    // Sometimes the page for registering the Flys is not visible until then
    // as well.
    void RegistFlys();

    //see layact.cxx
    void AddRefOfst( long nOfst ) { aRef.Y() += nOfst; }

    // #i26791#
    virtual void MakeObjPos();

    // invalidate anchor frame on invalidation of the position, because the
    // position is calculated during the format of the anchor frame
    virtual void _ActionOnInvalidation( const InvalidationType _nInvalid );
};

inline void SwFlyInCntFrm::InvalidateLayout() const
{
    ((SwFlyInCntFrm*)this)->bInvalidLayout = sal_True;
}
inline void SwFlyInCntFrm::InvalidateCntnt() const
{
    ((SwFlyInCntFrm*)this)->bInvalidCntnt = sal_True;
}

inline void SwFlyInCntFrm::ValidateLayout() const
{
    ((SwFlyInCntFrm*)this)->bInvalidLayout = sal_False;
}
inline void SwFlyInCntFrm::ValidateCntnt() const
{
    ((SwFlyInCntFrm*)this)->bInvalidCntnt = sal_False;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
