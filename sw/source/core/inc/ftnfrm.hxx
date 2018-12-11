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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_FTNFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_FTNFRM_HXX

#include "layfrm.hxx"

class SwContentFrame;
class SwRootFrame;
class SwTextNode;
class SwTextFootnote;
class SwBorderAttrs;
class SwFootnoteFrame;

void sw_RemoveFootnotes( SwFootnoteBossFrame* pBoss, bool bPageOnly, bool bEndNotes );

namespace sw {

void RemoveFootnotesForNode(
        SwRootFrame const& rLayout, SwTextNode const& rTextNode,
        std::vector<std::pair<sal_Int32, sal_Int32>> const*const pExtents);

}

// There exists a special section on a page for footnotes. It's called
// SwFootnoteContFrame. Each footnote is separated by a SwFootnoteFrame which contains
// the paragraphs of a footnote. SwFootnoteFrame can be splitted and will then
// continue on another page.
class SwFootnoteContFrame: public SwLayoutFrame
{
public:
    SwFootnoteContFrame( SwFrameFormat*, SwFrame* );

    const SwFootnoteFrame* FindFootNote() const;

    virtual SwTwips ShrinkFrame( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrame  ( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual void    Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual void    PaintSwFrameShadowAndBorder(
        const SwRect&,
        const SwPageFrame* pPage,
        const SwBorderAttrs&) const override;
    virtual void PaintSubsidiaryLines( const SwPageFrame*, const SwRect& ) const override;
            void    PaintLine( const SwRect &, const SwPageFrame * ) const;
};

class SwFootnoteFrame: public SwLayoutFrame
{
    // Pointer to FootnoteFrame in which the footnote will be continued:
    //  - 0     no following existent
    //  - this  for the last one
    //  - otherwise the following FootnoteFrame
    SwFootnoteFrame     *mpFollow;
    SwFootnoteFrame     *mpMaster;      // FootnoteFrame from which I am the following
    SwContentFrame   *mpReference;         // in this ContentFrame is the footnote reference
    SwTextFootnote     *mpAttribute;        // footnote attribute (for recognition)

    // if true paragraphs in this footnote are NOT permitted to flow backwards
    bool mbBackMoveLocked : 1;
    // #i49383# - control unlock of position of lower anchored objects.
    bool mbUnlockPosOfLowerObjs : 1;

public:
    SwFootnoteFrame( SwFrameFormat*, SwFrame*, SwContentFrame*, SwTextFootnote* );

    virtual void Cut() override;
    virtual void Paste( SwFrame* pParent, SwFrame* pSibling = nullptr ) override;

    virtual void PaintSubsidiaryLines( const SwPageFrame*, const SwRect& ) const override;

    bool operator<( const SwTextFootnote* pTextFootnote ) const;

#ifdef DBG_UTIL
    const SwContentFrame *GetRef() const;
         SwContentFrame  *GetRef();
#else
    const SwContentFrame *GetRef() const    { return mpReference; }
         SwContentFrame  *GetRef()          { return mpReference; }
#endif
    const SwContentFrame *GetRefFromAttr()  const;
          SwContentFrame *GetRefFromAttr();

    const SwFootnoteFrame *GetFollow() const   { return mpFollow; }
          SwFootnoteFrame *GetFollow()         { return mpFollow; }

    const SwFootnoteFrame *GetMaster() const   { return mpMaster; }
          SwFootnoteFrame *GetMaster()         { return mpMaster; }

    const SwTextFootnote   *GetAttr() const   { return mpAttribute; }
          SwTextFootnote   *GetAttr()         { return mpAttribute; }

    void SetFollow( SwFootnoteFrame *pNew ) { mpFollow = pNew; }
    void SetMaster( SwFootnoteFrame *pNew ) { mpMaster = pNew; }
    void SetRef   ( SwContentFrame *pNew ) { mpReference = pNew; }

    void InvalidateNxtFootnoteCnts( SwPageFrame const * pPage );

    void LockBackMove()     { mbBackMoveLocked = true; }
    void UnlockBackMove()   { mbBackMoveLocked = false;}
    bool IsBackMoveLocked() { return mbBackMoveLocked; }

    // prevents that the last content deletes the SwFootnoteFrame as well (Cut())
    void ColLock()       { mbColLocked = true; }
    void ColUnlock()     { mbColLocked = false; }

    // #i49383#
    void UnlockPosOfLowerObjs()
    {
        mbUnlockPosOfLowerObjs = true;
    }
    void KeepLockPosOfLowerObjs()
    {
        mbUnlockPosOfLowerObjs = false;
    }
    bool IsUnlockPosOfLowerObjs()
    {
        return mbUnlockPosOfLowerObjs;
    }

    /** search for last content in the current footnote frame

        OD 2005-12-02 #i27138#

        @return SwContentFrame*
        pointer to found last content frame. NULL, if none is found.
    */
    SwContentFrame* FindLastContent();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
