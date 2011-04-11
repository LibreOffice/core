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
#ifndef _SVXCSS1_HXX
#define _SVXCSS1_HXX

#include <tools/string.hxx>
#include <svl/itemset.hxx>
#include <editeng/svxenum.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#endif
#include <rtl/textenc.h>
#include "parcss1.hxx"

class SfxItemPool;
class SvxBoxItem;
class FontList;

/*  */

enum SvxCSS1Position
{
    SVX_CSS1_POS_NONE,          // nichts angegeben
    SVX_CSS1_POS_STATIC,        // normal
    SVX_CSS1_POS_ABSOLUTE,      // absolut
    SVX_CSS1_POS_RELATIVE,      // relativ
    SVX_CSS1_POS_END
};


enum SvxCSS1LengthType
{
    SVX_CSS1_LTYPE_NONE,        // nichts angegeben
    SVX_CSS1_LTYPE_AUTO,        // automatisch
    SVX_CSS1_LTYPE_TWIP,        // twip
    SVX_CSS1_LTYPE_PERCENTAGE,  // %-Angabe
    SVX_CSS1_LTYPE_END
};

// Feature: PrintExt
enum SvxCSS1SizeType
{
    SVX_CSS1_STYPE_NONE,        // nichts angegeben
    SVX_CSS1_STYPE_AUTO,        // automatisch
    SVX_CSS1_STYPE_TWIP,        // twip
    SVX_CSS1_STYPE_LANDSCAPE,   // Landscape
    SVX_CSS1_STYPE_PORTRAIT,    // Landscape
    SVX_CSS1_STYPE_END
};

enum SvxCSS1PageBreak
{
    SVX_CSS1_PBREAK_NONE,       // nichts angegeben
    SVX_CSS1_PBREAK_AUTO,       // automatisch
    SVX_CSS1_PBREAK_ALWAYS,     // immer
    SVX_CSS1_PBREAK_AVOID,      // nie
    SVX_CSS1_PBREAK_LEFT,       // naechste Seite ist eine linke
    SVX_CSS1_PBREAK_RIGHT,      // naechste Seite ist eine rechte
    SVX_CSS1_PBREAK_END
};

// /Feature: PrintExt

#define CSS1_SCRIPT_WESTERN 0x01
#define CSS1_SCRIPT_CJK     0x02
#define CSS1_SCRIPT_CTL     0x04
#define CSS1_SCRIPT_ALL     0x07

/*  */

struct CSS1PropertyEnum
{
    const sal_Char *pName;  // Wert einer Property
    sal_uInt16 nEnum;           // und der dazugehoerige Wert eines Enums
};


/*  */

namespace editeng { class SvxBorderLine; }

SV_DECL_PTRARR_DEL( CSS1Selectors, CSS1Selector*, 1, 1 )

#define SVX_CSS1_BORDERINFO_WIDTH 1
#define SVX_CSS1_BORDERINFO_COLOR 2
#define SVX_CSS1_BORDERINFO_STYLE 4

struct SvxCSS1BorderInfo;
class SvxCSS1PropertyInfo
{
    SvxCSS1BorderInfo *aBorderInfos[4];

    void DestroyBorderInfos();

public:

    String aId;             // ID fuer Bookmarks, Rahmen etc.

    sal_Bool bTopMargin : 1;
    sal_Bool bBottomMargin : 1;

    sal_Bool bLeftMargin : 1;
    sal_Bool bRightMargin : 1;
    sal_Bool bTextIndent : 1;

    SvxAdjust eFloat;

    SvxCSS1Position ePosition;

    sal_uInt16 nTopBorderDistance;
    sal_uInt16 nBottomBorderDistance;
    sal_uInt16 nLeftBorderDistance;
    sal_uInt16 nRightBorderDistance;

    long nLeft, nTop;
    long nWidth, nHeight;
    long nLeftMargin, nRightMargin;

    SvxCSS1LengthType eLeftType, eTopType;
    SvxCSS1LengthType eWidthType, eHeightType;

// Feature: PrintExt
    SvxCSS1SizeType eSizeType;

    SvxCSS1PageBreak ePageBreakBefore;
    SvxCSS1PageBreak ePageBreakAfter;
// /Feature: PrintExt

    SvxCSS1PropertyInfo();
    SvxCSS1PropertyInfo( const SvxCSS1PropertyInfo& rProp );
    ~SvxCSS1PropertyInfo();

    void Merge( const SvxCSS1PropertyInfo& rProp );

    void Clear();

    SvxCSS1BorderInfo *GetBorderInfo( sal_uInt16 nLine, sal_Bool bCreate=sal_True );
    void CopyBorderInfo( sal_uInt16 nSrcLine, sal_uInt16 nDstLine, sal_uInt16 nWhat );
    void CopyBorderInfo( sal_uInt16 nCount, sal_uInt16 nWhat );

    void SetBoxItem( SfxItemSet& rItemSet, sal_uInt16 nMinBorderDist,
                     const SvxBoxItem* pDflt=0, sal_Bool bTable = sal_False );

};

class SvxCSS1MapEntry
{
    String aKey;
    SfxItemSet aItemSet;
    SvxCSS1PropertyInfo aPropInfo;

public:

    SvxCSS1MapEntry( SfxItemPool& rPool, const sal_uInt16 *pWhichMap ) :
        aItemSet( rPool, pWhichMap )
    {}

    SvxCSS1MapEntry( const String& rKey, const SfxItemSet& rItemSet,
                     const SvxCSS1PropertyInfo& rProp );


    const SfxItemSet& GetItemSet() const { return aItemSet; }
    SfxItemSet& GetItemSet() { return aItemSet; }

    const SvxCSS1PropertyInfo& GetPropertyInfo() const { return aPropInfo; }
    SvxCSS1PropertyInfo& GetPropertyInfo() { return aPropInfo; }

    const String& GetKey()  const { return aKey; }
    // TODO: ToUpperAscii -> ???
    void SetKey( const String& rKey ) { aKey = rKey; aKey.ToUpperAscii(); }

    friend sal_Bool operator==( const SvxCSS1MapEntry& rE1,
                            const SvxCSS1MapEntry& rE2 );
    friend sal_Bool operator<( const SvxCSS1MapEntry& rE1,
                            const SvxCSS1MapEntry& rE2 );
};

typedef SvxCSS1MapEntry *SvxCSS1MapEntryPtr;
SV_DECL_PTRARR_SORT_DEL( SvxCSS1Map, SvxCSS1MapEntryPtr, 5, 5 )


#if !defined( ICC )
inline sal_Bool operator==( const SvxCSS1MapEntry& rE1, const SvxCSS1MapEntry& rE2 )
{
    return  rE1.aKey==rE2.aKey;
}

inline sal_Bool operator<( const SvxCSS1MapEntry& rE1,  const SvxCSS1MapEntry& rE2 )
{
    return  rE1.aKey<rE2.aKey;
}
#endif

// Diese Klasse bereitet den Output des CSS1-Parsers auf,
// indem die CSS1-Properties in SvxItem(Set)s umgewandelt werden.
// Ausserdem werden die Selektoren samt zugehoeriger Item-Set
// gespeichert.
// Ein abgeleiteter Parser kann dies fuer einzelne Selektoren unterdruecken,
// indem er die Methode StyleParsed ueberlaed.

class SvxCSS1Parser : public CSS1Parser
{
    CSS1Selectors aSelectors;   // Liste der "offenen" Selectoren

    SvxCSS1Map aIds;
    SvxCSS1Map aClasses;
    SvxCSS1Map aPages;
    SvxCSS1Map aTags;

    String sBaseURL;

    SfxItemSet *pSheetItemSet;  // der Item-Set fuer Style-Sheets
    SfxItemSet *pItemSet;       // der aktuelle Item-Set
    SvxCSS1MapEntry *pSearchEntry;

    SvxCSS1PropertyInfo *pSheetPropInfo;
    SvxCSS1PropertyInfo *pPropInfo;

    sal_uInt16 nMinFixLineSpace;    // Mindest-Abstand fuer festen Zeilenabstand

    rtl_TextEncoding    eDfltEnc;
    sal_uInt16          nScriptFlags;

    sal_Bool bIgnoreFontFamily;

    void ParseProperty( const String& rProperty,
                        const CSS1Expression *pExpr );

    SvUShorts aWhichMap;        // Which-Map des Parser

    using CSS1Parser::ParseStyleOption;

protected:

    using CSS1Parser::ParseStyleSheet;

    // Diese Methode wird fuer jeden Selektor mit dem zugehoerigen
    // Item-Set aufgerufen. Fuer einen Selektor koennen mehrere
    // Aufrufe erfolgen.
    // wenn sal_True zuruckgegeben wird, wird der Item-Set bzw. der
    // Selektor nicht mehr gespeichert!
    // Der ItemSet darf entsprechend modifiziert werden!
    // Die Implementierung dieser Methode gibt sal_False zurueck.
    virtual sal_Bool StyleParsed( const CSS1Selector *pSelector,
                              SfxItemSet& rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo );

    // Diese Methode wird aufgerufen, wenn ein Selektor geparst wurde
    // Wenn bFirst gesetzt ist, wird der Inhalt von aItemSet in alle
    // zuletzt angelegten Styles kopiert.
    // Diese Methode sollte in abgleiteten Parsern nicht mehr
    // ueberladen werden!
    virtual sal_Bool SelectorParsed( const CSS1Selector *pSelector,
                                 sal_Bool bFirst );

    // Diese Methode wird fuer jede geparste Property aufgerufen
    // sie fuegt das Item in den ItemSet 'pItemSet' ein
    // Sie sollte in abgeleiteten Parsern nicht mehr ueberladen werden!
    virtual sal_Bool DeclarationParsed( const String& rProperty,
                                    const CSS1Expression *pExpr );

public:


    SvxCSS1Parser( SfxItemPool& rPool,
                    const String& rBaseURL,
                   sal_uInt16 nMinFixLineSp,
                   sal_uInt16 *pWhichIds=0, sal_uInt16 nWhichIds=0 );
    virtual ~SvxCSS1Parser();

    sal_Bool IsIgnoreFontFamily() const { return bIgnoreFontFamily; }
    void SetIgnoreFontFamily( sal_Bool bSet ) { bIgnoreFontFamily = bSet; }

    // Parsen eines Style-Sheets. Fuer jeden gefundenen Selektor
    // wird StyleParsed mit dem entsprechenem Item-Set aufgerufen
    virtual sal_Bool ParseStyleSheet( const String& rIn );

    // Parsen einer Style-Option. Hier wird einfach nur der Item-Set
    // gefuellt.
    sal_Bool ParseStyleOption( const String& rIn, SfxItemSet& rItemSet,
                           SvxCSS1PropertyInfo& rPropInfo );

    // Umwandeln eines Strings in den Wert eines Enums
    static sal_Bool GetEnum( const CSS1PropertyEnum *pPropTable,
                         const String& rValue, sal_uInt16 &rEnum );

    // Pixel in Twips wandeln
    static void PixelToTwip( long &nWidth, long &nHeight );

    // Die Font-Hoehe fuer eine bestimmte Font-Groesse (0-6) ermitteln
    virtual sal_uInt32 GetFontHeight( sal_uInt16 nSize ) const;

    virtual const FontList *GetFontList() const;

    const sal_uInt16 *GetWhichMap() const { return aWhichMap.GetData(); }

    SvxCSS1MapEntry *GetMapEntry( const String& rKey,
                                  const SvxCSS1Map& rMap ) const;

    void InsertMapEntry( const String& rKey, const SfxItemSet& rItemSet,
                         const SvxCSS1PropertyInfo& rProp, SvxCSS1Map& rMap );

    void InsertId( const String& rId, const SfxItemSet& rItemSet,
                   const SvxCSS1PropertyInfo& rProp );

    inline SvxCSS1MapEntry *GetId( const String& rId ) const;

    void InsertClass( const String& rClass, const SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rProp );

    inline SvxCSS1MapEntry *GetClass( const String& rClass ) const;

    inline void InsertPage( const String& rPage, sal_Bool bPseudo,
                            const SfxItemSet& rItemSet,
                            const SvxCSS1PropertyInfo& rProp );

    inline SvxCSS1MapEntry *GetPage( const String& rPage, sal_Bool bPseudo ) const;

    inline SvxCSS1MapEntry *GetPage( sal_uInt16 i ) const { return aPages[i]; }
    sal_uInt16 GetPageCount() const { return aPages.Count(); }

    void InsertTag( const String& rTag, const SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rProp );

    inline SvxCSS1MapEntry *GetTag( const String& rTag ) const;

    void MergeStyles( const SfxItemSet& rSrcSet,
                      const SvxCSS1PropertyInfo& rSrcInfo,
                      SfxItemSet& rTargetSet,
                      SvxCSS1PropertyInfo& rTargetInfo,
                      sal_Bool bSmart );

    sal_uInt16 GetMinFixLineSpace() const { return nMinFixLineSpace; }

    virtual void SetDfltEncoding( rtl_TextEncoding eEnc );
    rtl_TextEncoding GetDfltEncoding() const { return eDfltEnc; }

    sal_Bool IsSetWesternProps() const { return (nScriptFlags & CSS1_SCRIPT_WESTERN) != 0; }
    sal_Bool IsSetCJKProps() const { return (nScriptFlags & CSS1_SCRIPT_CJK) != 0; }
    sal_Bool IsSetCTLProps() const { return (nScriptFlags & CSS1_SCRIPT_CTL) != 0; }

    const String& GetBaseURL() const { return sBaseURL;}

};

inline void SvxCSS1Parser::InsertId( const String& rId,
                                     const SfxItemSet& rItemSet,
                                     const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rId, rItemSet, rProp, aIds );
}

inline SvxCSS1MapEntry *SvxCSS1Parser::GetId( const String& rId ) const
{
    return GetMapEntry( rId, aIds );
}

inline void SvxCSS1Parser::InsertClass( const String& rClass,
                                        const SfxItemSet& rItemSet,
                                        const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rClass, rItemSet, rProp, aClasses );
}

inline SvxCSS1MapEntry *SvxCSS1Parser::GetClass( const String& rClass ) const
{
    return GetMapEntry( rClass, aClasses );
}

inline void SvxCSS1Parser::InsertPage( const String& rPage,
                                       sal_Bool bPseudo,
                                       const SfxItemSet& rItemSet,
                                       const SvxCSS1PropertyInfo& rProp )
{
    String aKey( rPage );
    if( bPseudo )
        aKey.Insert( ':', 0 );
    InsertMapEntry( aKey, rItemSet, rProp, aPages );
}

inline SvxCSS1MapEntry *SvxCSS1Parser::GetPage( const String& rPage,
                                                sal_Bool bPseudo ) const
{
    String aKey( rPage );
    if( bPseudo )
        aKey.Insert( ':', 0 );
    return GetMapEntry( aKey, aPages );
}

inline void SvxCSS1Parser::InsertTag( const String& rTag,
                                        const SfxItemSet& rItemSet,
                                        const SvxCSS1PropertyInfo& rProp )
{
    InsertMapEntry( rTag, rItemSet, rProp, aTags );
}

inline SvxCSS1MapEntry *SvxCSS1Parser::GetTag( const String& rTag ) const
{
    return GetMapEntry( rTag, aTags );
}


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
