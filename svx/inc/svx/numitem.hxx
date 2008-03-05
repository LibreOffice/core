/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numitem.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:38:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_NUMITEM_HXX
#define _SVX_NUMITEM_HXX

// include ---------------------------------------------------------------

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif
#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SVX_NUMDEF_HXX //autogen
#include <svx/numdef.hxx>
#endif
#ifndef _SV_COLOR_HXX //autogen
#include <tools/color.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_STYLE_NUMBERINGTYPE_HPP_
#include <com/sun/star/style/NumberingType.hpp>
#endif
#ifndef _SV_FONTCVT_HXX
#include <vcl/fontcvt.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

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
#define NUM_CONTINUOUS          0x0001 // fortlaufende Numerierung moeglich?
#define NUM_CHAR_TEXT_DISTANCE  0x0002 // Abstand Symbol<->Text?
#define NUM_CHAR_STYLE          0x0004 // Zeichenvorlagen?
#define NUM_BULLET_REL_SIZE     0x0008 // rel. Bulletgroesse?
#define NUM_BULLET_COLOR        0x0010 // Bullet color
#define NUM_SYMBOL_ALIGNMENT    0x0040 // alignment soll unter den Optionen angezeigt werden
#define NUM_NO_NUMBERS          0x0080 // Numberierungen sind nicht erlaubt
#define NUM_ENABLE_LINKED_BMP   0x0100 // linked bitmaps are available
#define NUM_ENABLE_EMBEDDED_BMP 0x0200 // embedded bitmaps are available

#define SVX_NO_NUM              200 // Markierung fuer keine Numerierung
#define SVX_NO_NUMLEVEL         0x20

// --> OD 2008-01-10 #newlistlevelattrs# - no longer used
//#define NUMITEM_VERSION_01        0x01
//#define NUMITEM_VERSION_02        0x02
//#define NUMITEM_VERSION_03        0x03
// <--

#define LINK_TOKEN  0x80 //indicate linked bitmaps - for use in dialog only
class SVX_DLLPUBLIC SvxNumberType
{
    static sal_Int32 nRefCount;
    static com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter> xFormatter;

    sal_Int16       nNumType;
    sal_Bool        bShowSymbol;        // Symbol auch anzeigen?

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

class SVX_DLLPUBLIC SvxNumberFormat : public SvxNumberType
{
public:
    // --> OD 2008-01-08 #newlistlevelattrs#
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
    // <--

private:
    String              sPrefix;
    String              sSuffix;

    SvxAdjust           eNumAdjust;

    BYTE                nInclUpperLevels;   //Nummern aus der vorigen Ebenen uebernehmen
    USHORT              nStart;             //Start der Zaehlung

    sal_Unicode         cBullet;            //Symbol
    USHORT              nBulletRelSize;     //proz. Groesse des Bullets
    Color               nBulletColor;       //Bullet color

    // --> OD 2008-01-08 #newlistlevelattrs#
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
    // <--

    short               nFirstLineOffset;   //Erstzeileneinzug
    short               nAbsLSpace;         //Abstand Rand<->Nummer
    short               nLSpace;            //relative Einrueckung zum Vorgaenger
    short               nCharTextDistance;  //Abstand Nummer<->Text

    // --> OD 2008-01-08 #newlistlevelattrs#
    // specifies what follows the list label before the text of the first line
    // of the list item starts
    SvxNumLabelFollowedBy       meLabelFollowedBy;
    // specifies an additional list tab stop position for meLabelFollowedBy = LISTTAB
    long                        mnListtabPos;
    // specifies the first line indent
    long                        mnFirstLineIndent;
    // specifies the indent before the text, e.g. in L2R-layout the left margin
    long                        mnIndentAt;
    // <--

    SvxBrushItem*       pGraphicBrush;          //
    sal_Int16           eVertOrient;        // vert. Ausrichtung einer Bitmap

    Size                aGraphicSize;       // immer! in 1/100 mm
    Font*               pBulletFont;        // Pointer auf den BulletFont

    String              sCharStyleName;     // Zeichenvorlage

    BitmapEx*           pScaledImageCache;  // Image scaled to aGraphicSize, only cached for WINDOW/VDEV

    DECL_STATIC_LINK( SvxNumberFormat, GraphicArrived, void * );
    virtual void NotifyGraphicArrived();
public:
    // --> OD 2008-01-09 #newlistlevelattrs#
    SvxNumberFormat( sal_Int16 nNumberingType,
                     SvxNumPositionAndSpaceMode ePositionAndSpaceMode = LABEL_WIDTH_AND_POSITION );
    // <--
    SvxNumberFormat(const SvxNumberFormat& rFormat);
    // --> OD 2008-01-09 #newlistlevelattrs# - no longer used
//    SvxNumberFormat(SvStream &rStream);
    // <--
    virtual ~SvxNumberFormat();

    // --> OD 2008-01-09 #newlistlevelattrs# - no longer used
//    SvStream&       Store(SvStream &rStream, FontToSubsFontConverter pConverter);
    // <--

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

    // --> OD 2008-01-09 #newlistlevelattrs#
    SvxNumPositionAndSpaceMode GetPositionAndSpaceMode() const;
    void SetPositionAndSpaceMode( SvxNumPositionAndSpaceMode ePositionAndSpaceMode );
    // <--

    void            SetLSpace(short nSet) {nLSpace = nSet;}
    // --> OD 2008-01-09 #newlistlevelattrs#
    short           GetLSpace() const;
    // <--
    void            SetAbsLSpace(short nSet) {nAbsLSpace = nSet;}
    // --> OD 2008-01-09 #newlistlevelattrs#
    short           GetAbsLSpace() const;
    // <--
    void            SetFirstLineOffset(short nSet) { nFirstLineOffset = nSet;}
    // --> OD 2008-01-09 #newlistlevelattrs#
    short           GetFirstLineOffset() const;
    // <--
    void            SetCharTextDistance(short nSet) { nCharTextDistance = nSet; }
    // --> OD 2008-01-09 #newlistlevelattrs#
    short           GetCharTextDistance() const;
    // <--

    // --> OD 2008-01-09 #newlistlevelattrs#
    void SetLabelFollowedBy( const SvxNumLabelFollowedBy eLabelFollowedBy );
    SvxNumLabelFollowedBy GetLabelFollowedBy() const;
    void SetListtabPos( const long nListtabPos );
    long GetListtabPos() const;
    void SetFirstLineIndent( const long nFirstLineIndent );
    long GetFirstLineIndent() const;
    void SetIndentAt( const long nIndentAt );
    long GetIndentAt() const;
    // <--

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

class SVX_DLLPUBLIC SvxNumRule
{
    USHORT              nLevelCount;            // Anzahl der unterstuetzten Levels
    ULONG               nFeatureFlags;          // was wird unterstuetzt?
    SvxNumRuleType      eNumberingType;         // was fuer eine Numerierung
    BOOL                bContinuousNumbering;   // fortlaufende Numerierung

    SvxNumberFormat*    aFmts[SVX_MAX_NUM];
    BOOL                aFmtsSet[SVX_MAX_NUM]; //Flags ueber Gueltigkeit der Ebenen

    static sal_Int32    nRefCount;
    com::sun::star::lang::Locale aLocale;
public:
    // --> OD 2008-02-11 #newlistlevelattrs#
    SvxNumRule( ULONG nFeatures,
                USHORT nLevels,
                BOOL bCont,
                SvxNumRuleType eType = SVX_RULETYPE_NUMBERING,
                SvxNumberFormat::SvxNumPositionAndSpaceMode
                        eDefaultNumberFormatPositionAndSpaceMode
                                = SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    // <--
    SvxNumRule(const SvxNumRule& rCopy);
    // --> OD 2008-01-09 #newlistlevelattrs# - no longer used
//  SvxNumRule(SvStream &rStream);
    // <--
    virtual ~SvxNumRule();

    int                     operator==( const SvxNumRule& ) const;
    int                     operator!=( const SvxNumRule& rRule ) const {return !(*this == rRule);}

    SvxNumRule&             operator=( const SvxNumRule&  );

    // --> OD 2008-01-09 #newlistlevelattrs# - no longer used
//    SvStream&               Store(SvStream &rStream);
    // <--

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
/* -----------------27.10.98 13:04-------------------
 *
 * --------------------------------------------------*/
class SVX_DLLPUBLIC SvxNumBulletItem : public SfxPoolItem
{
    SvxNumRule*             pNumRule;
public:
    SvxNumBulletItem(SvxNumRule& rRule);
    SvxNumBulletItem(SvxNumRule& rRule, USHORT nWhich );
    SvxNumBulletItem(const SvxNumBulletItem& rCopy);
    virtual ~SvxNumBulletItem();

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    // --> OD 2008-01-09 #newlistlevelattrs# - no longer used
//    virtual SfxPoolItem*     Create(SvStream &, USHORT) const;
//    virtual SvStream&        Store(SvStream &, USHORT nItemVersion ) const;
//    virtual USHORT           GetVersion( USHORT nFileVersion ) const;
    // <--
    virtual int              operator==( const SfxPoolItem& ) const;

    SvxNumRule*             GetNumRule() const {return pNumRule;}

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};
/* -----------------28.10.98 15:21-------------------
 *
 * --------------------------------------------------*/
class SvxNodeNum
{
    USHORT nLevelVal[ SVX_MAX_NUM ];    // Nummern aller Levels
    USHORT nSetValue;                   // vorgegebene Nummer
    BYTE nMyLevel;                      // akt. Level
    BOOL bStartNum;                     // Numerierung neu starten

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

/* --------------------------------------------------
 *
 * --------------------------------------------------*/
SvxNumRule* SvxConvertNumRule( const SvxNumRule* pRule, USHORT nLevel, SvxNumRuleType eType );

#endif

