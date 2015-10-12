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

class SwContentFrm;
class SwTextFootnote;
class SwBorderAttrs;
class SwFootnoteFrm;

void sw_RemoveFootnotes( SwFootnoteBossFrm* pBoss, bool bPageOnly, bool bEndNotes );

// There exists a special section on a page for footnotes. It's called
// SwFootnoteContFrm. Each footnote is separated by a SwFootnoteFrm which contains
// the paragraphs of a footnote. SwFootnoteFrm can be splitted and will then
// continue on another page.
class SwFootnoteContFrm: public SwLayoutFrm
{
public:
    SwFootnoteContFrm( SwFrameFormat*, SwFrm* );

    const SwFootnoteFrm* FindFootNote() const;

    virtual SwTwips ShrinkFrm( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrm  ( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual void    Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = 0 ) override;
    virtual void    PaintBorder( const SwRect &, const SwPageFrm *pPage,
                                 const SwBorderAttrs & ) const override;
    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const override;
            void    PaintLine( const SwRect &, const SwPageFrm * ) const;
};

class SwFootnoteFrm: public SwLayoutFrm
{
    // Pointer to FootnoteFrm in which the footnote will be continued:
    //  - 0     no following existent
    //  - this  for the last one
    //  - otherwise the following FootnoteFrm
    SwFootnoteFrm     *pFollow;
    SwFootnoteFrm     *pMaster;      // FootnoteFrm from which I am the following
    SwContentFrm   *pRef;         // in this ContentFrm is the footnote reference
    SwTextFootnote     *pAttr;        // footnote attribute (for recognition)

    // if true paragraphs in this footnote are NOT permitted to flow backwards
    bool bBackMoveLocked : 1;
    // #i49383# - control unlock of position of lower anchored objects.
    bool mbUnlockPosOfLowerObjs : 1;
#ifdef DBG_UTIL
protected:
    virtual SwTwips ShrinkFrm( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrm  ( SwTwips, bool bTst = false, bool bInfo = false ) override;
#endif

public:
    SwFootnoteFrm( SwFrameFormat*, SwFrm*, SwContentFrm*, SwTextFootnote* );

    virtual void Cut() override;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) override;

    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const override;

    bool operator<( const SwTextFootnote* pTextFootnote ) const;

#ifdef DBG_UTIL
    const SwContentFrm *GetRef() const;
         SwContentFrm  *GetRef();
#else
    const SwContentFrm *GetRef() const    { return pRef; }
         SwContentFrm  *GetRef()          { return pRef; }
#endif
    const SwContentFrm *GetRefFromAttr()  const;
          SwContentFrm *GetRefFromAttr();

    const SwFootnoteFrm *GetFollow() const   { return pFollow; }
          SwFootnoteFrm *GetFollow()         { return pFollow; }

    const SwFootnoteFrm *GetMaster() const   { return pMaster; }
          SwFootnoteFrm *GetMaster()         { return pMaster; }

    const SwTextFootnote   *GetAttr() const   { return pAttr; }
          SwTextFootnote   *GetAttr()         { return pAttr; }

    void SetFollow( SwFootnoteFrm *pNew ) { pFollow = pNew; }
    void SetMaster( SwFootnoteFrm *pNew ) { pMaster = pNew; }
    void SetRef   ( SwContentFrm *pNew ) { pRef = pNew; }

    void InvalidateNxtFootnoteCnts( SwPageFrm* pPage );

    void LockBackMove()     { bBackMoveLocked = true; }
    void UnlockBackMove()   { bBackMoveLocked = false;}
    bool IsBackMoveLocked() { return bBackMoveLocked; }

    // prevents that the last content deletes the SwFootnoteFrm as well (Cut())
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

        @return SwContentFrm*
        pointer to found last content frame. NULL, if none is found.
    */
    SwContentFrm* FindLastContent();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
