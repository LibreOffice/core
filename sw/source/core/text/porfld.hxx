/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
    XubString  aExpand;             // The expanded field
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
    const sal_Bool bPlaceHolder : 1;
    sal_Bool m_bNoLength : 1;       // HACK for meta suffix (no CH_TXTATR)

    inline void SetFont( SwFont *pNew ) { pFnt = pNew; }
    inline bool IsNoLength() const  { return m_bNoLength; }
    inline void SetNoLength()       { m_bNoLength = sal_True; }

public:
    SwFldPortion( const SwFldPortion& rFld );
    SwFldPortion( const XubString &rExpand, SwFont *pFnt = 0, sal_Bool bPlaceHolder = sal_False );
    ~SwFldPortion();

    void TakeNextOffset( const SwFldPortion* pFld );
    void CheckScript( const SwTxtSizeInfo &rInf );
    inline sal_Bool HasFont() const { return 0 != pFnt; }
    // #i89179# - made public
    inline const SwFont *GetFont() const { return pFnt; }

    inline const XubString &GetExp() const { return aExpand; }
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;
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
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;

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
    inline SwHiddenPortion( const XubString &rExpand, SwFont *pFntL = 0 )
         : SwFldPortion( rExpand, pFntL )
        { SetLen(1); SetWhichPor( POR_HIDDEN ); }
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual sal_Bool GetExpTxt( const SwTxtSizeInfo &rInf, XubString &rTxt ) const;

    // Field cloner for SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;
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
    SwNumberPortion( const XubString &rExpand,
                     SwFont *pFnt,
                     const sal_Bool bLeft,
                     const sal_Bool bCenter,
                     const KSHORT nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    virtual void Paint( const SwTxtPaintInfo &rInf ) const;
    virtual xub_StrLen GetCrsrOfst( const MSHORT nOfst ) const;
    virtual sal_Bool Format( SwTxtFormatInfo &rInf );

    // Field cloner for SplitGlue
    virtual SwFldPortion *Clone( const XubString &rExpand ) const;
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
                     const XubString& rBulletFollowedBy,
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
                     const XubString& rGraphicFollowedBy,
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
    SwCombinedPortion( const XubString &rExpand );
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
