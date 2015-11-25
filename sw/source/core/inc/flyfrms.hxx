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

#include <sal/config.h>

#include "flyfrm.hxx"

// #i28701#
class SwFlyAtContentFrame;

// Base class for those Flys that can "move freely" or better that are not
// bound in Content.
class SwFlyFreeFrame : public SwFlyFrame
{
    // #i34753# - flag for at-page anchored Writer fly frames
    // to prevent a positioning - call of method <MakeObjPos()> -, if Writer
    // fly frame is already clipped during its format by the object formatter.
    bool mbNoMakePos;

    // #i37068# - flag to prevent move in method <CheckClip(..)>
    bool mbNoMoveOnCheckClip;

    SwRect maUnclippedFrame;

    void CheckClip( const SwFormatFrameSize &rSz );  //'Emergency' Clipping.

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
    virtual void NotifyBackground( SwPageFrame *pPage,
                                   const SwRect& rRect, PrepareHint eHint) override;
    SwFlyFreeFrame( SwFlyFrameFormat*, SwFrame*, SwFrame *pAnchor );

    virtual void DestroyImpl() override;
    virtual ~SwFlyFreeFrame();

public:
    // #i28701#

    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;

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
        if ( IsFlyLayFrame() )
        {
            mbNoMakePos = _bNoMakePos;
        }
    }
    inline bool IsNoMakePos() const
    {
        if ( IsFlyLayFrame() )
        {
            return mbNoMakePos;
        }
        else
        {
            return false;
        }
    }

    inline const SwRect& GetUnclippedFrame( ) const
    {
        if ( maUnclippedFrame.HasArea( ) )
            return maUnclippedFrame;
        else
            return Frame();
    }

    /** method to determine, if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwFlyFrame::IsFormatPossible()> by:
        format isn't possible, if Writer fly frame isn't registered at a page frame
        and its anchor frame isn't inside another Writer fly frame.
    */
    virtual bool IsFormatPossible() const override;
};

// Flys that are bound to LayoutFrames and not to Content
class SwFlyLayFrame : public SwFlyFreeFrame
{
public:
    // #i28701#

    SwFlyLayFrame( SwFlyFrameFormat*, SwFrame*, SwFrame *pAnchor );

protected:
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
};

// Flys that are bound to Content but not in Content
class SwFlyAtContentFrame : public SwFlyFreeFrame
{
protected:
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;

    // #i28701#
    virtual bool _InvalidationAllowed( const InvalidationType _nInvalid ) const override;

    /** method to assure that anchored object is registered at the correct
        page frame

        #i28701#
    */
    virtual void RegisterAtCorrectPage() override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:
    // #i28701#

    SwFlyAtContentFrame( SwFlyFrameFormat*, SwFrame*, SwFrame *pAnchor );

    void SetAbsPos( const Point &rNew );

    // #i26791#
    virtual void MakeObjPos() override;

    /** method to determine, if a format on the Writer fly frame is possible

        #i28701#
        refine 'IsFormatPossible'-conditions of method
        <SwFlyFreeFrame::IsFormatPossible()> by:
        format isn't possible, if method <MakeAll()> is already in progress.
    */
    virtual bool IsFormatPossible() const override;
};

// Flys that are bound to a character in Content
class SwFlyInContentFrame : public SwFlyFrame
{
    Point aRef;  // relative to this point AbsPos is being calculated

    bool bInvalidLayout :1;
    bool bInvalidContent  :1;

    virtual void DestroyImpl() override;
    virtual ~SwFlyInContentFrame();

protected:
    virtual void NotifyBackground( SwPageFrame *pPage,
                                   const SwRect& rRect, PrepareHint eHint) override;
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;
    virtual void  Modify( const SfxPoolItem*, const SfxPoolItem* ) override;

public:
    // #i28701#

    SwFlyInContentFrame( SwFlyFrameFormat*, SwFrame*, SwFrame *pAnchor );

    virtual void  Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;

    void SetRefPoint( const Point& rPoint, const Point &rRelAttr,
        const Point &rRelPos );
    const Point &GetRefPoint() const { return aRef; }
    const Point GetRelPos() const;

    inline void InvalidateLayout() const;
    inline void InvalidateContent() const;
    bool IsInvalid() const { return (bInvalidLayout || bInvalidContent); }

    // (26.11.93, see tabfrm.hxx, but might also be valid for others)
    // For creation of a Fly after a FlyCnt was created _and_ inserted.
    // Must be called by creator because can be pasted only after creation.
    // Sometimes the page for registering the Flys is not visible until then
    // as well.
    void RegistFlys();

    //see layact.cxx
    void AddRefOfst( long nOfst ) { aRef.Y() += nOfst; }

    // #i26791#
    virtual void MakeObjPos() override;

    // invalidate anchor frame on invalidation of the position, because the
    // position is calculated during the format of the anchor frame
    virtual void _ActionOnInvalidation( const InvalidationType _nInvalid ) override;
};

inline void SwFlyInContentFrame::InvalidateLayout() const
{
    const_cast<SwFlyInContentFrame*>(this)->bInvalidLayout = true;
}
inline void SwFlyInContentFrame::InvalidateContent() const
{
    const_cast<SwFlyInContentFrame*>(this)->bInvalidContent = true;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
