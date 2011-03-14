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
#ifndef _SVX_NUMITEM_HXX
#define _SVX_NUMITEM_HXX

// include ---------------------------------------------------------------

#include <tools/link.hxx>
#include <tools/string.hxx>
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

class SvxBrushItem;
class Font;
class Graphic;
class SvxNodeNum;
class BitmapEx;
namespace com{namespace sun{ namespace star{
    namespace text{
        class XNumberingFormatter;
    }
}}}

// -----------------------------------------------------------------------
//Feature-Flags (only USHORT!)
#define NUM_CONTINUOUS          0x0001 // consecutive numbers possible?
#define NUM_CHAR_TEXT_DISTANCE  0x0002 // Distance Symbol<->Text?
#define NUM_CHAR_STYLE          0x0004 // Character styles?
#define NUM_BULLET_REL_SIZE     0x0008 // relative bullet size?
#define NUM_BULLET_COLOR        0x0010 // Bullet color
#define NUM_SYMBOL_ALIGNMENT    0x0040 // alignment to be shown in the options
#define NUM_NO_NUMBERS          0x0080 // Numbering are not allowed
#define NUM_ENABLE_LINKED_BMP   0x0100 // linked bitmaps are available
#define NUM_ENABLE_EMBEDDED_BMP 0x0200 // embedded bitmaps are available

#define SVX_NO_NUM              200 // Marker for no numbering
#define SVX_NO_NUMLEVEL         0x20

#define LINK_TOKEN  0x80 //indicate linked bitmaps - for use in dialog only
class EDITENG_DLLPUBLIC SvxNumberType
{
    static sal_Int32 nRefCount;
    static com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter> xFormatter;

    sal_Int16       nNumType;
    sal_Bool        bShowSymbol;        // Also show Symbol ?

public:
    SvxNumberType(sal_Int16 nType = com::sun::star::style::NumberingType::ARABIC);
    SvxNumberType(const SvxNumberType& rType);
    ~SvxNumberType();

    String          GetNumStr( ULONG nNo ) const;
    String          GetNumStr( ULONG nNo, const com::sun::star::lang::Locale& rLocale ) const;

    void            SetNumberingType(sal_Int16 nSet) {nNumType = nSet;}
    sal_Int16       GetNumberingType() const {return nNumType;}

    void            SetShowSymbol(sal_Bool bSet) {bShowSymbol = bSet;}
    sal_Bool        IsShowSymbol()const{return bShowSymbol;}

    sal_Bool        IsTxtFmt() const
                    {
                        return com::sun::star::style::NumberingType::NUMBER_NONE != nNumType &&
                               com::sun::star::style::NumberingType::CHAR_SPECIAL != nNumType &&
                               com::sun::star::style::NumberingType::BITMAP != nNumType;
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
    enum SvxNumLabelFollowedBy
    {
        LISTTAB,
        SPACE,
        NOTHING
    };

private:
    String              sPrefix;
    String              sSuffix;

    SvxAdjust           eNumAdjust;

    BYTE                nInclUpperLevels;   // Take over numbers from the previous level.
    USHORT              nStart;             // Start of counting

    sal_Unicode         cBullet;            // Symbol
    USHORT              nBulletRelSize;     // percentage size of bullets
    Color               nBulletColor;       // Bullet color

    // mode indicating, if the position and spacing of the list label is
    // determined by the former attributes (nFirstLineOffset, nAbsLSpace,
    // nLSpace and nCharTextDistance) called position and spacing via label
    // width and position (LABEL_WIDTH_AND_POSITION) or by the new attributes
    // (meLabelFollowedBy, mnListtabPos, mnFirstLineIndent and mnIndentAt)
    // called position and spacing via label alignment.
    // Note 1: Attribute <eNumAdjust> is relevant for both modes.
    // Note 2: The values of the former attributes are treated as 0, if mode
    //         LABEL_ALIGNMENT is active.
    SvxNumPositionAndSpaceMode mePositionAndSpaceMode;

    short               nFirstLineOffset;   // First line indent
    short               nAbsLSpace;         // Distance Border<->Number
    short               nLSpace;            // relative to the previous indentation
    short               nCharTextDistance;  // Distance Number<->Text

    // specifies what follows the list label before the text of the first line
    // of the list item starts
    SvxNumLabelFollowedBy       meLabelFollowedBy;
    // specifies an additional list tab stop position for meLabelFollowedBy = LISTTAB
    long                        mnListtabPos;
    // specifies the first line indent
    long                        mnFirstLineIndent;
    // specifies the indent before the text, e.g. in L2R-layout the left margin
    long                        mnIndentAt;

    SvxBrushItem*       pGraphicBrush;
    sal_Int16           eVertOrient;        // vertical alignment of a bitmap

    Size                aGraphicSize;       // Always! in 1/100 mm
    Font*               pBulletFont;        // Pointer to the bullet font

    String              sCharStyleName;     // Character Style

    BitmapEx*           pScaledImageCache;  // Image scaled to aGraphicSize, only cached for WINDOW/VDEV

    DECL_STATIC_LINK( SvxNumberFormat, GraphicArrived, void * );
    virtual void NotifyGraphicArrived();
public:
    SvxNumberFormat( sal_Int16 nNumberingType,
                     SvxNumPositionAndSpaceMode ePositionAndSpaceMode = LABEL_WIDTH_AND_POSITION );
    SvxNumberFormat(const SvxNumberFormat& rFormat);
    SvxNumberFormat(SvStream &rStream);

    virtual ~SvxNumberFormat();

    SvStream&       Store(SvStream &rStream, FontToSubsFontConverter pConverter);

    SvxNumberFormat& operator=( const SvxNumberFormat&  );
    BOOL            operator==( const SvxNumberFormat&  ) const;
    BOOL            operator!=( const SvxNumberFormat& rFmt) const {return !(*this == rFmt);}

    void            SetNumAdjust(SvxAdjust eSet) {eNumAdjust = eSet;}
    SvxAdjust       GetNumAdjust() const {return eNumAdjust;}
    void            SetPrefix(const String& rSet) { sPrefix = rSet;}
    const String&   GetPrefix() const { return sPrefix;}
    void            SetSuffix(const String& rSet) { sSuffix = rSet;}
    const String&   GetSuffix() const { return sSuffix;}

    void                    SetCharFmtName(const String& rSet){ sCharStyleName = rSet; }
    virtual const String&   GetCharFmtName()const;

    void            SetBulletFont(const Font* pFont);
    const Font*     GetBulletFont() const {return pBulletFont;}
    void            SetBulletChar(sal_Unicode cSet){cBullet = cSet;}
    sal_Unicode     GetBulletChar()const {return cBullet;}
    void            SetBulletRelSize(USHORT nSet) {nBulletRelSize = nSet;}
    USHORT          GetBulletRelSize() const { return nBulletRelSize;}
    void            SetBulletColor(Color nSet){nBulletColor = nSet;}
    Color           GetBulletColor()const {return nBulletColor;}

    void            SetIncludeUpperLevels( BYTE nSet ) { nInclUpperLevels = nSet;}
    BYTE            GetIncludeUpperLevels()const  { return nInclUpperLevels;}
    void            SetStart(USHORT nSet) {nStart = nSet;}
    USHORT          GetStart() const {return nStart;}

    virtual void    SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = 0, const sal_Int16* pOrient = 0);
    const SvxBrushItem*         GetBrush() const {return pGraphicBrush;}
    void            SetGraphic( const String& rName );
    virtual void        SetVertOrient(sal_Int16 eSet);
    virtual sal_Int16   GetVertOrient() const;
    void            SetGraphicSize(const Size& rSet) {aGraphicSize = rSet;}
    const Size&     GetGraphicSize() const {return aGraphicSize;}

    SvxNumPositionAndSpaceMode GetPositionAndSpaceMode() const;
    void SetPositionAndSpaceMode( SvxNumPositionAndSpaceMode ePositionAndSpaceMode );

    void            SetLSpace(short nSet) {nLSpace = nSet;}
    short           GetLSpace() const;
    void            SetAbsLSpace(short nSet) {nAbsLSpace = nSet;}
    short           GetAbsLSpace() const;
    void            SetFirstLineOffset(short nSet) { nFirstLineOffset = nSet;}
    short           GetFirstLineOffset() const;
    void            SetCharTextDistance(short nSet) { nCharTextDistance = nSet; }
    short           GetCharTextDistance() const;

    void SetLabelFollowedBy( const SvxNumLabelFollowedBy eLabelFollowedBy );
    SvxNumLabelFollowedBy GetLabelFollowedBy() const;
    void SetListtabPos( const long nListtabPos );
    long GetListtabPos() const;
    void SetFirstLineIndent( const long nFirstLineIndent );
    long GetFirstLineIndent() const;
    void SetIndentAt( const long nIndentAt );
    long GetIndentAt() const;

    static Size     GetGraphicSizeMM100(const Graphic* pGraphic);
    static String   CreateRomanString( ULONG nNo, BOOL bUpper );
};

enum SvxNumRuleType
{
    SVX_RULETYPE_NUMBERING,
    SVX_RULETYPE_OUTLINE_NUMBERING,
    SVX_RULETYPE_PRESENTATION_NUMBERING,
    SVX_RULETYPE_END
};

class EDITENG_DLLPUBLIC SvxNumRule
{
    USHORT              nLevelCount;            // Number of supported levels
    ULONG               nFeatureFlags;          // What is supported?
    SvxNumRuleType      eNumberingType;         // Type of numbering
    BOOL                bContinuousNumbering;   // sequential numbering

    SvxNumberFormat*    aFmts[SVX_MAX_NUM];
    BOOL                aFmtsSet[SVX_MAX_NUM]; // Flags indicating valid levels

    static sal_Int32    nRefCount;
    com::sun::star::lang::Locale aLocale;
public:
    SvxNumRule( ULONG nFeatures,
                USHORT nLevels,
                BOOL bCont,
                SvxNumRuleType eType = SVX_RULETYPE_NUMBERING,
                SvxNumberFormat::SvxNumPositionAndSpaceMode
                        eDefaultNumberFormatPositionAndSpaceMode
                                = SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    SvxNumRule(const SvxNumRule& rCopy);
    SvxNumRule(SvStream &rStream);
    virtual ~SvxNumRule();

    int                     operator==( const SvxNumRule& ) const;
    int                     operator!=( const SvxNumRule& rRule ) const {return !(*this == rRule);}

    SvxNumRule&             operator=( const SvxNumRule&  );

    SvStream&               Store(SvStream &rStream);

    const SvxNumberFormat*  Get(USHORT nLevel)const;
    const SvxNumberFormat&  GetLevel(USHORT nLevel)const;
    void                    SetLevel(USHORT nLevel, const SvxNumberFormat& rFmt, BOOL bIsValid = TRUE);
    void                    SetLevel(USHORT nLevel, const SvxNumberFormat* pFmt);

    BOOL                    IsContinuousNumbering()const
                                            {return bContinuousNumbering;}
    void                    SetContinuousNumbering(BOOL bSet)
                                            {bContinuousNumbering = bSet;}

    USHORT                  GetLevelCount() const {return nLevelCount;}
    BOOL                    IsFeatureSupported(ULONG nFeature) const
                                            {return 0 != (nFeatureFlags & nFeature);}
    ULONG                   GetFeatureFlags() const {return nFeatureFlags;}
    void                    SetFeatureFlag( ULONG nFlag, BOOL bSet = TRUE ) { if(bSet) nFeatureFlags |= nFlag; else nFeatureFlags &= ~nFlag; }

    String                  MakeNumString( const SvxNodeNum&, BOOL bInclStrings = TRUE ) const;

    SvxNumRuleType          GetNumRuleType() const { return eNumberingType; }
    void                    SetNumRuleType( const SvxNumRuleType& rType ) { eNumberingType = rType; }

    BOOL                    UnLinkGraphics();
};

class EDITENG_DLLPUBLIC SvxNumBulletItem : public SfxPoolItem
{
    SvxNumRule*             pNumRule;
public:
    SvxNumBulletItem(SvxNumRule& rRule);
    SvxNumBulletItem(SvxNumRule& rRule, USHORT nWhich );
    SvxNumBulletItem(const SvxNumBulletItem& rCopy);
    virtual ~SvxNumBulletItem();

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
    USHORT  GetVersion( USHORT nFileVersion ) const;
    virtual SvStream&        Store(SvStream &, USHORT nItemVersion ) const;
    virtual int              operator==( const SfxPoolItem& ) const;

    SvxNumRule*             GetNumRule() const {return pNumRule;}

    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};

class SvxNodeNum
{
    USHORT nLevelVal[ SVX_MAX_NUM ];    // Numbers of all levels
    USHORT nSetValue;                   // predetermined number
    BYTE nMyLevel;                      // Current Level
    BOOL bStartNum;                     // Restart numbering

public:
    inline SvxNodeNum( BYTE nLevel = SVX_NO_NUM, USHORT nSetVal = USHRT_MAX );
    inline SvxNodeNum& operator=( const SvxNodeNum& rCpy );

    BYTE GetLevel() const                   { return nMyLevel; }
    void SetLevel( BYTE nVal )              { nMyLevel = nVal; }

    BOOL IsStart() const                    { return bStartNum; }
    void SetStart( BOOL bFlag = TRUE )      { bStartNum = bFlag; }

    USHORT GetSetValue() const              { return nSetValue; }
    void SetSetValue( USHORT nVal )         { nSetValue = nVal; }

    const USHORT* GetLevelVal() const       { return nLevelVal; }
          USHORT* GetLevelVal()             { return nLevelVal; }
};

SvxNodeNum::SvxNodeNum( BYTE nLevel, USHORT nSetVal )
    : nSetValue( nSetVal ), nMyLevel( nLevel ), bStartNum( FALSE )
{
    memset( nLevelVal, 0, sizeof( nLevelVal ) );
}

inline SvxNodeNum& SvxNodeNum::operator=( const SvxNodeNum& rCpy )
{
    nSetValue = rCpy.nSetValue;
    nMyLevel = rCpy.nMyLevel;
    bStartNum = rCpy.bStartNum;

    memcpy( nLevelVal, rCpy.nLevelVal, sizeof( nLevelVal ) );
    return *this;
}


SvxNumRule* SvxConvertNumRule( const SvxNumRule* pRule, USHORT nLevel, SvxNumRuleType eType );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
