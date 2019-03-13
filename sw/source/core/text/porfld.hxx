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

#include <swtypes.hxx>
#include <swfont.hxx>
#include "porexp.hxx"
#include <fmtornt.hxx>
#include <o3tl/enumarray.hxx>

class SwFont;
class SvxBrushItem;
class SwFormatVertOrient;

class SwFieldPortion : public SwExpandPortion
{
    friend class SwTextFormatter;
protected:
    OUString  m_aExpand;          // The expanded field
    std::unique_ptr<SwFont> m_pFont;  // For multi-line fields
    TextFrameIndex m_nNextOffset;  // Offset of the follow in the original string
    TextFrameIndex m_nNextScriptChg;
    sal_uInt16  m_nViewWidth;     // Screen width for empty fields
    bool m_bFollow : 1;           // 2nd or later part of a field
    bool m_bLeft : 1;             // Used by SwNumberPortion
    bool m_bHide : 1;             // Used by SwNumberPortion
    bool m_bCenter : 1;           // Used by SwNumberPortion
    bool m_bHasFollow : 1;        // Continues on the next line
    bool m_bAnimated : 1;         // Used by SwGrfNumPortion
    bool m_bNoPaint : 1;          // Used by SwGrfNumPortion
    bool m_bReplace : 1;          // Used by SwGrfNumPortion
    const bool m_bPlaceHolder : 1;
    bool m_bNoLength : 1;       // HACK for meta suffix (no CH_TXTATR)

    void SetFont( std::unique_ptr<SwFont> pNew ) { m_pFont = std::move(pNew); }
    bool IsNoLength() const  { return m_bNoLength; }
    void SetNoLength()       { m_bNoLength = true; }

public:
    SwFieldPortion( const SwFieldPortion& rField );
    SwFieldPortion( const OUString &rExpand, std::unique_ptr<SwFont> pFnt = nullptr, bool bPlaceHolder = false );
    virtual ~SwFieldPortion() override;

    sal_uInt16 m_nAttrFieldType;
    void TakeNextOffset( const SwFieldPortion* pField );
    void CheckScript( const SwTextSizeInfo &rInf );
    bool HasFont() const { return nullptr != m_pFont; }
    // #i89179# - made public
    const SwFont *GetFont() const { return m_pFont.get(); }

    const OUString& GetExp() const { return m_aExpand; }
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;

    // Empty fields are also allowed
    virtual SwLinePortion *Compress() override;

    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const override;

    bool IsFollow() const { return m_bFollow; }
    void SetFollow( bool bNew ) { m_bFollow = bNew; }

    bool IsLeft() const { return m_bLeft; }
    void SetLeft( bool bNew ) { m_bLeft = bNew; }

    bool IsHide() const { return m_bHide; }
    void SetHide( bool bNew ) { m_bHide = bNew; }

    bool IsCenter() const { return m_bCenter; }
    void SetCenter( bool bNew ) { m_bCenter = bNew; }

    bool HasFollow() const { return m_bHasFollow; }
    void SetHasFollow( bool bNew ) { m_bHasFollow = bNew; }

    TextFrameIndex GetNextOffset() const { return m_nNextOffset; }
    void SetNextOffset(TextFrameIndex nNew) { m_nNextOffset = nNew; }

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const;

    // Extra GetTextSize because of pFnt
    virtual SwPosSize GetTextSize( const SwTextSizeInfo &rInfo ) const override;

    // Accessibility: pass information about this portion to the PortionHandler
    virtual void HandlePortion( SwPortionHandler& rPH ) const override;
};

/**
 * Distinguish only for painting/hide
 */
class SwHiddenPortion : public SwFieldPortion
{
public:
    SwHiddenPortion( const OUString &rExpand, std::unique_ptr<SwFont> pFntL = nullptr )
         : SwFieldPortion( rExpand, std::move(pFntL) )
        { SetLen(TextFrameIndex(1)); SetWhichPor( PortionType::Hidden ); }
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool GetExpText( const SwTextSizeInfo &rInf, OUString &rText ) const override;

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;
};

class SwNumberPortion : public SwFieldPortion
{
protected:
    sal_uInt16  nFixWidth;      // See Glues
    sal_uInt16 const  nMinDist;       // Minimal distance to the text
    bool const    mbLabelAlignmentPosAndSpaceModeActive;

public:
    SwNumberPortion( const OUString &rExpand,
                     std::unique_ptr<SwFont> pFnt,
                     const bool bLeft,
                     const bool bCenter,
                     const sal_uInt16 nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual TextFrameIndex GetCursorOfst(sal_uInt16 nOfst) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;
    virtual void FormatEOL( SwTextFormatInfo &rInf ) override;
};

class SwBulletPortion : public SwNumberPortion
{
public:
    SwBulletPortion( const sal_Unicode cCh,
                     const OUString& rBulletFollowedBy,
                     std::unique_ptr<SwFont> pFnt,
                     const bool bLeft,
                     const bool bCenter,
                     const sal_uInt16 nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
};

class SwGrfNumPortion : public SwNumberPortion
{
    std::unique_ptr<SvxBrushItem> pBrush;
    long            nId;    // For StopAnimation
    SwTwips         nYPos;  // _Always_ contains the current RelPos
    SwTwips         nGrfHeight;
    sal_Int16       eOrient;
public:
    SwGrfNumPortion( const OUString& rGraphicFollowedBy,
                     const SvxBrushItem* pGrfBrush,
                     const SwFormatVertOrient* pGrfOrient,
                     const Size& rGrfSize,
                     const bool bLeft,
                     const bool bCenter,
                     const sal_uInt16 nMinDst,
                     const bool bLabelAlignmentPosAndSpaceModeActive );
    virtual ~SwGrfNumPortion() override;
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;

    void SetBase( long nLnAscent, long nLnDescent,
        long nFlyAscent, long nFlyDescent );

    void StopAnimation( OutputDevice* pOut );

    bool IsAnimated() const { return m_bAnimated; }
    void SetAnimated( bool bNew ) { m_bAnimated = bNew; }
    void SetRelPos( SwTwips nNew ) { nYPos = nNew; }
    void SetId( long nNew ) const
        { const_cast<SwGrfNumPortion*>(this)->nId = nNew; }
    SwTwips GetRelPos() const { return nYPos; }
    SwTwips GetGrfHeight() const { return nGrfHeight; }
    sal_Int16 GetOrient() const { return eOrient; }
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
    o3tl::enumarray<SwFontScript,sal_uInt16> aWidth;   // one width for every scripttype
    SwFontScript aScrType[6];  // scripttype of every character
    sal_uInt16 nUpPos;      // the Y position of the upper baseline
    sal_uInt16 nLowPos;     // the Y position of the lower baseline
    sal_uInt8 nProportion;  // relative font height
public:
    explicit SwCombinedPortion( const OUString &rExpand );
    virtual void Paint( const SwTextPaintInfo &rInf ) const override;
    virtual bool Format( SwTextFormatInfo &rInf ) override;
    virtual sal_uInt16 GetViewWidth( const SwTextSizeInfo &rInf ) const override;
};

namespace sw { namespace mark {
    class IFieldmark;
} }

class SwFieldFormDropDownPortion : public SwFieldPortion
{
public:
    explicit SwFieldFormDropDownPortion(sw::mark::IFieldmark *pFieldMark, const OUString &rExpand)
        : SwFieldPortion(rExpand)
        , m_pFieldMark(pFieldMark)
    {
    }
    // Field cloner for SplitGlue
    virtual SwFieldPortion *Clone( const OUString &rExpand ) const override;

    virtual void Paint( const SwTextPaintInfo &rInf ) const override;

private:
    sw::mark::IFieldmark* m_pFieldMark;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
