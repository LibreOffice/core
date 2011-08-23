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

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <bf_svtools/poolitem.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX //autogen
#include <bf_svx/svxenum.hxx>
#endif
#ifndef _SV_GEN_HXX //autogen
#include <tools/gen.hxx>
#endif
#ifndef _SVX_NUMDEF_HXX //autogen
#include <bf_svx/numdef.hxx>
#endif
#ifndef _COLOR_HXX //autogen
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
#include <unotools/fontcvt.hxx>
#endif
namespace com{namespace sun{ namespace star{
    namespace text{
        class XNumberingFormatter;
    }
}}}
class Font;
class Graphic;
class BitmapEx;
namespace binfilter {

class SvxBrushItem;
class SvxNodeNum;


// -----------------------------------------------------------------------
//Feature-Flags (only USHORT!)
#define NUM_CONTINUOUS 			0x0001 // fortlaufende Numerierung moeglich?
#define NUM_CHAR_TEXT_DISTANCE 	0x0002 // Abstand Symbol<->Text?
#define NUM_CHAR_STYLE			0x0004 // Zeichenvorlagen?
#define NUM_BULLET_REL_SIZE		0x0008 // rel. Bulletgroesse?
#define NUM_BULLET_COLOR		0x0010 // Bullet color
#define NUM_HIDDEN_SYMBOLS		0x0020 // hidden symbols
#define NUM_SYMBOL_ALIGNMENT	0x0040 // alignment soll unter den Optionen angezeigt werden
#define NUM_NO_NUMBERS			0x0080 // Numberierungen sind nicht erlaubt
#define NUM_ENABLE_LINKED_BMP	0x0100 // linked bitmaps are available
#define NUM_ENABLE_EMBEDDED_BMP	0x0200 // embedded bitmaps are available

#define SVX_NO_NUM				200 // Markierung fuer keine Numerierung
#define SVX_NO_NUMLEVEL         0x20

#define NUMITEM_VERSION_01		0x01
#define NUMITEM_VERSION_02		0x02
#define NUMITEM_VERSION_03		0x03

#define LINK_TOKEN 	0x80 //indicate linked bitmaps - for use in dialog only
using namespace ::com::sun::star::style;

class SvxNumberType
{
    static sal_Int32 nRefCount;
    static ::com::sun::star::uno::Reference< ::com::sun::star::text::XNumberingFormatter> xFormatter;

    sal_Int16		nNumType;
    sal_Bool		bShowSymbol;		// Symbol auch anzeigen?

public:
    SvxNumberType(sal_Int16 nType = (sal_Int16) NumberingType::ARABIC);
    SvxNumberType(const SvxNumberType& rType);
    ~SvxNumberType();

    String 			GetNumStr( ULONG nNo ) const;
    String 			GetNumStr( ULONG nNo, const ::com::sun::star::lang::Locale& rLocale ) const;

    void			SetNumberingType(sal_Int16 nSet) {nNumType = nSet;}
    sal_Int16		GetNumberingType() const {return nNumType;}

    void 			SetShowSymbol(sal_Bool bSet) {bShowSymbol = bSet;}
    sal_Bool		IsShowSymbol()const{return bShowSymbol;}

    sal_Bool		IsTxtFmt() const
                    {
                        return ::com::sun::star::style::NumberingType::NUMBER_NONE != nNumType &&
                               ::com::sun::star::style::NumberingType::CHAR_SPECIAL != nNumType &&
                               ::com::sun::star::style::NumberingType::BITMAP != nNumType;
                    }
};

class SvxNumberFormat : public SvxNumberType
{
    String 				sPrefix;
    String 				sSuffix;

    SvxAdjust			eNumAdjust;

    BYTE				nInclUpperLevels;	//Nummern aus der vorigen Ebenen uebernehmen
    USHORT 				nStart;				//Start der Zaehlung

    sal_Unicode 		cBullet;			//Symbol
    USHORT				nBulletRelSize;		//proz. Groesse des Bullets
    Color				nBulletColor;		//Bullet color

    short 				nFirstLineOffset;   //Erstzeileneinzug
    short 				nAbsLSpace;			//Abstand Rand<->Nummer
    short 				nLSpace;			//relative Einrueckung zum Vorgaenger
    short				nCharTextDistance;	//Abstand Nummer<->Text

    SvxBrushItem* 	 	pGraphicBrush; 			//
    SvxFrameVertOrient 	eVertOrient;		// vert. Ausrichtung einer Bitmap

    Size 				aGraphicSize;		// immer! in 1/100 mm
    Font* 				pBulletFont;		// Pointer auf den BulletFont

    String				sCharStyleName;		// Zeichenvorlage

    BitmapEx*			pScaledImageCache;	// Image scaled to aGraphicSize, only cached for WINDOW/VDEV

//	void GetCharStr( ULONG nNo, String& rStr ) const;
//	void GetCharStrN( ULONG nNo, String& rStr ) const;
public:
    SvxNumberFormat(sal_Int16 nNumberingType);
    SvxNumberFormat(const SvxNumberFormat& rFormat);
    SvxNumberFormat(SvStream &rStream);
    ~SvxNumberFormat();

    SvStream&       Store(SvStream &rStream, FontToSubsFontConverter pConverter);

    SvxNumberFormat& operator=( const SvxNumberFormat&  );
    BOOL 			operator==( const SvxNumberFormat&  ) const;
    BOOL            operator!=( const SvxNumberFormat& rFmt) const {return !(*this == rFmt);}

    void			SetNumAdjust(SvxAdjust eSet) {eNumAdjust = eSet;}
    SvxAdjust		GetNumAdjust() const {return eNumAdjust;}
    void 			SetPrefix(const String& rSet) { sPrefix = rSet;}
    const String&	GetPrefix() const { return sPrefix;}
    void            SetSuffix(const String& rSet) { sSuffix = rSet;}
    const String&	GetSuffix() const { return sSuffix;}

    void					SetCharFmtName(const String& rSet){ sCharStyleName = rSet; }
    virtual const String&	GetCharFmtName()const;

    void			SetBulletFont(const Font* pFont);
    const Font* 	GetBulletFont() const {return pBulletFont;}
    void			SetBulletChar(sal_Unicode cSet){cBullet = cSet;}
    sal_Unicode 	GetBulletChar()const {return cBullet;}
    void			SetBulletRelSize(USHORT nSet) {nBulletRelSize = nSet;}
    USHORT			GetBulletRelSize() const { return nBulletRelSize;}
    void			SetBulletColor(Color nSet){nBulletColor = nSet;}
    Color			GetBulletColor()const {return nBulletColor;}

    void			SetIncludeUpperLevels( BYTE nSet ) { nInclUpperLevels = nSet;}
    BYTE 			GetIncludeUpperLevels()const  { return nInclUpperLevels;}
    void 			SetStart(USHORT nSet) {nStart = nSet;}
    USHORT			GetStart() const {return nStart;}

    virtual void	SetGraphicBrush( const SvxBrushItem* pBrushItem, const Size* pSize = 0, const SvxFrameVertOrient* pOrient = 0);
    const SvxBrushItem* 		GetBrush() const {return pGraphicBrush;}
    virtual SvxFrameVertOrient 	GetVertOrient() const;
    void			SetGraphicSize(const Size& rSet) {aGraphicSize = rSet;}
    const Size&		GetGraphicSize() const {return aGraphicSize;}

    void			SetLSpace(short nSet) {nLSpace = nSet;}
    short 			GetLSpace() const {return nLSpace;}
    void			SetAbsLSpace(short nSet) {nAbsLSpace = nSet;}
    short 			GetAbsLSpace() const {return nAbsLSpace;}
    void 			SetFirstLineOffset(short nSet) { nFirstLineOffset = nSet;}
    short 			GetFirstLineOffset() const {return nFirstLineOffset; }
    void			SetCharTextDistance(short nSet) { nCharTextDistance = nSet; }
    short			GetCharTextDistance() const {return nCharTextDistance;}

    static String 	CreateRomanString( ULONG nNo, BOOL bUpper );
};

enum SvxNumRuleType
{
    SVX_RULETYPE_NUMBERING,
    SVX_RULETYPE_OUTLINE_NUMBERING,
    SVX_RULETYPE_PRESENTATION_NUMBERING,
    SVX_RULETYPE_END
};

class SvxNumRule
{
    USHORT 				nLevelCount;            // Anzahl der unterstuetzten Levels
    ULONG 				nFeatureFlags;          // was wird unterstuetzt?
    SvxNumRuleType		eNumberingType;         // was fuer eine Numerierung
    BOOL 				bContinuousNumbering;	// fortlaufende Numerierung

    SvxNumberFormat* 	aFmts[SVX_MAX_NUM];
    BOOL			 	aFmtsSet[SVX_MAX_NUM]; //Flags ueber Gueltigkeit der Ebenen

    static sal_Int32	nRefCount;
    ::com::sun::star::lang::Locale aLocale;
public:
    SvxNumRule(ULONG nFeatures, USHORT nLevels, BOOL bCont, SvxNumRuleType eType = SVX_RULETYPE_NUMBERING);
    SvxNumRule(const SvxNumRule& rCopy);
    SvxNumRule(SvStream &rStream);
    virtual ~SvxNumRule();

    int              		operator==( const SvxNumRule& ) const;
    int              		operator!=( const SvxNumRule& rRule ) const {return !(*this == rRule);}


    SvStream&				Store(SvStream &rStream);

    const SvxNumberFormat* 	Get(USHORT nLevel)const;
    const SvxNumberFormat& 	GetLevel(USHORT nLevel)const;
    void					SetLevel(USHORT nLevel, const SvxNumberFormat& rFmt, BOOL bIsValid = TRUE);

    BOOL					IsContinuousNumbering()const
                                            {return bContinuousNumbering;}
    void					SetContinuousNumbering(BOOL bSet)
                                            {bContinuousNumbering = bSet;}

    USHORT 					GetLevelCount() const {return nLevelCount;}
    BOOL					IsFeatureSupported(ULONG nFeature) const
                                            {return 0 != (nFeatureFlags & nFeature);}
    ULONG 					GetFeatureFlags() const {return nFeatureFlags;}
    void					SetFeatureFlag( ULONG nFlag, BOOL bSet = TRUE ) { if(bSet) nFeatureFlags |= nFlag; else nFeatureFlags &= ~nFlag; }


    SvxNumRuleType			GetNumRuleType() const { return eNumberingType; }
    void                    SetNumRuleType( const SvxNumRuleType& rType ) { eNumberingType = rType; }

    BOOL					UnLinkGraphics();
};
/* -----------------27.10.98 13:04-------------------
 *
 * --------------------------------------------------*/
class SvxNumBulletItem : public SfxPoolItem
{
    SvxNumRule* 			pNumRule;
public:
    SvxNumBulletItem(SvxNumRule& rRule);
    SvxNumBulletItem(SvxNumRule& rRule, USHORT nWhich );
    SvxNumBulletItem(const SvxNumBulletItem& rCopy);
    virtual ~SvxNumBulletItem();

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*	 Create(SvStream &, USHORT) const;
    virtual SvStream&		 Store(SvStream &, USHORT nItemVersion ) const;
    virtual USHORT			 GetVersion( USHORT nFileVersion ) const;
    virtual int              operator==( const SfxPoolItem& ) const;

    SvxNumRule* 			GetNumRule() const {return pNumRule;}

    virtual	sal_Bool		PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMemberId = 0 );
};
/* -----------------28.10.98 15:21-------------------
 *
 * --------------------------------------------------*/
class SvxNodeNum
{
    USHORT nLevelVal[ SVX_MAX_NUM ];	// Nummern aller Levels
    USHORT nSetValue;					// vorgegebene Nummer
    BYTE nMyLevel;						// akt. Level
    BOOL bStartNum;						// Numerierung neu starten

public:
    inline SvxNodeNum( BYTE nLevel = SVX_NO_NUM, USHORT nSetVal = USHRT_MAX );
    inline SvxNodeNum& operator=( const SvxNodeNum& rCpy );

//	BOOL operator==( const SwNodeNum& ) const;

    BYTE GetLevel() const 					{ return nMyLevel; }
    void SetLevel( BYTE nVal )  			{ nMyLevel = nVal; }

    BOOL IsStart() const					{ return bStartNum; }
    void SetStart( BOOL bFlag = TRUE ) 		{ bStartNum = bFlag; }

    USHORT GetSetValue() const 				{ return nSetValue; }
    void SetSetValue( USHORT nVal )  		{ nSetValue = nVal; }

    const USHORT* GetLevelVal() const 		{ return nLevelVal; }
          USHORT* GetLevelVal() 	 		{ return nLevelVal; }
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

}//end of namespace binfilter
#endif

