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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX
#include <tools/mempool.hxx>

#include "layfrm.hxx"
#include "flowfrm.hxx"
#include <set>

class SwSection;
class SwSectionFmt;
class SwAttrSetChg;
class SwFtnContFrm;
class SwLayouter;

#define FINDMODE_ENDNOTE 1
#define FINDMODE_LASTCNT 2
#define FINDMODE_MYLAST  4

class SwSectionFrm: public SwLayoutFrm, public SwFlowFrm
{
    SwSection* pSection;
    bool bFtnAtEnd; // footnotes at the end of section
    bool bEndnAtEnd; // endnotes at the end of section
    bool bCntntLock; // content locked
    bool bOwnFtnNum; // special numbering of footnotes
    bool bFtnLock; // ftn, don't leave this section bwd

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );
    void _Cut( bool bRemove );
    // Is there a FtnContainer?
    // An empty sectionfrm without FtnCont is superfluous
    bool IsSuperfluous() const { return !ContainsAny() && !ContainsFtnCont(); }
    void CalcFtnAtEndFlag();
    void CalcEndAtEndFlag();
    const SwSectionFmt* _GetEndSectFmt() const;
    bool IsEndnoteAtMyEnd() const;

    virtual void DestroyImpl() SAL_OVERRIDE;
    virtual ~SwSectionFrm();

protected:
    virtual void MakeAll() SAL_OVERRIDE;
    virtual bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, bool bHead, bool &rReformat ) SAL_OVERRIDE;
    virtual void Format( const SwBorderAttrs *pAttrs = 0 ) SAL_OVERRIDE;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) SAL_OVERRIDE;
    virtual void SwClientNotify( const SwModify&, const SfxHint& ) SAL_OVERRIDE;

public:
    SwSectionFrm( SwSection &, SwFrm* ); // Content is not created!
    SwSectionFrm( SwSectionFrm &, bool bMaster ); // _ONLY_ for creating Master/Follows!

    void Init();
    virtual void CheckDirection( bool bVert ) SAL_OVERRIDE;

    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const SAL_OVERRIDE;

    virtual void Cut() SAL_OVERRIDE;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) SAL_OVERRIDE;

    inline const SwSectionFrm *GetFollow() const;
    inline       SwSectionFrm *GetFollow();
    SwSectionFrm* FindMaster() const;

                 SwCntntFrm *FindLastCntnt( sal_uInt8 nMode = 0 );
    inline const SwCntntFrm *FindLastCntnt( sal_uInt8 nMode = 0 ) const;
    inline SwSection* GetSection() { return pSection; }
    inline const SwSection* GetSection() const { return pSection; }
    inline void ColLock()       { mbColLocked = true; }
    inline void ColUnlock()     { mbColLocked = false; }

    void CalcFtnCntnt();
    void SimpleFormat();
    bool IsDescendantFrom( const SwSectionFmt* pSect ) const;
    bool HasToBreak( const SwFrm* pFrm ) const;
    void MergeNext( SwSectionFrm* pNxt );

    /**
     * Splits the SectionFrm surrounding the pFrm up in two parts:
     * pFrm and the start of the 2nd part
     */
    bool SplitSect( SwFrm* pFrm, bool bApres );
    void DelEmpty( bool bRemove ); // Like Cut(), except for that Follow chaining is maintained
    bool IsToIgnore() const        // No size, no content; need to be ignored
    { return !Frm().Height() && !ContainsCntnt(); }
    SwFtnContFrm* ContainsFtnCont( const SwFtnContFrm* pCont = NULL ) const;
    bool Growable() const;
    SwTwips _Shrink( SwTwips, bool bTst );
    SwTwips _Grow  ( SwTwips, bool bTst );

    /**
     * A sectionfrm has to maximize, if he has a follow or a ftncontainer at
     * the end of the page. A superfluous follow will be ignored,
     * if bCheckFollow is set.
     */
    bool ToMaximize( bool bCheckFollow ) const;
    inline bool _ToMaximize() const
        { if( !pSection ) return false; return ToMaximize( false ); }
    bool MoveAllowed( const SwFrm* ) const;
    bool CalcMinDiff( SwTwips& rMinDiff ) const;

    /**
     * If we don't pass a @param bOverSize or false, the return value is > 0 for
     * undersized Frames, or 0
     *
     * If @param bOverSize == true, we can also get a negative return value,
     * if the SectionFrm is not completely filled, which happens often for
     * e.g. SectionFrms with Follows.
     * We need this in the FormatWidthCols to "deflate" columns there.
     */
    long Undersize( bool bOverSize = false );

    /// Adapt size to surroundings
    void _CheckClipping( bool bGrow, bool bMaximize );

    void InvalidateFtnPos();
    void CollectEndnotes( SwLayouter* pLayouter );
    const SwSectionFmt* GetEndSectFmt() const
        { if( IsEndnAtEnd() ) return _GetEndSectFmt(); return NULL; }

    static void MoveCntntAndDelete( SwSectionFrm* pDel, bool bSave );

    bool IsBalancedSection() const;

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const SAL_OVERRIDE;

    bool IsFtnAtEnd() const { return bFtnAtEnd; }
    bool IsEndnAtEnd() const { return bEndnAtEnd;   }
    bool IsAnyNoteAtEnd() const { return bFtnAtEnd || bEndnAtEnd; }
    bool AreNotesAtEnd() const { return bFtnAtEnd && bEndnAtEnd; }

    void SetCntntLock( bool bNew ) { bCntntLock = bNew; }
    bool IsCntntLocked() const { return bCntntLock; }

    bool IsOwnFtnNum() const { return bOwnFtnNum; }

    void SetFtnLock( bool bNew ) { bFtnLock = bNew; }
    bool IsFtnLock() const { return bFtnLock; }

    DECL_FIXEDMEMPOOL_NEWDEL(SwSectionFrm)
};

class SwDestroyList : public std::set<SwSectionFrm*> {};

inline const SwSectionFrm *SwSectionFrm::GetFollow() const
{
    return static_cast<const SwSectionFrm*>(SwFlowFrm::GetFollow());
}
inline SwSectionFrm *SwSectionFrm::GetFollow()
{
    return static_cast<SwSectionFrm*>(SwFlowFrm::GetFollow());
}
inline const SwCntntFrm *SwSectionFrm::FindLastCntnt( sal_uInt8 nMode ) const
{
    return const_cast<SwSectionFrm*>(this)->FindLastCntnt( nMode );
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
