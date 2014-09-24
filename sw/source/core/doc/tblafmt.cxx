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

#include <tools/resid.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/docfile.hxx>
#include <svl/urihelper.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/app.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <com/sun/star/text/VertOrientation.hpp>
#include <swtypes.hxx>
#include <doc.hxx>
#include <poolfmt.hxx>
#include <tblafmt.hxx>
#include <cellatr.hxx>
#include <SwStyleNameMapper.hxx>
#include <hintids.hxx>
#include <fmtornt.hxx>
#include <editsh.hxx>

/*
 * XXX: BIG RED NOTICE! Changes MUST be binary file format compatible and MUST
 * be synchronized with Calc's ScAutoFormat sc/source/core/tool/autoform.cxx
 */

using ::editeng::SvxBorderLine;

SwBoxAutoFmt* SwTableAutoFmt::pDfltBoxAutoFmt = 0;

#define AUTOTABLE_FORMAT_NAME "autotbl.fmt"

namespace
{
    /// Begins a writer-specific data block. Call before serializing any writer-specific properties.
    sal_uInt64 BeginSwBlock(SvStream& rStream)
    {
        // We need to write down the offset of the end of the writer-specific data, so that
        // calc can skip it. We'll only have that value after writing the data, so we
        // write a placeholder value first, write the data, then jump back and write the
        // real offset.

        // Note that we explicitly use sal_uInt64 instead of sal_Size (which can be 32
        // or 64 depending on platform) to ensure 64-bit portability on this front. I don't
        // actually know if autotbl.fmt as a whole is portable, since that requires all serialization
        // logic to be written with portability in mind.
        sal_uInt64 whereToWriteEndOfSwBlock = rStream.Tell();

        sal_uInt64 endOfSwBlock = 0;
        rStream.WriteUInt64( endOfSwBlock );

        return whereToWriteEndOfSwBlock;
    }

    /// Ends a writer-specific data block. Call after serializing writer-specific properties.
    /// Closes a corresponding BeginSwBlock call.
    void EndSwBlock(SvStream& rStream, sal_uInt64 whereToWriteEndOfSwBlock)
    {
        sal_uInt64 endOfSwBlock = rStream.Tell();
        rStream.Seek(whereToWriteEndOfSwBlock);
        rStream.WriteUInt64( endOfSwBlock );
        rStream.Seek(endOfSwBlock);
    }

    /**
    Helper class for writer-specific blocks. Begins a writer-specific block on construction,
    and closes it on destruction.

    See also: BeginSwBlock and EndSwBlock.
    */
    class WriterSpecificAutoFormatBlock : ::boost::noncopyable
    {
    public:
        WriterSpecificAutoFormatBlock(SvStream &rStream) : _rStream(rStream)
        {
            _whereToWriteEndOfBlock = BeginSwBlock(rStream);
        }

        ~WriterSpecificAutoFormatBlock()
        {
            EndSwBlock(_rStream, _whereToWriteEndOfBlock);
        }

    private:
        SvStream &_rStream;
        sal_uInt64 _whereToWriteEndOfBlock;
    };

    /// Checks whether a writer-specific block exists (i.e. size is not zero)
    bool WriterSpecificBlockExists(SvStream &stream)
    {
        sal_uInt64 endOfSwBlock = 0;
        stream.ReadUInt64( endOfSwBlock );

        // end-of-block pointing to itself indicates a zero-size block.
        return endOfSwBlock != stream.Tell();
    }
}

SwBoxAutoFmt::SwBoxAutoFmt()
    : aFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_FONT ) ),
    aHeight( 240, 100, RES_CHRATR_FONTSIZE ),
    aWeight( WEIGHT_NORMAL, RES_CHRATR_WEIGHT ),
    aPosture( ITALIC_NONE, RES_CHRATR_POSTURE ),

    aCJKFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CJK_FONT ) ),
    aCJKHeight( 240, 100, RES_CHRATR_CJK_FONTSIZE ),
    aCJKWeight( WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT ),
    aCJKPosture( ITALIC_NONE, RES_CHRATR_CJK_POSTURE ),

    aCTLFont( *(SvxFontItem*)GetDfltAttr( RES_CHRATR_CTL_FONT ) ),
    aCTLHeight( 240, 100, RES_CHRATR_CTL_FONTSIZE ),
    aCTLWeight( WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT ),
    aCTLPosture( ITALIC_NONE, RES_CHRATR_CTL_POSTURE ),

    aUnderline( UNDERLINE_NONE, RES_CHRATR_UNDERLINE ),
    aOverline( UNDERLINE_NONE, RES_CHRATR_OVERLINE ),
    aCrossedOut( STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT ),
    aContour( false, RES_CHRATR_CONTOUR ),
    aShadowed( false, RES_CHRATR_SHADOWED ),
    aColor( RES_CHRATR_COLOR ),
    aBox( RES_BOX ),
    aTLBR( 0 ),
    aBLTR( 0 ),
    aBackground( RES_BACKGROUND ),
    aAdjust( SVX_ADJUST_LEFT, RES_PARATR_ADJUST ),
    m_aTextOrientation(FRMDIR_ENVIRONMENT, RES_FRAMEDIR),
    m_aVerticalAlignment(0, com::sun::star::text::VertOrientation::NONE, com::sun::star::text::RelOrientation::FRAME),
    aHorJustify( SVX_HOR_JUSTIFY_STANDARD, 0),
    aVerJustify( SVX_VER_JUSTIFY_STANDARD, 0),
    aStacked( 0 ),
    aMargin( 0 ),
    aLinebreak( 0 ),
    aRotateAngle( 0 ),

// FIXME - add attribute IDs for the diagonal line items
//    aTLBR( RES_... ),
//    aBLTR( RES_... ),
    aRotateMode( SVX_ROTATE_MODE_STANDARD, 0 )
{
    eSysLanguage = eNumFmtLanguage = ::GetAppLanguage();
    aBox.SetDistance( 55 );
}

SwBoxAutoFmt::SwBoxAutoFmt( const SwBoxAutoFmt& rNew )
    : aFont( rNew.aFont ),
    aHeight( rNew.aHeight ),
    aWeight( rNew.aWeight ),
    aPosture( rNew.aPosture ),
    aCJKFont( rNew.aCJKFont ),
    aCJKHeight( rNew.aCJKHeight ),
    aCJKWeight( rNew.aCJKWeight ),
    aCJKPosture( rNew.aCJKPosture ),
    aCTLFont( rNew.aCTLFont ),
    aCTLHeight( rNew.aCTLHeight ),
    aCTLWeight( rNew.aCTLWeight ),
    aCTLPosture( rNew.aCTLPosture ),
    aUnderline( rNew.aUnderline ),
    aOverline( rNew.aOverline ),
    aCrossedOut( rNew.aCrossedOut ),
    aContour( rNew.aContour ),
    aShadowed( rNew.aShadowed ),
    aColor( rNew.aColor ),
    aBox( rNew.aBox ),
    aTLBR( rNew.aTLBR ),
    aBLTR( rNew.aBLTR ),
    aBackground( rNew.aBackground ),
    aAdjust( rNew.aAdjust ),
    m_aTextOrientation(rNew.m_aTextOrientation),
    m_aVerticalAlignment(rNew.m_aVerticalAlignment),
    aHorJustify( rNew.aHorJustify ),
    aVerJustify( rNew.aVerJustify ),
    aStacked( rNew.aStacked ),
    aMargin( rNew.aMargin ),
    aLinebreak( rNew.aLinebreak ),
    aRotateAngle( rNew.aRotateAngle ),
    aRotateMode( rNew.aRotateMode ),
    sNumFmtString( rNew.sNumFmtString ),
    eSysLanguage( rNew.eSysLanguage ),
    eNumFmtLanguage( rNew.eNumFmtLanguage )
{
}

SwBoxAutoFmt::~SwBoxAutoFmt()
{
}

SwBoxAutoFmt& SwBoxAutoFmt::operator=( const SwBoxAutoFmt& rNew )
{
    aFont = rNew.aFont;
    aHeight = rNew.aHeight;
    aWeight = rNew.aWeight;
    aPosture = rNew.aPosture;
    aCJKFont = rNew.aCJKFont;
    aCJKHeight = rNew.aCJKHeight;
    aCJKWeight = rNew.aCJKWeight;
    aCJKPosture = rNew.aCJKPosture;
    aCTLFont = rNew.aCTLFont;
    aCTLHeight = rNew.aCTLHeight;
    aCTLWeight = rNew.aCTLWeight;
    aCTLPosture = rNew.aCTLPosture;
    aUnderline = rNew.aUnderline;
    aOverline = rNew.aOverline;
    aCrossedOut = rNew.aCrossedOut;
    aContour = rNew.aContour;
    aShadowed = rNew.aShadowed;
    aColor = rNew.aColor;
    SetAdjust( rNew.aAdjust );
    m_aTextOrientation = rNew.m_aTextOrientation;
    m_aVerticalAlignment = rNew.m_aVerticalAlignment;
    aBox = rNew.aBox;
    aTLBR = rNew.aTLBR;
    aBLTR = rNew.aBLTR;
    aBackground = rNew.aBackground;

    aHorJustify = rNew.aHorJustify;
    aVerJustify = rNew.aVerJustify;
    aStacked.SetValue( rNew.aStacked.GetValue() );
    aMargin = rNew.aMargin;
    aLinebreak.SetValue( rNew.aLinebreak.GetValue() );
    aRotateAngle.SetValue( rNew.aRotateAngle.GetValue() );
    aRotateMode.SetValue( rNew.aRotateMode.GetValue() );

    sNumFmtString = rNew.sNumFmtString;
    eSysLanguage = rNew.eSysLanguage;
    eNumFmtLanguage = rNew.eNumFmtLanguage;

    return *this;
}

#define READ( aItem, aItemType )\
    pNew = aItem.Create(rStream ); \
    aItem = *(aItemType*)pNew; \
    delete pNew;

bool SwBoxAutoFmt::Load( SvStream& rStream )
{
    SfxPoolItem* pNew;
    SvxOrientationItem aOrientation( SVX_ORIENTATION_STANDARD, 0);

    READ( aFont,        SvxFontItem )

    if( rStream.GetStreamCharSet() == aFont.GetCharSet() )
        aFont.SetCharSet(::osl_getThreadTextEncoding());

    READ( aHeight,      SvxFontHeightItem )
    READ( aWeight,      SvxWeightItem  )
    READ( aPosture,     SvxPostureItem )
    READ( aCJKFont,     SvxFontItem )
    READ( aCJKHeight,   SvxFontHeightItem )
    READ( aCJKWeight,   SvxWeightItem )
    READ( aCJKPosture,  SvxPostureItem )
    READ( aCTLFont,     SvxFontItem )
    READ( aCTLHeight,   SvxFontHeightItem )
    READ( aCTLWeight,   SvxWeightItem )
    READ( aCTLPosture,  SvxPostureItem )
    READ( aUnderline,   SvxUnderlineItem )
    READ( aOverline,    SvxOverlineItem )
    READ( aCrossedOut,  SvxCrossedOutItem )
    READ( aContour,     SvxContourItem )
    READ( aShadowed,    SvxShadowedItem )
    READ( aColor,       SvxColorItem )

    READ( aBox,         SvxBoxItem )

    READ( aTLBR, SvxLineItem)
    READ( aBLTR, SvxLineItem)

    READ( aBackground,  SvxBrushItem )

    pNew = aAdjust.Create(rStream );
    SetAdjust( *(SvxAdjustItem*)pNew );
    delete pNew;

    if (WriterSpecificBlockExists(rStream))
    {
        READ(m_aTextOrientation, SvxFrameDirectionItem);
        READ(m_aVerticalAlignment, SwFmtVertOrient);
    }

    READ( aHorJustify,  SvxHorJustifyItem )
    READ( aVerJustify,  SvxVerJustifyItem )

    READ( aOrientation, SvxOrientationItem )
    READ( aMargin, SvxMarginItem )

    pNew = aLinebreak.Create(rStream );
    aLinebreak.SetValue( ((SfxBoolItem*)pNew)->GetValue() );
    delete pNew;

    pNew = aRotateAngle.Create( rStream );
    aRotateAngle.SetValue( ((SfxInt32Item*)pNew)->GetValue() );
    delete pNew;
    pNew = aRotateMode.Create( rStream );
    aRotateMode.SetValue( ((SvxRotateModeItem*)pNew)->GetValue() );
    delete pNew;

    sal_uInt16 eSys, eLge;
    sNumFmtString = rStream.ReadUniOrByteString( RTL_TEXTENCODING_UTF8 );
    rStream.ReadUInt16( eSys ).ReadUInt16( eLge );
    eSysLanguage = (LanguageType) eSys;
    eNumFmtLanguage = (LanguageType) eLge;

    aStacked.SetValue( aOrientation.IsStacked() );
    aRotateAngle.SetValue( aOrientation.GetRotation( aRotateAngle.GetValue() ) );

    return 0 == rStream.GetError();
}

bool SwBoxAutoFmt::Save( SvStream& rStream ) const
{
    SvxOrientationItem aOrientation( aRotateAngle.GetValue(), aStacked.GetValue(), 0 );

    aFont.Store( rStream  );
    aHeight.Store( rStream );
    aWeight.Store( rStream );
    aPosture.Store( rStream );
    aCJKFont.Store( rStream );
    aCJKHeight.Store( rStream );
    aCJKWeight.Store( rStream );
    aCJKPosture.Store( rStream );
    aCTLFont.Store( rStream  );
    aCTLHeight.Store( rStream );
    aCTLWeight.Store( rStream);
    aCTLPosture.Store( rStream );
    aUnderline.Store( rStream );
    aOverline.Store( rStream );
    aCrossedOut.Store( rStream );
    aContour.Store( rStream );
    aShadowed.Store( rStream );
    aColor.Store( rStream );
    aBox.Store( rStream );
    aTLBR.Store( rStream );
    aBLTR.Store( rStream );
    aBackground.Store( rStream );

    aAdjust.Store( rStream );

    WriterSpecificAutoFormatBlock block(rStream);
    m_aTextOrientation.Store(rStream );
    m_aVerticalAlignment.Store(rStream);

    aHorJustify.Store( rStream );
    aVerJustify.Store( rStream );
    aOrientation.Store( rStream );
    aMargin.Store( rStream );
    aLinebreak.Store( rStream );
    aRotateAngle.Store( rStream );
    aRotateMode.Store( rStream );

    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, sNumFmtString,
        RTL_TEXTENCODING_UTF8);
    rStream.WriteUInt16( (sal_uInt16)eSysLanguage ).WriteUInt16( (sal_uInt16)eNumFmtLanguage );

    return 0 == rStream.GetError();
}

SwTableAutoFmt::SwTableAutoFmt( const OUString& rName )
    : m_aName( rName )
    , nStrResId( USHRT_MAX )
    , m_aBreak( SVX_BREAK_NONE, RES_BREAK )
    , m_aKeepWithNextPara( false, RES_KEEP )
    , m_aRepeatHeading( 0 )
    , m_bLayoutSplit( true )
    , m_bRowSplit( true )
    , m_bCollapsingBorders(true)
    , m_aShadow( RES_SHADOW )
{
    bInclFont = true;
    bInclJustify = true;
    bInclFrame = true;
    bInclBackground = true;
    bInclValueFormat = true;
    bInclWidthHeight = true;

    memset( aBoxAutoFmt, 0, sizeof( aBoxAutoFmt ) );
}

SwTableAutoFmt::SwTableAutoFmt( const SwTableAutoFmt& rNew )
    : m_aBreak( rNew.m_aBreak )
    , m_aKeepWithNextPara( false, RES_KEEP )
    , m_aShadow( RES_SHADOW )
{
    for( sal_uInt8 n = 0; n < 16; ++n )
        aBoxAutoFmt[ n ] = 0;
    *this = rNew;
}

SwTableAutoFmt& SwTableAutoFmt::operator=( const SwTableAutoFmt& rNew )
{
    if (&rNew == this)
        return *this;

    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( aBoxAutoFmt[ n ] )
            delete aBoxAutoFmt[ n ];

        SwBoxAutoFmt* pFmt = rNew.aBoxAutoFmt[ n ];
        if( pFmt )      // if is set -> copy
            aBoxAutoFmt[ n ] = new SwBoxAutoFmt( *pFmt );
        else            // else default
            aBoxAutoFmt[ n ] = 0;
    }

    m_aName = rNew.m_aName;
    nStrResId = rNew.nStrResId;
    bInclFont = rNew.bInclFont;
    bInclJustify = rNew.bInclJustify;
    bInclFrame = rNew.bInclFrame;
    bInclBackground = rNew.bInclBackground;
    bInclValueFormat = rNew.bInclValueFormat;
    bInclWidthHeight = rNew.bInclWidthHeight;

    m_aBreak = rNew.m_aBreak;
    m_aPageDesc = rNew.m_aPageDesc;
    m_aKeepWithNextPara = rNew.m_aKeepWithNextPara;
    m_aRepeatHeading = rNew.m_aRepeatHeading;
    m_bLayoutSplit = rNew.m_bLayoutSplit;
    m_bRowSplit = rNew.m_bRowSplit;
    m_bCollapsingBorders = rNew.m_bCollapsingBorders;
    m_aShadow = rNew.m_aShadow;

    return *this;
}

SwTableAutoFmt::~SwTableAutoFmt()
{
    SwBoxAutoFmt** ppFmt = aBoxAutoFmt;
    for( sal_uInt8 n = 0; n < 16; ++n, ++ppFmt )
        if( *ppFmt )
            delete *ppFmt;
}

void SwTableAutoFmt::SetBoxFmt( const SwBoxAutoFmt& rNew, sal_uInt8 nPos )
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFmt* pFmt = aBoxAutoFmt[ nPos ];
    if( pFmt )      // if is set -> copy
        *aBoxAutoFmt[ nPos ] = rNew;
    else            // else set anew
        aBoxAutoFmt[ nPos ] = new SwBoxAutoFmt( rNew );
}

const SwBoxAutoFmt& SwTableAutoFmt::GetBoxFmt( sal_uInt8 nPos ) const
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFmt* pFmt = aBoxAutoFmt[ nPos ];
    if( pFmt )      // if is set -> copy
        return *pFmt;
    else            // else return the default
    {
        // If it doesn't exist yet:
        if( !pDfltBoxAutoFmt )
            pDfltBoxAutoFmt = new SwBoxAutoFmt;
        return *pDfltBoxAutoFmt;
    }
}

void SwTableAutoFmt::UpdateFromSet( sal_uInt8 nPos,
                                    const SfxItemSet& rSet,
                                    UpdateFlags eFlags,
                                    SvNumberFormatter* pNFmtr)
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFmt* pFmt = aBoxAutoFmt[ nPos ];
    if( !pFmt )     // if is set -> copy
    {
        pFmt = new SwBoxAutoFmt;
        aBoxAutoFmt[ nPos ] = pFmt;
    }

    if( UPDATE_CHAR & eFlags )
    {
        pFmt->SetFont( (SvxFontItem&)rSet.Get( RES_CHRATR_FONT ) );
        pFmt->SetHeight( (SvxFontHeightItem&)rSet.Get( RES_CHRATR_FONTSIZE ) );
        pFmt->SetWeight( (SvxWeightItem&)rSet.Get( RES_CHRATR_WEIGHT ) );
        pFmt->SetPosture( (SvxPostureItem&)rSet.Get( RES_CHRATR_POSTURE ) );
        pFmt->SetCJKFont( (SvxFontItem&)rSet.Get( RES_CHRATR_CJK_FONT ) );
        pFmt->SetCJKHeight( (SvxFontHeightItem&)rSet.Get( RES_CHRATR_CJK_FONTSIZE ) );
        pFmt->SetCJKWeight( (SvxWeightItem&)rSet.Get( RES_CHRATR_CJK_WEIGHT ) );
        pFmt->SetCJKPosture( (SvxPostureItem&)rSet.Get( RES_CHRATR_CJK_POSTURE ) );
        pFmt->SetCTLFont( (SvxFontItem&)rSet.Get( RES_CHRATR_CTL_FONT ) );
        pFmt->SetCTLHeight( (SvxFontHeightItem&)rSet.Get( RES_CHRATR_CTL_FONTSIZE ) );
        pFmt->SetCTLWeight( (SvxWeightItem&)rSet.Get( RES_CHRATR_CTL_WEIGHT ) );
        pFmt->SetCTLPosture( (SvxPostureItem&)rSet.Get( RES_CHRATR_CTL_POSTURE ) );
        pFmt->SetUnderline( (SvxUnderlineItem&)rSet.Get( RES_CHRATR_UNDERLINE ) );
        pFmt->SetOverline( (SvxOverlineItem&)rSet.Get( RES_CHRATR_OVERLINE ) );
        pFmt->SetCrossedOut( (SvxCrossedOutItem&)rSet.Get( RES_CHRATR_CROSSEDOUT ) );
        pFmt->SetContour( (SvxContourItem&)rSet.Get( RES_CHRATR_CONTOUR ) );
        pFmt->SetShadowed( (SvxShadowedItem&)rSet.Get( RES_CHRATR_SHADOWED ) );
        pFmt->SetColor( (SvxColorItem&)rSet.Get( RES_CHRATR_COLOR ) );
        pFmt->SetAdjust( (SvxAdjustItem&)rSet.Get( RES_PARATR_ADJUST ) );
    }
    if( UPDATE_BOX & eFlags )
    {
        pFmt->SetBox( (SvxBoxItem&)rSet.Get( RES_BOX ) );
// FIXME - add attribute IDs for the diagonal line items
//        pFmt->SetTLBR( (SvxLineItem&)rSet.Get( RES_... ) );
//        pFmt->SetBLTR( (SvxLineItem&)rSet.Get( RES_... ) );
        pFmt->SetBackground( (SvxBrushItem&)rSet.Get( RES_BACKGROUND ) );
        pFmt->SetTextOrientation(static_cast<const SvxFrameDirectionItem&>(rSet.Get(RES_FRAMEDIR)));
        pFmt->SetVerticalAlignment(static_cast<const SwFmtVertOrient&>(rSet.Get(RES_VERT_ORIENT)));

        const SwTblBoxNumFormat* pNumFmtItem;
        const SvNumberformat* pNumFormat = 0;
        if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMAT, true,
            (const SfxPoolItem**)&pNumFmtItem ) && pNFmtr &&
            0 != (pNumFormat = pNFmtr->GetEntry( pNumFmtItem->GetValue() )) )
            pFmt->SetValueFormat( ((SvNumberformat*)pNumFormat)->GetFormatstring(),
                                    pNumFormat->GetLanguage(),
                                    ::GetAppLanguage());
        else
        {
            // default
            pFmt->SetValueFormat( OUString(), LANGUAGE_SYSTEM,
                                  ::GetAppLanguage() );
        }
    }

    // we cannot handle the rest, that's specific to StarCalc
}

void SwTableAutoFmt::UpdateToSet(sal_uInt8 nPos, SfxItemSet& rSet,
                                 UpdateFlags eFlags, SvNumberFormatter* pNFmtr) const
{
    const SwBoxAutoFmt& rChg = GetBoxFmt( nPos );

    if( UPDATE_CHAR & eFlags )
    {
        if( IsFont() )
        {
            rSet.Put( rChg.GetFont() );
            rSet.Put( rChg.GetHeight() );
            rSet.Put( rChg.GetWeight() );
            rSet.Put( rChg.GetPosture() );
            // do not insert empty CJK font
            const SvxFontItem& rCJKFont = rChg.GetCJKFont();
            if (!rCJKFont.GetStyleName().isEmpty())
            {
                rSet.Put( rChg.GetCJKFont() );
                rSet.Put( rChg.GetCJKHeight() );
                rSet.Put( rChg.GetCJKWeight() );
                rSet.Put( rChg.GetCJKPosture() );
            }
            else
            {
                rSet.Put( rChg.GetHeight(), RES_CHRATR_CJK_FONTSIZE );
                rSet.Put( rChg.GetWeight(), RES_CHRATR_CJK_WEIGHT );
                rSet.Put( rChg.GetPosture(), RES_CHRATR_CJK_POSTURE );
            }
            // do not insert empty CTL font
            const SvxFontItem& rCTLFont = rChg.GetCTLFont();
            if (!rCTLFont.GetStyleName().isEmpty())
            {
                rSet.Put( rChg.GetCTLFont() );
                rSet.Put( rChg.GetCTLHeight() );
                rSet.Put( rChg.GetCTLWeight() );
                rSet.Put( rChg.GetCTLPosture() );
            }
            else
            {
                rSet.Put( rChg.GetHeight(), RES_CHRATR_CTL_FONTSIZE );
                rSet.Put( rChg.GetWeight(), RES_CHRATR_CTL_WEIGHT );
                rSet.Put( rChg.GetPosture(), RES_CHRATR_CTL_POSTURE );
            }
            rSet.Put( rChg.GetUnderline() );
            rSet.Put( rChg.GetOverline() );
            rSet.Put( rChg.GetCrossedOut() );
            rSet.Put( rChg.GetContour() );
            rSet.Put( rChg.GetShadowed() );
            rSet.Put( rChg.GetColor() );
        }
        if( IsJustify() )
            rSet.Put( rChg.GetAdjust() );
    }

    if( UPDATE_BOX & eFlags )
    {
        if( IsFrame() )
        {
            rSet.Put( rChg.GetBox() );
// FIXME - uncomment the lines to put the diagonal line items
//            rSet.Put( rChg.GetTLBR() );
//            rSet.Put( rChg.GetBLTR() );
        }
        if( IsBackground() )
            rSet.Put( rChg.GetBackground() );

        rSet.Put(rChg.GetTextOrientation());
        rSet.Put(rChg.GetVerticalAlignment());

        if( IsValueFormat() && pNFmtr )
        {
            OUString sFmt;
            LanguageType eLng, eSys;
            rChg.GetValueFormat( sFmt, eLng, eSys );
            if( !sFmt.isEmpty() )
            {
                short nType;
                bool bNew;
                sal_Int32 nCheckPos;
                sal_uInt32 nKey = pNFmtr->GetIndexPuttingAndConverting( sFmt, eLng,
                                                                        eSys, nType, bNew, nCheckPos);
                rSet.Put( SwTblBoxNumFormat( nKey ));
            }
            else
                rSet.ClearItem( RES_BOXATR_FORMAT );
        }
    }

    // we cannot handle the rest, that's specific to StarCalc
}

void SwTableAutoFmt::RestoreTableProperties(SwTable &table) const
{
    SwTableFmt *pFormat = table.GetTableFmt();
    if (!pFormat)
        return;

    SwDoc *pDoc = pFormat->GetDoc();
    if (!pDoc)
        return;

    SfxItemSet rSet(pDoc->GetAttrPool(), aTableSetRange);

    rSet.Put(m_aBreak);
    rSet.Put(m_aPageDesc);
    rSet.Put(SwFmtLayoutSplit(m_bLayoutSplit));
    rSet.Put(SfxBoolItem(RES_COLLAPSING_BORDERS, m_bCollapsingBorders));
    rSet.Put(m_aKeepWithNextPara);
    rSet.Put(m_aShadow);

    pFormat->SetFmtAttr(rSet);

    SwEditShell *pShell = pDoc->GetEditShell();
    pDoc->SetRowSplit(*pShell->getShellCrsr(false), SwFmtRowSplit(m_bRowSplit));

    table.SetRowsToRepeat(m_aRepeatHeading);
}

void SwTableAutoFmt::StoreTableProperties(const SwTable &table)
{
    SwTableFmt *pFormat = table.GetTableFmt();
    if (!pFormat)
        return;

    SwDoc *pDoc = pFormat->GetDoc();
    if (!pDoc)
        return;

    SwEditShell *pShell = pDoc->GetEditShell();
    SwFmtRowSplit *pRowSplit = 0;
    pDoc->GetRowSplit(*pShell->getShellCrsr(false), pRowSplit);
    m_bRowSplit = pRowSplit ? pRowSplit->GetValue() : sal_False;
    delete pRowSplit;
    pRowSplit = 0;

    const SfxItemSet &rSet = pFormat->GetAttrSet();

    m_aBreak = static_cast<const SvxFmtBreakItem&>(rSet.Get(RES_BREAK));
    m_aPageDesc = static_cast<const SwFmtPageDesc&>(rSet.Get(RES_PAGEDESC));
    const SwFmtLayoutSplit &layoutSplit = static_cast<const SwFmtLayoutSplit&>(rSet.Get(RES_LAYOUT_SPLIT));
    m_bLayoutSplit = layoutSplit.GetValue();
    m_bCollapsingBorders = static_cast<const SfxBoolItem&>(rSet.Get(RES_COLLAPSING_BORDERS)).GetValue();

    m_aKeepWithNextPara = static_cast<const SvxFmtKeepItem&>(rSet.Get(RES_KEEP));
    m_aRepeatHeading = table.GetRowsToRepeat();
    m_aShadow = static_cast<const SvxShadowItem&>(rSet.Get(RES_SHADOW));
}

bool SwTableAutoFmt::Load( SvStream& rStream )
{
    bool bRet = 0 == rStream.GetError();

    if( bRet )
    {
        bool b;
        m_aName = rStream.ReadUniOrByteString( RTL_TEXTENCODING_UTF8 );
        rStream.ReadUInt16( nStrResId );
        sal_uInt16 nId = RID_SVXSTR_TBLAFMT_BEGIN + nStrResId;
        if( RID_SVXSTR_TBLAFMT_BEGIN <= nId &&
            nId < RID_SVXSTR_TBLAFMT_END )
        {
            m_aName = SVX_RESSTR( nId );
        }
        else
            nStrResId = USHRT_MAX;
        rStream.ReadCharAsBool( b ); bInclFont = b;
        rStream.ReadCharAsBool( b ); bInclJustify = b;
        rStream.ReadCharAsBool( b ); bInclFrame = b;
        rStream.ReadCharAsBool( b ); bInclBackground = b;
        rStream.ReadCharAsBool( b ); bInclValueFormat = b;
        rStream.ReadCharAsBool( b ); bInclWidthHeight = b;

        if (WriterSpecificBlockExists(rStream))
        {
            SfxPoolItem* pNew = 0;

            READ(m_aBreak, SvxFmtBreakItem);
            READ(m_aPageDesc, SwFmtPageDesc);
            READ(m_aKeepWithNextPara, SvxFmtKeepItem);

            rStream.ReadUInt16( m_aRepeatHeading ).ReadCharAsBool( m_bLayoutSplit ).ReadCharAsBool( m_bRowSplit ).ReadCharAsBool( m_bCollapsingBorders );

            READ(m_aShadow, SvxShadowItem);
        }

        bRet = 0 == rStream.GetError();

        for( sal_uInt8 i = 0; bRet && i < 16; ++i )
        {
            SwBoxAutoFmt* pFmt = new SwBoxAutoFmt;
            bRet = pFmt->Load( rStream );
            if( bRet )
                aBoxAutoFmt[ i ] = pFmt;
            else
            {
                delete pFmt;
                break;
            }
        }
    }
    return bRet;
}

bool SwTableAutoFmt::Save( SvStream& rStream ) const
{
    bool b;
    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, m_aName,
        RTL_TEXTENCODING_UTF8 );
    rStream.WriteUInt16( nStrResId );
    rStream.WriteUChar( ( b = bInclFont ) );
    rStream.WriteUChar( ( b = bInclJustify ) );
    rStream.WriteUChar( ( b = bInclFrame ) );
    rStream.WriteUChar( ( b = bInclBackground ) );
    rStream.WriteUChar( ( b = bInclValueFormat ) );
    rStream.WriteUChar( ( b = bInclWidthHeight ) );

    {
        WriterSpecificAutoFormatBlock block(rStream);

        m_aBreak.Store(rStream);
        m_aPageDesc.Store(rStream);
        m_aKeepWithNextPara.Store(rStream);
        rStream.WriteUInt16( m_aRepeatHeading ).WriteUChar( m_bLayoutSplit ).WriteUChar( m_bRowSplit ).WriteUChar( m_bCollapsingBorders );
        m_aShadow.Store(rStream);
    }

    bool bRet = 0 == rStream.GetError();

    for( int i = 0; bRet && i < 16; ++i )
    {
        SwBoxAutoFmt* pFmt = aBoxAutoFmt[ i ];
        if( !pFmt )     // if not set -> write default
        {
            // If it doesn't exist yet:
            if( !pDfltBoxAutoFmt )
                pDfltBoxAutoFmt = new SwBoxAutoFmt;
            pFmt = pDfltBoxAutoFmt;
        }
        bRet = pFmt->Save( rStream );
    }
    return bRet;
}

struct SwTableAutoFmtTbl::Impl
{
    boost::ptr_vector<SwTableAutoFmt> m_AutoFormats;
};

size_t SwTableAutoFmtTbl::size() const
{
    return m_pImpl->m_AutoFormats.size();
}

SwTableAutoFmt const& SwTableAutoFmtTbl::operator[](size_t const i) const
{
    return m_pImpl->m_AutoFormats[i];
}
SwTableAutoFmt      & SwTableAutoFmtTbl::operator[](size_t const i)
{
    return m_pImpl->m_AutoFormats[i];
}

void
SwTableAutoFmtTbl::InsertAutoFmt(size_t const i, SwTableAutoFmt *const pFmt)
{
    m_pImpl->m_AutoFormats.insert(m_pImpl->m_AutoFormats.begin() + i, pFmt);
}

void SwTableAutoFmtTbl::EraseAutoFmt(size_t const i)
{
    m_pImpl->m_AutoFormats.erase(m_pImpl->m_AutoFormats.begin() + i);
}

SwTableAutoFmt* SwTableAutoFmtTbl::ReleaseAutoFmt(size_t const i)
{
    return m_pImpl->m_AutoFormats.release(m_pImpl->m_AutoFormats.begin() + i).release();
}

SwTableAutoFmtTbl::~SwTableAutoFmtTbl()
{
}

SwTableAutoFmtTbl::SwTableAutoFmtTbl()
    : m_pImpl(new Impl)
{
    OUString sNm;
    SwTableAutoFmt* pNew = new SwTableAutoFmt(
                            SwStyleNameMapper::GetUIName( RES_POOLCOLL_STANDARD, sNm ) );

    SwBoxAutoFmt aNew;

    sal_uInt8 i;

    Color aColor( COL_BLUE );
    SvxBrushItem aBrushItem( aColor, RES_BACKGROUND );
    aNew.SetBackground( aBrushItem );
    aNew.SetColor( SvxColorItem(Color( COL_WHITE ), RES_CHRATR_COLOR) );

    for( i = 0; i < 4; ++i )
        pNew->SetBoxFmt( aNew, i );

    // 70% gray
    aBrushItem.SetColor( RGB_COLORDATA( 0x4d, 0x4d, 0x4d ) );
    aNew.SetBackground( aBrushItem );
    for( i = 4; i <= 12; i += 4 )
        pNew->SetBoxFmt( aNew, i );

    // 20% gray
    aBrushItem.SetColor( RGB_COLORDATA( 0xcc, 0xcc, 0xcc ) );
    aNew.SetBackground( aBrushItem );
    aColor.SetColor( COL_BLACK );
    aNew.SetColor( SvxColorItem( aColor, RES_CHRATR_COLOR) );
    for( i = 7; i <= 15; i += 4 )
        pNew->SetBoxFmt( aNew, i );
    for( i = 13; i <= 14; ++i )
        pNew->SetBoxFmt( aNew, i );

    aBrushItem.SetColor( Color( COL_WHITE ) );
    aNew.SetBackground( aBrushItem );
    for( i = 5; i <= 6; ++i )
        pNew->SetBoxFmt( aNew, i );
    for( i = 9; i <= 10; ++i )
        pNew->SetBoxFmt( aNew, i );

    SvxBoxItem aBox( RES_BOX );
    aBox.SetDistance( 55 );
    SvxBorderLine aLn( &aColor, DEF_LINE_WIDTH_0 );
    aBox.SetLine( &aLn, BOX_LINE_LEFT );
    aBox.SetLine( &aLn, BOX_LINE_BOTTOM );

    for( i = 0; i <= 15; ++i )
    {
        aBox.SetLine( i <= 3 ? &aLn : 0, BOX_LINE_TOP );
        aBox.SetLine( (3 == ( i & 3 )) ? &aLn : 0, BOX_LINE_RIGHT );
        ((SwBoxAutoFmt&)pNew->GetBoxFmt( i )).SetBox( aBox );
    }

    m_pImpl->m_AutoFormats.push_back(pNew);
}

bool SwTableAutoFmtTbl::Load()
{
    bool bRet = false;
    OUString sNm(AUTOTABLE_FORMAT_NAME);
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm, SvtPathOptions::PATH_USERCONFIG ))
    {
        SfxMedium aStream( sNm, STREAM_STD_READ );
        bRet = Load( *aStream.GetInStream() );
    }
    else
        bRet = false;
    return bRet;
}

bool SwTableAutoFmtTbl::Save() const
{
    SvtPathOptions aPathOpt;
    const OUString sNm( aPathOpt.GetUserConfigPath() + "/" AUTOTABLE_FORMAT_NAME );
    SfxMedium aStream(sNm, STREAM_STD_WRITE );
    return Save( *aStream.GetOutStream() ) && aStream.Commit();
}

bool SwTableAutoFmtTbl::Load( SvStream& rStream )
{
    bool bRet = 0 == rStream.GetError();
    if (bRet)
    {
        // Attention: We need to read a general Header here
        sal_uInt16 nVal = 0;
        rStream.ReadUInt16( nVal );
        bRet = 0 == rStream.GetError();

        if( bRet )
        {
            sal_uInt8 nChrSet, nCnt;
            long nPos = rStream.Tell();
            rStream.ReadUChar( nCnt ).ReadUChar( nChrSet );
            if( rStream.Tell() != sal_uLong(nPos + nCnt) )
            {
                OSL_ENSURE( false, "The Header contains more or newer Data" );
                rStream.Seek( nPos + nCnt );
            }
            rStream.SetStreamCharSet( (rtl_TextEncoding)nChrSet );
            rStream.SetVersion( SOFFICE_FILEFORMAT_50 );

            SwTableAutoFmt* pNew;
            sal_uInt16 nAnz = 0;
            rStream.ReadUInt16( nAnz );

            bRet = 0 == rStream.GetError();

            for( sal_uInt16 i = 0; i < nAnz; ++i )
            {
                pNew = new SwTableAutoFmt( OUString() );
                bRet = pNew->Load( rStream );
                if( bRet )
                {
                    m_pImpl->m_AutoFormats.push_back(pNew);
                }
                else
                {
                    delete pNew;
                    break;
                }
            }
        }
    }
    return bRet;
}

bool SwTableAutoFmtTbl::Save( SvStream& rStream ) const
{
    bool bRet = 0 == rStream.GetError();
    if (bRet)
    {
        rStream.SetVersion(SOFFICE_FILEFORMAT_50);

        // Attention: We need to save a general Header here
        rStream.WriteUChar( (sal_uInt8)2 ) // Character count of the Header including this value
               .WriteUChar( (sal_uInt8)GetStoreCharSet( ::osl_getThreadTextEncoding() ) );

        bRet = 0 == rStream.GetError();
        if (!bRet)
            return false;

        rStream.WriteUInt16( static_cast<sal_uInt16>(m_pImpl->m_AutoFormats.size() - 1) );
        bRet = 0 == rStream.GetError();

        for (sal_uInt16 i = 1; bRet && i < m_pImpl->m_AutoFormats.size(); ++i)
        {
            SwTableAutoFmt const& rFmt = m_pImpl->m_AutoFormats[i];
            bRet = rFmt.Save(rStream);
        }
    }
    rStream.Flush();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
