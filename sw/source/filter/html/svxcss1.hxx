/*************************************************************************
 *
 *  $RCSfile: svxcss1.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVXCSS1_HXX
#define _SVXCSS1_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autogen
#include <svtools/itemset.hxx>
#endif

#ifndef _SVX_SVXENUM_HXX //autogen
#include <svx/svxenum.hxx>
#endif

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#include <svtools/svstdarr.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

#ifndef _PARCSS1_HXX
#include "parcss1.hxx"
#endif

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

/*  */

struct CSS1PropertyEnum
{
    const sal_Char *pName;  // Wert einer Property
    sal_uInt16 nEnum;           // und der dazugehoerige Wert eines Enums
};


/*  */

class SvxBorderLine;

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


#if !defined( MAC ) && !defined( ICC ) && !defined( BLC )
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

    SfxItemSet *pSheetItemSet;  // der Item-Set fuer Style-Sheets
    SfxItemSet *pItemSet;       // der aktuelle Item-Set
    SvxCSS1MapEntry *pSearchEntry;

    SvxCSS1PropertyInfo *pSheetPropInfo;
    SvxCSS1PropertyInfo *pPropInfo;

    sal_uInt16 nMinFixLineSpace;    // Mindest-Abstand fuer festen Zeilenabstand

    rtl_TextEncoding    eDfltEnc;

    sal_Bool bIgnoreFontFamily;

    void ParseProperty( const String& rProperty,
                        const CSS1Expression *pExpr );

    SvUShorts aWhichMap;        // Which-Map des Parser

protected:

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

    // Die Breite einer Umrandung einstellen
    static void SetBorderWidth( SvxBorderLine& aBorderLine, sal_uInt16 nWidth,
                                sal_Bool bDouble, sal_Bool bTable=sal_False );

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


