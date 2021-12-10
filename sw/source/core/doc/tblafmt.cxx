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

#include <comphelper/fileformat.h>
#include <tools/stream.hxx>
#include <sfx2/docfile.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>
#include <svl/zformat.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/pathoptions.hxx>
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
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>

#include <editeng/adjustitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/legacyitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svx/algitem.hxx>
#include <svx/rotmodit.hxx>
#include <legacyitem.hxx>

#include <memory>
#include <vector>

/*
 * XXX: BIG RED NOTICE! Changes MUST be binary file format compatible and MUST
 * be synchronized with Calc's ScAutoFormat sc/source/core/tool/autoform.cxx
 */

using ::editeng::SvxBorderLine;

// until SO5PF
const sal_uInt16 AUTOFORMAT_ID_X        = 9501;
const sal_uInt16 AUTOFORMAT_ID_358      = 9601;
const sal_uInt16 AUTOFORMAT_DATA_ID_X   = 9502;

// from SO5
//! In follow-up versions these IDs' values need to increase
const sal_uInt16 AUTOFORMAT_ID_504      = 9801;
const sal_uInt16 AUTOFORMAT_DATA_ID_504 = 9802;

const sal_uInt16 AUTOFORMAT_DATA_ID_552 = 9902;

// --- from 680/dr25 on: store strings as UTF-8
const sal_uInt16 AUTOFORMAT_ID_680DR25      = 10021;

// --- Bug fix to fdo#31005: Table Autoformats does not save/apply all properties (Writer and Calc)
const sal_uInt16 AUTOFORMAT_ID_31005      = 10041;
const sal_uInt16 AUTOFORMAT_DATA_ID_31005 = 10042;

// current version
const sal_uInt16 AUTOFORMAT_ID          = AUTOFORMAT_ID_31005;
const sal_uInt16 AUTOFORMAT_DATA_ID     = AUTOFORMAT_DATA_ID_31005;
const sal_uInt16 AUTOFORMAT_FILE_VERSION= SOFFICE_FILEFORMAT_50;

SwBoxAutoFormat* SwTableAutoFormat::s_pDefaultBoxAutoFormat = nullptr;

constexpr OUStringLiteral AUTOTABLE_FORMAT_NAME = u"autotbl.fmt";

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

        rStream.WriteUInt64( 0 ); // endOfSwBlock

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
    class WriterSpecificAutoFormatBlock
    {
    public:
        explicit WriterSpecificAutoFormatBlock(SvStream& rStream)
            : mrStream(rStream)
            , mnWhereToWriteEndOfBlock(BeginSwBlock(rStream))
        {
        }

        ~WriterSpecificAutoFormatBlock() { EndSwBlock(mrStream, mnWhereToWriteEndOfBlock); }

    private:
        WriterSpecificAutoFormatBlock(WriterSpecificAutoFormatBlock const&) = delete;
        WriterSpecificAutoFormatBlock& operator=(WriterSpecificAutoFormatBlock const&) = delete;

        SvStream& mrStream;
        sal_uInt64 mnWhereToWriteEndOfBlock;
    };

    /// Checks whether a writer-specific block exists (i.e. size is not zero)
    sal_Int64 WriterSpecificBlockExists(SvStream &stream)
    {
        sal_uInt64 endOfSwBlock = 0;
        stream.ReadUInt64( endOfSwBlock );

        // end-of-block pointing to itself indicates a zero-size block.
        return endOfSwBlock - stream.Tell();
    }
}

// Struct with version numbers of the Items

struct SwAfVersions : public AutoFormatVersions
{
public:
    sal_uInt16 m_nTextOrientationVersion;
    sal_uInt16 m_nVerticalAlignmentVersion;

    SwAfVersions();
    void Load( SvStream& rStream, sal_uInt16 nVer );
    static void Write(SvStream& rStream, sal_uInt16 fileVersion);
};

SwAfVersions::SwAfVersions()
:   m_nTextOrientationVersion(0),
    m_nVerticalAlignmentVersion(0)
{
}

void SwAfVersions::Load( SvStream& rStream, sal_uInt16 nVer )
{
    LoadBlockA(rStream, nVer);
    if (nVer >= AUTOFORMAT_ID_31005 && WriterSpecificBlockExists(rStream))
    {
        rStream.ReadUInt16( m_nTextOrientationVersion );
        rStream.ReadUInt16( m_nVerticalAlignmentVersion );
    }
    LoadBlockB(rStream, nVer);
}

void SwAfVersions::Write(SvStream& rStream, sal_uInt16 fileVersion)
{
    AutoFormatVersions::WriteBlockA(rStream, fileVersion);

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriterSpecificAutoFormatBlock block(rStream);

        rStream.WriteUInt16(legacy::SvxFrameDirection::GetVersion(fileVersion));
        rStream.WriteUInt16(legacy::SwFormatVert::GetVersion(fileVersion));
    }

    AutoFormatVersions::WriteBlockB(rStream, fileVersion);
}



SwBoxAutoFormat::SwBoxAutoFormat()
:   m_aTextOrientation(std::make_unique<SvxFrameDirectionItem>(SvxFrameDirection::Environment, RES_FRAMEDIR)),
    m_aVerticalAlignment(std::make_unique<SwFormatVertOrient>(0, css::text::VertOrientation::NONE, css::text::RelOrientation::FRAME)),
    m_eSysLanguage(::GetAppLanguage()),
    m_eNumFormatLanguage(::GetAppLanguage())
{
    // need to set default instances for base class AutoFormatBase here
    // due to resource defines (e.g. RES_CHRATR_FONT) which are not available
    // in svx and different in the different usages of derivations
    m_aFont = std::make_unique<SvxFontItem>(*GetDfltAttr( RES_CHRATR_FONT ) );
    m_aHeight = std::make_unique<SvxFontHeightItem>(240, 100, RES_CHRATR_FONTSIZE );
    m_aWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, RES_CHRATR_WEIGHT );
    m_aPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, RES_CHRATR_POSTURE );
    m_aCJKFont = std::make_unique<SvxFontItem>(*GetDfltAttr( RES_CHRATR_CJK_FONT ) );
    m_aCJKHeight = std::make_unique<SvxFontHeightItem>(240, 100, RES_CHRATR_CJK_FONTSIZE );
    m_aCJKWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, RES_CHRATR_CJK_WEIGHT );
    m_aCJKPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, RES_CHRATR_CJK_POSTURE );
    m_aCTLFont = std::make_unique<SvxFontItem>(*GetDfltAttr( RES_CHRATR_CTL_FONT ) );
    m_aCTLHeight = std::make_unique<SvxFontHeightItem>(240, 100, RES_CHRATR_CTL_FONTSIZE );
    m_aCTLWeight = std::make_unique<SvxWeightItem>(WEIGHT_NORMAL, RES_CHRATR_CTL_WEIGHT );
    m_aCTLPosture = std::make_unique<SvxPostureItem>(ITALIC_NONE, RES_CHRATR_CTL_POSTURE );
    m_aUnderline = std::make_unique<SvxUnderlineItem>(LINESTYLE_NONE, RES_CHRATR_UNDERLINE );
    m_aOverline = std::make_unique<SvxOverlineItem>(LINESTYLE_NONE, RES_CHRATR_OVERLINE );
    m_aCrossedOut = std::make_unique<SvxCrossedOutItem>(STRIKEOUT_NONE, RES_CHRATR_CROSSEDOUT );
    m_aContour = std::make_unique<SvxContourItem>(false, RES_CHRATR_CONTOUR );
    m_aShadowed = std::make_unique<SvxShadowedItem>(false, RES_CHRATR_SHADOWED );
    m_aColor = std::make_unique<SvxColorItem>(RES_CHRATR_COLOR );
    m_aBox = std::make_unique<SvxBoxItem>(RES_BOX );
    m_aTLBR = std::make_unique<SvxLineItem>(0 );
    m_aBLTR = std::make_unique<SvxLineItem>(0 );
    m_aBackground = std::make_unique<SvxBrushItem>(RES_BACKGROUND );
    m_aAdjust = std::make_unique<SvxAdjustItem>(SvxAdjust::Left, RES_PARATR_ADJUST );
    m_aHorJustify = std::make_unique<SvxHorJustifyItem>(SvxCellHorJustify::Standard, 0);
    m_aVerJustify = std::make_unique<SvxVerJustifyItem>(SvxCellVerJustify::Standard, 0);
    m_aStacked = std::make_unique<SfxBoolItem>(0 );
    m_aMargin = std::make_unique<SvxMarginItem>(0 );
    m_aLinebreak = std::make_unique<SfxBoolItem>(0 );
    m_aRotateAngle = std::make_unique<SfxInt32Item>(0 );
    m_aRotateMode = std::make_unique<SvxRotateModeItem>(SVX_ROTATE_MODE_STANDARD, 0 );

// FIXME - add attribute IDs for the diagonal line items
//    aTLBR( RES_... ),
//    aBLTR( RES_... ),
    m_aBox->SetAllDistances(55);
}

SwBoxAutoFormat::SwBoxAutoFormat( const SwBoxAutoFormat& rNew )
:   AutoFormatBase(rNew),
    m_aTextOrientation(rNew.m_aTextOrientation->Clone()),
    m_aVerticalAlignment(rNew.m_aVerticalAlignment->Clone()),
    m_sNumFormatString( rNew.m_sNumFormatString ),
    m_eSysLanguage( rNew.m_eSysLanguage ),
    m_eNumFormatLanguage( rNew.m_eNumFormatLanguage )
{
}

SwBoxAutoFormat::~SwBoxAutoFormat()
{
}

SwBoxAutoFormat& SwBoxAutoFormat::operator=(const SwBoxAutoFormat& rRef)
{
    // check self-assignment
    if(this == &rRef)
    {
        return *this;
    }

    // call baseclass implementation
    AutoFormatBase::operator=(rRef);

    // copy local members - this will use ::Clone() on all involved Items
    SetTextOrientation(rRef.GetTextOrientation());
    SetVerticalAlignment(rRef.GetVerticalAlignment());
    SetNumFormatString(rRef.GetNumFormatString());
    SetSysLanguage(rRef.GetSysLanguage());
    SetNumFormatLanguage(rRef.GetNumFormatLanguage());

    // m_wXObject used to not be copied before 1e2682235cded9a7cd90e55f0bfc60a1285e9a46
    // "WIP: Further preparations for deeper Item changes" by this operator, so do not do it now, too
    // rRef.SetXObject(GetXObject());

    return *this;
}

bool SwBoxAutoFormat::operator==(const SwBoxAutoFormat& rRight) const
{
    return GetBackground().GetColor() == rRight.GetBackground().GetColor();
}

bool SwBoxAutoFormat::Load( SvStream& rStream, const SwAfVersions& rVersions, sal_uInt16 nVer )
{
    LoadBlockA( rStream, rVersions, nVer );

    if (nVer >= AUTOFORMAT_DATA_ID_31005)
    {
        sal_Int64 const nSize(WriterSpecificBlockExists(rStream));
        if (0 < nSize && nSize < std::numeric_limits<sal_uInt16>::max())
        {
            legacy::SvxFrameDirection::Create(*m_aTextOrientation, rStream, rVersions.m_nTextOrientationVersion);
            // HORRIBLE HACK to read both 32-bit and 64-bit "long": abuse nSize
            legacy::SwFormatVert::Create(*m_aVerticalAlignment, rStream, /*rVersions.m_nVerticalAlignmentVersion*/ nSize);
        }
    }

    LoadBlockB( rStream, rVersions, nVer );

    if( 0 == rVersions.nNumFormatVersion )
    {
        sal_uInt16 eSys, eLge;
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVer >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        m_sNumFormatString = rStream.ReadUniOrByteString( eCharSet );
        rStream.ReadUInt16( eSys ).ReadUInt16( eLge );
        m_eSysLanguage = LanguageType(eSys);
        m_eNumFormatLanguage = LanguageType(eLge);
        if ( m_eSysLanguage == LANGUAGE_SYSTEM )      // from old versions (Calc)
            m_eSysLanguage = ::GetAppLanguage();
    }

    return ERRCODE_NONE == rStream.GetError();
}

bool SwBoxAutoFormat::Save( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    SaveBlockA( rStream, fileVersion );

    if (fileVersion >= SOFFICE_FILEFORMAT_50)
    {
        WriterSpecificAutoFormatBlock block(rStream);

        legacy::SvxFrameDirection::Store(*m_aTextOrientation, rStream, legacy::SvxFrameDirection::GetVersion(fileVersion));
        legacy::SwFormatVert::Store(*m_aVerticalAlignment, rStream, legacy::SwFormatVert::GetVersion(fileVersion));
    }

    SaveBlockB( rStream, fileVersion );

    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, m_sNumFormatString,
        RTL_TEXTENCODING_UTF8);
    rStream.WriteUInt16( static_cast<sal_uInt16>(m_eSysLanguage) ).WriteUInt16( static_cast<sal_uInt16>(m_eNumFormatLanguage) );

    return ERRCODE_NONE == rStream.GetError();
}

SwTableAutoFormat::SwTableAutoFormat( const OUString& rName )
    : m_aName( rName )
    , m_nStrResId( USHRT_MAX )
    , m_aKeepWithNextPara(std::make_shared<SvxFormatKeepItem>(false, RES_KEEP))
    , m_aRepeatHeading( 0 )
    , m_bLayoutSplit( true )
    , m_bRowSplit( true )
    , m_bCollapsingBorders(true)
    , m_aShadow(std::make_shared<SvxShadowItem>(RES_SHADOW))
    , m_bHidden( false )
    , m_bUserDefined( true )
{
    m_bInclFont = true;
    m_bInclJustify = true;
    m_bInclFrame = true;
    m_bInclBackground = true;
    m_bInclValueFormat = true;
    m_bInclWidthHeight = true;
}

SwTableAutoFormat::SwTableAutoFormat( const SwTableAutoFormat& rNew )
    : m_aShadow(std::make_shared<SvxShadowItem>(RES_SHADOW))
{
    for(SwBoxAutoFormat* & rp : m_aBoxAutoFormat)
        rp = nullptr;
    *this = rNew;
}

SwTableAutoFormat& SwTableAutoFormat::operator=( const SwTableAutoFormat& rNew )
{
    if (&rNew == this)
        return *this;

    for( sal_uInt8 n = 0; n < 16; ++n )
    {
        if( m_aBoxAutoFormat[ n ] )
            delete m_aBoxAutoFormat[ n ];

        SwBoxAutoFormat* pFormat = rNew.m_aBoxAutoFormat[ n ];
        if( pFormat )      // if is set -> copy
            m_aBoxAutoFormat[ n ] = new SwBoxAutoFormat( *pFormat );
        else            // else default
            m_aBoxAutoFormat[ n ] = nullptr;
    }

    m_aName = rNew.m_aName;
    m_nStrResId = rNew.m_nStrResId;
    m_bInclFont = rNew.m_bInclFont;
    m_bInclJustify = rNew.m_bInclJustify;
    m_bInclFrame = rNew.m_bInclFrame;
    m_bInclBackground = rNew.m_bInclBackground;
    m_bInclValueFormat = rNew.m_bInclValueFormat;
    m_bInclWidthHeight = rNew.m_bInclWidthHeight;

    m_aKeepWithNextPara.reset(rNew.m_aKeepWithNextPara->Clone());
    m_aRepeatHeading = rNew.m_aRepeatHeading;
    m_bLayoutSplit = rNew.m_bLayoutSplit;
    m_bRowSplit = rNew.m_bRowSplit;
    m_bCollapsingBorders = rNew.m_bCollapsingBorders;
    m_aShadow.reset(rNew.m_aShadow->Clone());
    m_bHidden = rNew.m_bHidden;
    m_bUserDefined = rNew.m_bUserDefined;

    return *this;
}

SwTableAutoFormat::~SwTableAutoFormat()
{
    SwBoxAutoFormat** ppFormat = m_aBoxAutoFormat;
    for( sal_uInt8 n = 0; n < 16; ++n, ++ppFormat )
        if( *ppFormat )
            delete *ppFormat;
}

void SwTableAutoFormat::SetBoxFormat( const SwBoxAutoFormat& rNew, sal_uInt8 nPos )
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFormat* pFormat = m_aBoxAutoFormat[ nPos ];
    if( pFormat )      // if is set -> copy
        *m_aBoxAutoFormat[ nPos ] = rNew;
    else            // else set anew
        m_aBoxAutoFormat[ nPos ] = new SwBoxAutoFormat( rNew );
}

const SwBoxAutoFormat& SwTableAutoFormat::GetBoxFormat( sal_uInt8 nPos ) const
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFormat* pFormat = m_aBoxAutoFormat[ nPos ];
    if( pFormat )      // if is set -> copy
        return *pFormat;
    else            // else return the default
    {
        // If it doesn't exist yet:
        if( !s_pDefaultBoxAutoFormat )
            s_pDefaultBoxAutoFormat = new SwBoxAutoFormat;
        return *s_pDefaultBoxAutoFormat;
    }
}

SwBoxAutoFormat& SwTableAutoFormat::GetBoxFormat( sal_uInt8 nPos )
{
    SAL_WARN_IF(!(nPos < 16), "sw.core", "GetBoxFormat wrong area");

    SwBoxAutoFormat** pFormat = &m_aBoxAutoFormat[ nPos ];
    if( !*pFormat )
    {
        // If default doesn't exist yet:
        if( !s_pDefaultBoxAutoFormat )
            s_pDefaultBoxAutoFormat = new SwBoxAutoFormat();
        *pFormat = new SwBoxAutoFormat(*s_pDefaultBoxAutoFormat);
    }
    return **pFormat;
}

const SwBoxAutoFormat& SwTableAutoFormat::GetDefaultBoxFormat()
{
    if(!s_pDefaultBoxAutoFormat)
        s_pDefaultBoxAutoFormat = new SwBoxAutoFormat();

    return *s_pDefaultBoxAutoFormat;
}

void SwTableAutoFormat::UpdateFromSet( sal_uInt8 nPos,
                                    const SfxItemSet& rSet,
                                    SwTableAutoFormatUpdateFlags eFlags,
                                    SvNumberFormatter const * pNFormatr)
{
    OSL_ENSURE( nPos < 16, "wrong area" );

    SwBoxAutoFormat* pFormat = m_aBoxAutoFormat[ nPos ];
    if( !pFormat )     // if is set -> copy
    {
        pFormat = new SwBoxAutoFormat;
        m_aBoxAutoFormat[ nPos ] = pFormat;
    }

    if( SwTableAutoFormatUpdateFlags::Char & eFlags )
    {
        pFormat->SetFont( rSet.Get( RES_CHRATR_FONT ) );
        pFormat->SetHeight( rSet.Get( RES_CHRATR_FONTSIZE ) );
        pFormat->SetWeight( rSet.Get( RES_CHRATR_WEIGHT ) );
        pFormat->SetPosture( rSet.Get( RES_CHRATR_POSTURE ) );
        pFormat->SetCJKFont( rSet.Get( RES_CHRATR_CJK_FONT ) );
        pFormat->SetCJKHeight( rSet.Get( RES_CHRATR_CJK_FONTSIZE ) );
        pFormat->SetCJKWeight( rSet.Get( RES_CHRATR_CJK_WEIGHT ) );
        pFormat->SetCJKPosture( rSet.Get( RES_CHRATR_CJK_POSTURE ) );
        pFormat->SetCTLFont( rSet.Get( RES_CHRATR_CTL_FONT ) );
        pFormat->SetCTLHeight( rSet.Get( RES_CHRATR_CTL_FONTSIZE ) );
        pFormat->SetCTLWeight( rSet.Get( RES_CHRATR_CTL_WEIGHT ) );
        pFormat->SetCTLPosture( rSet.Get( RES_CHRATR_CTL_POSTURE ) );
        pFormat->SetUnderline( rSet.Get( RES_CHRATR_UNDERLINE ) );
        pFormat->SetOverline( rSet.Get( RES_CHRATR_OVERLINE ) );
        pFormat->SetCrossedOut( rSet.Get( RES_CHRATR_CROSSEDOUT ) );
        pFormat->SetContour( rSet.Get( RES_CHRATR_CONTOUR ) );
        pFormat->SetShadowed( rSet.Get( RES_CHRATR_SHADOWED ) );
        pFormat->SetColor( rSet.Get( RES_CHRATR_COLOR ) );
        pFormat->SetAdjust( rSet.Get( RES_PARATR_ADJUST ) );
    }
    if( !(SwTableAutoFormatUpdateFlags::Box & eFlags) )
        return;

    pFormat->SetBox( rSet.Get( RES_BOX ) );
// FIXME - add attribute IDs for the diagonal line items
//        pFormat->SetTLBR( (SvxLineItem&)rSet.Get( RES_... ) );
//        pFormat->SetBLTR( (SvxLineItem&)rSet.Get( RES_... ) );
    pFormat->SetBackground( rSet.Get( RES_BACKGROUND ) );
    pFormat->SetTextOrientation(rSet.Get(RES_FRAMEDIR));
    pFormat->SetVerticalAlignment(rSet.Get(RES_VERT_ORIENT));

    const SwTableBoxNumFormat* pNumFormatItem;
    const SvNumberformat* pNumFormat = nullptr;
    if( SfxItemState::SET == rSet.GetItemState( RES_BOXATR_FORMAT, true,
        reinterpret_cast<const SfxPoolItem**>(&pNumFormatItem) ) && pNFormatr &&
        nullptr != (pNumFormat = pNFormatr->GetEntry( pNumFormatItem->GetValue() )) )
        pFormat->SetValueFormat( pNumFormat->GetFormatstring(),
                                pNumFormat->GetLanguage(),
                                ::GetAppLanguage());
    else
    {
        // default
        pFormat->SetValueFormat( OUString(), LANGUAGE_SYSTEM,
                              ::GetAppLanguage() );
    }

    // we cannot handle the rest, that's specific to StarCalc
}

void SwTableAutoFormat::UpdateToSet(const sal_uInt8 nPos, const bool bSingleRowTable, const bool bSingleColTable, SfxItemSet& rSet,
                                 SwTableAutoFormatUpdateFlags eFlags, SvNumberFormatter* pNFormatr) const
{
    const SwBoxAutoFormat& rChg = GetBoxFormat( nPos );

    if( SwTableAutoFormatUpdateFlags::Char & eFlags )
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
                rSet.Put( rChg.GetHeight().CloneSetWhich(RES_CHRATR_CJK_FONTSIZE) );
                rSet.Put( rChg.GetWeight().CloneSetWhich(RES_CHRATR_CJK_WEIGHT) );
                rSet.Put( rChg.GetPosture().CloneSetWhich(RES_CHRATR_CJK_POSTURE) );
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
                rSet.Put( rChg.GetHeight().CloneSetWhich(RES_CHRATR_CTL_FONTSIZE) );
                rSet.Put( rChg.GetWeight().CloneSetWhich(RES_CHRATR_CTL_WEIGHT) );
                rSet.Put( rChg.GetPosture().CloneSetWhich(RES_CHRATR_CTL_POSTURE) );
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

    if( !(SwTableAutoFormatUpdateFlags::Box & eFlags) )
        return;

    if( IsFrame() )
    {
        SvxBoxItem aAutoFormatBox = rChg.GetBox();

        // No format box is adequate to specify the borders of single column/row tables, so combine first/last.
        if ( bSingleRowTable || bSingleColTable )
        {
            sal_uInt8 nSingleRowOrColumnId = 15; //LAST_ROW_END_COLUMN
            if ( !bSingleRowTable )
                nSingleRowOrColumnId = nPos + 3;  //LAST COLUMN (3, 7, 11, 15)
            else if ( !bSingleColTable )
                nSingleRowOrColumnId = nPos + 12; //LAST ROW (12, 13, 14, 15)

            assert( nSingleRowOrColumnId < 16 );
            const SvxBoxItem aLastAutoFormatBox( GetBoxFormat(nSingleRowOrColumnId).GetBox() );
            if ( bSingleRowTable )
                aAutoFormatBox.SetLine( aLastAutoFormatBox.GetLine(SvxBoxItemLine::BOTTOM), SvxBoxItemLine::BOTTOM );
            if ( bSingleColTable )
                aAutoFormatBox.SetLine( aLastAutoFormatBox.GetLine(SvxBoxItemLine::RIGHT), SvxBoxItemLine::RIGHT );
        }

        rSet.Put( aAutoFormatBox );
// FIXME - uncomment the lines to put the diagonal line items
//            rSet.Put( rChg.GetTLBR() );
//            rSet.Put( rChg.GetBLTR() );
    }
    if( IsBackground() )
        rSet.Put( rChg.GetBackground() );

    rSet.Put(rChg.GetTextOrientation());

    // Do not put a VertAlign when it has default value.
    // It prevents the export of default value by automatic cell-styles export.
    if (rChg.GetVerticalAlignment().GetVertOrient() != GetDefaultBoxFormat().GetVerticalAlignment().GetVertOrient())
        rSet.Put(rChg.GetVerticalAlignment());

    if( !(IsValueFormat() && pNFormatr) )
        return;

    OUString sFormat;
    LanguageType eLng, eSys;
    rChg.GetValueFormat( sFormat, eLng, eSys );
    if( !sFormat.isEmpty() )
    {
        SvNumFormatType nType;
        bool bNew;
        sal_Int32 nCheckPos;
        sal_uInt32 nKey = pNFormatr->GetIndexPuttingAndConverting( sFormat, eLng,
                                                                eSys, nType, bNew, nCheckPos);
        rSet.Put( SwTableBoxNumFormat( nKey ));
    }
    else
        rSet.ClearItem( RES_BOXATR_FORMAT );

    // we cannot handle the rest, that's specific to StarCalc
}

void SwTableAutoFormat::RestoreTableProperties(SwTable &table) const
{
    SwTableFormat* pFormat = table.GetFrameFormat();
    if (!pFormat)
        return;

    SwDoc *pDoc = pFormat->GetDoc();
    if (!pDoc)
        return;

    SfxItemSet rSet(pDoc->GetAttrPool(), aTableSetRange);

    rSet.Put(SwFormatLayoutSplit(m_bLayoutSplit));
    rSet.Put(SfxBoolItem(RES_COLLAPSING_BORDERS, m_bCollapsingBorders));
    if ( m_aKeepWithNextPara->GetValue() )
        rSet.Put(*m_aKeepWithNextPara);
    rSet.Put(*m_aShadow);

    pFormat->SetFormatAttr(rSet);

    SwEditShell *pShell = pDoc->GetEditShell();
    pDoc->SetRowSplit(*pShell->getShellCursor(false), SwFormatRowSplit(m_bRowSplit));

    table.SetRowsToRepeat(m_aRepeatHeading);
}

void SwTableAutoFormat::StoreTableProperties(const SwTable &table)
{
    SwTableFormat* pFormat = table.GetFrameFormat();
    if (!pFormat)
        return;

    SwDoc *pDoc = pFormat->GetDoc();
    if (!pDoc)
        return;

    SwEditShell *pShell = pDoc->GetEditShell();
    std::unique_ptr<SwFormatRowSplit> pRowSplit = SwDoc::GetRowSplit(*pShell->getShellCursor(false));
    m_bRowSplit = pRowSplit && pRowSplit->GetValue();
    pRowSplit.reset();

    const SfxItemSet &rSet = pFormat->GetAttrSet();

    const SwFormatLayoutSplit &layoutSplit = rSet.Get(RES_LAYOUT_SPLIT);
    m_bLayoutSplit = layoutSplit.GetValue();
    m_bCollapsingBorders = rSet.Get(RES_COLLAPSING_BORDERS).GetValue();

    m_aKeepWithNextPara.reset(rSet.Get(RES_KEEP).Clone());
    m_aRepeatHeading = table.GetRowsToRepeat();
    m_aShadow.reset(rSet.Get(RES_SHADOW).Clone());
}

bool SwTableAutoFormat::FirstRowEndColumnIsRow()
{
    return GetBoxFormat(3) == GetBoxFormat(2);
}
bool SwTableAutoFormat::FirstRowStartColumnIsRow()
{
    return GetBoxFormat(0) == GetBoxFormat(1);
}
bool SwTableAutoFormat::LastRowEndColumnIsRow()
{
    return GetBoxFormat(14) == GetBoxFormat(15);
}
bool SwTableAutoFormat::LastRowStartColumnIsRow()
{
    return GetBoxFormat(12) == GetBoxFormat(13);
}

bool SwTableAutoFormat::Load( SvStream& rStream, const SwAfVersions& rVersions )
{
    sal_uInt16  nVal = 0;
    rStream.ReadUInt16( nVal );
    bool bRet = ERRCODE_NONE == rStream.GetError();

    if( bRet && (nVal == AUTOFORMAT_DATA_ID_X ||
            (AUTOFORMAT_DATA_ID_504 <= nVal && nVal <= AUTOFORMAT_DATA_ID)) )
    {
        bool b;
        // --- from 680/dr25 on: store strings as UTF-8
        rtl_TextEncoding eCharSet = (nVal >= AUTOFORMAT_ID_680DR25) ? RTL_TEXTENCODING_UTF8 : rStream.GetStreamCharSet();
        m_aName = rStream.ReadUniOrByteString( eCharSet );
        if( AUTOFORMAT_DATA_ID_552 <= nVal )
        {
            rStream.ReadUInt16( m_nStrResId );
            // start from 3d because default is added via constructor
            if( m_nStrResId < RES_POOLTABLESTYLE_END - RES_POOLTABLESTYLE_3D )
            {
                m_aName = SwStyleNameMapper::GetUIName(RES_POOLTABLESTYLE_3D + m_nStrResId, m_aName);
            }
            else
                m_nStrResId = USHRT_MAX;
        }
        rStream.ReadCharAsBool( b ); m_bInclFont = b;
        rStream.ReadCharAsBool( b ); m_bInclJustify = b;
        rStream.ReadCharAsBool( b ); m_bInclFrame = b;
        rStream.ReadCharAsBool( b ); m_bInclBackground = b;
        rStream.ReadCharAsBool( b ); m_bInclValueFormat = b;
        rStream.ReadCharAsBool( b ); m_bInclWidthHeight = b;

        if (nVal >= AUTOFORMAT_DATA_ID_31005 && WriterSpecificBlockExists(rStream))
        {
            //this only exists for file format compat
            SvxFormatBreakItem aBreak(SvxBreak::NONE, RES_BREAK);
            legacy::SvxFormatBreak::Create(aBreak, rStream, AUTOFORMAT_FILE_VERSION);
            legacy::SvxFormatKeep::Create(*m_aKeepWithNextPara, rStream, AUTOFORMAT_FILE_VERSION);

            rStream.ReadUInt16( m_aRepeatHeading ).ReadCharAsBool( m_bLayoutSplit ).ReadCharAsBool( m_bRowSplit ).ReadCharAsBool( m_bCollapsingBorders );

            legacy::SvxShadow::Create(*m_aShadow, rStream, AUTOFORMAT_FILE_VERSION);
        }

        bRet = ERRCODE_NONE== rStream.GetError();

        for( sal_uInt8 i = 0; bRet && i < 16; ++i )
        {
            SwBoxAutoFormat* pFormat = new SwBoxAutoFormat;
            bRet = pFormat->Load( rStream, rVersions, nVal );
            if( bRet )
                m_aBoxAutoFormat[ i ] = pFormat;
            else
            {
                delete pFormat;
                break;
            }
        }
    }
    m_bUserDefined = false;
    return bRet;
}

bool SwTableAutoFormat::Save( SvStream& rStream, sal_uInt16 fileVersion ) const
{
    rStream.WriteUInt16( AUTOFORMAT_DATA_ID );
    // --- from 680/dr25 on: store strings as UTF-8
    write_uInt16_lenPrefixed_uInt8s_FromOUString(rStream, m_aName,
        RTL_TEXTENCODING_UTF8 );
    rStream.WriteUInt16( m_nStrResId );
    rStream.WriteBool( m_bInclFont );
    rStream.WriteBool( m_bInclJustify );
    rStream.WriteBool( m_bInclFrame );
    rStream.WriteBool( m_bInclBackground );
    rStream.WriteBool( m_bInclValueFormat );
    rStream.WriteBool( m_bInclWidthHeight );

    {
        WriterSpecificAutoFormatBlock block(rStream);
        //this only exists for file format compat
        SvxFormatBreakItem aBreak(SvxBreak::NONE, RES_BREAK);
        legacy::SvxFormatBreak::Store(aBreak, rStream, legacy::SvxFormatBreak::GetVersion(fileVersion));
        legacy::SvxFormatKeep::Store(*m_aKeepWithNextPara, rStream, legacy::SvxFormatKeep::GetVersion(fileVersion));
        rStream.WriteUInt16( m_aRepeatHeading ).WriteBool( m_bLayoutSplit ).WriteBool( m_bRowSplit ).WriteBool( m_bCollapsingBorders );
        legacy::SvxShadow::Store(*m_aShadow, rStream, legacy::SvxShadow::GetVersion(fileVersion));
    }

    bool bRet = ERRCODE_NONE == rStream.GetError();

    for( int i = 0; bRet && i < 16; ++i )
    {
        SwBoxAutoFormat* pFormat = m_aBoxAutoFormat[ i ];
        if( !pFormat )     // if not set -> write default
        {
            // If it doesn't exist yet:
            if( !s_pDefaultBoxAutoFormat )
                s_pDefaultBoxAutoFormat = new SwBoxAutoFormat;
            pFormat = s_pDefaultBoxAutoFormat;
        }
        bRet = pFormat->Save( rStream, fileVersion );
    }
    return bRet;
}

OUString SwTableAutoFormat::GetTableTemplateCellSubName(const SwBoxAutoFormat& rBoxFormat) const
{
    sal_Int32 nIndex = 0;
    for (; nIndex < 16; ++nIndex)
        if (m_aBoxAutoFormat[nIndex] == &rBoxFormat) break;

    // box format doesn't belong to this table format
    if (16 <= nIndex)
        return OUString();

    const std::vector<sal_Int32> aTableTemplateMap = GetTableTemplateMap();
    for (size_t i=0; i < aTableTemplateMap.size(); ++i)
    {
        if (aTableTemplateMap[i] == nIndex)
            return "." + OUString::number(i + 1);
    }

    // box format doesn't belong to a table template
    return OUString();
}

/*
 * Mapping schema
 *          0            1            2           3           4           5
 *      +-----------------------------------------------------------------------+
 *   0  |   FRSC    |  FR       |  FREC     |           |           |  FRENC    |
 *      +-----------------------------------------------------------------------+
 *   1  |   FC      |  ER       |  EC       |           |           |  LC       |
 *      +-----------------------------------------------------------------------+
 *   2  |   OR      |  OC       |  BODY     |           |           |  BCKG     |
 *      +-----------------------------------------------------------------------+
 *   3  |           |           |           |           |           |           |
 *      +-----------------------------------------------------------------------+
 *   4  |           |           |           |           |           |           |
 *      +-----------------------------------------------------------------------+
 *   5  |   LRSC    |  LR       |  LREC     |           |           |  LRENC    |
 *      +-----------+-----------+-----------+-----------+-----------+-----------+
 * ODD  = 1, 3, 5, ...
 * EVEN = 2, 4, 6, ...
 */
const std::vector<sal_Int32> & SwTableAutoFormat::GetTableTemplateMap()
{
    static std::vector<sal_Int32> const aTableTemplateMap
    {
        1 , // FIRST_ROW              // FR
        13, // LAST_ROW               // LR
        4 , // FIRST_COLUMN           // FC
        7 , // LAST_COLUMN            // LC
        5 , // EVEN_ROWS              // ER
        8 , // ODD_ROWS               // OR
        6 , // EVEN_COLUMNS           // EC
        9 , // ODD_COLUMNS            // OC
        10, // BODY
        11, // BACKGROUND             // BCKG
        0 , // FIRST_ROW_START_COLUMN // FRSC
        3 , // FIRST_ROW_END_COLUMN   // FRENC
        12, // LAST_ROW_START_COLUMN  // LRSC
        15, // LAST_ROW_END_COLUMN    // LRENC
        2 , // FIRST_ROW_EVEN_COLUMN  // FREC
        14, // LAST_ROW_EVEN_COLUMN   // LREC
    };
    return aTableTemplateMap;
}

sal_uInt8 SwTableAutoFormat::CountPos(sal_uInt32 nCol, sal_uInt32 nCols, sal_uInt32 nRow,
                                      sal_uInt32 nRows)
{
    sal_uInt8 nRet = static_cast<sal_uInt8>(
        !nRow ? 0 : ((nRow + 1 == nRows) ? 12 : (4 * (1 + ((nRow - 1) & 1)))));
    nRet = nRet
           + static_cast<sal_uInt8>(!nCol ? 0 : (nCol + 1 == nCols ? 3 : (1 + ((nCol - 1) & 1))));
    return nRet;
}

struct SwTableAutoFormatTable::Impl
{
    std::vector<std::unique_ptr<SwTableAutoFormat>> m_AutoFormats;
};

size_t SwTableAutoFormatTable::size() const
{
    return m_pImpl->m_AutoFormats.size();
}

SwTableAutoFormat const& SwTableAutoFormatTable::operator[](size_t const i) const
{
    return *m_pImpl->m_AutoFormats[i];
}
SwTableAutoFormat      & SwTableAutoFormatTable::operator[](size_t const i)
{
    return *m_pImpl->m_AutoFormats[i];
}

void SwTableAutoFormatTable::AddAutoFormat(const SwTableAutoFormat& rTableStyle)
{
    // don't insert when we already have style of this name
    if (FindAutoFormat(rTableStyle.GetName()))
        return;

    InsertAutoFormat(size(), std::make_unique<SwTableAutoFormat>(rTableStyle));
}

void SwTableAutoFormatTable::InsertAutoFormat(size_t const i, std::unique_ptr<SwTableAutoFormat> pFormat)
{
    m_pImpl->m_AutoFormats.insert(m_pImpl->m_AutoFormats.begin() + i, std::move(pFormat));
}

void SwTableAutoFormatTable::EraseAutoFormat(size_t const i)
{
    m_pImpl->m_AutoFormats.erase(m_pImpl->m_AutoFormats.begin() + i);
}

void SwTableAutoFormatTable::EraseAutoFormat(const OUString& rName)
{
    auto iter = std::find_if(m_pImpl->m_AutoFormats.begin(), m_pImpl->m_AutoFormats.end(),
        [&rName](const std::unique_ptr<SwTableAutoFormat>& rpFormat) { return rpFormat->GetName() == rName; });
    if (iter != m_pImpl->m_AutoFormats.end())
    {
        m_pImpl->m_AutoFormats.erase(iter);
        return;
    }
    SAL_INFO("sw.core", "SwTableAutoFormatTable::EraseAutoFormat, SwTableAutoFormat with given name not found");
}

std::unique_ptr<SwTableAutoFormat> SwTableAutoFormatTable::ReleaseAutoFormat(size_t const i)
{
    auto const iter(m_pImpl->m_AutoFormats.begin() + i);
    std::unique_ptr<SwTableAutoFormat> pRet(std::move(*iter));
    m_pImpl->m_AutoFormats.erase(iter);
    return pRet;
}

std::unique_ptr<SwTableAutoFormat> SwTableAutoFormatTable::ReleaseAutoFormat(const OUString& rName)
{
    std::unique_ptr<SwTableAutoFormat> pRet;
    auto iter = std::find_if(m_pImpl->m_AutoFormats.begin(), m_pImpl->m_AutoFormats.end(),
        [&rName](const std::unique_ptr<SwTableAutoFormat>& rpFormat) { return rpFormat->GetName() == rName; });
    if (iter != m_pImpl->m_AutoFormats.end())
    {
        pRet = std::move(*iter);
        m_pImpl->m_AutoFormats.erase(iter);
    }
    return pRet;
}

SwTableAutoFormat* SwTableAutoFormatTable::FindAutoFormat(std::u16string_view rName) const
{
    for (const auto &rFormat : m_pImpl->m_AutoFormats)
    {
        if (rFormat->GetName() == rName)
            return rFormat.get();
    }

    return nullptr;
}

SwTableAutoFormatTable::~SwTableAutoFormatTable()
{
}

SwTableAutoFormatTable::SwTableAutoFormatTable()
    : m_pImpl(new Impl)
{
    std::unique_ptr<SwTableAutoFormat> pNew(new SwTableAutoFormat(
                SwStyleNameMapper::GetUIName(RES_POOLTABLESTYLE_DEFAULT, OUString())));

    sal_uInt8 i;

    Color aColor( COL_BLACK );
    SvxBoxItem aBox( RES_BOX );

    aBox.SetAllDistances(55);
    SvxBorderLine aLn( &aColor, SvxBorderLineWidth::VeryThin );
    aBox.SetLine( &aLn, SvxBoxItemLine::LEFT );
    aBox.SetLine( &aLn, SvxBoxItemLine::BOTTOM );

    for( i = 0; i <= 15; ++i )
    {
        aBox.SetLine( i <= 3 ? &aLn : nullptr, SvxBoxItemLine::TOP );
        aBox.SetLine( (3 == ( i & 3 )) ? &aLn : nullptr, SvxBoxItemLine::RIGHT );
        pNew->GetBoxFormat( i ).SetBox( aBox );
    }

    pNew->SetUserDefined(false);
    m_pImpl->m_AutoFormats.push_back(std::move(pNew));
}

void SwTableAutoFormatTable::Load()
{
    if (utl::ConfigManager::IsFuzzing())
        return;
    OUString sNm(AUTOTABLE_FORMAT_NAME);
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm ))
    {
        SfxMedium aStream( sNm, StreamMode::STD_READ );
        Load( *aStream.GetInStream() );
    }
}

bool SwTableAutoFormatTable::Save() const
{
    if (utl::ConfigManager::IsFuzzing())
        return false;
    SvtPathOptions aPathOpt;
    const OUString sNm( aPathOpt.GetUserConfigPath() + "/" + AUTOTABLE_FORMAT_NAME );
    SfxMedium aStream(sNm, StreamMode::STD_WRITE );
    return Save( *aStream.GetOutStream() ) && aStream.Commit();
}

bool SwTableAutoFormatTable::Load( SvStream& rStream )
{
    bool bRet = ERRCODE_NONE == rStream.GetError();
    if (bRet)
    {
        // Attention: We need to read a general Header here
        sal_uInt16 nVal = 0;
        rStream.ReadUInt16( nVal );
        bRet = ERRCODE_NONE == rStream.GetError();

        if( bRet )
        {
            SwAfVersions aVersions;

            // Default version is 5.0, unless we detect an old format ID.
            sal_uInt16 nFileVers = SOFFICE_FILEFORMAT_50;
            if(nVal < AUTOFORMAT_ID_31005)
                nFileVers = SOFFICE_FILEFORMAT_40;

            if( nVal == AUTOFORMAT_ID_358 ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                sal_uInt8 nChrSet, nCnt;
                sal_uInt64 nPos = rStream.Tell();
                rStream.ReadUChar( nCnt ).ReadUChar( nChrSet );
                if( rStream.Tell() != nPos + nCnt )
                {
                    OSL_ENSURE( false, "The Header contains more or newer Data" );
                    rStream.Seek( nPos + nCnt );
                }
                rStream.SetStreamCharSet( static_cast<rtl_TextEncoding>(nChrSet) );
                rStream.SetVersion( nFileVers );
            }

            if( nVal == AUTOFORMAT_ID_358 || nVal == AUTOFORMAT_ID_X ||
                    (AUTOFORMAT_ID_504 <= nVal && nVal <= AUTOFORMAT_ID) )
            {
                aVersions.Load( rStream, nVal );        // Item versions

                sal_uInt16 nCount = 0;
                rStream.ReadUInt16( nCount );

                bRet = ERRCODE_NONE== rStream.GetError();
                if (bRet)
                {
                    const size_t nMinRecordSize = sizeof(sal_uInt16);
                    const size_t nMaxRecords = rStream.remainingSize() / nMinRecordSize;
                    if (nCount > nMaxRecords)
                    {
                        SAL_WARN("sw.core", "Parsing error: " << nMaxRecords <<
                                 " max possible entries, but " << nCount << " claimed, truncating");
                        nCount = nMaxRecords;
                    }
                    for (sal_uInt16 i = 0; i < nCount; ++i)
                    {
                        std::unique_ptr<SwTableAutoFormat> pNew(
                            new SwTableAutoFormat( OUString() ));
                        bRet = pNew->Load( rStream, aVersions );
                        if( bRet )
                        {
                            m_pImpl->m_AutoFormats.push_back(std::move(pNew));
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }
            else
            {
                bRet = false;
            }
        }
    }
    return bRet;
}

bool SwTableAutoFormatTable::Save( SvStream& rStream ) const
{
    bool bRet = ERRCODE_NONE == rStream.GetError();
    if (bRet)
    {
        rStream.SetVersion(AUTOFORMAT_FILE_VERSION);

        // Attention: We need to save a general Header here
        rStream.WriteUInt16( AUTOFORMAT_ID )
               .WriteUChar( 2 ) // Character count of the Header including this value
               .WriteUChar( GetStoreCharSet( ::osl_getThreadTextEncoding() ) );

        bRet = ERRCODE_NONE == rStream.GetError();
        if (!bRet)
            return false;

        // Write this version number for all attributes
        SwAfVersions::Write(rStream, AUTOFORMAT_FILE_VERSION);

        rStream.WriteUInt16( m_pImpl->m_AutoFormats.size() - 1 );
        bRet = ERRCODE_NONE == rStream.GetError();

        for (size_t i = 1; bRet && i < m_pImpl->m_AutoFormats.size(); ++i)
        {
            SwTableAutoFormat const& rFormat = *m_pImpl->m_AutoFormats[i];
            bRet = rFormat.Save(rStream, AUTOFORMAT_FILE_VERSION);
        }
    }
    rStream.Flush();
    return bRet;
}

SwCellStyleTable::SwCellStyleTable()
{ }

SwCellStyleTable::~SwCellStyleTable()
{
}

size_t SwCellStyleTable::size() const
{
    return m_aCellStyles.size();
}

void SwCellStyleTable::clear()
{
    m_aCellStyles.clear();
}

SwCellStyleDescriptor SwCellStyleTable::operator[](size_t i) const
{
    return SwCellStyleDescriptor(m_aCellStyles[i]);
}

void SwCellStyleTable::AddBoxFormat(const SwBoxAutoFormat& rBoxFormat, const OUString& sName)
{
    m_aCellStyles.emplace_back(sName, std::make_unique<SwBoxAutoFormat>(rBoxFormat));
}

void SwCellStyleTable::RemoveBoxFormat(const OUString& sName)
{
    auto iter = std::find_if(m_aCellStyles.begin(), m_aCellStyles.end(),
        [&sName](const std::pair<OUString, std::unique_ptr<SwBoxAutoFormat>>& rStyle) { return rStyle.first == sName; });
    if (iter != m_aCellStyles.end())
    {
        m_aCellStyles.erase(iter);
        return;
    }
    SAL_INFO("sw.core", "SwCellStyleTable::RemoveBoxFormat, format with given name doesn't exists");
}

OUString SwCellStyleTable::GetBoxFormatName(const SwBoxAutoFormat& rBoxFormat) const
{
    for (size_t i=0; i < m_aCellStyles.size(); ++i)
    {
        if (m_aCellStyles[i].second.get() == &rBoxFormat)
            return m_aCellStyles[i].first;
    }

    // box format not found
    return OUString();
}

SwBoxAutoFormat* SwCellStyleTable::GetBoxFormat(std::u16string_view sName) const
{
    for (size_t i=0; i < m_aCellStyles.size(); ++i)
    {
        if (m_aCellStyles[i].first == sName)
            return m_aCellStyles[i].second.get();
    }

    return nullptr;
}

void SwCellStyleTable::ChangeBoxFormatName(std::u16string_view sFromName, const OUString& sToName)
{
    if (!GetBoxFormat(sToName))
    {
        SAL_INFO("sw.core", "SwCellStyleTable::ChangeBoxName, box with given name already exists");
        return;
    }
    for (size_t i=0; i < m_aCellStyles.size(); ++i)
    {
        if (m_aCellStyles[i].first == sFromName)
        {
            m_aCellStyles[i].first = sToName;
            // changed successfully
            return;
        }
    }
    SAL_INFO("sw.core", "SwCellStyleTable::ChangeBoxName, box with given name not found");
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
