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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_LAYFRM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_LAYFRM_HXX

#include "frame.hxx"
#include <swdllapi.h>

class SwAnchoredObject;
class SwContentFrame;
class SwFormatCol;
struct SwCursorMoveState;
class SwFrameFormat;
class SwBorderAttrs;
class SwFormatFrameSize;
class SwCellFrame;

/// A layout frame is a frame that contains other frames (m_pLower), e.g. SwPageFrame or SwTabFrame.
class SAL_DLLPUBLIC_RTTI SwLayoutFrame: public SwFrame
{
    // The SwFrame in disguise
    friend class SwFlowFrame;
    friend class SwFrame;

    // Releases the Lower while restructuring columns
    friend SwFrame* SaveContent( SwLayoutFrame *, SwFrame * );
    friend void   RestoreContent( SwFrame *, SwLayoutFrame *, SwFrame *pSibling );

protected:

    virtual void DestroyImpl() override;
    virtual ~SwLayoutFrame() override;

    virtual void Format( vcl::RenderContext* pRenderContext, const SwBorderAttrs *pAttrs = nullptr ) override;
    virtual void MakeAll(vcl::RenderContext* pRenderContext) override;

    SwFrame * m_pLower;
    std::vector<SwAnchoredObject*> m_VertPosOrientFramesFor;

    virtual SwTwips ShrinkFrame( SwTwips, bool bTst = false, bool bInfo = false ) override;
    virtual SwTwips GrowFrame  ( SwTwips, bool bTst = false, bool bInfo = false ) override;

    tools::Long CalcRel( const SwFormatFrameSize &rSz ) const;

public:
    // --> #i28701#

    virtual void PaintSubsidiaryLines( const SwPageFrame*, const SwRect& ) const;
    void RefreshLaySubsidiary( const SwPageFrame*, const SwRect& ) const;
    void RefreshExtraData( const SwRect & ) const;

    /// Change size of lowers proportionally
    void ChgLowersProp( const Size& rOldSize );

    void AdjustColumns( const SwFormatCol *pCol, bool bAdjustAttributes );

    void ChgColumns( const SwFormatCol &rOld, const SwFormatCol &rNew,
        const bool bChgFootnote = false );

    /// Paints the column separation line for the inner columns
    void PaintColLines( const SwRect &, const SwFormatCol &,
                        const SwPageFrame * ) const;

    virtual bool    FillSelection( SwSelectionList& rList, const SwRect& rRect ) const override;

    virtual bool GetModelPositionForViewPoint( SwPosition *, Point&,
                               SwCursorMoveState* = nullptr, bool bTestBackground = false ) const override;

    virtual void Cut() override;
    virtual void Paste( SwFrame* pParent, SwFrame* pSibling = nullptr ) override;

    /**
     * Finds the closest Content for the SPoint
     * Is used for Pages, Flys and Cells if GetModelPositionForViewPoint failed
     */
    const SwContentFrame* GetContentPos( Point &rPoint, const bool bDontLeave,
                                   const bool bBodyOnly = false,
                                   SwCursorMoveState *pCMS = nullptr,
                                   const bool bDefaultExpand = true ) const;

    SwLayoutFrame( SwFrameFormat*, SwFrame* );

    virtual void PaintSwFrame( vcl::RenderContext& rRenderContext, SwRect const&, PaintFrameMode mode = PAINT_ALL ) const override;
    const SwFrame *Lower() const { return m_pLower; }
          SwFrame *Lower()       { return m_pLower; }
    bool ContainsDeleteForbiddenLayFrame() const;
    SW_DLLPUBLIC const SwContentFrame *ContainsContent() const;
    inline SwContentFrame *ContainsContent();
    const SwCellFrame *FirstCell() const;
    inline SwCellFrame *FirstCell();

    /**
     * Method <ContainsAny()> doesn't investigate content of footnotes by default.
     * But under certain circumstances this investigation is intended.
     * Thus, introduce new optional parameter <_bInvestigateFootnoteForSections>.
     * It's default is <false>, still indicating that content of footnotes isn't
     * investigated for sections.
     */
    const SwFrame *ContainsAny( const bool _bInvestigateFootnoteForSections = false ) const;
    inline SwFrame *ContainsAny( const bool _bInvestigateFootnoteForSections = false );
    bool IsAnLower( const SwFrame * ) const;

    virtual const SwFrameFormat *GetFormat() const;
    virtual       SwFrameFormat *GetFormat();
    void        SetFrameFormat( SwFrameFormat* );

    /**
     * Moving the Footnotes of all Lowers - starting from StartContent
     *
     * @returns true if at least one Footnote was moved
     * Calls the page number update if bFootnoteNums is set
     */
    bool MoveLowerFootnotes( SwContentFrame *pStart, SwFootnoteBossFrame *pOldBoss,
                        SwFootnoteBossFrame *pNewBoss, const bool bFootnoteNums );

    // --> #i28701# - change purpose of method and its name
    // --> #i44016# - add parameter <_bUnlockPosOfObjs> to
    /// force an unlockposition call for the lower objects.
    void NotifyLowerObjs( const bool _bUnlockPosOfObjs = false );

    /**
     * Invalidates the inner Frames, whose width and/or height are
     * calculated using percentages.
     * Frames that are anchored to this or inner Frames, are also invalidated.
     */
    void InvaPercentLowers( SwTwips nDiff = 0 );

    /// Called by Format for Frames and Areas with columns
    void FormatWidthCols( const SwBorderAttrs &, const SwTwips nBorder,
                          const SwTwips nMinHeight );

    /**
     * InnerHeight returns the height of the content and may be bigger or
     * less than the PrtArea-Height of the layoutframe himself
     */
    SwTwips InnerHeight() const;

    /** method to check relative position of layout frame to
        a given layout frame.

        refactoring of pseudo-local method <lcl_Apres(..)> in
        <txtftn.cxx> for #104840#.

        @param _aCheckRefLayFrame
        constant reference of an instance of class <SwLayoutFrame> which
        is used as the reference for the relative position check.

        @return true, if <this> is positioned before the layout frame <p>
    */
    bool IsBefore( const SwLayoutFrame* _pCheckRefLayFrame ) const;

    SW_DLLPUBLIC const SwFrame* GetLastLower() const;
    inline SwFrame* GetLastLower();

    virtual void PaintBreak() const;

    void SetVertPosOrientFrameFor(SwAnchoredObject *pObj)
    {
        m_VertPosOrientFramesFor.push_back(pObj);
    }

    void ClearVertPosOrientFrameFor(SwAnchoredObject *pObj)
    {
        std::erase(m_VertPosOrientFramesFor, pObj);
    }
    void dumpAsXmlAttributes(xmlTextWriterPtr writer) const override;
};

/**
 * In order to save us from duplicating implementations, we cast here
 * a little.
 */
inline SwContentFrame* SwLayoutFrame::ContainsContent()
{
    return const_cast<SwContentFrame*>(static_cast<const SwLayoutFrame*>(this)->ContainsContent());
}

inline SwCellFrame* SwLayoutFrame::FirstCell()
{
    return const_cast<SwCellFrame*>(static_cast<const SwLayoutFrame*>(this)->FirstCell());
}

inline SwFrame* SwLayoutFrame::ContainsAny( const bool _bInvestigateFootnoteForSections )
{
    return const_cast<SwFrame*>(static_cast<const SwLayoutFrame*>(this)->ContainsAny( _bInvestigateFootnoteForSections ));
}

/**
 * These SwFrame inlines are here, so that frame.hxx does not need to include layfrm.hxx
 */
inline bool SwFrame::IsColBodyFrame() const
{
    return mnFrameType == SwFrameType::Body && GetUpper()->IsColumnFrame();
}

inline bool SwFrame::IsPageBodyFrame() const
{
    return mnFrameType == SwFrameType::Body && GetUpper()->IsPageFrame();
}

inline SwFrame* SwLayoutFrame::GetLastLower()
{
    return const_cast<SwFrame*>(static_cast<const SwLayoutFrame*>(this)->GetLastLower());
}

#endif // INCLUDED_SW_SOURCE_CORE_INC_LAYFRM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
