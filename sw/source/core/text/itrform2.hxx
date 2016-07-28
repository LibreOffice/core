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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_ITRFORM2_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_ITRFORM2_HXX
#include "itrpaint.hxx"

class SwFlyCntPortion;
class SwInterHyphInfo;
class SwDropPortion;
class SwFormatDrop;
class SwTextAttr;
class SwNumberPortion;
class SwErgoSumPortion;
class SwExpandPortion;
class SwMultiPortion;
class SwFootnotePortion;

class SwTextFormatter : public SwTextPainter
{
    const SwFormatDrop *pDropFormat;
    SwMultiPortion* pMulti; // during formatting a multi-portion
    sal_uInt8 nCntEndHyph;  // Counts consecutive hyphens at the line end
    sal_uInt8 nCntMidHyph;  // Counts consecutive hyphens before flies
    sal_Int32 nLeftScanIdx; // for increasing performance during
    sal_Int32 nRightScanIdx; // scanning for portion ends
    bool bOnceMore : 1; // Another round?
    bool bFlyInCntBase : 1; // Base reference that sets a character-bound frame
    bool bChanges : 1; // Flag for calculating the repaint rectangle
    bool bTruncLines : 1; // Flag for extending the repaint rect, if needed
    bool bUnclipped : 1; // Flag whether repaint is larger than the fixed line height
    size_t m_nHintEndIndex; // HACK for TryNewNoLengthPortion
    SwLinePortion* m_pFirstOfBorderMerge; // The first text portion of a joined border (during portion building)

    SwLinePortion *NewPortion( SwTextFormatInfo &rInf );
    SwTextPortion  *NewTextPortion( SwTextFormatInfo &rInf );
    SwLinePortion *NewExtraPortion( SwTextFormatInfo &rInf );
    SwTabPortion *NewTabPortion( SwTextFormatInfo &rInf, bool bAuto ) const;
    SwNumberPortion *NewNumberPortion( SwTextFormatInfo &rInf ) const;
    SwDropPortion *NewDropPortion( SwTextFormatInfo &rInf );
    SwNumberPortion *NewFootnoteNumPortion( SwTextFormatInfo &rInf ) const;
    SwErgoSumPortion *NewErgoSumPortion( SwTextFormatInfo &rInf ) const;
    SwExpandPortion *NewFieldPortion( SwTextFormatInfo &rInf,
                                    const SwTextAttr *pHt ) const;
    SwFootnotePortion *NewFootnotePortion( SwTextFormatInfo &rInf, SwTextAttr *pHt );

    /**
        Sets a new portion for an object anchored as character
     */
    SwFlyCntPortion *NewFlyCntPortion( SwTextFormatInfo &rInf,
                                       SwTextAttr *pHt ) const;
    SwLinePortion *WhichFirstPortion( SwTextFormatInfo &rInf );
    SwTextPortion *WhichTextPor( SwTextFormatInfo &rInf ) const;
    SwExpandPortion * TryNewNoLengthPortion( SwTextFormatInfo & rInfo );

    // The center piece of formatting
    void BuildPortions( SwTextFormatInfo &rInf );

    bool BuildMultiPortion( SwTextFormatInfo &rInf, SwMultiPortion& rMulti );

    /**
        Calculation of the emulated right side.

        Determines the next object, that reaches into the rest of the line and
        constructs the appropriate FlyPortion.
        SwTextFly::GetFrame(const SwRect&, bool) will be needed for this.

        The right edge can be shortened by flys
     */
    void CalcFlyWidth( SwTextFormatInfo &rInf );

    // Is overloaded by SwTextFormatter because of UpdatePos
    void CalcAdjustLine( SwLineLayout *pCurr );

    // considers line spacing attributes
    void CalcRealHeight( bool bNewLine = false );

    // Transfers the data to rInf
    void FeedInf( SwTextFormatInfo &rInf ) const;

    // Treats underflow situations
    SwLinePortion *Underflow( SwTextFormatInfo &rInf );

    // Calculates the ascent and the height from the fontmetric
    void CalcAscent( SwTextFormatInfo &rInf, SwLinePortion *pPor );

    // determines, if a optimized repaint rectangle is allowed
    bool AllowRepaintOpt() const;

    // Is called by FormatLine
    void FormatReset( SwTextFormatInfo &rInf );

    /**
        The position of the portions changes with the adjustment.

        This method updates the reference point of the anchored as character objects,
        for example after adjustment change (right alignment, justified, etc.)
        Mainly to correct the X position.
     */
    void UpdatePos( SwLineLayout *pCurr, Point aStart, sal_Int32 nStartIdx,
            bool bAlways = false ) const;

    /**
        Set all anchored as character objects to the passed BaseLine
        (in Y direction).
     */
    void AlignFlyInCntBase( long nBaseLine ) const;

    /**
        This is called after the real height of the line has been calculated
        Therefore it is possible, that more flys from below intersect with the
        line, or that flys from above do not intersect with the line anymore.
        We check this and return true, meaning that the line has to be
        formatted again.
     */
    bool ChkFlyUnderflow( SwTextFormatInfo &rInf ) const;

    // Insert portion
    void InsertPortion( SwTextFormatInfo &rInf, SwLinePortion *pPor ) const;

    // Guess height for the DropPortion
    void GuessDropHeight( const sal_uInt16 nLines );

public:
    // Calculate the height for the DropPortion
    void CalcDropHeight( const sal_uInt16 nLines );

    // Calculates the paragraphs bottom, takes anchored objects within it into
    // account which have a wrap setting of "wrap at 1st paragraph"
    SwTwips CalcBottomLine() const;

    // Takes character-bound objects into account when calculating the
    // repaint rect in lines with fixed line height
    void CalcUnclipped( SwTwips& rTop, SwTwips& rBottom );

    // Amongst others for DropCaps
    bool CalcOnceMore();

    void CtorInitTextFormatter( SwTextFrame *pFrame, SwTextFormatInfo *pInf );
    SwTextFormatter(SwTextFrame *pTextFrame, SwTextFormatInfo *pTextFormatInf)
        : SwTextPainter(pTextFrame->GetTextNode())
        , bUnclipped(false)
    {
        CtorInitTextFormatter( pTextFrame, pTextFormatInf );
    }
    virtual ~SwTextFormatter();

    sal_Int32 FormatLine( const sal_Int32 nStart );

    void RecalcRealHeight();

    // We format a line for interactive hyphenation
    bool Hyphenate( SwInterHyphInfo &rInf );

    // A special method for QuoVadis texts:
    // nErgo is the page number of the ErgoSum Footnote
    // At 0 it's still unclear
    sal_Int32 FormatQuoVadis( const sal_Int32 nStart );

    // The emergency break: Cancel formatting, discard line
    bool IsStop() const { return GetInfo().IsStop(); }

    // The counterpart: Continue formatting at all costs
    bool IsNewLine() const { return GetInfo().IsNewLine(); }

    // FormatQuick(); Refresh formatting information
    bool IsQuick() const { return GetInfo().IsQuick(); }

    // Create a SwLineLayout if needed, which avoids Footnote/Fly to oscillate
    void MakeDummyLine();

    // SwTextIter functionality
    void Insert( SwLineLayout *pLine );

    // The remaining height to the page border
    sal_uInt16 GetFrameRstHeight() const;

    // How wide would you be without any bounds (Flys etc.)?
    SwTwips CalcFitToContent_( );

    SwLinePortion* MakeRestPortion(const SwLineLayout* pLine, sal_Int32 nPos);

    const SwFormatDrop *GetDropFormat() const { return pDropFormat; }
    void ClearDropFormat() { pDropFormat = nullptr; }

    SwMultiPortion *GetMulti() const { return pMulti; }

    bool IsOnceMore() const { return bOnceMore; }
    void SetOnceMore( bool bNew ) { bOnceMore = bNew; }

    void SetChanges()       { bChanges = true; }

    bool HasTruncLines() const { return bTruncLines; }
    void SetTruncLines( bool bNew ) { bTruncLines = bNew; }

    bool IsUnclipped() const { return bUnclipped; }
    void SetUnclipped( bool bNew ) { bUnclipped = bNew; }

    bool IsFlyInCntBase() const { return bFlyInCntBase; }
    void SetFlyInCntBase( bool bNew = true ) { bFlyInCntBase = bNew; }

    SwTextFormatInfo &GetInfo()
        { return static_cast<SwTextFormatInfo&>(SwTextIter::GetInfo()); }
    const SwTextFormatInfo &GetInfo() const
        { return static_cast<const SwTextFormatInfo&>(SwTextIter::GetInfo()); }

    void InitCntHyph() { CntHyphens( nCntEndHyph, nCntMidHyph ); }
    const sal_uInt8 &CntEndHyph() const { return nCntEndHyph; }
    const sal_uInt8 &CntMidHyph() const { return nCntMidHyph; }
    sal_uInt8 &CntEndHyph() { return nCntEndHyph; }
    sal_uInt8 &CntMidHyph() { return nCntMidHyph; }

    /**
     * Merge border of the drop portion with modifying the font of
     * the portions' part. Removing left or right border.
     * @param   rPortion    drop portion for merge
    **/
    static void MergeCharacterBorder( SwDropPortion& rPortion );

    /**
     * Merge border of the line portion with setting the portion's
     * m_bJoinBorderWidthNext and m_bJoinBorderWidthPrev members and
     * changing the size (width, height and ascent) of the portion
     * to get a merged border.
     * @param   rPortion    portion for merge
     * @param   pPrev       portion immediately before rPortion
     * @param   rInf        contain information
    **/
    void MergeCharacterBorder( SwLinePortion& rPortion, SwLinePortion *pPrev, SwTextFormatInfo& rInf );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
