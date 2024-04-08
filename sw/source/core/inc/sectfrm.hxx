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

#include "layfrm.hxx"
#include "flowfrm.hxx"

#include <svl/listener.hxx>

class SwSection;
class SwSectionFormat;
class SwAttrSetChg;
class SwFootnoteContFrame;
class SwLayouter;

enum class SwFindMode
{
    None = 0, EndNote = 1, LastCnt = 2, MyLast = 4
};

enum class SwSectionFrameInvFlags : sal_uInt8
{
    NONE = 0x00,
    InvalidateSize = 0x01,
    SetCompletePaint = 0x10,
};

namespace o3tl {
    template<> struct typed_flags<SwSectionFrameInvFlags> : is_typed_flags<SwSectionFrameInvFlags, 0x0011> {};
}

class SAL_DLLPUBLIC_RTTI SwSectionFrame final: public SwLayoutFrame, public SwFlowFrame
    , public SvtListener // TODO?
{
    SwSection* m_pSection;
    bool m_bFootnoteAtEnd; // footnotes at the end of section
    bool m_bEndnAtEnd; // endnotes at the end of section
    bool m_bContentLock; // content locked
    bool m_bOwnFootnoteNum; // special numbering of footnotes
    bool m_bFootnoteLock; // ftn, don't leave this section bwd

    void UpdateAttr_( const SfxPoolItem*, const SfxPoolItem*, SwSectionFrameInvFlags &,
                      SwAttrSetChg *pa = nullptr, SwAttrSetChg *pb = nullptr );
    void Cut_( bool bRemove );
    // Is there a FootnoteContainer?
    // An empty sectionfrm without FootnoteCont is superfluous
    bool IsSuperfluous() const { return !ContainsAny() && !ContainsFootnoteCont(); }
    void CalcFootnoteAtEndFlag();
    void CalcEndAtEndFlag();
    const SwSectionFormat* GetEndSectFormat_() const;
    bool IsEndnoteAtMyEnd() const;

    virtual void DestroyImpl() override;
    virtual ~SwSectionFrame() override;

    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;
    virtual bool ShouldBwdMoved( SwLayoutFrame *pNewUpper, bool &rReformat ) override;
    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual void Notify(SfxHint const& rHint) override;
    virtual void SwClientNotify( const SwModify&, const SfxHint& ) override;

public:
    SwSectionFrame( SwSection &, SwFrame* ); // Content is not created!
    SwSectionFrame( SwSectionFrame &, bool bMaster ); // _ONLY_ for creating Master/Follows!

    void Init();
    virtual void CheckDirection( bool bVert ) override;

    virtual void PaintSubsidiaryLines( const SwPageFrame*, const SwRect& ) const override;

    virtual void Cut() override;
    virtual void Paste( SwFrame* pParent, SwFrame* pSibling = nullptr ) override;

    virtual bool IsHiddenNow() const override;

    inline const SwSectionFrame *GetFollow() const;
    inline       SwSectionFrame *GetFollow();
    SwSectionFrame* FindMaster() const;

                 SwContentFrame *FindLastContent( SwFindMode nMode = SwFindMode::None );
    inline const SwContentFrame *FindLastContent() const;
    SwSection* GetSection() { return m_pSection; }
    const SwSection* GetSection() const { return m_pSection; }
    void ColLock()       { mbColLocked = true; }
    void ColUnlock()     { mbColLocked = false; }

    void CalcFootnoteContent();
    void SimpleFormat();
    bool IsDescendantFrom( const SwSectionFormat* pSect ) const;
    bool HasToBreak( const SwFrame* pFrame ) const;
    void MergeNext( SwSectionFrame* pNxt );

    /**
     * Splits the SectionFrame surrounding the pFrame up in two parts:
     * pFrame and the start of the 2nd part
     */
    SwSectionFrame* SplitSect( SwFrame* pFrameStartAfter, SwFrame* pFramePutAfter );
    void DelEmpty( bool bRemove ); // Like Cut(), except for that Follow chaining is maintained
    SwFootnoteContFrame* ContainsFootnoteCont( const SwFootnoteContFrame* pCont = nullptr ) const;
    bool Growable() const;
    SwTwips Shrink_( SwTwips, bool bTst );
    SwTwips Grow_  ( SwTwips, bool bTst );

    /**
     * A sectionfrm has to maximize, if he has a follow or a ftncontainer at
     * the end of the page. A superfluous follow will be ignored,
     * if bCheckFollow is set.
     */
    bool ToMaximize( bool bCheckFollow ) const;
    bool ToMaximize_() const {
        if( !m_pSection ) return false;
        return ToMaximize( false );
    }
    bool MoveAllowed( const SwFrame* ) const;
    bool CalcMinDiff( SwTwips& rMinDiff ) const;

    /**
     * Returns the size delta that the section would like to be
     * greater if it has undersized TextFrames in it.
     *
     * The return value is > 0 for undersized Frames, or 0 otherwise.
     *
     * If necessary the undersized-flag is corrected.
     * We need this in the FormatWidthCols to "deflate" columns there.
     */
    SwTwips Undersize();
    SwTwips CalcUndersize() const;

    /// Adapt size to surroundings
    void CheckClipping( bool bGrow, bool bMaximize );

    void InvalidateFootnotePos();
    void CollectEndnotes( SwLayouter* pLayouter );
    const SwSectionFormat* GetEndSectFormat() const {
        if( IsEndnAtEnd() ) return GetEndSectFormat_();
        return nullptr;
    }

    static void MoveContentAndDelete( SwSectionFrame* pDel, bool bSave );

    bool IsBalancedSection() const;

    virtual void dumpAsXml(xmlTextWriterPtr writer = nullptr) const override;
    void dumpAsXmlAttributes(xmlTextWriterPtr writer) const override;

    bool IsFootnoteAtEnd() const { return m_bFootnoteAtEnd; }
    bool IsEndnAtEnd() const { return m_bEndnAtEnd;   }
    bool IsAnyNoteAtEnd() const { return m_bFootnoteAtEnd || m_bEndnAtEnd; }

    void SetContentLock( bool bNew ) { m_bContentLock = bNew; }
    bool IsContentLocked() const { return m_bContentLock; }

    bool IsOwnFootnoteNum() const { return m_bOwnFootnoteNum; }

    void SetFootnoteLock( bool bNew ) { m_bFootnoteLock = bNew; }
    bool IsFootnoteLock() const { return m_bFootnoteLock; }
};

inline const SwSectionFrame *SwSectionFrame::GetFollow() const
{
    return static_cast<const SwSectionFrame*>(SwFlowFrame::GetFollow());
}
inline SwSectionFrame *SwSectionFrame::GetFollow()
{
    return static_cast<SwSectionFrame*>(SwFlowFrame::GetFollow());
}
inline const SwContentFrame *SwSectionFrame::FindLastContent() const
{
    return const_cast<SwSectionFrame*>(this)->FindLastContent();
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
