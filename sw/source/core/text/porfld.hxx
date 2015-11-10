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
#ifndef INCLUDED_SW_SOURCE_CORE_TEXT_PORFLD_HXX
#define INCLUDED_SW_SOURCE_CORE_TEXT_PORFLD_HXX

#include "swtypes.hxx"
#include "porexp.hxx"
#include <fmtornt.hxx>

class SwFont;
class SvxBrushItem;
class SwFormatVertOrient;
class SwFrm;

class SwFieldPortion : public SwExpandPortion
{
    friend class SwTextFormatter;
protected:
    OUString  aExpand;          // The expanded field
    SwFont  *pFnt;              // For multi-line fields
    sal_Int32 nNextOffset;      // Offset of the follow in the original string
    sal_Int32 nNextScriptChg;
    sal_uInt16  nViewWidth;     // Screen width for empty fields
    bool bFollow : 1;           // 2nd or later part of a field
    bool bLeft : 1;             // Used by SwNumberPortion
    bool bHide : 1;             // Used by SwNumberPortion
    bool bCenter : 1;           // Used by SwNumberPortion
    bool bHasFollow : 1;        // Continues on the next line
    bool bAnimated : 1;         // Used by SwGrfNumPortion
    bool bNoPaint : 1;          // Used by SwGrfNumPortion
    bool bReplace : 1;          // Used by SwGrfNumPortion
    const bool bPlaceHolder : 1;
    bool m_bNoLength : 1;       // HACK for meta suffix (no CH_TXTATR)

    inline void SetFont( SwFont *pNew ) { pFnt = pNew; }
    inline bool IsNoLength() const  { return m_bNoLength; }
    inline void SetNoLength()       { m_bNoLength = true; }

public:
    SwFieldPortion( const SwFieldPortion& rField );
    SwFieldPortion( const OUString &rExpand, SwFont *pFnt = nullptr, bool bPlaceHolder = false );
    virtual ~SwFieldPortion();

    sal_uInt16 m_nAttrFieldType;
    void TakeNextOffset( const SwFieldPortion* pField );
    void CheckScript( const SwTextSizeInfo &rInf );
    inline bool HasFont() const { return nullptr != pFnt; }
    // #i89179# - made public
    inline const SwFont *GetFont() const { return pFnt; }

    inline OUString GetExp() const { return aExpand; }
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;

    // Empty fields are also allowed
    virtual SwLinePortion *Compress() override;

    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const override;

    inline bool IsFollow() const { return bFollow; }
    inline void SetFollow( bool bNew ) { bFollow = bNew; }

    inline bool IsLeft() const { return bLeft; }
    inline void SetLeft( bool bNew ) { bLeft = bNew; }

    inline bool IsHide() const { return bHide; }
    inline void SetHide( bool bNew ) { bHide = bNew; }

    inline bool IsCenter() const { return bCenter; }
    inline void SetCenter( bool bNew ) { bCenter = bNew; }

    inline bool HasFollow() const { return bHasFollow; }
    inline void SetHasFollow( bool bNew ) { bHasFollow = bNew; }

    inline sal_Int32 GetNextOffset() const { return nNextOffset; }
    inline void SetNextOffset( sal_Int32 nNew ) { nNextOffset = nNew; }

    inline sal_Int32 GetNextScriptChg() const { return nNextScriptChg; }

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const;

    // Extra GetTextSize because of pFnt
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;

    OUTPUT_OPERATOR_OVERRIDE
};

/**
 * Distinguish only for painting/hide
 */
class SwHiddenPortion : public SwFieldPortion
{
public:
    inline SwHiddenPortion( const OUString &rExpand, SwFont *pFntL = nullptr )
         : SwFieldPortion( rExpand, pFntL )
        { SetLen(1); SetWhichPor( POR_HIDDEN ); }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;
    OUTPUT_OPERATOR_OVERRIDE
};

class SwNumberPortion : public SwFieldPortion
{
protected:
    sal_uInt16  nFixWidth;      // See Glues
    sal_uInt16  nMinDist;       // Minimal distance to the text
    bool    mbLabelAlignmentPosAndSpaceModeActive;

public:
    SwNumberPortion( const OUString &rExpand,
                     SwFont *pFnt,
                     const bool bLeft,
                     const bool bCenter,
                     const sal_uInt16 nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual sal_Int32 GetCrsrOfst( const sal_uInt16 nOfst ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;

    OUTPUT_OPERATOR_OVERRIDE
};

class SwBulletPortion : public SwNumberPortion
{
public:
    SwBulletPortion( const sal_Unicode cCh,
                     const OUString& rBulletFollowedBy,
                     SwFont *pFnt,
                     const bool bLeft,
                     const bool bCenter,
                     const sal_uInt16 nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    OUTPUT_OPERATOR_OVERRIDE
};

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
                     const SwFormatVertOrient* pGrfOrient,
                     const Size& rGrfSize,
                     const bool bLeft,
                     const bool bCenter,
                     const sal_uInt16 nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    virtual ~SwGrfNumPortion();
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    void SetBase( long nLnAscent, long nLnDescent,
        long nFlyAscent, long nFlyDescent );

    void StopAnimation( OutputDevice* pOut );

    inline bool IsAnimated() const { return bAnimated; }
    inline void SetAnimated( bool bNew ) { bAnimated = bNew; }
    inline bool DontPaint() const { return bNoPaint; }
    inline void SetNoPaint( bool bNew ) { bNoPaint = bNew; }
    inline void SetRelPos( SwTwips nNew ) { nYPos = nNew; }
    inline void SetId( long nNew ) const
        { const_cast<SwGrfNumPortion*>(this)->nId = nNew; }
    inline SwTwips GetRelPos() const { return nYPos; }
    inline SwTwips GetGrfHeight() const { return nGrfHeight; }
    inline sal_Int16 GetOrient() const { return eOrient; }

    OUTPUT_OPERATOR_OVERRIDE
};

/**
 * Used in for asian layout specialities to display up to six characters
 * in 2 rows and 2-3 columns.
 * E.g.: <pre>
 *        A..  A..  A.B  A.B   A.B.C   A.B.C
 *        ...  ..B  .C.  C.D   .D.E.   D.E.F
 *        </pre>
 */
class SwCombinedPortion : public SwFieldPortion
{
    sal_uInt16 aPos[6];     // up to six X positions
    sal_uInt16 aWidth[3];   // one width for every scripttype
    sal_uInt8 aScrType[6];  // scripttype of every character
    sal_uInt16 nUpPos;      // the Y position of the upper baseline
    sal_uInt16 nLowPos;     // the Y position of the lower baseline
    sal_uInt8 nProportion;  // relative font height
public:
    explicit SwCombinedPortion( const OUString &rExpand );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const override;
    OUTPUT_OPERATOR_OVERRIDE
};

namespace sw { namespace mark {
    class IFieldmark;
} }

class SwFieldFormDropDownPortion : public SwFieldPortion
{
public:
    explicit SwFieldFormDropDownPortion(const OUString &rExpand)
        : SwFieldPortion(rExpand)
    {
    }
    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
