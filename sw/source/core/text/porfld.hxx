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
#ifndef _PORFLD_HXX
#define _PORFLD_HXX

#include "swtypes.hxx"
#include "porexp.hxx"
#include <fmtornt.hxx>

class SwFont;
class SvxBrushItem;
class SwFmtVertOrient;
class SwFrm;

/*************************************************************************
 *                      class SwFldPortion
 *************************************************************************/

class SwFldPortion : public SwExpandPortion
{
    friend class SwTxtFormatter;
protected:
    OUString  aExpand;              // The expanded field
    SwFont  *pFnt;                  // For multi-line fields
    xub_StrLen nNextOffset;         // Offset of the follow in the original string
    xub_StrLen nNextScriptChg;
    KSHORT  nViewWidth;             // Screen width for empty fields
    sal_Bool bFollow : 1;           // 2nd or later part of a field
    sal_Bool bLeft : 1;             // Used by SwNumberPortion
    sal_Bool bHide : 1;             // Used by SwNumberPortion
    sal_Bool bCenter : 1;           // Used by SwNumberPortion
    sal_Bool bHasFollow : 1;        // Continues on the next line
    sal_Bool bAnimated : 1;         // Used by SwGrfNumPortion
    sal_Bool bNoPaint : 1;          // Used by SwGrfNumPortion
    sal_Bool bReplace : 1;          // Used by SwGrfNumPortion
    const bool bPlaceHolder : 1;
    bool m_bNoLength : 1;       // HACK for meta suffix (no CH_TXTATR)

    inline void SetFont( SwFont *pNew ) { pFnt = pNew; }
    inline bool IsNoLength() const  { return m_bNoLength; }
    inline void SetNoLength()       { m_bNoLength = true; }

public:
    SwFldPortion( const SwFldPortion& rFld );
    SwFldPortion( const OUString &rExpand, SwFont *pFnt = 0, bool bPlaceHolder = false );
    ~SwFldPortion();

    void TakeNextOffset( const SwFldPortion* pFld );
    void CheckScript( const SwTxtSizeInfo &rInf );
    inline bool HasFont() const { return 0 != pFnt; }
    // #i89179# - made public
    inline const SwFont *GetFont() const { return pFnt; }

    inline OUString GetExp() const { return aExpand; }
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;

    // Empty fields are also allowed
    virtual SwLinePortion *Compress();

    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;

    inline sal_Bool IsFollow() const { return bFollow; }
    inline void SetFollow( sal_Bool bNew ) { bFollow = bNew; }

    inline sal_Bool IsLeft() const { return bLeft; }
    inline void SetLeft( sal_Bool bNew ) { bLeft = bNew; }

    inline sal_Bool IsHide() const { return bHide; }
    inline void SetHide( sal_Bool bNew ) { bHide = bNew; }

    inline sal_Bool IsCenter() const { return bCenter; }
    inline void SetCenter( sal_Bool bNew ) { bCenter = bNew; }

    inline sal_Bool HasFollow() const { return bHasFollow; }
    inline void SetHasFollow( sal_Bool bNew ) { bHasFollow = bNew; }

    inline xub_StrLen GetNextOffset() const { return nNextOffset; }
    inline void SetNextOffset( xub_StrLen nNew ) { nNextOffset = nNew; }

    inline xub_StrLen GetNextScriptChg() const { return nNextScriptChg; }
    inline void SetNextScriptChg( xub_StrLen nNew ) { nNextScriptChg = nNew; }

    // Field cloner for SplitGlue
    virtual SwFldPortion *Clone( const OUString &rExpand ) const;

    // Extra GetTxtSize because of pFnt
    virtual SwPosSize GetTxtSize( const SwTxtSizeInfo &rInfo ) const;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const;

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwHiddenPortion
 *************************************************************************/
// Distinguish only for painting/hide

class SwHiddenPortion : public SwFldPortion
{
public:
    inline SwHiddenPortion( const OUString &rExpand, SwFont *pFntL = 0 )
         : SwFldPortion( rExpand, pFntL )
        { SetLen(1); SetWhichPor( POR_HIDDEN ); }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, OUString &rTxt ) const;

    // Field cloner for SplitGlue
    virtual SwFldPortion *Clone( const OUString &rExpand ) const;
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwNumberPortion
 *************************************************************************/

class SwNumberPortion : public SwFldPortion
{
protected:
    KSHORT  nFixWidth;      // See Glues
    KSHORT  nMinDist;       // Minimal distance to the text
    bool    mbLabelAlignmentPosAndSpaceModeActive;

public:
    SwNumberPortion( const OUString &rExpand,
                     SwFont *pFnt,
                     const sal_Bool bLeft,
                     const sal_Bool bCenter,
                     const KSHORT nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual xub_StrLen GetCrsrOfst( const MSHORT nOfst ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    // Field cloner for SplitGlue
    virtual SwFldPortion *Clone( const OUString &rExpand ) const;
    virtual void FormatEOL( SwTxtFormatInfo &rInf );

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwBulletPortion
 *************************************************************************/

class SwBulletPortion : public SwNumberPortion
{
public:
    SwBulletPortion( const sal_Unicode cCh,
                     const OUString& rBulletFollowedBy,
                     SwFont *pFnt,
                     const sal_Bool bLeft,
                     const sal_Bool bCenter,
                     const KSHORT nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwBmpBulletPortion
 *************************************************************************/

class SwGrfNumPortion : public SwNumberPortion
{
    SvxBrushItem* pBrush;
    long            nId;    // For StopAnimation
    SwTwips         nYPos;  // _Always_ contains the current RelPos
    SwTwips         nGrfHeight;
    sal_Int16       eOrient;
public:
    SwGrfNumPortion( SwFrm *pFrm,
                     const OUString& rGraphicFollowedBy,
                     const SvxBrushItem* pGrfBrush,
                     const SwFmtVertOrient* pGrfOrient,
                     const Size& rGrfSize,
                     const sal_Bool bLeft,
                     const sal_Bool bCenter,
                     const KSHORT nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    ~SwGrfNumPortion();
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    void SetBase( long nLnAscent, long nLnDescent,
        long nFlyAscent, long nFlyDescent );

    void StopAnimation( OutputDevice* pOut );

    inline sal_Bool IsAnimated() const { return bAnimated; }
    inline void SetAnimated( sal_Bool bNew ) { bAnimated = bNew; }
    inline sal_Bool DontPaint() const { return bNoPaint; }
    inline void SetNoPaint( sal_Bool bNew ) { bNoPaint = bNew; }
    inline void SetRelPos( SwTwips nNew ) { nYPos = nNew; }
    inline void SetId( long nNew ) const
        { ((SwGrfNumPortion*)this)->nId = nNew; }
    inline SwTwips GetRelPos() const { return nYPos; }
    inline SwTwips GetGrfHeight() const { return nGrfHeight; }
    inline SwTwips GetId() const { return nId; }
    inline sal_Int16 GetOrient() const { return eOrient; }

    OUTPUT_OPERATOR
};

/*************************************************************************
 *                      class SwCombinedPortion
 * Used in for asian layout specialities to display up to six characters
 * in 2 rows and 2-3 columns.
 * e.g.
 *
 *       A..  A..  A.B  A.B   A.B.C   A.B.C
 *       ...  ..B  .C.  C.D   .D.E.   D.E.F
 *************************************************************************/

class SwCombinedPortion : public SwFldPortion
{
    sal_uInt16 aPos[6];     // up to six X positions
    sal_uInt16 aWidth[3];   // one width for every scripttype
    sal_uInt8 aScrType[6];  // scripttype of every character
    sal_uInt16 nUpPos;      // the Y position of the upper baseline
    sal_uInt16 nLowPos;     // the Y position of the lower baseline
    sal_uInt8 nProportion;  // relative font height
public:
    SwCombinedPortion( const OUString &rExpand );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );
    virtual KSHORT GetViewWidth( const SwTxtSizeInfo &rInf ) const;
    OUTPUT_OPERATOR
};


CLASSIO( SwHiddenPortion )
CLASSIO( SwNumberPortion )
CLASSIO( SwBulletPortion )
CLASSIO( SwGrfNumPortion )
CLASSIO( SwCombinedPortion )


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
