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
#ifndef _ITRFORM2_HXX
#define _ITRFORM2_HXX
#include "itrpaint.hxx"

class SwFlyCntPortion;
class SwInterHyphInfo;
class SwDropPortion;
class SwFmtDrop;
class SwTxtAttr;
class SwNumberPortion;
class SwErgoSumPortion;
class SwExpandPortion;
class SwMultiPortion;
class SwFtnPortion;


class SwTxtFormatter : public SwTxtPainter
{
    const SwFmtDrop *pDropFmt;
    SwMultiPortion* pMulti; // during formatting a multi-portion
    sal_uInt8 nCntEndHyph;  // Counts consecutive hyphens at the line end
    sal_uInt8 nCntMidHyph;  // Counts consecutive hyphens before flies
    xub_StrLen nLeftScanIdx; // for increasing performance during
    xub_StrLen nRightScanIdx; // scanning for portion ends
    sal_Bool bOnceMore : 1; // Another round?
    sal_Bool bFlyInCntBase : 1; // Base reference that sets a character-bound frame
    sal_Bool bChanges : 1; // Flag for calculating the repaint rectangle
    sal_Bool bTruncLines : 1; // Flag for extending the repaint rect, if needed
    sal_Bool bUnclipped : 1; // Flag whether repaint is larger than the fixed line height
    sal_uInt16 m_nHintEndIndex; // HACK for TryNewNoLengthPortion
    SwLinePortion* m_pFirstOfBorderMerge; // The first text portion of a joined border (during portion bulding)

    SwLinePortion *NewPortion( SwTxtFormatInfo &rInf );
    SwTxtPortion  *NewTxtPortion( SwTxtFormatInfo &rInf );
    SwLinePortion *NewExtraPortion( SwTxtFormatInfo &rInf );
    SwTabPortion *NewTabPortion( SwTxtFormatInfo &rInf, bool bAuto ) const;
    SwNumberPortion *NewNumberPortion( SwTxtFormatInfo &rInf ) const;
    SwDropPortion *NewDropPortion( SwTxtFormatInfo &rInf );
    SwNumberPortion *NewFtnNumPortion( SwTxtFormatInfo &rInf ) const;
    SwErgoSumPortion *NewErgoSumPortion( SwTxtFormatInfo &rInf ) const;
    SwExpandPortion *NewFldPortion( SwTxtFormatInfo &rInf,
                                    const SwTxtAttr *pHt ) const;
    SwFtnPortion *NewFtnPortion( SwTxtFormatInfo &rInf, SwTxtAttr *pHt );

    /**
        Sets a new portion for an object anchored as character
     */
    SwFlyCntPortion *NewFlyCntPortion( SwTxtFormatInfo &rInf,
                                       SwTxtAttr *pHt ) const;
    SwLinePortion *WhichFirstPortion( SwTxtFormatInfo &rInf );
    SwTxtPortion *WhichTxtPor( SwTxtFormatInfo &rInf ) const;
    SwExpandPortion * TryNewNoLengthPortion( SwTxtFormatInfo & rInfo );

    // The center pice of formatting
    void BuildPortions( SwTxtFormatInfo &rInf );

    sal_Bool BuildMultiPortion( SwTxtFormatInfo &rInf, SwMultiPortion& rMulti );

    /**
        Calculation of the emulated right side.

        Determines the next object, that reaches into the rest of the line and
        constructs the appropriate FlyPortion.
        SwTxtFly::GetFrm(const SwRect&, sal_Bool) will be needed for this.

        The right edge can be shortened by flys
     */
    void CalcFlyWidth( SwTxtFormatInfo &rInf );

    // Is overloaded by SwTxtFormatter because of UpdatePos
    void CalcAdjustLine( SwLineLayout *pCurr );

    // consideres line spacing attributes
    void CalcRealHeight( sal_Bool bNewLine = sal_False );

    // Transfers the data to rInf
    void FeedInf( SwTxtFormatInfo &rInf ) const;

    // Treats underflow situations
    SwLinePortion *UnderFlow( SwTxtFormatInfo &rInf );

    // Calculates the ascent and the height from the fontmetric
    void CalcAscent( SwTxtFormatInfo &rInf, SwLinePortion *pPor );

    // determines, if a optimized repaint rectange is allowed
    sal_Bool AllowRepaintOpt() const;

    // Is called by by FormatLine
    void FormatReset( SwTxtFormatInfo &rInf );

    /**
        The position of the portions changes with the adjustment.

        This method updates the reference point of the anchored as character objects,
        for example after adjustment change (right alignment, justified, etc.)
        Mainly to correct the X position.
     */
    void UpdatePos( SwLineLayout *pCurr, Point aStart, xub_StrLen nStartIdx,
            sal_Bool bAlways = sal_False ) const;

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
    sal_Bool ChkFlyUnderflow( SwTxtFormatInfo &rInf ) const;

    // Insert portion
    void InsertPortion( SwTxtFormatInfo &rInf, SwLinePortion *pPor ) const;

    // Guess height for the DropPortion
    void GuessDropHeight( const MSHORT nLines );

public:
    // Calculate the height for the DropPortion
    void CalcDropHeight( const MSHORT nLines );

    // Calculates the paragraphs bottom, takes anchored objects within it into
    // account which have a wrap setting of "wrap at 1st paragraph"
    SwTwips CalcBottomLine() const;

    // Takes character-bound objects into account when calculating the
    // repaint rect in lines with fixed line height
    void CalcUnclipped( SwTwips& rTop, SwTwips& rBottom );

    // Amongst others for DropCaps
    sal_Bool CalcOnceMore();

    void CtorInitTxtFormatter( SwTxtFrm *pFrm, SwTxtFormatInfo *pInf );
    inline SwTxtFormatter( SwTxtFrm *pTxtFrm, SwTxtFormatInfo *pTxtFmtInf ) : SwTxtPainter(pTxtFrm!=NULL?pTxtFrm->GetTxtNode():NULL)
           { CtorInitTxtFormatter( pTxtFrm, pTxtFmtInf ); }
    ~SwTxtFormatter();

    xub_StrLen FormatLine( const xub_StrLen nStart );

    void RecalcRealHeight();

    // We format a line for interactive hyphenation
    sal_Bool Hyphenate( SwInterHyphInfo &rInf );

    // A special method for QuoVadis texts:
    // nErgo is the page number of the ErgoSum Ftn
    // At 0 it's still unclear
    xub_StrLen FormatQuoVadis( const xub_StrLen nStart );

    // The emergency break: Cancel formatting, discard line
    inline sal_Bool IsStop() const { return GetInfo().IsStop(); }

    // The counterpart: Continue formatting at all costs
    inline sal_Bool IsNewLine() const { return GetInfo().IsNewLine(); }

    // FormatQuick(); Refresh formatting information
    inline sal_Bool IsQuick() const { return GetInfo().IsQuick(); }

    // Create a SwLineLayout if needed, which avoids Ftn/Fly to oscillate
    void MakeDummyLine();

    // SwTxtIter functionality
    void Insert( SwLineLayout *pLine );

    // The remaining height to the page border
    KSHORT GetFrmRstHeight() const;

    // How wide would you be without any bounds (Flys etc.)?
    SwTwips _CalcFitToContent( );

    SwLinePortion* MakeRestPortion(const SwLineLayout* pLine, xub_StrLen nPos);

    inline const SwFmtDrop *GetDropFmt() const { return pDropFmt; }
    inline void ClearDropFmt() { pDropFmt = 0; }

    inline SwMultiPortion *GetMulti() const { return pMulti; }

    inline sal_Bool IsOnceMore() const { return bOnceMore; }
    inline void       SetOnceMore( sal_Bool bNew ) { bOnceMore = bNew; }

    inline sal_Bool HasChanges() const { return bChanges; }
    inline void       SetChanges()       { bChanges = sal_True; }

    inline sal_Bool HasTruncLines() const { return bTruncLines; }
    inline void       SetTruncLines( sal_Bool bNew ) { bTruncLines = bNew; }

    inline sal_Bool IsUnclipped() const { return bUnclipped; }
    inline void       SetUnclipped( sal_Bool bNew ) { bUnclipped = bNew; }

    inline sal_Bool IsFlyInCntBase() const { return bFlyInCntBase; }
    inline void  SetFlyInCntBase( sal_Bool bNew = sal_True ){ bFlyInCntBase = bNew; }

    inline SwTxtFormatInfo &GetInfo()
        { return (SwTxtFormatInfo&)SwTxtIter::GetInfo(); }
    inline const SwTxtFormatInfo &GetInfo() const
        { return (const SwTxtFormatInfo&)SwTxtIter::GetInfo(); }

    inline void InitCntHyph() { CntHyphens( nCntEndHyph, nCntMidHyph ); }
    inline const sal_uInt8 &CntEndHyph() const { return nCntEndHyph; }
    inline const sal_uInt8 &CntMidHyph() const { return nCntMidHyph; }
    inline sal_uInt8 &CntEndHyph() { return nCntEndHyph; }
    inline sal_uInt8 &CntMidHyph() { return nCntMidHyph; }

    /**
     * Merge border of the drop portion with modifying the font of
     * the portions' part. Removing left or right border.
     * @param   rPortion    drop portion for merge
    **/
    void MergeCharacterBorder( SwDropPortion& rPortion );

    /**
     * Merge border of the line portion with setting the portion's
     * m_bJoinBorderWidthNext and m_bJoinBorderWidthPrev members and
     * changing the size (width, height and ascent) of the portion
     * to get a merged border.
     * @param   rPortion    portion for merge
     * @param   rInf        contain information
    **/
    void MergeCharacterBorder( SwLinePortion& rPortion, SwTxtFormatInfo& rInf );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
