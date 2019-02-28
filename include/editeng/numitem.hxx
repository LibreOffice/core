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
#include <tools/link.hxx>
#include <tools/solar.h>
#include <svl/poolitem.hxx>
#include <editeng/svxenum.hxx>
#include <tools/gen.hxx>
#include <editeng/numdef.hxx>
#include <tools/color.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/style/NumberingType.hpp>
#include <unotools/fontcvt.hxx>
#include <editeng/editengdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <memory>
#include <algorithm>

class SvxBrushItem;
namespace vcl { class Font; }
class Graphic;
class SvxNodeNum;
namespace com{namespace sun{ namespace star{
    namespace text{
        class XNumberingFormatter;
    }
}}}


#define SVX_NO_NUM              200 // Marker for no numbering
#define SVX_NO_NUMLEVEL         0x20
#define SVX_NUM_REL_SIZE_MIN    25 // Lower limit for numbering relative size


#define LINK_TOKEN  0x80 //indicate linked bitmaps - for use in dialog only
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
    OUString        GetNumStr( sal_Int32 nNo, const css::lang::Locale& rLocale ) const;

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

    SvxAdjust           eNumAdjust;

    sal_uInt8           nInclUpperLevels;   // Take over numbers from the previous level.
    sal_uInt16          nStart;             // Start of counting

    sal_Unicode         cBullet;            // Symbol
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
    long                mnListtabPos;
    // specifies the first line indent
    long                mnFirstLineIndent;
    // specifies the indent before the text, e.g. in L2R-layout the left margin
    long                mnIndentAt;

    std::unique_ptr<SvxBrushItem>
                        pGraphicBrush;
    sal_Int16           eVertOrient;        // vertical alignment of a bitmap

    Size                aGraphicSize;       // Always! in 1/100 mm
    std::unique_ptr<vcl::Font>
                        pBulletFont;        // Pointer to the bullet font

    OUString            sCharStyleName;     // Character Style

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
    void            SetPrefix(const OUString& rSet) { sPrefix = rSet;}
    const OUString& GetPrefix() const { return sPrefix;}
    void            SetSuffix(const OUString& rSet) { sSuffix = rSet;}
    const OUString& GetSuffix() const { return sSuffix;}

    void                    SetCharFormatName(const OUString& rSet){ sCharStyleName = rSet; }
    virtual OUString        GetCharFormatName()const;

    void            SetBulletFont(const vcl::Font* pFont);
    const vcl::Font* GetBulletFont() const {return pBulletFont.get();}
    void            SetBulletChar(sal_Unicode cSet){cBullet = cSet;}
    sal_Unicode     GetBulletChar()const {return cBullet;}
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
    void SetListtabPos( const long nListtabPos );
    long GetListtabPos() const { return mnListtabPos;}
    void SetFirstLineIndent( const long nFirstLineIndent );
    long GetFirstLineIndent() const { return mnFirstLineIndent;}
    void SetIndentAt( const long nIndentAt );
    long GetIndentAt() const { return mnIndentAt;}

    static Size     GetGraphicSizeMM100(const Graphic* pGraphic);
    static OUString CreateRomanString( sal_uLong nNo, bool bUpper );
};

//Feature-Flags (only sal_uInt16!)
enum class SvxNumRuleFlags
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
    template<> struct typed_flags<SvxNumRuleFlags> : is_typed_flags<SvxNumRuleFlags, 0x03dd> {};
}

enum class SvxNumRuleType
{
    NUMBERING,
    OUTLINE_NUMBERING,
    PRESENTATION_NUMBERING
};

class EDITENG_DLLPUBLIC SvxNumRule final
{
    sal_uInt16          nLevelCount;            // Number of supported levels
    SvxNumRuleFlags     nFeatureFlags;          // What is supported?
    SvxNumRuleType      eNumberingType;         // Type of numbering
    bool                bContinuousNumbering;   // sequential numbering

    std::unique_ptr<SvxNumberFormat> aFmts[SVX_MAX_NUM];
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
    SvxNumRule(SvStream &rStream);
    ~SvxNumRule();

    bool                    operator==( const SvxNumRule& ) const;
    bool                    operator!=( const SvxNumRule& rRule ) const {return !(*this == rRule);}

    SvxNumRule&             operator=( const SvxNumRule&  );

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

class EDITENG_DLLPUBLIC SvxNumBulletItem : public SfxPoolItem
{
    std::unique_ptr<SvxNumRule> pNumRule;
public:
    explicit SvxNumBulletItem(SvxNumRule const & rRule);
    SvxNumBulletItem(SvxNumRule const & rRule, sal_uInt16 nWhich );
    SvxNumBulletItem(const SvxNumBulletItem& rCopy);
    virtual ~SvxNumBulletItem() override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    sal_uInt16  GetVersion( sal_uInt16 nFileVersion ) const override;
    virtual bool             operator==( const SfxPoolItem& ) const override;

    SvxNumRule*             GetNumRule() const {return pNumRule.get();}

    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;
    virtual void            dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SvxNodeNum
{
    sal_uInt16 nLevelVal[ SVX_MAX_NUM ];    // Numbers of all levels
    sal_uInt8  nMyLevel;                    // Current Level

public:
    explicit inline SvxNodeNum();
    inline SvxNodeNum& operator=( const SvxNodeNum& rCpy );

    sal_uInt8 GetLevel() const                  { return nMyLevel; }
    void SetLevel( sal_uInt8 nVal )             { nMyLevel = nVal; }

    const sal_uInt16* GetLevelVal() const       { return nLevelVal; }
          sal_uInt16* GetLevelVal()             { return nLevelVal; }
};

SvxNodeNum::SvxNodeNum()
    : nMyLevel( 0 )
{
    memset( nLevelVal, 0, sizeof( nLevelVal ) );
}

inline SvxNodeNum& SvxNodeNum::operator=( const SvxNodeNum& rCpy )
{
    if ( &rCpy != this)
    {
        nMyLevel = rCpy.nMyLevel;

        memcpy( nLevelVal, rCpy.nLevelVal, sizeof( nLevelVal ) );
    }
    return *this;
}

std::unique_ptr<SvxNumRule> SvxConvertNumRule( const SvxNumRule* pRule, sal_uInt16 nLevel, SvxNumRuleType eType );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
