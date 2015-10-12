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
class SwSectionFormat;
class SwAttrSetChg;
class SwFootnoteContFrm;
class SwLayouter;

#define FINDMODE_ENDNOTE 1
#define FINDMODE_LASTCNT 2
#define FINDMODE_MYLAST  4

class SwSectionFrm: public SwLayoutFrm, public SwFlowFrm
{
    SwSection* pSection;
    bool bFootnoteAtEnd; // footnotes at the end of section
    bool bEndnAtEnd; // endnotes at the end of section
    bool bContentLock; // content locked
    bool bOwnFootnoteNum; // special numbering of footnotes
    bool bFootnoteLock; // ftn, don't leave this section bwd

    void _UpdateAttr( const SfxPoolItem*, const SfxPoolItem*, sal_uInt8 &,
                      SwAttrSetChg *pa = 0, SwAttrSetChg *pb = 0 );
    void _Cut( bool bRemove );
    // Is there a FootnoteContainer?
    // An empty sectionfrm without FootnoteCont is superfluous
    bool IsSuperfluous() const { return !ContainsAny() && !ContainsFootnoteCont(); }
    void CalcFootnoteAtEndFlag();
    void CalcEndAtEndFlag();
    const SwSectionFormat* _GetEndSectFormat() const;
    bool IsEndnoteAtMyEnd() const;

    virtual void DestroyImpl() override;
    virtual ~SwSectionFrm();

protected:
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;
    virtual bool ShouldBwdMoved( SwLayoutFrm *pNewUpper, bool bHead, bool &rReformat ) override;
    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = 0 ) override;
    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* ) override;
    virtual void SwClientNotify( const SwModify&, const SfxHint& ) override;

public:
    SwSectionFrm( SwSection &, SwFrm* ); // Content is not created!
    SwSectionFrm( SwSectionFrm &, bool bMaster ); // _ONLY_ for creating Master/Follows!

    void Init();
    virtual void CheckDirection( bool bVert ) override;

    virtual void PaintSubsidiaryLines( const SwPageFrm*, const SwRect& ) const override;

    virtual void Cut() override;
    virtual void Paste( SwFrm* pParent, SwFrm* pSibling = 0 ) override;

    inline const SwSectionFrm *GetFollow() const;
    inline       SwSectionFrm *GetFollow();
    SwSectionFrm* FindMaster() const;

                 SwContentFrm *FindLastContent( sal_uInt8 nMode = 0 );
    inline const SwContentFrm *FindLastContent( sal_uInt8 nMode = 0 ) const;
    inline SwSection* GetSection() { return pSection; }
    inline const SwSection* GetSection() const { return pSection; }
    inline void ColLock()       { mbColLocked = true; }
    inline void ColUnlock()     { mbColLocked = false; }

    void CalcFootnoteContent();
    void SimpleFormat();
    bool IsDescendantFrom( const SwSectionFormat* pSect ) const;
    bool HasToBreak( const SwFrm* pFrm ) const;
    void MergeNext( SwSectionFrm* pNxt );

    /**
     * Splits the SectionFrm surrounding the pFrm up in two parts:
     * pFrm and the start of the 2nd part
     */
    bool SplitSect( SwFrm* pFrm, bool bApres );
    void DelEmpty( bool bRemove ); // Like Cut(), except for that Follow chaining is maintained
    SwFootnoteContFrm* ContainsFootnoteCont( const SwFootnoteContFrm* pCont = NULL ) const;
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

    void InvalidateFootnotePos();
    void CollectEndnotes( SwLayouter* pLayouter );
    const SwSectionFormat* GetEndSectFormat() const
        { if( IsEndnAtEnd() ) return _GetEndSectFormat(); return NULL; }

    static void MoveContentAndDelete( SwSectionFrm* pDel, bool bSave );

    bool IsBalancedSection() const;

    virtual void dumpAsXmlAttributes(xmlTextWriterPtr writer) const override;

    bool IsFootnoteAtEnd() const { return bFootnoteAtEnd; }
    bool IsEndnAtEnd() const { return bEndnAtEnd;   }
    bool IsAnyNoteAtEnd() const { return bFootnoteAtEnd || bEndnAtEnd; }

    void SetContentLock( bool bNew ) { bContentLock = bNew; }
    bool IsContentLocked() const { return bContentLock; }

    bool IsOwnFootnoteNum() const { return bOwnFootnoteNum; }

    void SetFootnoteLock( bool bNew ) { bFootnoteLock = bNew; }
    bool IsFootnoteLock() const { return bFootnoteLock; }

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
inline const SwContentFrm *SwSectionFrm::FindLastContent( sal_uInt8 nMode ) const
{
    return const_cast<SwSectionFrm*>(this)->FindLastContent( nMode );
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_SECTFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
