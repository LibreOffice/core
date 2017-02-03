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
#ifndef INCLUDED_SW_SOURCE_FILTER_HTML_SVXCSS1_HXX
#define INCLUDED_SW_SOURCE_FILTER_HTML_SVXCSS1_HXX

#include <svl/itemset.hxx>
#include <editeng/svxenum.hxx>
#include <rtl/textenc.h>
#include "parcss1.hxx"
#include <o3tl/typed_flags_set.hxx>

#include <memory>
#include <vector>
#include <map>

class SfxItemPool;
class SvxBoxItem;
class FontList;

enum SvxCSS1Position
{
    SVX_CSS1_POS_NONE,          // nichts angegeben
    SVX_CSS1_POS_STATIC,        // normal
    SVX_CSS1_POS_ABSOLUTE,      // absolut
    SVX_CSS1_POS_RELATIVE,      // relativ
};

enum SvxCSS1LengthType
{
    SVX_CSS1_LTYPE_NONE,        // nichts angegeben
    SVX_CSS1_LTYPE_AUTO,        // automatisch
    SVX_CSS1_LTYPE_TWIP,        // twip
    SVX_CSS1_LTYPE_PERCENTAGE,  // %-Angabe
};

// Feature: PrintExt
enum SvxCSS1SizeType
{
    SVX_CSS1_STYPE_NONE,        // nichts angegeben
    SVX_CSS1_STYPE_AUTO,        // automatisch
    SVX_CSS1_STYPE_TWIP,        // twip
    SVX_CSS1_STYPE_LANDSCAPE,   // Landscape
    SVX_CSS1_STYPE_PORTRAIT,    // Landscape
};

enum SvxCSS1PageBreak
{
    SVX_CSS1_PBREAK_NONE,       // nichts angegeben
    SVX_CSS1_PBREAK_AUTO,       // automatisch
    SVX_CSS1_PBREAK_ALWAYS,     // immer
    SVX_CSS1_PBREAK_AVOID,      // nie
    SVX_CSS1_PBREAK_LEFT,       // naechste Seite ist eine linke
    SVX_CSS1_PBREAK_RIGHT,      // naechste Seite ist eine rechte
};


enum class Css1ScriptFlags {
    Western = 0x01,
    CJK     = 0x02,
    CTL     = 0x04,
    AllMask = Western | CJK | CTL,
};
namespace o3tl {
    template<> struct typed_flags<Css1ScriptFlags> : is_typed_flags<Css1ScriptFlags, 0x07> {};
}

struct CSS1PropertyEnum
{
    const sal_Char *pName;  // Wert einer Property
    sal_uInt16 nEnum;       // und der dazugehoerige Wert eines Enums
};

namespace editeng { class SvxBorderLine; }

#define SVX_CSS1_BORDERINFO_WIDTH 1
#define SVX_CSS1_BORDERINFO_COLOR 2
#define SVX_CSS1_BORDERINFO_STYLE 4

struct SvxCSS1BorderInfo;
class SvxCSS1PropertyInfo
{
    SvxCSS1BorderInfo *m_aBorderInfos[4];

    void DestroyBorderInfos();

public:

    OUString m_aId;             // ID fuer Bookmarks, Rahmen etc.

    bool m_bTopMargin : 1;
    bool m_bBottomMargin : 1;

    bool m_bLeftMargin : 1;
    bool m_bRightMargin : 1;
    bool m_bTextIndent : 1;

    SvxAdjust m_eFloat;

    SvxCSS1Position m_ePosition;

    sal_uInt16 m_nTopBorderDistance;
    sal_uInt16 m_nBottomBorderDistance;
    sal_uInt16 m_nLeftBorderDistance;
    sal_uInt16 m_nRightBorderDistance;

    sal_uInt16 m_nColumnCount;

    long m_nLeft, m_nTop;
    long m_nWidth, m_nHeight;
    long m_nLeftMargin, m_nRightMargin;

    SvxCSS1LengthType m_eLeftType, m_eTopType;
    SvxCSS1LengthType m_eWidthType, m_eHeightType;

    SvxCSS1SizeType m_eSizeType;

    SvxCSS1PageBreak m_ePageBreakBefore;
    SvxCSS1PageBreak m_ePageBreakAfter;

    SvxCSS1PropertyInfo();
    SvxCSS1PropertyInfo( const SvxCSS1PropertyInfo& rProp );
    ~SvxCSS1PropertyInfo();

    void Merge( const SvxCSS1PropertyInfo& rProp );

    void Clear();

    SvxCSS1BorderInfo *GetBorderInfo( SvxBoxItemLine nLine, bool bCreate=true );
    void CopyBorderInfo( SvxBoxItemLine nSrcLine, SvxBoxItemLine nDstLine, sal_uInt16 nWhat );
    void CopyBorderInfo( sal_uInt16 nCount, sal_uInt16 nWhat );

    void SetBoxItem( SfxItemSet& rItemSet, sal_uInt16 nMinBorderDist,
                     const SvxBoxItem* pDflt=nullptr );

};

class SvxCSS1MapEntry
{
    SfxItemSet aItemSet;
    SvxCSS1PropertyInfo aPropInfo;

public:
    SvxCSS1MapEntry( SfxItemPool& rPool, const sal_uInt16 *pWhichMap ) :
        aItemSet( rPool, pWhichMap )
    {}

    SvxCSS1MapEntry( const SfxItemSet& rItemSet,
                     const SvxCSS1PropertyInfo& rProp );

    const SfxItemSet& GetItemSet() const { return aItemSet; }
    SfxItemSet& GetItemSet() { return aItemSet; }

    const SvxCSS1PropertyInfo& GetPropertyInfo() const { return aPropInfo; }
    SvxCSS1PropertyInfo& GetPropertyInfo() { return aPropInfo; }
};

// Diese Klasse bereitet den Output des CSS1-Parsers auf,
// indem die CSS1-Properties in SvxItem(Set)s umgewandelt werden.
// Ausserdem werden die Selektoren samt zugehoeriger Item-Set
// gespeichert.
// Ein abgeleiteter Parser kann dies fuer einzelne Selektoren unterdruecken,
// indem er die Methode StyleParsed ueberlaed.

class SvxCSS1Parser : public CSS1Parser
{
    typedef std::vector<std::unique_ptr<CSS1Selector>> CSS1Selectors;
    typedef std::map<OUString, std::unique_ptr<SvxCSS1MapEntry>> CSS1Map;
    CSS1Selectors m_Selectors;   // List of "open" Selectors

    CSS1Map m_Ids;
    CSS1Map m_Classes;
    CSS1Map m_Pages;
    CSS1Map m_Tags;

    OUString sBaseURL;

    SfxItemSet *pSheetItemSet;  // der Item-Set fuer Style-Sheets
    SfxItemSet *pItemSet;       // der aktuelle Item-Set
    SvxCSS1MapEntry *pSearchEntry;

    SvxCSS1PropertyInfo *pSheetPropInfo;
    SvxCSS1PropertyInfo *pPropInfo;

    sal_uInt16 nMinFixLineSpace;    // Mindest-Abstand fuer festen Zeilenabstand

    rtl_TextEncoding    eDfltEnc;
    Css1ScriptFlags     nScriptFlags;

    bool bIgnoreFontFamily;

    void ParseProperty( const OUString& rProperty,
                        const CSS1Expression *pExpr );

    std::vector<sal_uInt16> aWhichMap;        // Which-Map des Parser

    using CSS1Parser::ParseStyleOption;

protected:

    using CSS1Parser::ParseStyleSheet;

    // Diese Methode wird fuer jeden Selektor mit dem zugehoerigen
    // Item-Set aufgerufen. Fuer einen Selektor koennen mehrere
    // Aufrufe erfolgen.
    // wenn true zuruckgegeben wird, wird der Item-Set bzw. der
    // Selektor nicht mehr gespeichert!
    // Der ItemSet darf entsprechend modifiziert werden!
    // Die Implementierung dieser Methode gibt false zurueck.
    virtual void StyleParsed( const CSS1Selector *pSelector,
                              SfxItemSet& rItemSet,
                              SvxCSS1PropertyInfo& rPropInfo );

    /// Will be called when a Selector is parsed.  If bFirst is true,
    /// the content of the aItemSet will be copied into all recently
    /// created Styles.
    /// Derived classes should not override this method!
    virtual bool SelectorParsed( CSS1Selector *pSelector, bool bFirst ) override;

    /// Will be called for every parsed Property.  Adds the item to the
    /// pItemSet.
    /// Derived classes should not override this method!
    virtual bool DeclarationParsed( const OUString& rProperty,
                                    const CSS1Expression *pExpr ) override;

public:

    SvxCSS1Parser( SfxItemPool& rPool,
                    const OUString& rBaseURL,
                   sal_uInt16 *pWhichIds, sal_uInt16 nWhichIds=0 );
    virtual ~SvxCSS1Parser() override;

    bool IsIgnoreFontFamily() const { return bIgnoreFontFamily; }
    void SetIgnoreFontFamily( bool bSet ) { bIgnoreFontFamily = bSet; }

    // Parsen eines Style-Sheets. Fuer jeden gefundenen Selektor
    // wird StyleParsed mit dem entsprechenem Item-Set aufgerufen
    virtual bool ParseStyleSheet( const OUString& rIn );

    // Parsen einer Style-Option. Hier wird einfach nur der Item-Set
    // gefuellt.
    void ParseStyleOption( const OUString& rIn, SfxItemSet& rItemSet,
                           SvxCSS1PropertyInfo& rPropInfo );

    // Umwandeln eines Strings in den Wert eines Enums
    static bool GetEnum( const CSS1PropertyEnum *pPropTable,
                         const OUString& rValue, sal_uInt16 &rEnum );

    // Pixel in Twips wandeln
    static void PixelToTwip( long &nWidth, long &nHeight );

    // Die Font-Hoehe fuer eine bestimmte Font-Groesse (0-6) ermitteln
    virtual sal_uInt32 GetFontHeight( sal_uInt16 nSize ) const;

    virtual const FontList *GetFontList() const;

    const sal_uInt16 *GetWhichMap() const { return aWhichMap.data(); }

    static void InsertMapEntry( const OUString& rKey, const SfxItemSet& rItemSet,
                         const SvxCSS1PropertyInfo& rProp, CSS1Map& rMap );

    void InsertId( const OUString& rId, const SfxItemSet& rItemSet,
                   const SvxCSS1PropertyInfo& rProp );

    const SvxCSS1MapEntry* GetId( const OUString& rId ) const;

    void InsertClass( const OUString& rClass, const SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rProp );

    const SvxCSS1MapEntry* GetClass( const OUString& rClass ) const;

    void InsertPage( const OUString& rPage, bool bPseudo,
                            const SfxItemSet& rItemSet,
                            const SvxCSS1PropertyInfo& rProp );

    SvxCSS1MapEntry* GetPage( const OUString& rPage, bool bPseudo );

    void InsertTag( const OUString& rTag, const SfxItemSet& rItemSet,
                      const SvxCSS1PropertyInfo& rProp );

    SvxCSS1MapEntry* GetTag( const OUString& rTag );

    static void MergeStyles( const SfxItemSet& rSrcSet,
                      const SvxCSS1PropertyInfo& rSrcInfo,
                      SfxItemSet& rTargetSet,
                      SvxCSS1PropertyInfo& rTargetInfo,
                      bool bSmart );

    sal_uInt16 GetMinFixLineSpace() const { return nMinFixLineSpace; }

    virtual void SetDfltEncoding( rtl_TextEncoding eEnc );
    rtl_TextEncoding GetDfltEncoding() const { return eDfltEnc; }

    bool IsSetWesternProps() const { return bool(nScriptFlags & Css1ScriptFlags::Western); }
    bool IsSetCJKProps() const { return bool(nScriptFlags & Css1ScriptFlags::CJK); }
    bool IsSetCTLProps() const { return bool(nScriptFlags & Css1ScriptFlags::CTL); }

    const OUString& GetBaseURL() const { return sBaseURL;}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
