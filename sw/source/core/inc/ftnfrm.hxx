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
class SwTextFootnote;
class SwBorderAttrs;
class SwFootnoteFrame;

void sw_RemoveFootnotes( SwFootnoteBossFrame* pBoss, bool bPageOnly, bool bEndNotes );

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
    virtual void    PaintBorder( const SwRect &, const SwPageFrame *pPage,
                                 const SwBorderAttrs & ) const override;
    virtual void PaintSubsidiaryLines( const SwPageFrame*, const SwRect& ) const override;
            void    PaintLine( const SwRect &, const SwPageFrame * ) const;
};

class SwFootnoteFrame: public SwLayoutFrame
{
    // Pointer to FootnoteFrame in which the footnote will be continued:
    //  - 0     no following existent
    //  - this  for the last one
    //  - otherwise the following FootnoteFrame
    SwFootnoteFrame     *pFollow;
    SwFootnoteFrame     *pMaster;      // FootnoteFrame from which I am the following
    SwContentFrame   *pRef;         // in this ContentFrame is the footnote reference
    SwTextFootnote     *pAttr;        // footnote attribute (for recognition)

    // if true paragraphs in this footnote are NOT permitted to flow backwards
    bool bBackMoveLocked : 1;
    // #i49383# - control unlock of position of lower anchored objects.
    bool mbUnlockPosOfLowerObjs : 1;
#ifdef DBG_UTIL
protected:
    virtual SwTwips ShrinkFrame( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrame  ( SwTwips, bool bTst = false, bool bInfo = false ) override;
#endif

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
    const SwContentFrame *GetRef() const    { return pRef; }
         SwContentFrame  *GetRef()          { return pRef; }
#endif
    const SwContentFrame *GetRefFromAttr()  const;
          SwContentFrame *GetRefFromAttr();

    const SwFootnoteFrame *GetFollow() const   { return pFollow; }
          SwFootnoteFrame *GetFollow()         { return pFollow; }

    const SwFootnoteFrame *GetMaster() const   { return pMaster; }
          SwFootnoteFrame *GetMaster()         { return pMaster; }

    const SwTextFootnote   *GetAttr() const   { return pAttr; }
          SwTextFootnote   *GetAttr()         { return pAttr; }

    void SetFollow( SwFootnoteFrame *pNew ) { pFollow = pNew; }
    void SetMaster( SwFootnoteFrame *pNew ) { pMaster = pNew; }
    void SetRef   ( SwContentFrame *pNew ) { pRef = pNew; }

    void InvalidateNxtFootnoteCnts( SwPageFrame* pPage );

    void LockBackMove()     { bBackMoveLocked = true; }
    void UnlockBackMove()   { bBackMoveLocked = false;}
    bool IsBackMoveLocked() { return bBackMoveLocked; }

    // prevents that the last content deletes the SwFootnoteFrame as well (Cut())
    inline void ColLock()       { mbColLocked = true; }
    inline void ColUnlock()     { mbColLocked = false; }

    // #i49383#
    inline void UnlockPosOfLowerObjs()
    {
        mbUnlockPosOfLowerObjs = true;
    }
    inline void KeepLockPosOfLowerObjs()
    {
        mbUnlockPosOfLowerObjs = false;
    }
    inline bool IsUnlockPosOfLowerObjs()
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
