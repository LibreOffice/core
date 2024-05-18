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
#ifndef INCLUDED_EDITENG_NUMITEM_HXX
#define INCLUDED_EDITENG_NUMITEM_HXX

#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <editeng/svxenum.hxx>
#include <tools/gen.hxx>
#include <editeng/numdef.hxx>
#include <tools/color.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <unotools/fontcvt.hxx>
#include <editeng/editengdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/font.hxx>
#include <memory>
#include <optional>
#include <algorithm>

class SvxBrushItem;
class Graphic;
class SvxNodeNum;
namespace com::sun::star::text { class XNumberingFormatter; }

namespace com::sun::star::lang { struct Locale; }


#define SVX_NO_NUM              200 // Marker for no numbering
#define SVX_NO_NUMLEVEL         0x20
#define SVX_NUM_REL_SIZE_MIN    25 // Lower limit for numbering relative size


#define LINK_TOKEN  0x80 //indicate linked bitmaps - for use in dialog only

typedef struct _xmlTextWriter* xmlTextWriterPtr;

class EDITENG_DLLPUBLIC SvxNumberType
{
    static sal_Int32 nRefCount;
    static css::uno::Reference<css::text::XNumberingFormatter> xFormatter;

    SvxNumType      nNumType;
    bool            bShowSymbol;        // Also show Symbol ?

public:
    explicit SvxNumberType(SvxNumType nType = SVX_NUM_ARABIC);
    SvxNumberType(const SvxNumberType& rType);
    ~SvxNumberType();
    SvxNumberType & operator =(SvxNumberType const &) = default;

    OUString        GetNumStr( sal_Int32 nNo ) const;
    OUString        GetNumStr( sal_Int32 nNo, const css::lang::Locale& rLocale, bool bIsLegal = false ) const;

    void            SetNumberingType(SvxNumType nSet) {nNumType = nSet;}
    SvxNumType      GetNumberingType() const {return nNumType;}

    void            SetShowSymbol(bool bSet) {bShowSymbol = bSet;}
    bool            IsShowSymbol()const{return bShowSymbol;}

    bool            IsTextFormat() const
                    {
                        return css::style::NumberingType::NUMBER_NONE != nNumType &&
                               css::style::NumberingType::CHAR_SPECIAL != nNumType &&
                               css::style::NumberingType::BITMAP != nNumType;
                    }

    void dumpAsXml(xmlTextWriterPtr w) const;
};

class EDITENG_DLLPUBLIC SvxNumberFormat : public SvxNumberType
{
public:
    enum SvxNumPositionAndSpaceMode
    {
        LABEL_WIDTH_AND_POSITION,
        LABEL_ALIGNMENT
    };
    enum LabelFollowedBy
    {
        LISTTAB,
        SPACE,
        NOTHING,
        NEWLINE
    };

private:
    OUString            sPrefix;
    OUString            sSuffix;
    std::optional<OUString> sListFormat;        // Format string ">%1%.%2%<" can be used instead of prefix/suffix
                                                // Right now it is optional value to distinguish empty list format
                                                // and not set list format when we need to fallback to prefix/suffix.

    SvxAdjust           eNumAdjust;

    sal_uInt8           nInclUpperLevels;   // Take over numbers from the previous level.
    sal_uInt16          nStart;             // Start of counting

    sal_UCS4            cBullet;            // Symbol
    sal_uInt16          nBulletRelSize;     // percentage size of bullets
    Color               nBulletColor;       // Bullet color

    // mode indicating, if the position and spacing of the list label is
    // determined by the former attributes (nFirstLineOffset, nAbsLSpace
    // and nCharTextDistance) called position and spacing via label
    // width and position (LABEL_WIDTH_AND_POSITION) or by the new attributes
    // (meLabelFollowedBy, mnListtabPos, mnFirstLineIndent and mnIndentAt)
    // called position and spacing via label alignment.
    // Note 1: Attribute <eNumAdjust> is relevant for both modes.
    // Note 2: The values of the former attributes are treated as 0, if mode
    //         LABEL_ALIGNMENT is active.
    SvxNumPositionAndSpaceMode mePositionAndSpaceMode;

    sal_Int32           nFirstLineOffset;   // First line indent
    sal_Int32           nAbsLSpace;         // Distance Border<->Number
    short               nCharTextDistance;  // Distance Number<->Text

    // specifies what follows the list label before the text of the first line
    // of the list item starts
    LabelFollowedBy     meLabelFollowedBy;
    // specifies an additional list tab stop position for meLabelFollowedBy = LISTTAB
    tools::Long                mnListtabPos;
    // specifies the first line indent
    tools::Long                mnFirstLineIndent;
    // specifies the indent before the text, e.g. in L2R-layout the left margin
    tools::Long                mnIndentAt;

    std::unique_ptr<SvxBrushItem>
                        pGraphicBrush;
    sal_Int16           eVertOrient;        // vertical alignment of a bitmap

    Size                aGraphicSize;       // Always! in 1/100 mm
    std::optional<vcl::Font>
                        pBulletFont;        // Pointer to the bullet font

    OUString            sCharStyleName;     // Character Style

    bool mbIsLegal = false; // "Legal" level numbering = all levels use arabic numbering

public:
    explicit SvxNumberFormat( SvxNumType nNumberingType );
    SvxNumberFormat(const SvxNumberFormat& rFormat);
    SvxNumberFormat( SvStream & rStream );

    virtual ~SvxNumberFormat();

    void            Store(SvStream &rStream, FontToSubsFontConverter pConverter);

    SvxNumberFormat& operator=( const SvxNumberFormat&  );
    bool            operator==( const SvxNumberFormat&  ) const;
    bool            operator!=( const SvxNumberFormat& rFmt) const {return !(*this == rFmt);}

    void            SetNumAdjust(SvxAdjust eSet) {eNumAdjust = eSet;}
    SvxAdjust       GetNumAdjust() const {return eNumAdjust;}
    void            SetPrefix(const OUString& rSet);
    const OUString& GetPrefix() const { return sPrefix;}
    void            SetSuffix(const OUString& rSet);
    const OUString& GetSuffix() const { return sSuffix;}
    // Based on prefix and suffix initialize them (for backward compatibility) and generate listformat string
    void            SetListFormat(const OUString& rPrefix, const OUString& rSuffix, int nLevel);
    void            SetListFormat(std::optional<OUString> oSet = std::nullopt);
    bool            HasListFormat() const { return sListFormat.has_value(); }
    OUString        GetListFormat(bool bIncludePrefixSuffix = true) const;

    void                    SetCharFormatName(const OUString& rSet){ sCharStyleName = rSet; }
    virtual OUString        GetCharFormatName()const;

    void            SetBulletFont(const vcl::Font* pFont);
    const std::optional<vcl::Font>& GetBulletFont() const { return pBulletFont; }
    void            SetBulletChar(sal_UCS4 cSet){cBullet = cSet;}
    sal_UCS4        GetBulletChar()const {return cBullet;}
    void            SetBulletRelSize(sal_uInt16 nSet) {nBulletRelSize = std::max(nSet,sal_uInt16(SVX_NUM_REL_SIZE_MIN));}
    sal_uInt16      GetBulletRelSize() const { return nBulletRelSize;}
    void            SetBulletColor(Color nSet){nBulletColor = nSet;}
    const Color&    GetBulletColor()const {return nBulletColor;}

    void            SetIncludeUpperLevels( sal_uInt8 nSet ) { nInclUpperLevels = nSet;}
    sal_uInt8       GetIncludeUpperLevels()const  { return nInclUpperLevels;}
    void            SetStart(sal_uInt16 nSet) {nStart = nSet;}
    sal_uInt16      GetStart() const {return nStart;}

    virtual void    SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = nullptr, const sal_Int16* pOrient = nullptr);
    const SvxBrushItem*         GetBrush() const {return pGraphicBrush.get();}
    void            SetGraphic( const OUString& rName );
    sal_Int16       GetVertOrient() const;
    void            SetGraphicSize(const Size& rSet) {aGraphicSize = rSet;}
    const Size&     GetGraphicSize() const {return aGraphicSize;}

    SvxNumPositionAndSpaceMode GetPositionAndSpaceMode() const { return mePositionAndSpaceMode;}
    void SetPositionAndSpaceMode( SvxNumPositionAndSpaceMode ePositionAndSpaceMode );

    void            SetAbsLSpace(sal_Int32 nSet) {nAbsLSpace = nSet;}
    sal_Int32       GetAbsLSpace() const;
    void            SetFirstLineOffset(sal_Int32 nSet) { nFirstLineOffset = nSet;}
    sal_Int32       GetFirstLineOffset() const;
    void            SetCharTextDistance(short nSet) { nCharTextDistance = nSet; }
    short           GetCharTextDistance() const;

    void SetLabelFollowedBy( const LabelFollowedBy eLabelFollowedBy );
    LabelFollowedBy GetLabelFollowedBy() const { return meLabelFollowedBy;}
    OUString GetLabelFollowedByAsString() const;
    void SetListtabPos( const tools::Long nListtabPos );
    tools::Long GetListtabPos() const { return mnListtabPos;}
    void SetFirstLineIndent( const tools::Long nFirstLineIndent );
    tools::Long GetFirstLineIndent() const { return mnFirstLineIndent;}
    void SetIndentAt( const tools::Long nIndentAt );
    tools::Long GetIndentAt() const { return mnIndentAt;}

    static Size     GetGraphicSizeMM100(const Graphic* pGraphic);
    static OUString CreateRomanString( sal_Int32 nNo, bool bUpper );

    bool GetIsLegal() const { return mbIsLegal; }
    void SetIsLegal(bool val) { mbIsLegal = val; }
};

//Feature-Flags (only sal_uInt16!)
enum class SvxNumRuleFlags : sal_uInt16
{
    NONE                = 0x0000,
    CONTINUOUS          = 0x0001, // consecutive numbers possible?
    CHAR_STYLE          = 0x0004, // Character styles?
    BULLET_REL_SIZE     = 0x0008, // relative bullet size?
    BULLET_COLOR        = 0x0010, // Bullet color
    NO_NUMBERS          = 0x0080, // Numbering are not allowed
    ENABLE_LINKED_BMP   = 0x0100, // linked bitmaps are available
    ENABLE_EMBEDDED_BMP = 0x0200  // embedded bitmaps are available
};
namespace o3tl
{
    template<> struct typed_flags<SvxNumRuleFlags> : is_typed_flags<SvxNumRuleFlags, 0x039d> {};
}

enum class SvxNumRuleType : sal_uInt8
{
    NUMBERING,
    OUTLINE_NUMBERING,
    PRESENTATION_NUMBERING
};

class EDITENG_DLLPUBLIC SvxNumRule final
{
    std::unique_ptr<SvxNumberFormat> aFmts[SVX_MAX_NUM];
    sal_uInt16          nLevelCount;            // Number of supported levels
    SvxNumRuleFlags     nFeatureFlags;          // What is supported?
    SvxNumRuleType      eNumberingType;         // Type of numbering
    bool                bContinuousNumbering;   // sequential numbering
    bool                aFmtsSet[SVX_MAX_NUM]; // Flags indicating valid levels

    static sal_Int32    nRefCount;
public:
    SvxNumRule( SvxNumRuleFlags nFeatures,
                sal_uInt16 nLevels,
                bool bCont,
                SvxNumRuleType eType = SvxNumRuleType::NUMBERING,
                SvxNumberFormat::SvxNumPositionAndSpaceMode
                        eDefaultNumberFormatPositionAndSpaceMode
                                = SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    SvxNumRule(const SvxNumRule& rCopy);
    SvxNumRule(SvxNumRule&&) noexcept;
    SvxNumRule(SvStream &rStream);
    ~SvxNumRule();

    bool                    operator==( const SvxNumRule& ) const;
    bool                    operator!=( const SvxNumRule& rRule ) const {return !(*this == rRule);}

    SvxNumRule&             operator=( const SvxNumRule&  );
    SvxNumRule&             operator=( SvxNumRule&&  ) noexcept;

    void                    Store(SvStream &rStream);
    void                    dumpAsXml(xmlTextWriterPtr pWriter) const;
    const SvxNumberFormat*  Get(sal_uInt16 nLevel)const;
    const SvxNumberFormat&  GetLevel(sal_uInt16 nLevel)const;
    void                    SetLevel(sal_uInt16 nLevel, const SvxNumberFormat& rFmt, bool bIsValid = true);
    void                    SetLevel(sal_uInt16 nLevel, const SvxNumberFormat* pFmt);

    bool                    IsContinuousNumbering()const
                                            {return bContinuousNumbering;}
    void                    SetContinuousNumbering(bool bSet)
                                            {bContinuousNumbering = bSet;}

    sal_uInt16              GetLevelCount() const {return nLevelCount;}
    bool                    IsFeatureSupported(SvxNumRuleFlags nFeature) const
                                            { return bool(nFeatureFlags & nFeature); }
    SvxNumRuleFlags         GetFeatureFlags() const {return nFeatureFlags;}
    void                    SetFeatureFlag( SvxNumRuleFlags nFlag, bool bSet = true ) { if(bSet) nFeatureFlags |= nFlag; else nFeatureFlags &= ~nFlag; }

    OUString                MakeNumString( const SvxNodeNum& ) const;

    SvxNumRuleType          GetNumRuleType() const { return eNumberingType; }

    void                    UnLinkGraphics();
};

class EDITENG_DLLPUBLIC SvxNumBulletItem final : public SfxPoolItem
{
    SvxNumRule maNumRule;
public:
    explicit SvxNumBulletItem(SvxNumRule const & rRule);
    explicit SvxNumBulletItem(SvxNumRule && rRule);
    SvxNumBulletItem(SvxNumRule const & rRule, sal_uInt16 nWhich );
    SvxNumBulletItem(SvxNumRule && rRule, sal_uInt16 nWhich );
    SvxNumBulletItem(const SvxNumBulletItem& rCopy);
    virtual ~SvxNumBulletItem() override;

    virtual SvxNumBulletItem* Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool             operator==( const SfxPoolItem& ) const override;

    const SvxNumRule&       GetNumRule() const { return maNumRule; }
    SvxNumRule&             GetNumRule() { return maNumRule; }

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual void            dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SvxNodeNum
{
    sal_uInt16 nLevelVal[ SVX_MAX_NUM ] = {};    // Numbers of all levels
    sal_uInt8  nMyLevel = 0;                // Current Level

public:
    explicit inline SvxNodeNum() = default;
    inline SvxNodeNum& operator=( const SvxNodeNum& rCpy );

    sal_uInt8 GetLevel() const                  { return nMyLevel; }
    void SetLevel( sal_uInt8 nVal )             { nMyLevel = nVal; }

    const sal_uInt16* GetLevelVal() const       { return nLevelVal; }
          sal_uInt16* GetLevelVal()             { return nLevelVal; }
};

inline SvxNodeNum& SvxNodeNum::operator=( const SvxNodeNum& rCpy )
{
    if ( &rCpy != this)
    {
        nMyLevel = rCpy.nMyLevel;

        memcpy( nLevelVal, rCpy.nLevelVal, sizeof( nLevelVal ) );
    }
    return *this;
}

SvxNumRule SvxConvertNumRule( const SvxNumRule& rRule, sal_uInt16 nLevel, SvxNumRuleType eType );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
