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
#ifndef _WRTRTF_HXX
#define _WRTRTF_HXX

#include <shellio.hxx>
#include <wrt_fn.hxx>
#include "../inc/msfilter.hxx"
#include <deque>

#include <com/sun/star/util/DateTime.hpp>

// einige Forward Deklarationen
class Color;
class Font;
class SvPtrarr;
class SwFmt;
class SwFlyFrmFmt;
class SwPosFlyFrms;
class SwPageDesc;
class SwTableNode;
class SwTxtFmtColl;
class SwNumRule;
class SwNumRuleTbl;
class DateTime;
class RTFEndPosLst;
class SvxBorderLine;

extern SwAttrFnTab aRTFAttrFnTab;
extern SwNodeFnTab aRTFNodeFnTab;

// the default text encoding for the export, if it doesn't fit unicode will
// be used
#define DEF_ENCODING        RTL_TEXTENCODING_ASCII_US

typedef std::deque<Color> RTFColorTbl;

class RTF_WrtRedlineAuthor : public sw::util::WrtRedlineAuthor
{
    public:
    virtual void Write(Writer &rWrt);
};

// der RTF-Writer

class SwRTFWriter : public Writer
{
    friend class RTFEndPosLst;

    SvPtrarr* pFontRemoveLst;
    RTFColorTbl* pColTbl;
    SwPosFlyFrms* pFlyPos;          // Pointer auf die aktuelle "FlyFrmTabelle"
    RTFEndPosLst* pCurEndPosLst;
    const SfxItemSet* pAttrSet;     // akt. Format/Collection vom Node
                                    // fuer den Zugriff auf einige Attribute
                                    // z.B. Font-Size, LR-Space,..
    SwNumRuleTbl* pNumRuleTbl;      // list of all exported numrules
    RTF_WrtRedlineAuthor *pRedlAuthors;

    USHORT nAktFlyPos;              // Index auf das naechste "FlyFrmFmt"
    void OutRTFColorTab();
    void OutRTFFontTab();
    const rtl::OUString XlateFmtName( const rtl::OUString &rName, SwGetPoolIdFromName eFlags );
    void OutRTFStyleTab();
    void OutRTFListTab();
    bool OutRTFRevTab();

    void MakeHeader();
    void OutUnicodeSafeRecord(const sal_Char *pToken,
        const String &rContent);
    void OutDocInfoStat();
    void OutInfoDateTime( const sal_Char*,
        const ::com::sun::star::util::DateTime& );
    void CheckEndNodeForSection( const SwNode& rNd );

    void BuildNumRuleTbl();



public:
    // --- public Member --------------------------------------------------

    USHORT nCurRedline;


    const SwFlyFrmFmt* pFlyFmt; // liegt der Node in einem FlyFrame,
                                        // ist das Format gesetzt, sonst 0
    const SwPageDesc* pAktPageDesc;     // aktuell gesetzter PageDesc.
    sal_Int32 nBkmkTabPos;              // akt. Position in der Bookmark-Tabelle
    USHORT nCurScript;                  // actual scripttype
    rtl_TextEncoding eDefaultEncoding;
    rtl_TextEncoding eCurrentEncoding;

#if defined(UNX)
    static const sal_Char sNewLine;                 // nur \012 oder \015
#else
    static const sal_Char __FAR_DATA sNewLine[];    // \015\012
#endif


    BOOL bFirstLine : 1;            // wird die 1. Zeile ausgegeben ?
    BOOL bOutFmtAttr : 1;           // TRUE: beim Schreiben eines Formates
                                    // existierte mindestens ein Attribut
    BOOL bRTFFlySyntax : 1;         // gebe nur original RTFSyntax aus
                                    // (nur fuer die fliegenden Rahmen)
    BOOL bOutPageDesc: 1;           // gebe einen PageDescriptor aus
    BOOL bOutPageDescTbl: 1;        // gebe die PageDescriptor-Tabelle aus
    BOOL bOutTable : 1;             // gebe eine Tabelle aus
    BOOL bTxtAttr : 1;              // werden TextAttribute ausgegeben ?
    BOOL bWriteHelpFmt : 1;         // schreibe Win-RTF-HelpFileFmt
    BOOL bOutStyleTab : 1;          // gebe die StyleSheet-Tabelle aus
    BOOL bOutPageAttr : 1;          // PageDescAttribut ausgeben?
    BOOL bAutoAttrSet : 1;          // TRUE:  pAttrSet ist harte Attributierung
                                    // FALSE: pAttrSet ist vom Format/Collection
    BOOL bOutOutlineOnly : 1;       // TRUE: nur Gliederungs-Absaetze schreiben
    BOOL bOutListNumTxt : 1;        // TRUE: der ListNumText wird ausgegeben
    BOOL bOutLeftHeadFoot : 1;      // gebe vom PageDesc. den linkten
                                    // Header/Footer aus
    BOOL bOutSection : 1;           // TRUE: Section PageDesc ausgeben
    BOOL bIgnoreNextPgBreak : 1;    // TRUE: naechsten PageDesc/Break ignorieren
    BOOL bAssociated : 1;           // use associated tokens

    BOOL bNonStandard : 1;          // use non-standard tags (for cut and paste)

    // --- public Methoden ------------------------------------------------

    SwRTFWriter( const String& rFilterName, const String& rBaseURL );
    virtual ~SwRTFWriter();
    virtual ULONG WriteStream();

    void Out_SwDoc( SwPaM* );       // schreibe den makierten Bereich

        // gebe die evt. an der akt. Position stehenden FlyFrame aus.
    void OutFlyFrm();
    void OutRTFFlyFrms( const SwFlyFrmFmt& );
        // gebe alle an der Position stehenden Bookmarks aus
    void OutBookmarks( xub_StrLen nCntntPos );
        // gebe die PageDesc-Daten im normalen RTF-Format aus
    void OutRTFPageDescription( const SwPageDesc&, BOOL , BOOL );
    void OutRTFBorders( SvxBoxItem aBox );
    void OutRTFBorder( const SvxBorderLine* aLine, const USHORT nSpace );
    BOOL OutBreaks( const SfxItemSet& rSet );
    void OutRedline( xub_StrLen nCntntPos );

        // gebe die PageDescriptoren aus
    USHORT GetId( const Color& ) const;
    USHORT GetId( const SvxFontItem& ) const;
    USHORT GetId( const Font& ) const;
    USHORT GetId( const SwTxtFmtColl& ) const;
    USHORT GetId( const SwCharFmt& ) const;
    USHORT GetId( const SwNumRuleItem& rItem ) const;

    void OutPageDesc();

    BOOL OutListNum( const SwTxtNode& rNd );
    USHORT GetNumRuleId( const SwNumRule& rRule );

    // fuer RTFSaveData
    SwPaM* GetEndPaM()              { return pOrigPam; }
    void SetEndPaM( SwPaM* pPam )   { pOrigPam = pPam; }

    const SfxPoolItem& GetItem( USHORT nWhich ) const;

    const SfxItemSet* GetAttrSet() const    { return pAttrSet; }
    void SetAttrSet( const SfxItemSet* p )  { pAttrSet = p; }

    const RTFEndPosLst* GetEndPosLst() const { return pCurEndPosLst; }

    void SetAssociatedFlag( BOOL b )        { bAssociated = b; }
    BOOL IsAssociatedFlag() const           { return bAssociated; }

    void SetCurrScriptType( USHORT n )      { nCurScript = n; }
    USHORT GetCurrScriptType() const        { return nCurScript; }

    short TrueFrameDirection(const SwFrmFmt &rFlyFmt) const;
    short GetCurrentPageDirection() const;
};


// Struktur speichert die aktuellen Daten des Writers zwischen, um
// einen anderen Dokument-Teil auszugeben, wie z.B. Header/Footer
// Mit den beiden USHORTs im CTOR wird ein neuer PaM erzeugt und auf
// die Position im Dokument gesetzt.
// Im Destructor werden alle Daten wieder restauriert und der angelegte
// Pam wieder geloescht.

struct RTFSaveData
{
    SwRTFWriter& rWrt;
    SwPaM* pOldPam, *pOldEnd;
    const SwFlyFrmFmt* pOldFlyFmt;
    const SwPageDesc* pOldPageDesc;
    const SfxItemSet* pOldAttrSet;          // akt. Attribute vom Node

    BOOL bOldWriteAll : 1;
    BOOL bOldOutTable : 1;
    BOOL bOldOutPageAttr : 1;
    BOOL bOldAutoAttrSet : 1;
    BOOL bOldOutSection : 1;

    RTFSaveData( SwRTFWriter&, ULONG nStt, ULONG nEnd );
    ~RTFSaveData();
};


// einige Funktions-Deklarationen
Writer& OutRTF_AsByteString( Writer& rWrt, const String& rStr, rtl_TextEncoding eEncoding);
Writer& OutRTF_SwFmt( Writer& rWrt, const SwFmt& );
Writer& OutRTF_SwTblNode(Writer& , const SwTableNode&);
Writer& OutRTF_SwSectionNode( Writer& , SwSectionNode & );


// Augabe von RTF-Bitmaps (steht im File "wrtpict.cxx")
//struct SvxRTFPictureType;
//class Bitmap;
//USHORT WriteRTFPict( const SwPictureType&, Bitmap&, SvStream& );

// Ausagbe von Footer-/Headers
Writer& OutRTF_SwFmtHeader( Writer& , const SfxPoolItem& );
Writer& OutRTF_SwFmtFooter( Writer& , const SfxPoolItem& );

// Kommentar und zusaetzlichen String ausgeben
SvStream& OutComment( Writer& rWrt, const sal_Char* pStr );
    // zusaetzlich das bOutFmtAttr-Flag manipulieren
SvStream& OutComment( Writer& rWrt, const sal_Char* pStr, BOOL bSetFlag );
bool ExportAsInline(const SwFlyFrmFmt& rFlyFrmFmt);

#endif  //  _WRTRTF_HXX


